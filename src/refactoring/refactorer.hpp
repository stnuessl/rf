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

class refactorer : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    typedef clang::ast_matchers::MatchFinder::MatchResult match_result;

    refactorer();
    
    void set_victim(const std::string &str);
    void set_repl_str(const std::string &str);
    void set_replacements(clang::tooling::Replacements *repls);
    
    virtual void run(const match_result &result) = 0;
    
    clang::ast_matchers::MatchFinder *match_finder();
    
protected:
    void add_replacement(const match_result &result, 
                         const clang::SourceLocation &loc);
    void add_replacement(const clang::SourceManager &sm, 
                         const clang::SourceLocation &loc);
    
    std::string &qualified_name(const clang::NamedDecl *decl);
    
    clang::ast_matchers::MatchFinder _finder;
    clang::tooling::Replacements *_repls;
    
    std::string _victim;
    std::string _repl_str;
    
    std::size_t _repl_size;

private:
    std::string _buffer;
};

#endif /* _REFACTORER_HPP_ */
