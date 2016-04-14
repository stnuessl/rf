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

void TagRefactorerNew::visitEnumDecl(const clang::EnumDecl *Decl)
{
    if (!isVictim(Decl))
        return;
    
    addReplacement(Decl->getLocation());
}

void 
TagRefactorerNew::visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl)
{
    if (!isVictim(Decl->getParent()))
        return;
    
    addReplacement(Decl->getNameInfo().getLoc());
}

void TagRefactorerNew::visitRecordDecl(const clang::RecordDecl *Decl)
{
    if (!isVictim(Decl))
        return;
    
    addReplacement(Decl->getLocation());
}

void TagRefactorerNew::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    auto Type = TypeLoc.getType();
    
    /*
     * Problem: Given
     *      template<typename T> func(void) { T() }
     *                                        ^(1) 
     * and
     *      func<VictimType>();
     *           ^(2)
     * two TypeLocs will be visited but only (2) can be refactored without
     * breaking the code.
     * Filter out type (1) locations.
     */
    auto STTPTypeLoc = TypeLoc.getAs<clang::SubstTemplateTypeParmTypeLoc>();
    if (STTPTypeLoc)
        return;
    
    /* 
     * This handles declarations like:
     * 
     *      template <typename T> class a;
     *      template <typename T> class b<const a<T>> {};
     *      template <typename T> void f(a<T> a) { }
     * 
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
    
    /*
     * This handles templated functions like:
     * 
     *      template <typename T> class a { };
     *      template <typename T, typename U>
     *      bool equals(a<T> &lhs, a<U> &rhs) { return true; }
     * 
     * Don't now why the approach later on won't handle this case,
     * so it is done explicitly here.
     */
    auto LValueRefTypeLoc = TypeLoc.getAs<clang::LValueReferenceTypeLoc>();
    if (LValueRefTypeLoc) {
        auto QualType = LValueRefTypeLoc.getTypePtr()->getPointeeType();
        auto TSpecType = QualType->getAs<clang::TemplateSpecializationType>();
        
        if (TSpecType) {
            auto TemplateName = TSpecType->getTemplateName();
            auto TemplateDecl = TemplateName.getAsTemplateDecl();
            
            if (isVictim(TemplateDecl)) {
                auto Loc = getLastTypeLocation(LValueRefTypeLoc);
                addReplacement(Loc);
            }
        }
        
        return;
    }
    
    auto TagDecl = Type->getAsTagDecl();
    if (!TagDecl || !isVictim(TagDecl))
        return;
    
    auto Loc = getLastTypeLocation(TypeLoc);
    addReplacement(Loc);
}


#if 0

TagRefactorer::TagRefactorer()
    : Refactorer()
{
    using namespace clang::ast_matchers;
    
    /* 
     * Could not find a 'TagDeclMatcher' 
     *      -> handle Records and Enums explicitly
     */
    
    auto RecordDeclMatcher = recordDecl().bind("RecordDecl");
    auto EnumDeclMatcher = enumDecl().bind("EnumDecl");
    auto TypeLocMatcher = typeLoc().bind("TypeLoc");
    auto CXXMethodDecl = methodDecl().bind("CXXMethodDecl");
    
    _Finder.addMatcher(RecordDeclMatcher, this);
    _Finder.addMatcher(EnumDeclMatcher, this);
    _Finder.addMatcher(TypeLocMatcher, this);
    _Finder.addMatcher(CXXMethodDecl, this);
}

void TagRefactorer::run(const MatchResult &Result)
{
    runRecordDecl(Result);
    runEnumDecl(Result);
    runTypeLoc(Result);
    runCXXMethodDecl(Result);
}

void TagRefactorer::runRecordDecl(const MatchResult &Result)
{
    auto RecordDecl = Result.Nodes.getNodeAs<clang::RecordDecl>("RecordDecl");
    if (!RecordDecl || !isVictim(RecordDecl))
        return;
        
    addReplacement(Result, RecordDecl->getLocation());
}

void TagRefactorer::runEnumDecl(const MatchResult &Result)
{
    auto EnumDecl = Result.Nodes.getNodeAs<clang::EnumDecl>("EnumDecl");
    if (!EnumDecl || !isVictim(EnumDecl))
        return;
    
    addReplacement(Result, EnumDecl->getLocation());
}

void TagRefactorer::runTypeLoc(const MatchResult &Result)
{
    auto TypeLoc = Result.Nodes.getNodeAs<clang::TypeLoc>("TypeLoc");
    if (!TypeLoc)
        return;
    
    /*
     * Problem: Given
     *      template<typename T> func(void) { T() }
     *                                        ^(1) 
     * and
     *      func<VictimType>();
     *           ^(2)
     * two TypeLocs will be visited but only (2) can be refactored without
     * breaking the code.
     * Filter out type (1) locations.
     */
    auto STTPTypeLoc = TypeLoc->getAs<clang::SubstTemplateTypeParmTypeLoc>();
    if (STTPTypeLoc)
        return;
    
    /* 
     * This handles declarations like:
     * 
     *      template <typename T> class a;
     *      template <typename T> class b<const a<T>> {};
     *      template <typename T> void f(a<T> a) { }
     * 
     * Casting the Typeloc to a TemplateSpecializationTypeLoc did not
     * always work. Casting the Type to the TemplateSpecializationType
     * seems to be doing fine.
     */
    auto Type = TypeLoc->getType();
    auto TSpecType = Type->getAs<clang::TemplateSpecializationType>();
    if (TSpecType) {
        auto TemplateName = TSpecType->getTemplateName();
        auto TemplateDecl = TemplateName.getAsTemplateDecl();
        
        if (isVictim(TemplateDecl)) {
            auto Loc = getLastTypeLocation(*TypeLoc);
            addReplacement(Result, Loc);
        }
        
        return;
    }
    
    /*
     * This handles templated functions like:
     * 
     *      template <typename T> class a { };
     *      template <typename T, typename U>
     *      bool equals(a<T> &lhs, a<U> &rhs) { return true; }
     * 
     * Don't now why the approach later on won't handle this case,
     * so it is done explicitly here.
     */
    auto LValueRefTypeLoc = TypeLoc->getAs<clang::LValueReferenceTypeLoc>();
    if (LValueRefTypeLoc) {
        auto QualType = LValueRefTypeLoc.getTypePtr()->getPointeeType();
        auto TSpecType = QualType->getAs<clang::TemplateSpecializationType>();
        
        if (TSpecType) {
            auto TemplateName = TSpecType->getTemplateName();
            auto TemplateDecl = TemplateName.getAsTemplateDecl();
            
            if (isVictim(TemplateDecl)) {
                auto Loc = getLastTypeLocation(LValueRefTypeLoc);
                addReplacement(Result, Loc);
            }
        }
        
        return;
    }
    
    auto TagDecl = TypeLoc->getType()->getAsTagDecl();
    if (!TagDecl || !isVictim(TagDecl))
        return;
    
    auto Loc = getLastTypeLocation(*TypeLoc);
    addReplacement(Result, Loc);
}

void TagRefactorer::runCXXMethodDecl(const MatchResult &Result)
{
    auto Decl = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("CXXMethodDecl");
    if (!Decl || !isVictim(Decl->getParent()))
        return;
    
    auto CXXConstructorDecl = clang::dyn_cast<clang::CXXConstructorDecl>(Decl);
    if (!CXXConstructorDecl)
        return;
    
    addReplacement(Result, CXXConstructorDecl->getNameInfo().getLoc());
    
    /* 
     * Legacy code to handle CXXDestructorDecls, but in contrast
     * to CXXConstructorDecls they are handled for some reason by TypeLocs
     */ 
#if 0
    auto CXXDestructorDecl = clang::dyn_cast<clang::CXXDestructorDecl>(Decl);
    if (CXXDestructorDecl) {
        /* Keep the '~' in the destructor name */
        auto Loc = CXXDestructorDecl->getNameInfo().getLoc();
        addReplacement(Result, Loc.getLocWithOffset(1));
    }
#endif
}
#endif
