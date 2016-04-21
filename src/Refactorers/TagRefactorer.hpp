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

#include <Refactorers/NameRefactorer.hpp>

class TagRefactorer : public NameRefactorer {
public:
    virtual void visitEnumConstantDecl(const clang::EnumConstantDecl *Decl) 
                                                                       override;
    virtual void visitEnumDecl(const clang::EnumDecl *Decl) override;
    virtual void visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl) 
                                                                       override;
    virtual void visitRecordDecl(const clang::RecordDecl *Decl) override;
    virtual void visitTypedefNameDecl(const clang::TypedefNameDecl *Decl)
                                                                       override;
    
    virtual void visitDeclRefExpr(const clang::DeclRefExpr *Expr) override;
    virtual void visitTypeLoc(const clang::TypeLoc &TypeLoc) override;
};

#endif /* _TAGREFACTORER_HPP_ */
