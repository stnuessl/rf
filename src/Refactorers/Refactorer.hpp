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

#ifndef _REFACTORER_HPP_
#define _REFACTORER_HPP_

#include <string>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Refactoring.h>

class Refactorer : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    typedef clang::ast_matchers::MatchFinder::MatchResult MatchResult;
    
    Refactorer();
    
    /* TODO: change to setVictimQualifier() */
    virtual void setVictimQualifier(const std::string &Str);
    virtual void setVictimQualifier(std::string &&Str);
    
    const std::string &victimQualifier() const;
    
    /* TODO: ReplacementQualifier? */
    void setReplacementQualifier(const std::string &Str);
    void setReplacementQualifier(std::string &&Str);
    const std::string &replacementQualifier() const;
    
    void setReplacementSet(clang::tooling::Replacements *Repls);
    const clang::tooling::Replacements *replacementSet() const;
    
    void setVerbose(bool Verbose);
    bool verbose() const;
    
    void setForce(bool Force);
    bool force() const;
    
    virtual void run(const MatchResult &Result) = 0;
    
    clang::ast_matchers::MatchFinder *matchFinder();
    
protected:
    
    void addReplacement(const MatchResult &Result, 
                        const clang::SourceLocation &Loc);
    void addReplacement(const clang::SourceManager &SM, 
                        const clang::SourceLocation &Loc);
    
    bool isVictim(const clang::NamedDecl *NamedDecl);
    
    clang::ast_matchers::MatchFinder _Finder;
    clang::tooling::Replacements *_Repls;
    
    std::string _Victim;
    std::string _ReplName;
    
    std::size_t _ReplSize;
    
private:
    std::string &qualifiedName(const clang::NamedDecl *NamedDecl);

    std::string _Buffer;
    
    unsigned int _DupCount;
    
    bool _Verbose;
    bool _Force;
};

#endif /* _REFACTORER_HPP_ */
