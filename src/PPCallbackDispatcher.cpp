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

#include <PPCallbackDispatcher.hpp>


void PPCallbackDispatcher::setRefactorers(RefactorerVector *Refactorers)
{
    _Refactorers = Refactorers;
}

void PPCallbackDispatcher::InclusionDirective(clang::SourceLocation LocBegin, 
                                              const clang::Token &Token, 
                                              llvm::StringRef FileName, 
                                              bool isAngled, 
                                              clang::CharSourceRange NameRange, 
                                              const clang::FileEntry *File, 
                                              llvm::StringRef SearchPath, 
                                              llvm::StringRef RelativePath, 
                                              const clang::Module *Module)
{
    for (auto &Refactorer : *_Refactorers) {
        Refactorer->InclusionDirective(LocBegin, Token, FileName, isAngled, 
                                       NameRange, File, SearchPath, 
                                       RelativePath, Module);
    }
}

void PPCallbackDispatcher::FileSkipped(const clang::FileEntry &SkippedFile, 
                                       const clang::Token &FileNameToken, 
                                       clang::SrcMgr::CharacteristicKind Kind)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->FileSkipped(SkippedFile, FileNameToken, Kind);
}

void PPCallbackDispatcher::MacroExpands(const clang::Token &Token, 
                                        const clang::MacroDefinition &MacroDef, 
                                        clang::SourceRange Range, 
                                        const clang::MacroArgs *Args)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->MacroExpands(Token, MacroDef, Range, Args);
}

void PPCallbackDispatcher::MacroDefined(const clang::Token &MacroName, 
                                        const clang::MacroDirective *MD)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->MacroDefined(MacroName, MD);
}

void PPCallbackDispatcher::MacroUndefined(const clang::Token &MacroName, 
                                          const clang::MacroDefinition &MD)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->MacroUndefined(MacroName, MD);
}

