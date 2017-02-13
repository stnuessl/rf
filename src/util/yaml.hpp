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

namespace util {
namespace yaml {

template <typename T>
void read(const std::string &Path, T &Object)
{
    auto Buffer = llvm::MemoryBuffer::getFile(Path);
    if (!Buffer) {
        llvm::errs() << util::cl::Error() << "failed to open file \"" 
                     << Path << "\" - " 
                     << Buffer.getError().message() << "\n";
        std::exit(EXIT_FAILURE);
    }
    
    llvm::yaml::Input YAMLInput(Buffer.get()->getBuffer());
    YAMLInput >> Object;
    
    if (YAMLInput.error()) {
        llvm::errs() << util::cl::Error() << "failed to parse file \"" 
                     << Path << "\".\n"; 
        std::exit(EXIT_FAILURE);
    }
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
    
struct ReplacementsInfo {
    ReplacementsInfo() = default;
    ReplacementsInfo(clang::tooling::Replacements &Repls);
    
    std::vector<clang::tooling::Replacement> Replacements;
};

}

LLVM_YAML_IS_SEQUENCE_VECTOR(clang::tooling::Replacement)
LLVM_YAML_IS_SEQUENCE_VECTOR(std::string)


namespace llvm {
namespace yaml {
        
template<>
struct MappingTraits<util::RefactoringArgs> {
    static void mapping(llvm::yaml::IO &IO, util::RefactoringArgs &Args)
    {
        IO.mapOptional("Enum-Constants", Args.EnumConstants);
        IO.mapOptional("Functions", Args.Functions);
        IO.mapOptional("Macros", Args.Macros);
        IO.mapOptional("Namespaces", Args.Namespaces);
        IO.mapOptional("Tags", Args.Tags);
        IO.mapOptional("Variables", Args.Variables);
    }
};

template <>
struct MappingTraits<util::ReplacementsInfo> {
    static void mapping(llvm::yaml::IO &IO, util::ReplacementsInfo &Info)
    {
        IO.mapRequired("Replacements", Info.Replacements);
    }
};

template <>
struct MappingTraits<clang::tooling::Replacement> {
    static void mapping(llvm::yaml::IO &IO, clang::tooling::Replacement &Repl)
    {
        auto File = Repl.getFilePath().str();
        auto Offset = Repl.getOffset();
        auto Length = Repl.getLength();
        auto Text = Repl.getReplacementText().str();
        
        IO.mapRequired("File", File);
        IO.mapRequired("Offset", Offset);
        IO.mapRequired("Length", Length);
        IO.mapRequired("Text", Text);
        
        Repl = clang::tooling::Replacement(File, Offset, Length, Text);
    }
};

}
}




#endif /* RF_YAML_HPP_ */
