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

void TagRefactorer::visitEnumConstantDecl(const clang::EnumConstantDecl *Decl)
{
    if (!isVictim(Decl))
        return;
    
    addReplacement(Decl->getLocation());
}

void TagRefactorer::visitEnumDecl(const clang::EnumDecl *Decl)
{
    if (!isVictim(Decl))
        return;
    
    addReplacement(Decl->getLocation());
}

void 
TagRefactorer::visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl)
{
    if (!isVictim(Decl->getParent()))
        return;
    
    addReplacement(Decl->getNameInfo().getLoc());
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


void TagRefactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    /* This handles enum constants, e.g.:
     *      enum a { a };
     *      int main() { auto a = a::a; };
     *                         (1)^  ^(2)
     * EnumConstantDecl's are not considered to be TagDecl's in clang.
     * rf however won't differentiate between them in the hope to create
     * a better user experience as this saves an extra commandline flag.
     *
     * 'Expr->getLocStart()' retrieves the incorrect position (1)
     * 'Expr->getLocation()' retrieves the correct position (2)
     */
    
    auto Decl = Expr->getDecl();
    if (!clang::dyn_cast<clang::EnumConstantDecl>(Decl) || !isVictim(Decl))
        return;
    
    addReplacement(Expr->getLocation());
}


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
        Type = PointerType->getPointeeType();
    
    auto ReferenceType = Type->getAs<clang::ReferenceType>();
    if (ReferenceType)
        Type = ReferenceType->getPointeeType();
    
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
    auto TypedefType = Type->getAs<clang::TypedefType>();
    if (TypedefType) {
        auto TypedefNameDecl = TypedefType->getDecl();
        
        if (isVictim(TypedefNameDecl)) {
            auto Loc = getLastTypeLocation(TypeLoc);
            addReplacement(Loc);
        }
        
        return;
    }
    
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
    
    auto TSpecType = Type->getAs<clang::TemplateSpecializationType>();
    if (TSpecType) {
        auto TemplateName = TSpecType->getTemplateName();
        auto TemplateDecl = TemplateName.getAsTemplateDecl();
        
        if (isVictim(TemplateDecl)) {
            auto Loc = getLastTypeLocation(TypeLoc);
            addReplacement(Loc);
        }

        return;
    }
    
    auto TagDecl = Type->getAsTagDecl();
    if (!TagDecl || !isVictim(TagDecl))
        return;
    
    auto Loc = getLastTypeLocation(TypeLoc);
    addReplacement(Loc);
}

