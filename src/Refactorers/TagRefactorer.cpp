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
    /*
     * Problem: Given
     *      template<typename T> func(void) { T() }
     *                                        ^(1) 
     * and
     *      func<VictimType>();
     *           ^(2)
     * two TypeLocs will be visited but only (2) can be refactored without
     * breaking the code. Also for some reason TypeLoc::getLocStart() equals
     * TypeLoc::getLocEnd() but using clang::Lexer a different 
     * End' SourceLocation can be generated.
     * Sigh...
     * The approach here reads bytes from the source location and
     * checks if the bytes matches the victim's name.
     */
    
    auto TypeLoc = Result.Nodes.getNodeAs<clang::TypeLoc>("TypeLoc");
    if (!TypeLoc)
        return;
    
    auto UnqualifiedTypeLoc = TypeLoc->getUnqualifiedLoc();
    
    auto TagDecl = UnqualifiedTypeLoc.getType()->getAsTagDecl();
    if (!TagDecl || !isVictim(TagDecl))
        return;
    
    auto LocStart = TypeLoc->getUnqualifiedLoc().getLocStart();
    auto LocEnd = LocStart.getLocWithOffset(_ReplSize);

    auto &SM = *Result.SourceManager;
    
    if (SM.isInSystemHeader(LocStart) || !SM.isLocalSourceLocation(LocStart))
        return;
    
    bool Invalid;
    
    auto Begin = SM.getCharacterData(LocStart, &Invalid);
    if (Invalid)
        return;
    
    auto End = SM.getCharacterData(LocEnd, &Invalid);
    if (Invalid)
        return;
    
    if (End - Begin != static_cast<std::ptrdiff_t>(_ReplSize))
        return;

    /* Skip qualifiers in the victim name */
    if (!std::equal(Begin, End, _Victim.end() - _ReplSize))
        return;
    
    auto NextTypeLoc = TypeLoc->getNextTypeLoc();
    if (!NextTypeLoc.isNull())
        LocStart = NextTypeLoc.getBeginLoc();

    addReplacement(Result, LocStart);
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
/*
bool TagRefactorer::isVictim(const clang::TagDecl *TagDecl)
{
    TagDecl = TagDecl->getCanonicalDecl();
    
    if (_VictimDecl == TagDecl)
        return true;
    
    auto Match = _Victim == qualifiedName(TagDecl);
    
    if (Match && !_VictimDecl)
        _VictimDecl = TagDecl;
    
    return Match;
}*/
