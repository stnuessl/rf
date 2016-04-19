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

#include <functional>

#include <clang/AST/DeclTemplate.h>

#include <Refactorers/IncludeRefactorer.hpp>

void IncludeRefactorer::InclusionDirective(clang::SourceLocation LocBegin, 
                                           const clang::Token &Token, 
                                           llvm::StringRef FileName, 
                                           bool isAngled, 
                                           clang::CharSourceRange NameRange, 
                                           const clang::FileEntry *File, 
                                           llvm::StringRef SearchPath, 
                                           llvm::StringRef RelativePath, 
                                           const clang::Module *Module)
{
    (void) Token;
    (void) isAngled;
    (void) SearchPath;
    (void) RelativePath;
    (void) Module;
    
    auto &SM = _CompilerInstance->getSourceManager();
    
    if (!isHeaderFile(FileName))
        return;
    
    if (SM.isInSystemHeader(LocBegin))
        return;
    
    unsigned int IncludingUID;
    bool Ok;
    
    std::tie(IncludingUID, Ok) = getFileUID(LocBegin);
    if (!Ok)
        return;
    
    auto IncludedUID = File->getUID();
    
    /*
     * This source range can be visualized as
     * 
     *      #include <memory>
     *      ^~~~~~~~~~~~~~~~^
     */
    auto Range = clang::SourceRange(LocBegin, NameRange.getEnd());
    
    _IncludeMap[{IncludingUID, IncludedUID}].push_back(std::move(Range));
}

void IncludeRefactorer::FileSkipped(const clang::FileEntry &SkippedFile, 
                                    const clang::Token &FileNameToken, 
                                    clang::SrcMgr::CharacteristicKind Kind)
{
    /*
     * Remove Source Locations which cannot possible achieve a hit
     * in the Vistitor code below since they were included twice
     * (only the first include will be hitted).
     */
    (void) Kind;
    
    auto EndLoc = FileNameToken.getEndLoc();
    
    unsigned int IncludingUID;
    bool Ok;
    
    std::tie(IncludingUID, Ok) = getFileUID(EndLoc);
    if (!Ok)
        return;
    
    auto IncludedUID = SkippedFile.getUID();
    
    auto MapIt = _IncludeMap.find({IncludingUID, IncludedUID});
    if (MapIt == _IncludeMap.end())
        return;
    
    auto &Vec = MapIt->second;
    
    for (auto It = Vec.begin(), End = Vec.end(); It != End; ++It) {
        if (EndLoc == It->getEnd()) {
            /* 
             * Given:
             *      main.cpp:
             *          #include <string>
             *          #include <file.hpp>
             *      file.hpp:
             *          #include <string>
             * 
             * The string header is included twice and the second include
             * cannot be checked for usage (note how the #include in file.hpp
             * will be skipped). Such includes will be removed from
             * testing in the following if-branch. 
             * However if main.cpp really depends on <string> compile
             * times will not be effected if the #include in file.hpp is
             * redundant. This basically means that in order to remove all 
             * superfluous includes the program has to be run as many times as
             * it finds includes to remove (here two times if string is 
             * redundant in both files).
             * 
             * The next case is a little easier:
             *      main:cpp
             *          #include <string>
             *          #include <string>
             * 
             * Something like that can easily happen if there are a lot files
             * included between those two shown include directives.
             * This results in the following mapping:
             *      (main.cpp, string) --> clang::SourceRange_1
             *                         --> clang::SourceRange_2
             *                         --> ...
             *                         --> clang::SourceRange_N
             * 
             * This means that if we remove an element from the vector here
             * and the vector is not empty it was a redundant include in the
             * same file which can be removed. This will be done in the
             * else-branch.
             */
            
            if (Vec.size() > 1) {
                addReplacement(*It);
                Vec.erase(It);
            } else {
                _IncludeMap.erase(MapIt);
            }

//             auto Range = *It;
//             Vec.erase(It);
//             
//             if (Vec.empty())
//                 _IncludeMap.erase(MapIt);
//             else
//                 addReplacement(Range);
            
            break;
        }
    }
}

void IncludeRefactorer::MacroExpands(const clang::Token &Token, 
                                     const clang::MacroDefinition &MacroDef, 
                                     clang::SourceRange Range, 
                                     const clang::MacroArgs *Args)
{
    /* 
     * Handle Macro dependencies to header includes here
     */
    (void) Token;
    (void) Args;
    
    if (_IncludeMap.empty())
        return;
    
    auto &SM = _CompilerInstance->getSourceManager();

    auto IncludingLoc = Range.getBegin();
    auto Info = MacroDef.getMacroInfo();
    auto DefinitionLoc = Info->getDefinitionLoc();
    
    if (SM.isWrittenInSameFile(IncludingLoc, DefinitionLoc))
        return;
    
    removeUsedIncludes(IncludingLoc, DefinitionLoc);
}

void IncludeRefactorer::afterSourceFileAction()
{
    for (const auto &Item : _IncludeMap) {
        for (const auto &Range : Item.second)
            addReplacement(Range);
    }
    
    /* Get ready for next source file */
    _IncludeMap.clear();
}

void IncludeRefactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    if (_IncludeMap.empty())
        return;
    
    auto &SM = _CompilerInstance->getSourceManager();
    
    auto IncludingLoc = Expr->getLocStart();
    
    if (SM.isInSystemHeader(IncludingLoc))
        return;
    
    unsigned int IncludingUID;
    bool Ok;
    
    std::tie(IncludingUID, Ok) = getFileUID(IncludingLoc);
    if (!Ok)
        return;
    
    auto IncludedLoc = Expr->getDecl()->getLocStart();
    
    if (SM.isWrittenInSameFile(IncludingLoc, IncludedLoc))
        return;
    
    removeUsedIncludes(IncludingLoc, IncludedLoc);
}

void IncludeRefactorer::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    if (_IncludeMap.empty())
        return;
    
    auto Type = TypeLoc.getType();
    auto &SM = _CompilerInstance->getSourceManager();
    
    auto IncludingLoc = TypeLoc.getLocStart();
    
    if (SM.isInSystemHeader(IncludingLoc))
        return;
    
    auto TemplateSpecType = Type->getAs<clang::TemplateSpecializationType>();    
    if (TemplateSpecType) {
        auto TemplateName = TemplateSpecType->getTemplateName();
        auto TemplateDecl = TemplateName.getAsTemplateDecl();
        
        removeUsedIncludes(IncludingLoc, TemplateDecl->getLocStart());
        return;
    }
    
    /* 
     * I don't know why the TagRefactorer handles pointers correctly
     * and here I have to make a seperate case for them
     */
    auto PointerType = Type->getAs<clang::PointerType>();
    if (PointerType)
        Type = PointerType->getPointeeType();
    
    auto TypedefType = Type->getAs<clang::TypedefType>();
    if (TypedefType) {
        auto TypedefNameDecl = TypedefType->getDecl();

        removeUsedIncludes(IncludingLoc, TypedefNameDecl->getLocStart());
        return;
    }
    
    auto TagDecl = TypeLoc.getType()->getAsTagDecl();
    if (!TagDecl)
        return;
    
    removeUsedIncludes(IncludingLoc, TagDecl->getLocStart());
}


std::size_t 
IncludeRefactorer::UIntPairHash::operator()(const UIntPair &Pair) const
{
    static const auto Hasher = std::hash<unsigned int>();
    
    return Hasher(Pair.first) ^ Hasher(Pair.second);
}

std::pair<unsigned int, bool> 
IncludeRefactorer::getFileUID(clang::SourceLocation Loc) const
{
    auto &SM = _CompilerInstance->getSourceManager();
    auto Pair = std::make_pair(std::numeric_limits<unsigned int>::max(), false);
    
    auto FileID = SM.getFileID(Loc);
    auto File = SM.getFileEntryForID(FileID);
    
    if (File && File->isValid()) {
        Pair.first  = File->getUID();
        Pair.second = true;
    }
    
    return Pair;
}

void IncludeRefactorer::removeUsedIncludes(clang::SourceLocation IncludingLoc,
                                           clang::SourceLocation IncludedLoc)
{
    unsigned int IncludingUID;
    bool Ok;
    
    std::tie(IncludingUID, Ok) = getFileUID(IncludingLoc);
    if (!Ok)
        return;
    
    removeUsedIncludes(IncludingUID, IncludedLoc);
}

void IncludeRefactorer::removeUsedIncludes(unsigned int IncludingUID,
                                           clang::SourceLocation IncludedLoc)
{
    /*
     * Given the dependency chain:
     * 
     *      main.cpp:
     *          #include "file_1.hpp"
     *      int main() { function(); }
     * 
     *      file_1.hpp:
     *          #include "file_2.hpp"
     *      ... 
     *      file_N.hpp:
     *          void function();
     * 
     * We have to walk down from file0N.hpp and mark all encountered files
     * as used.
     */
    
    auto &SM = _CompilerInstance->getSourceManager();
    
    while (IncludedLoc.isValid() && !_IncludeMap.empty()) {
        unsigned int IncludedUID;
        bool Ok;
        
        std::tie(IncludedUID, Ok) = getFileUID(IncludedLoc);
        if (Ok) {
            if (IncludedUID == IncludingUID)
                return;
            
            _IncludeMap.erase({IncludingUID, IncludedUID});
        }
        
        /* 
         * Get including location of current included location, e.g. 
         *      file_N.hpp -> file_(N-1).hpp 
         */
        auto FileID = SM.getFileID(IncludedLoc);
        IncludedLoc = SM.getIncludeLoc(FileID);
    }
}


bool IncludeRefactorer::isHeaderFile(const StringRef &FileName) const
{
    /* Catch standard headers */
    if (FileName.rfind('.') == clang::StringRef::npos)
        return true;
    
    if (FileName.endswith_lower(".h"))
        return true;
    
    if (FileName.endswith_lower(".hpp"))
        return true;
    
    if (FileName.endswith_lower(".hxx"))
        return true;
    
    return false;
}

void IncludeRefactorer::addReplacement(const clang::SourceRange Range)
{
    auto &SM = _CompilerInstance->getSourceManager();
    
    auto Begin = Range.getBegin();
    auto BeginInfo = SM.getDecomposedLoc(Begin);
    auto EndInfo = SM.getDecomposedLoc(Range.getEnd());
    
    if (BeginInfo.first != EndInfo.first)
        return;
    
    addReplacement(Begin, EndInfo.second - BeginInfo.second);
}

void IncludeRefactorer::addReplacement(const clang::SourceLocation Loc, 
                                       unsigned int Length)
{
    Refactorer::addReplacement(Loc, Length, "");
}
