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

#include <memory>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <util/commandline.hpp>
#include <util/filesystem.hpp>
#include <util/memory.hpp>
#include <util/yaml.hpp>

#include <ConfigurationFile.hpp>

const std::string &ConfigurationFile::path() const
{
    return ConfigPath_;
}


void ConfigurationFile::write(llvm::StringRef Path)
{
    if (!llvm::sys::fs::exists(Path)) {
        auto Error = util::fs::createDirectoriesForFile(Path);
        if (Error) {
            llvm::errs() << util::cl::Error() 
                         << "failed to create \"" << Path << "\" - "
                         << Error.message() << "\n";
            std::exit(EXIT_FAILURE);
        }
    }
    
    util::yaml::write(Path, *this);
}

std::unique_ptr<ConfigurationFile> 
ConfigurationFile::detectFrom(llvm::StringRef Path)
{
    Path = util::fs::searchParents(Path, "/.rf/config");
    
    if (Path.empty())
        std::unique_ptr<ConfigurationFile>(nullptr);
    
    auto Config = std::make_unique<ConfigurationFile>();
    Config->ConfigPath_ = llvm::Twine(Path, "/.rf/config").str();
    
    util::yaml::read(Config->ConfigPath_, *Config);
    
    return Config;
}
