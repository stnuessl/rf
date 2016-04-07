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

#ifndef _NAMESPACEREFACTORER_HPP_
#define _NAMESPACEREFACTORER_HPP_

#include <Refactorers/Refactorer.hpp>

class NamespaceRefactorer : public Refactorer {
public:
    NamespaceRefactorer();
    
    virtual void run(const MatchResult &Result) override;
private:
    void runNamespaceDecl(const MatchResult &Result);
    void runNestedNameSpecifierLoc(const MatchResult &Result);
    void runUsingDecl(const MatchResult &Result);
};

#endif /* _NAMESPACEREFACTORER_HPP_ */