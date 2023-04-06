/*
 * Copyright (C) 2017  Steffen Nüssle
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

#ifndef RF_INCLUDE_REFACTORER_HPP_
#define RF_INCLUDE_REFACTORER_HPP_

#include "Refactorers/Base/Refactorer.hpp"

class IncludeRefactorer : public Refactorer {
public:
    IncludeRefactorer() = default;

    void setVictimQualifier(std::string Victim);
    const std::string &victimQualifier() const;

    void setReplacementQualifier(std::string Repl);
    const std::string &replacementQualifier() const;

    void
    InclusionDirective(clang::SourceLocation HashLoc,
                       const clang::Token &IncludeTok,
                       llvm::StringRef FileName,
                       bool IsAngled,
                       clang::CharSourceRange FilenameRange,
                       clang::Optional<clang::FileEntryRef> File,
                       llvm::StringRef SearchPath,
                       llvm::StringRef RelativePath,
                       const clang::Module *Imported,
                       clang::SrcMgr::CharacteristicKind FileType) override;

private:
    void addReplacement(clang::SourceLocation Loc);

    std::string Victim_;
    std::string ReplName_;
};

#endif /* RF_INCLUDE_REFACTORER_HPP_ */
