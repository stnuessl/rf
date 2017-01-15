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

#ifndef _REFACTORINGASTVISITOR_HPP_
#define _REFACTORINGASTVISITOR_HPP_

#include <vector>

#include <clang/AST/RecursiveASTVisitor.h>

#include <Refactorers/Refactorer.hpp>

class RefactoringASTVisitor 
    : public clang::RecursiveASTVisitor<RefactoringASTVisitor> {
public:
    void setRefactorers(Refactorers *Refactorers);
    void setASTContext(clang::ASTContext &ASTContext);
    
    bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *Decl);
    bool VisitCXXDestructorDecl(clang::CXXDestructorDecl *Decl);
    bool VisitCXXMethodDecl(clang::CXXMethodDecl *Decl);
    bool VisitCXXRecordDecl(clang::CXXRecordDecl *Decl);
    bool VisitDecl(clang::Decl *Decl);
    bool VisitEnumConstantDecl(clang::EnumConstantDecl *Decl);
    bool VisitEnumDecl(clang::EnumDecl *Decl);
    bool VisitFieldDecl(clang::FieldDecl *Decl);
    bool VisitFunctionDecl(clang::FunctionDecl *Decl);
    bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl *Decl);
    bool VisitNamespaceDecl(clang::NamespaceDecl *Decl);
    bool VisitRecordDecl(clang::RecordDecl *Decl);
    bool VisitTypedefNameDecl(clang::TypedefNameDecl *Decl);
    bool VisitUsingDecl(clang::UsingDecl *Decl);
    bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl *Decl);
    bool VisitUsingShadowDecl(clang::UsingShadowDecl *Decl);
    bool VisitVarDecl(clang::VarDecl *Decl);
    
    bool VisitExpr(clang::Expr *Expr);
    bool VisitCallExpr(clang::CallExpr *Expr);
    bool VisitDeclRefExpr(clang::DeclRefExpr *Expr);
    bool VisitMemberExpr(clang::MemberExpr *Expr);
    
    bool VisitTypeLoc(clang::TypeLoc &TypeLoc);
    
private:
    Refactorers *_Refactorers;
};

#endif /* _REFACTORINGASTVISITOR_HPP_ */
