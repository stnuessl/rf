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

#include <llvm/Support/FileSystem.h>

#include <util/commandline.hpp>

#include <Refactorers/IncludeRefactorer.hpp>


static bool hasEncloser(llvm::StringRef String)
{
    if (String.empty())
        return false;
    
    auto Front = String.front();
    auto Back = String.back();
    
    return (Front == '<' && Back == '>') || (Front == '"' && Back == '"');
}

void IncludeRefactorer::setVictimQualifier(std::string Victim)
{
    if (Victim.empty()) {
        llvm::errs() << util::cl::Error() 
                     << "empty victim qualifier in include refactorer.\n";
        std::exit(EXIT_FAILURE);
    }
    
    if (!ReplName_.empty() && hasEncloser(ReplName_) != hasEncloser(Victim)) {
        llvm::errs() << util::cl::Error()
                     << "\"" <<  Victim << "\" and \"" << ReplName_ << "\": "
                     << "either both specify an encloser or none.\n";
        std::exit(EXIT_FAILURE);
    }
        
    Victim_ = std::move(Victim);
}

const std::string &IncludeRefactorer::victimQualifier() const
{
    return Victim_;
}

void IncludeRefactorer::setReplacementQualifier(std::string Repl)
{
    if (Repl.empty()) {
        llvm::errs() << util::cl::Error() 
                     << "empty replacement qualifier in include refactorer.\n";
        std::exit(EXIT_FAILURE);
    }
    
    auto Front = Repl.front();
    auto Back  = Repl.back();
    
    if ((Front == '<' && Back != '>') || (Front == '"' && Back != '"')) {
        llvm::errs() << util::cl::Error() 
                     << "\"" << Repl << "\" "
                     << "has unmatching enclosing characters.\n";
        std::exit(EXIT_FAILURE);
    }
    
    if (!Victim_.empty() && hasEncloser(Victim_) != hasEncloser(Repl)) {
        llvm::errs() << util::cl::Error()
                     << "\"" <<  Victim_ << "\" and \"" << Repl << "\": "
                     << "either both specify an encloser or none.\n";
        std::exit(EXIT_FAILURE);
    }

    
    ReplName_ = std::move(Repl);
}

const std::string &IncludeRefactorer::replacementQualifier() const
{
    return ReplName_;
}


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
    (void) LocBegin;
    (void) Token;
    (void) File;
    (void) SearchPath;
    (void) RelativePath;
    (void) Module;
    
    if (Victim_.empty() || ReplName_.empty())
        return;

    if ((isAngled && Victim_[0] == '<') || (!isAngled && Victim_[0] == '"')) {
        auto VBegin = std::next(Victim_.begin(), 1);
        auto VEnd = std::prev(Victim_.end(), 1);
        
        auto FBegin = FileName.begin();
        auto FEnd = FileName.end();
        
        if (std::distance(VBegin, VEnd) != std::distance(FBegin, FEnd))
            return;
        
        if (!std::equal(VBegin, VEnd, FBegin))
            return;
        
        auto EncloserLocBegin = NameRange.getBegin();
        addReplacement(EncloserLocBegin);
        
    } else if (Victim_ == FileName) {
        /* Skip the enclosing '"' or '<' */
        auto EncloserLocBegin = NameRange.getBegin();
        addReplacement(EncloserLocBegin.getLocWithOffset(1));
    }
}

void IncludeRefactorer::addReplacement(clang::SourceLocation Loc)
{
    Refactorer::addReplacement(Loc, Victim_.size(), ReplName_);
}
