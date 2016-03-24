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

#ifndef _TAGREFACTORER_HPP_
#define _TAGREFACTORER_HPP_

#include <Refactorers/Refactorer.hpp>

class TagRefactorer : public Refactorer {
public:
    TagRefactorer();
    
    virtual void run(const MatchResult &Result) override;
private:
    void runRecordDecl(const MatchResult &Result);
    void runEnumDecl(const MatchResult &Result);
    void runTypeLoc(const MatchResult &Result);
    void runCXXMethodDecl(const MatchResult &Result);
};


#endif /* _TAGREFACTORER_HPP_ */
