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

#ifndef RF_VARIABLEREFACTORER_HPP_
#define RF_VARIABLEREFACTORER_HPP_

#include <Refactorers/Base/NameRefactorer.hpp>

class VariableRefactorer : public NameRefactorer {
public:
    virtual void
    visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl) override;

    virtual void visitUsingDecl(const clang::UsingDecl *Decl) override;
    virtual void visitFieldDecl(const clang::FieldDecl *Decl) override;
    virtual void visitVarDecl(const clang::VarDecl *Decl) override;

    virtual void visitDeclRefExpr(const clang::DeclRefExpr *Expr) override;
    virtual void visitMemberExpr(const clang::MemberExpr *Expr) override;
};

#endif /* RF_VARIABLEREFACTORER_HPP_ */
