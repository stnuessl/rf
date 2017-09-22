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

#ifndef RF_PPCALLBACKDISPATCHER_HPP_
#define RF_PPCALLBACKDISPATCHER_HPP_

#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>

#include <Refactorers/Base/Refactorer.hpp>

class PPCallbackDispatcher : public clang::PPCallbacks {
public:
    void setRefactorers(std::vector<std::unique_ptr<Refactorer>> *Refactorers);

    virtual void InclusionDirective(clang::SourceLocation LocBegin,
                                    const clang::Token &Token,
                                    llvm::StringRef FileName,
                                    bool isAngled,
                                    clang::CharSourceRange NameRange,
                                    const clang::FileEntry *File,
                                    llvm::StringRef SearchPath,
                                    llvm::StringRef RelativePath,
                                    const clang::Module *Module) override;

    virtual void FileSkipped(const clang::FileEntry &SkippedFile,
                             const clang::Token &FileNameToken,
                             clang::SrcMgr::CharacteristicKind Kind) override;

    virtual void MacroExpands(const clang::Token &Token,
                              const clang::MacroDefinition &MacroDef,
                              clang::SourceRange Range,
                              const clang::MacroArgs *Args) override;

    virtual void MacroDefined(const clang::Token &MacroName,
                              const clang::MacroDirective *MD) override;

    virtual void MacroUndefined(const clang::Token &MacroName,
                                const clang::MacroDefinition &MD,
                                const clang::MacroDirective *Undef) override;

    virtual void Defined(const clang::Token &MacroNameTok,
                         const clang::MacroDefinition &MD,
                         clang::SourceRange Range) override;

    virtual void If(clang::SourceLocation Loc,
                    clang::SourceRange ConditionRange,
                    clang::PPCallbacks::ConditionValueKind ValueKind) override;

    virtual void Elif(clang::SourceLocation Loc,
                      clang::SourceRange ConditionRange,
                      clang::PPCallbacks::ConditionValueKind Kind,
                      clang::SourceLocation IfLoc) override;

    virtual void Ifdef(clang::SourceLocation Loc,
                       const clang::Token &MacroNameTok,
                       const clang::MacroDefinition &MD) override;

    virtual void Ifndef(clang::SourceLocation Loc,
                        const clang::Token &MacroNameTok,
                        const clang::MacroDefinition &MD) override;

private:
    std::vector<std::unique_ptr<Refactorer>> *Refactorers_;
};

#endif /* RF_PPCALLBACKDISPATCHER_HPP_ */
