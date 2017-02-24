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

#ifndef RF_FILESYSTEM_HPP_
#define RF_FILESYSTEM_HPP_

#include <system_error>

#include <llvm/ADT/StringRef.h>

namespace util {
namespace fs {

std::error_code createDirectoriesForFile(llvm::StringRef Path);
    
std::string currentPath();

bool exists(llvm::StringRef Prefix, llvm::StringRef Suffix);

std::error_code realpath(llvm::StringRef Path, std::string &Result);

std::error_code realpath(std::string &Path);

llvm::StringRef searchParents(llvm::StringRef Path, llvm::StringRef FileName);

llvm::StringRef projectPath(llvm::StringRef Path);


}
}

#endif /* RF_FILESYSTEM_HPP_ */
