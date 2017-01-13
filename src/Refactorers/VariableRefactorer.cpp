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

#include <Refactorers/VariableRefactorer.hpp>

void VariableRefactorer::visitCXXConstructorDecl(
    const clang::CXXConstructorDecl *Decl)
{
    /* Handle field declarations initialized in an initializer list */
    if (!Decl->hasBody())
        return;
    
    for (const auto &Init : Decl->inits()) {
        if (!Init->isMemberInitializer() || !Init->isWritten())
            continue;
        
        if (!isVictim(Init->getMember()))
            continue;
        
        addReplacement(Init->getSourceLocation());
    }
}

void VariableRefactorer::visitFieldDecl(const clang::FieldDecl *Decl)
{
    if (!isVictim(Decl))
        return;
    
    addReplacement(Decl->getLocation());
}

void VariableRefactorer::visitVarDecl(const clang::VarDecl *Decl)
{
    if (!isVictim(Decl))
        return;
    
    addReplacement(Decl->getLocation());
}

void VariableRefactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    if (!isVictim(Expr->getDecl()))
        return;
        
    addReplacement(Expr->getNameInfo().getLoc());
}

void VariableRefactorer::visitMemberExpr(const clang::MemberExpr *Expr)
{
    auto Decl = Expr->getMemberDecl();
    if (!clang::dyn_cast<clang::FieldDecl>(Decl) || !isVictim(Decl))
        return;
    
    addReplacement(Expr->getMemberLoc());
}

