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

#ifndef _INCLUDESANITIZER_HPP_
#define _INCLUDESANITIZER_HPP_

#include <unordered_map>
// #include <vector>

#include <Sanitizers/Sanitizer.hpp>


class IncludeSanitizer : public Sanitizer {
public:
    typedef std::pair<unsigned int, unsigned int> UIntPair;
    struct PairHash {
        std::size_t operator()(const UIntPair &Pair) const
        {
            auto Hasher = std::hash<unsigned int>();
            auto First = Hasher(Pair.first);
            auto Second = Hasher(Pair.second);

            return First ^ Second;
        }
    };
    
    struct PairEqual {
        bool operator()(const UIntPair &A, const UIntPair &B) const
        {
            return A == B;
        }
    };
    typedef std::vector<clang::SourceRange> SourceRangeVector;
    typedef std::unordered_map<UIntPair, SourceRangeVector, PairHash, PairEqual>
    IncludeMap;
    
    IncludeMap &includeMap();
    const IncludeMap &includeMap() const;
    
    virtual void ExecuteAction() override;
    virtual void EndSourceFileAction() override;
    
    virtual std::unique_ptr<clang::ASTConsumer> 
    CreateASTConsumer(clang::CompilerInstance &CI, 
                      clang::StringRef File) override;
private:
    IncludeMap _IncludeMap;
};

#endif /* _INCLUDESANITIZER_HPP_ */