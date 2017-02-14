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

#ifndef RF_YAML_HPP_
#define RF_YAML_HPP_

#include <clang/Tooling/Refactoring.h>

#include <llvm/Support/YAMLTraits.h>

#include <util/CommandLine.hpp>
#include <util/memory.hpp>

namespace util {
namespace yaml {

template <typename T>
void read(const llvm::StringRef Path, T &Object)
{
    auto MemBuffer = util::memory::createMemoryBufferFromFile(Path);
    
    llvm::yaml::Input YAMLInput(MemBuffer->getBuffer());
    YAMLInput >> Object;
    
    if (YAMLInput.error()) {
        llvm::errs() << util::cl::Error() << "failed to parse file \"" 
                     << Path << "\".\n"; 
        std::exit(EXIT_FAILURE);
    }
}

struct RefactoringArgs {    
    std::vector<std::string> EnumConstants;
    std::vector<std::string> Functions;
    std::vector<std::string> Macros;
    std::vector<std::string> Namespaces;
    std::vector<std::string> Tags;
    std::vector<std::string> Variables;
};

}
}

LLVM_YAML_IS_SEQUENCE_VECTOR(clang::tooling::Replacement)
LLVM_YAML_IS_SEQUENCE_VECTOR(std::string)

namespace llvm {
namespace yaml {
        
template<>
struct MappingTraits<util::yaml::RefactoringArgs> {
    static void mapping(llvm::yaml::IO &IO, util::yaml::RefactoringArgs &Args)
    {
        IO.mapOptional("Enum-Constants", Args.EnumConstants);
        IO.mapOptional("Functions", Args.Functions);
        IO.mapOptional("Macros", Args.Macros);
        IO.mapOptional("Namespaces", Args.Namespaces);
        IO.mapOptional("Tags", Args.Tags);
        IO.mapOptional("Variables", Args.Variables);
    }
};

}
}




#endif /* RF_YAML_HPP_ */
