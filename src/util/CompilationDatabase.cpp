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

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/Support/FileSystem.h>

#include <util/CompilationDatabase.hpp>

namespace util {
namespace compilation_database {

std::unique_ptr<clang::tooling::CompilationDatabase> 
detect(llvm::StringRef Path, std::string &ErrMsg)
{
    using clang::tooling::CompilationDatabase;
    using clang::tooling::JSONCompilationDatabase;
    
    auto JSONSyntax = clang::tooling::JSONCommandLineSyntax::AutoDetect;
    
    if (!Path.empty())
        return JSONCompilationDatabase::loadFromFile(Path, ErrMsg, JSONSyntax);
    
    llvm::SmallString<64> Buffer;
    auto Error = llvm::sys::fs::current_path(Buffer);
    if (Error) {
        Buffer.clear();
        Buffer.append("./");
    }
    
    auto WorkDir = Buffer.str();
    
    return CompilationDatabase::autoDetectFromDirectory(WorkDir, ErrMsg);
}

} /* namespace compilation_database */
} /* namespace util */
