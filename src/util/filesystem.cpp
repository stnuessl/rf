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

#include <cstdlib>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <util/commandline.hpp>
#include <util/filesystem.hpp>


namespace util {
namespace fs {
    
std::error_code createDirectoriesForFile(llvm::StringRef Path)
{
    Path = llvm::sys::path::parent_path(Path);
    
    if (llvm::sys::fs::exists(Path))
        return std::error_code();
    
    const auto Permissions = llvm::sys::fs::owner_all | 
                             llvm::sys::fs::all_read  | 
                             llvm::sys::fs::all_exe;
    
    return llvm::sys::fs::create_directories(Path, true, Permissions);
}

std::string currentPath()
{
    llvm::SmallVector<char, 64> Buffer;
    
    auto Error = llvm::sys::fs::current_path(Buffer);
    if (Error) {
        llvm::errs() << "failed to retrieve the current working directory - "
                     << Error.message() << "\n";
        std::exit(EXIT_FAILURE);
    }
    
    return std::string(Buffer.data(), Buffer.size());
}


bool exists(llvm::StringRef Prefix, llvm::StringRef Suffix)
{
    const llvm::Twine Path(Prefix, Suffix.data());
    
    return llvm::sys::fs::exists(Path);
}

std::error_code realpath(llvm::StringRef Path, std::string &Result)
{
    llvm::SmallVector<char, 64> Buffer;
    
    if (llvm::sys::path::is_absolute(Path) && Path.data() != Result.data()) {
        Result = Path.str();
        return std::error_code();
    }
    
    Buffer.append(Path.begin(), Path.end());
    
    auto Error = llvm::sys::fs::make_absolute(Buffer);
    if (!Error) {
        Result.clear();
        Result.insert(Result.end(), Buffer.begin(), Buffer.end());
    }
    
    return Error;
}

std::error_code realpath(std::string &Path)
{
    return realpath(Path, Path);
}

llvm::StringRef searchParents(llvm::StringRef Path, llvm::StringRef FileName)
{
    llvm::SmallVector<char, 64> Buffer;
    
    if (!llvm::sys::path::is_absolute(Path)) {
        Buffer.append(Path.begin(), Path.end());
        
        auto Error = llvm::sys::fs::make_absolute(Buffer);
        if (Error)
            return llvm::StringRef();
        
        Path = llvm::StringRef(Buffer.data(), Buffer.size());
    }
    
    Path = Path.rtrim("/");
    
    while (!Path.empty()) {
        if (exists(Path, FileName))
            break;
    }
    
    return Path;
}

llvm::StringRef projectPath(llvm::StringRef Path)
{
    Path = Path.rtrim("/");
    
    while (!Path.empty()) {
        if (exists(Path, "/compile_commands.json"))
            break;
        
        if (exists(Path, "/.rf"))
            break;
        
        if (exists(Path, "/.clang-format"))
            break;
        
        if (exists(Path, "/.git"))
            break;
        
        Path = llvm::sys::path::parent_path(Path);
    }
    
    return Path;
}

}
}
