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

#include <string>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Refactoring.h>

#ifndef _REFACTORER_HPP_
#define _REFACTORER_HPP_

class Refactorer : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    typedef clang::ast_matchers::MatchFinder::MatchResult MatchResult;
    
    Refactorer();
    
    void setVictimName(const std::string &Str);
    void setVictimName(std::string &&Str);
    const std::string &victimName() const;
    
    void setReplacementName(const std::string &Str);
    void setReplacementName(std::string &&Str);
    const std::string &replacementName() const;
    
    void setReplacementSet(clang::tooling::Replacements *Repls);
    const clang::tooling::Replacements *replacementSet() const;
    
    void setVerbose(bool Verbose);
    bool verbose() const;
    
    virtual void run(const MatchResult &Result) = 0;
    
    clang::ast_matchers::MatchFinder *matchFinder();
    
protected:
    
    void addReplacement(const MatchResult &Result, 
                        const clang::SourceLocation &Loc);
    void addReplacement(const clang::SourceManager &SM, 
                        const clang::SourceLocation &Loc);
    
    std::string &qualifiedName(const clang::NamedDecl *NamedDecl);
    
    clang::ast_matchers::MatchFinder _Finder;
    clang::tooling::Replacements *_Repls;
    
    std::string _Victim;
    std::string _ReplName;
    
    std::size_t _ReplSize;
    
private:
    std::string _Buffer;
    bool _Verbose;
};

#endif /* _REFACTORER_HPP_ */
