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

#ifndef _PPCALLBACKDISPATCHER_HPP_
#define _PPCALLBACKDISPATCHER_HPP_

#include <clang/Lex/PPCallbacks.h>

#include <Refactorers/Refactorer.hpp>

class PPCallbackDispatcher : public clang::PPCallbacks {
public:
    void setRefactorers(RefactorerVector *Refactorers);
    
    virtual void InclusionDirective(clang::SourceLocation LocBegin, 
                                    const clang::Token &Token, 
                                    llvm::StringRef FileName, 
                                    bool isAngled, 
                                    clang::CharSourceRange NameRange, 
                                    const clang::FileEntry* File, 
                                    llvm::StringRef SearchPath, 
                                    llvm::StringRef RelativePath, 
                                    const clang::Module* Module) override;
                            
    virtual void FileSkipped(const clang::FileEntry &SkippedFile,
                             const clang::Token &FileNameToken,
                             clang::SrcMgr::CharacteristicKind Kind) override;
    
    virtual void MacroExpands(const clang::Token &Token,
                              const clang::MacroDefinition &MacroDef,
                              clang::SourceRange Range,
                              const clang::MacroArgs *Args) override;
private:
    RefactorerVector *_Refactorers;
};

#endif /* _PPCALLBACKDISPATCHER_HPP_ */