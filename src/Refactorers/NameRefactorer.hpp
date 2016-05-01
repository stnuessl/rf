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

#include <functional>

#include <clang/Lex/MacroInfo.h>

#include <Refactorers/Refactorer.hpp>

/*
 * Base class for Refactorers which want to replace a name with a new one.
 * Handles qualifier parsing, checking if an entity has to be replaced, and
 * conveniently adding replacements with just one argument. With this most
 * subclasses only need to implement the corresponding 'visit*()' or
 * PPCallbacks functions.
 */

class NameRefactorer : public Refactorer {
public:
    NameRefactorer();
    
    void setVictimQualifier(const std::string &Victim);
    void setVictimQualifier(std::string &&Victim);
    const std::string &victimQualifier() const;
    
    void setReplacementQualifier(const std::string &Repl);
    void setReplacementQualifier(std::string &&Repl);
    const std::string &replacementQualifier() const;
    
protected:
    bool isVictim(const clang::NamedDecl *NamedDecl);
    bool isVictim(const clang::Token &MacroName, 
                  const clang::MacroInfo *MacroInfo);
    
    void addReplacement(clang::SourceLocation Loc);
private:
    bool isEqualToVictim(const std::string &Name) const;
    bool isEqualToVictimPrefix(const std::string &Name) const;
    
    void setVictimQualifier(std::string &&Victim, 
                            std::string::iterator Begin,
                            std::string::iterator End);
    
    bool isVictimLocation(const clang::SourceLocation Loc);

    const std::string &qualifiedName(const clang::NamedDecl *NamedDecl);
    
    std::string _Victim;
    std::string _ReplName;
    std::string _Buffer;
    clang::SourceLocation _VictimLoc;
    
    std::size_t _ReplSize;
    unsigned int _Line;
    unsigned int _Column;
   
    std::function<bool(const NameRefactorer &, const std::string &)> 
    _IsEqualFunc;
};

#endif /* _NAMEREFACTORER_HPP_ */
