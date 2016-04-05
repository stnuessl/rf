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
     *      class MyClass function();
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
#if 1
    auto STTPTypeLoc = TypeLoc->getAs<clang::SubstTemplateTypeParmTypeLoc>();
    if (STTPTypeLoc)
        return;
#endif
    /*
     * Handles declarations like:
     *      template <typename T> a {};
     *      template <typename T> void f(a<T> a) { }
     *                                   ^(1)
     * even if it is never instantiated in the program.
     * If it is there will also be a corresponding 'TypeLoc' which is
     * handled later.
     */
    auto TSpecTypeLoc = TypeLoc->getAs<clang::TemplateSpecializationTypeLoc>();
    if (TSpecTypeLoc) {
        auto TemplateName = TSpecTypeLoc.getTypePtr()->getTemplateName();
        auto TemplateDecl = TemplateName.getAsTemplateDecl();
        
        /* 'getLocStart()' seems to handle all the cases correctly */
        if (isVictim(TemplateDecl))
            addReplacement(Result, TSpecTypeLoc.getLocStart());
        
        return;
    }
    
    /*
     * This handles variable declarations like
     *      namespace::class a;
     *                 ^(1)
     *      class namespace::class a;
     *                       ^(2)
     * Such declarations would be handled later on anyway but the
     * access to the correct source location is much easier with 
     * an ElaboratedTypeLoc.
     */
    
    auto ElaboratedTypeLoc = TypeLoc->getAs<clang::ElaboratedTypeLoc>();
    if (ElaboratedTypeLoc) {
        auto TagDecl = ElaboratedTypeLoc.getInnerType()->getAsTagDecl();
        if (TagDecl && isVictim(TagDecl)) {
            auto NamedTypeLoc = ElaboratedTypeLoc.getNamedTypeLoc();
            addReplacement(Result, NamedTypeLoc.getBeginLoc());
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
        
#if 1
    /* 
     * Given something like
     *      namespace::class
     *      ^(1)       ^(2)
     * 
     * there will be a qualified type location beginning at (1) and a
     * unqualified type location at (2). Filter out type (1) locations.
     */
    
    auto UnqualTypeLoc = TypeLoc->getAs<clang::UnqualTypeLoc>();
    if (!UnqualTypeLoc)
        return;
    
    auto TagDecl = UnqualTypeLoc.getType()->getAsTagDecl();
    if (!TagDecl || !isVictim(TagDecl)) {
//         llvm::errs() << "No Replacement at: ";
//         UnqualTypeLoc.getLocStart().dump(*Result.SourceManager);
//         llvm::errs() << "\n";
//         UnqualTypeLoc.getTypePtr()->dump();
//         llvm::errs() << "\n";
        return;
    }

    auto Loc = getLastTypeLocation(UnqualTypeLoc);
    addReplacement(Result, Loc);
#endif
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