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


void RefactoringASTVisitor::setRefactorers(RefactorerVector *Vec)
{
    _Refactorers = Vec;
}

void RefactoringASTVisitor::setASTContext(clang::ASTContext &ASTContext)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->setASTContext(&ASTContext);
}

bool 
RefactoringASTVisitor::VisitCXXConstructorDecl(clang::CXXConstructorDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitCXXConstructorDecl(Decl);
    
    return true;
}

bool 
RefactoringASTVisitor::VisitCXXDestructorDecl(clang::CXXDestructorDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitCXXDestructorDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitCXXMethodDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitCXXRecordDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitEnumConstantDecl(clang::EnumConstantDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitEnumConstantDecl(Decl);
    
    return true;
}


bool RefactoringASTVisitor::VisitEnumDecl(clang::EnumDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitEnumDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitFieldDecl(clang::FieldDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitFieldDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitFunctionDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitNamespaceDecl(clang::NamespaceDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitNamespaceDecl(Decl);
    
    return true;
}


bool RefactoringASTVisitor::VisitRecordDecl(clang::RecordDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitRecordDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitTypedefNameDecl(clang::TypedefNameDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitTypedefNameDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitUsingDecl(clang::UsingDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitUsingDecl(Decl);
    
    return true;
}

bool 
RefactoringASTVisitor::VisitUsingDirectiveDecl(clang::UsingDirectiveDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitUsingDirectiveDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitVarDecl(clang::VarDecl *Decl)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitVarDecl(Decl);
    
    return true;
}

bool RefactoringASTVisitor::VisitCallExpr(const clang::CallExpr *Expr)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitCallExpr(Expr);
    
    return true;
}

bool RefactoringASTVisitor::VisitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitDeclRefExpr(Expr);
    
    return true;
}

bool RefactoringASTVisitor::VisitMemberExpr(const clang::MemberExpr *Expr)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitMemberExpr(Expr);
    
    return true;
}


bool RefactoringASTVisitor::
VisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc &NNSLoc)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitNestedNameSpecifierLoc(NNSLoc);
    
    return true;
}


bool RefactoringASTVisitor::VisitTypeLoc(clang::TypeLoc &TypeLoc)
{
    for (auto &Refactorer : *_Refactorers)
        Refactorer->visitTypeLoc(TypeLoc);
    
    return true;
}


