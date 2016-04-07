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

#ifndef _VARIABLEREFACTORER_HPP_
#define _VARIABLEREFACTORER_HPP_

#include <Refactorers/Refactorer.hpp>

class VariableRefactorer : public Refactorer {
public:
    VariableRefactorer();
    
    virtual void setVictimQualifier(const std::string &Str) override;
    virtual void setVictimQualifier(std::string &&Str) override;
    
    virtual void run(const MatchResult &Result) override;
private:
    void runVarDecl(const MatchResult &Result);
    void runDeclRefExpr(const MatchResult &Result);
    void runFieldDecl(const MatchResult &Result);
    void runMemberExpr(const MatchResult &Result);
    void runCXXConstructorDecl(const MatchResult &Result);
    
    bool isVictim(const clang::NamedDecl *NamedDecl, const MatchResult &Result);
    bool isVictim(const clang::NamedDecl *NamedDecl, 
                  const clang::SourceManager &SM);

    clang::SourceLocation _VictimLoc;
    unsigned int _LineNum;
};

#endif /* _VARIABLEREFACTORER_HPP_ */