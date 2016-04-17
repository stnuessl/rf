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

#ifndef _NAMEREFACTORER_HPP_
#define _NAMEREFACTORER_HPP_

#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/MacroInfo.h>

#include <Refactorers/Refactorer.hpp>

class NameRefactorer : public Refactorer {
public:
    NameRefactorer();
    
    void setVictimQualifier(const std::string &Victim);
    void setVictimQualifier(const std::string &Victim, unsigned int Line);
    void setVictimQualifier(std::string &&Victim);
    void setVictimQualifier(std::string &&Victim, unsigned int Line);
    const std::string &victimQualifier() const;
    
    void setReplacementQualifier(const std::string &Repl);
    void setReplacementQualifier(std::string &&Repl);
    const std::string replacementQualifier() const;
    
protected:
    bool isVictim(const clang::NamedDecl *NamedDecl);
    bool isVictim(const clang::Token &MacroName,
                  const clang::MacroInfo *MacroInfo);
    
    void addReplacement(const clang::SourceLocation Loc);
private:
    bool isVictimLine(const clang::SourceLocation Loc);
    
    const std::string &qualifiedName(const clang::NamedDecl *NamedDecl);
    
    std::string _Victim;
    std::string _ReplName;
    std::string _Buffer;
    clang::SourceLocation _LineLoc;
    
    std::size_t _ReplSize;
    unsigned int _Line;
};

#endif /* _NAMEREFACTORER_HPP_ */
