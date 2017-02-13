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


void PPCallbackDispatcher::setRefactorers(Refactorers *Refactorers)
{
    Refactorers_ = Refactorers;
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
    for (auto &Refactorer : *Refactorers_) {
        Refactorer->InclusionDirective(LocBegin, Token, FileName, isAngled, 
                                       NameRange, File, SearchPath, 
                                       RelativePath, Module);
    }
}

void PPCallbackDispatcher::FileSkipped(const clang::FileEntry &SkippedFile, 
                                       const clang::Token &FileNameToken, 
                                       clang::SrcMgr::CharacteristicKind Kind)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->FileSkipped(SkippedFile, FileNameToken, Kind);
}

void PPCallbackDispatcher::MacroExpands(const clang::Token &Token, 
                                        const clang::MacroDefinition &MacroDef, 
                                        clang::SourceRange Range, 
                                        const clang::MacroArgs *Args)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->MacroExpands(Token, MacroDef, Range, Args);
}

void PPCallbackDispatcher::MacroDefined(const clang::Token &MacroName, 
                                        const clang::MacroDirective *MD)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->MacroDefined(MacroName, MD);
}

void PPCallbackDispatcher::MacroUndefined(const clang::Token &MacroName, 
                                          const clang::MacroDefinition &MD)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->MacroUndefined(MacroName, MD);
}

void PPCallbackDispatcher::Defined(const clang::Token &MacroNameTok, 
                                   const clang::MacroDefinition &MD, 
                                   clang::SourceRange Range)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->Defined(MacroNameTok, MD, Range);
}


void PPCallbackDispatcher::If(clang::SourceLocation Loc, 
                              clang::SourceRange ConditionRange, 
                              clang::PPCallbacks::ConditionValueKind ValueKind)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->If(Loc, ConditionRange, ValueKind);
}

void PPCallbackDispatcher::Elif(clang::SourceLocation Loc, 
                                clang::SourceRange ConditionRange, 
                                clang::PPCallbacks::ConditionValueKind Kind, 
                                clang::SourceLocation IfLoc)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->Elif(Loc, ConditionRange, Kind, IfLoc);
}

void PPCallbackDispatcher::Ifdef(clang::SourceLocation Loc, 
                                 const clang::Token &MacroNameTok, 
                                 const clang::MacroDefinition &MD)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->Ifdef(Loc, MacroNameTok, MD);
}

void PPCallbackDispatcher::Ifndef(clang::SourceLocation Loc, 
                                  const clang::Token &MacroNameTok, 
                                  const clang::MacroDefinition &MD)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->Ifndef(Loc, MacroNameTok, MD);
}
