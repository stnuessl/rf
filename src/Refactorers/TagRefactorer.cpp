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

static clang::SourceLocation getLastTypeLocation(const clang::TypeLoc &TypeLoc)
{
    /*
     * Statements like
     *      class a var;
     *      ^(1)  ^(2)
     * generate the two shown type locations.
     * Calling "getNextTypeLoc()" on (1) will return location (2)
     */

    auto Loc = TypeLoc.getBeginLoc();
    auto NextTypeLoc = TypeLoc.getNextTypeLoc();

    while (NextTypeLoc) {
        Loc = NextTypeLoc.getBeginLoc();
        NextTypeLoc = NextTypeLoc.getNextTypeLoc();
    }

    return Loc;
}

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

// void TagRefactorer::visitPointerTypeLoc(const clang::PointerTypeLoc &TypeLoc)
// {
//     auto TagDecl = TypeLoc.getPointeeLoc().getType()->getAsTagDecl();
//     if (TagDecl && isVictim(TagDecl)) {
//         auto Loc = TypeLoc.getPointeeLoc().getLocalSourceRange().getBegin();
//         addReplacement(Loc);
//         llvm::errs() << "Added: ";
//         Loc.dump(CompilerInstance_->getSourceManager());
//         llvm::errs() << "\n";
//     }
// }
//
// void TagRefactorer::visitReferenceTypeLoc(
//     const clang::ReferenceTypeLoc &TypeLoc)
// {
//     auto TagDecl = TypeLoc.getPointeeLoc().getType()->getAsTagDecl();
//     if (TagDecl && isVictim(TagDecl)) {
//         auto Loc = TypeLoc.getLocStart();
//         addReplacement(Loc);
//     }
// }

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
            auto Loc = getLastTypeLocation(TypeLoc);
            addReplacement(Loc);
        }
    }
}

#if 0
void TagRefactorer::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    auto Type = TypeLoc.getType();

    /*
     * Problem: Given
     *      template<typename T> func() { T() }
     *                                    ^(1)
     * and
     *      func<VictimType>();
     *           ^(2)
     * two TypeLocs will be visited but only (2) can be refactored without
     * breaking the code.
     * Filter out type (1) locations.
     */
    auto STTypeParmType = Type->getAs<clang::SubstTemplateTypeParmType>();
    if (STTypeParmType)
        return;
    
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
    
//     auto TypedefType = Type->getAs<clang::TypedefType>();
//     if (TypedefType) {
//         auto TypedefNameDecl = TypedefType->getDecl();
// 
//         if (isVictim(TypedefNameDecl)) {
//             auto Loc = getLastTypeLocation(TypeLoc);
//             addReplacement(Loc);
//         }
//         
//         return;
//     }

    /*
     * Given
     *      struct a { };
     *      typedef a b;
     * and
     *      f(b *val) { }   and     g(b &val) { }
     *        ^(1)                    ^(2)
     *
     * (1) Generates a PointerType which wraps a reference type 'b'
     * (2) Generates a ReferenceType which also wraps a reference type 'b'
     *
     * If 'b' is about to be refactored both locations have to be refactored,
     * so we unwrap the pointer and reference types.
     * The location of this unwrapping is intentional.
     */
    auto PointerType = Type->getAs<clang::PointerType>();
    if (PointerType)
        return;
//         Type = PointerType->getPointeeType();

    auto ReferenceType = Type->getAs<clang::ReferenceType>();
    if (ReferenceType)
        return;
//         Type = ReferenceType->getPointeeType();

//     /*
//      * Given;
//      *      typedef std::vector<int> s32_vector;
//      *
//      * This handles all the occurences ((1) and (2)) of 's32_vector' like:
//      *      s32_vector v;   and     std::vector<s32_vector> s32_matrix;
//      *      ^(1)                                ^(2)
//      *
//      * This has to be done explicitly here since clang thinks of typedef types
//      * not as tag types but rf does.
//      */
    
//     auto &SM = CompilerInstance_->getSourceManager();
//     auto TypedefType = Type->getAs<clang::TypedefType>();
//     if (TypedefType) {
//         auto TypedefNameDecl = TypedefType->getDecl();
//         
//         llvm::errs() << "yes\n\n";
// 
//         if (isVictim(TypedefNameDecl)) {
//             auto Loc = getLastTypeLocation(TypeLoc);
//             llvm::errs() << __FILE__ << ":" << __LINE__ << ":  ";
//             Loc.dump(SM);
//             llvm::errs() << "\n";
//             addReplacement(Loc);
//         }
// 
//         return;
//     }

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

//     auto TSpecType = Type->getAs<clang::TemplateSpecializationType>();
//     if (TSpecType) {
//         auto TemplateName = TSpecType->getTemplateName();
//         auto TemplateDecl = TemplateName.getAsTemplateDecl();
// 
//         if (isVictim(TemplateDecl)) {
//             auto Loc = getLastTypeLocation(TypeLoc);
//             addReplacement(Loc);
//         }
// 
//         return;
//     }
    
    auto TagDecl = Type->getAsTagDecl();
    if (TagDecl && isVictim(TagDecl)) {
        auto Loc = getLastTypeLocation(TypeLoc);
//         addReplacement(Loc);
        llvm::errs() << "Added:\n";
        Type->dump();
        llvm::errs() << "at: ";
        Loc.dump(CompilerInstance_->getSourceManager());
        llvm::errs() << "\n-------------\n";
        return;
    }
}
#endif
