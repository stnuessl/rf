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

#ifndef RF_NAMESPACEREFACTORER_HPP_
#define RF_NAMESPACEREFACTORER_HPP_

#include <Refactorers/Base/NameRefactorer.hpp>

class NamespaceRefactorer : public NameRefactorer {
public:
    virtual void
    visitDeclaratorDecl(const clang::DeclaratorDecl *Decl) override;
    virtual void
    visitNamespaceAliasDecl(const clang::NamespaceAliasDecl *Decl) override;
    virtual void visitNamespaceDecl(const clang::NamespaceDecl *Decl) override;
    virtual void visitUsingDecl(const clang::UsingDecl *Decl) override;
    virtual void
    visitUsingDirectiveDecl(const clang::UsingDirectiveDecl *Decl) override;

    virtual void visitDeclRefExpr(const clang::DeclRefExpr *Expr) override;
    virtual void
    visitUnresolvedLookupExpr(const clang::UnresolvedLookupExpr *Expr) override;

    virtual void
    visitElaboratedTypeLoc(const clang::ElaboratedTypeLoc &TypeLoc) override;

protected:
    void traverse(clang::NestedNameSpecifierLoc NNSLoc);
};

#endif /* RF_NAMESPACEREFACTORER_HPP_ */
