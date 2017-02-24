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

#ifndef RF_CONFIGURATIONFILE_HPP_
#define RF_CONFIGURATIONFILE_HPP_

#include <llvm/Support/YAMLTraits.h>

class ConfigurationFile {
public:
    ConfigurationFile() = default;
    
    const std::string &path() const;
        
    void write(llvm::StringRef Path);
    static std::unique_ptr<ConfigurationFile> detectFrom(llvm::StringRef Path);
    
    std::string CompileCommands;
    unsigned int NumThreads;
    bool Interactive;
    bool Verbose;
//     bool CachePTHs;
    
private:
    std::string ConfigPath_;
};

namespace llvm {
namespace yaml {
        
template<>
struct MappingTraits<ConfigurationFile> {
    static void mapping(llvm::yaml::IO &IO, ConfigurationFile &Config)
    {
        IO.mapRequired("Compile-Commands", Config.CompileCommands);
        IO.mapRequired("Num-Threads", Config.NumThreads);
        IO.mapRequired("Interactive", Config.Interactive);
        IO.mapRequired("Verbose", Config.Verbose);
    }
};

}
}

#endif /* RF_CONFIGURATIONFILE_HPP_ */
