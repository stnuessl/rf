/*
 * Copyright (C) 2016  Steffen Nüssle
 * rf - refactor
 *
 * This file is part of rf.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <clang/AST/DeclTemplate.h>

#include <Refactorers/TagRefactorer.hpp>

void TagRefactorer::visitEnumDecl(const clang::EnumDecl *Decl)
{
    if (!isVictim(Decl))
        return;

    addReplacement(Decl->getLocation());
}

void TagRefactorer::visitCXXConstructorDecl(
    const clang::CXXConstructorDecl *Decl)
{
    if (!isVictim(Decl->getParent()))
        return;

    addReplacement(Decl->getLocation());
}

void TagRefactorer::visitRecordDecl(const clang::RecordDecl *Decl)
{
    if (!isVictim(Decl))
        return;

    addReplacement(Decl->getLocation());
}

void TagRefactorer::visitTypedefNameDecl(const clang::TypedefNameDecl *Decl)
{
    if (!isVictim(Decl))
        return;

    addReplacement(Decl->getLocation());
}

void TagRefactorer::visitUsingDecl(const clang::UsingDecl *Decl)
{
    /*
     * This handles using declarations like:
     *      namespace n { struct s {}; }
     *
     *      void f() { using n::s; s var; }
     *                          ^(1)
     */
    for (const auto &UsingShadowDecl : Decl->shadows()) {
        auto NamedDecl = UsingShadowDecl->getUnderlyingDecl();
        auto TagDecl = clang::dyn_cast<clang::TagDecl>(NamedDecl);

        if (TagDecl && isVictim(TagDecl)) {
            addReplacement(Decl->getLocation());
            break;
        }
    }
}

void TagRefactorer::visitInjectedClassNameTypeLoc(
    const clang::InjectedClassNameTypeLoc &TypeLoc)
{
    /*
     * Deal with injected class names e.g. positions (1), (2) and (3)
     * in the following code.
     *
     *      template <typename T>
     *      class c {
     *      public:
     *          ...
     *          ~c();
     *           ^(1)
     *
     *          c &operator=(const c &other);
     *          ^(2)               ^(3)
     *
     *          c<T> &operator(c<T> &&other);
     *          ^(4)           ^(5)
     *      };
     *
     * Types at positions (4) and (5) are called 'TemplateSpecializationTypes'
     */
    auto RecordDecl = TypeLoc.getDecl();
    if (isVictim(RecordDecl)) {
        auto Loc = TypeLoc.getLocStart();
        addReplacement(Loc);
    }
}

void TagRefactorer::visitMemberPointerTypeLoc(
    const clang::MemberPointerTypeLoc &TypeLoc)
{
    /*
     * Deal with function pointers to class methods, e.g.:
     *      void (namespace::class::*ptr)(int, int);
     *                       ^(1)
     */

    auto CXXRecordDecl = TypeLoc.getClass()->getAsCXXRecordDecl();
    if (CXXRecordDecl && isVictim(CXXRecordDecl)) {
        auto Loc = TypeLoc.getLocalSourceRange().getBegin();
        addReplacement(Loc);
    }
}

void TagRefactorer::visitTagTypeLoc(const clang::TagTypeLoc &TypeLoc)
{
    auto TagDecl = TypeLoc.getDecl();
    if (isVictim(TagDecl)) {
        auto Loc = TypeLoc.getLocStart();
        addReplacement(Loc);
    }
}

void TagRefactorer::visitTemplateSpecializationTypeLoc(
    const clang::TemplateSpecializationTypeLoc &TypeLoc)
{
    /*
     * This handles declarations like:
     *
     *      template <typename T> class a {};
     *      template <typename T> class b<const a<T>> {};
     *                                          ^
     *      template <typename T> void f(a<T> a) { }
     *                                   ^
     * Casting the Typeloc to a TemplateSpecializationTypeLoc did not
     * always work. Casting the Type to the TemplateSpecializationType
     * seems to be doing fine.
     */

    auto Type = TypeLoc.getType()->getAs<clang::TemplateSpecializationType>();
    if (Type) {
        auto TemplateName = Type->getTemplateName();
        auto TemplateDecl = TemplateName.getAsTemplateDecl();

        if (isVictim(TemplateDecl)) {
            auto Loc = TypeLoc.getLocStart();
            addReplacement(Loc);
        }
    }
}

void TagRefactorer::visitTypedefTypeLoc(const clang::TypedefTypeLoc &TypeLoc)
{
    /*
     * Given;
     *      typedef std::vector<int> s32_vector;
     *
     * This handles all the occurences ((1) and (2)) of 's32_vector' like:
     *      s32_vector v;   and     std::vector<s32_vector> s32_matrix;
     *      ^(1)                                ^(2)
     *
     * This has to be done explicitly here since clang thinks of typedef types
     * not as tag types but rf does.
     */

    auto TypedefType = TypeLoc.getType()->getAs<clang::TypedefType>();
    if (TypedefType) {
        auto TypedefNameDecl = TypedefType->getDecl();

        if (isVictim(TypedefNameDecl)) {
            auto Loc = TypedefNameDecl->getLocation();
            addReplacement(Loc);
        }
    }
}
