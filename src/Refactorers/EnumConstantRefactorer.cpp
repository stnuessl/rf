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

#include <Refactorers/EnumConstantRefactorer.hpp>

void EnumConstantRefactorer::visitEnumConstantDecl(
    const clang::EnumConstantDecl *Decl)
{
    if (!isVictim(Decl))
        return;

    addReplacement(Decl->getLocation());
}

void EnumConstantRefactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    /* This handles enum constants, e.g.:
     *      enum a { a };
     *      int main() { auto a = a::a; };
     *                         (1)^  ^(2)
     *
     * 'Expr->getLocStart()' retrieves the incorrect position (1)
     * 'Expr->getLocation()' retrieves the correct position (2)
     */

    auto Decl = Expr->getDecl();
    if (!clang::dyn_cast<clang::EnumConstantDecl>(Decl) || !isVictim(Decl))
        return;

    addReplacement(Expr->getLocation());
}
