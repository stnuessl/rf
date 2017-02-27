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

#include <RefactoringASTVisitor.hpp>

void RefactoringASTVisitor::setRefactorers(
    std::vector<std::unique_ptr<Refactorer>> *Refactorers)
{
    Refactorers_ = Refactorers;
}

void RefactoringASTVisitor::setASTContext(clang::ASTContext &ASTContext)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->setASTContext(&ASTContext);
}

bool RefactoringASTVisitor::VisitCXXConstructorDecl(
    clang::CXXConstructorDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitCXXConstructorDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitCXXDestructorDecl(
    clang::CXXDestructorDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitCXXDestructorDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitCXXMethodDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitCXXRecordDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitDecl(clang::Decl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitEnumConstantDecl(clang::EnumConstantDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitEnumConstantDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitEnumDecl(clang::EnumDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitEnumDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitFieldDecl(clang::FieldDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitFieldDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitFunctionDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitNamespaceAliasDecl(
    clang::NamespaceAliasDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitNamespaceAliasDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitNamespaceDecl(clang::NamespaceDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitNamespaceDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitRecordDecl(clang::RecordDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitRecordDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitTypedefNameDecl(clang::TypedefNameDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitTypedefNameDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitUsingDecl(clang::UsingDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitUsingDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitUsingDirectiveDecl(
    clang::UsingDirectiveDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitUsingDirectiveDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitUsingShadowDecl(clang::UsingShadowDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitUsingShadowDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitVarDecl(clang::VarDecl *Decl)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitVarDecl(Decl);

    return true;
}

bool RefactoringASTVisitor::VisitExpr(clang::Expr *Expr)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitExpr(Expr);

    return true;
}

bool RefactoringASTVisitor::VisitCallExpr(clang::CallExpr *Expr)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitCallExpr(Expr);

    return true;
}

bool RefactoringASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *Expr)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitDeclRefExpr(Expr);

    return true;
}

bool RefactoringASTVisitor::VisitMemberExpr(clang::MemberExpr *Expr)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitMemberExpr(Expr);

    return true;
}

bool RefactoringASTVisitor::VisitMemberPointerTypeLoc(
    clang::MemberPointerTypeLoc& TypeLoc)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitMemberPointerTypeLoc(TypeLoc);
    
    return true;
}

bool RefactoringASTVisitor::VisitQualifiedTypeLoc(
    clang::QualifiedTypeLoc &TypeLoc)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitQualifiedTypeLoc(TypeLoc);
    
    return true;
}

bool RefactoringASTVisitor::VisitTemplateSpecializationTypeLoc(
    clang::TemplateSpecializationTypeLoc& TypeLoc)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitTemplateSpecializationTypeLoc(TypeLoc);
    
    return true;
}

bool RefactoringASTVisitor::VisitTypedefTypeLoc(clang::TypedefTypeLoc &TypeLoc)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitTypedefTypeLoc(TypeLoc);
    
    return true;
}


bool RefactoringASTVisitor::VisitTypeLoc(clang::TypeLoc &TypeLoc)
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->visitTypeLoc(TypeLoc);
    
    return true;
}
