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

#ifndef RF_REFACTORINGASTIVISITOR_HPP_
#define RF_REFACTORINGASTIVISITOR_HPP_

#include <vector>

#include <clang/AST/RecursiveASTVisitor.h>

#include <Refactorers/Base/Refactorer.hpp>

class RefactoringASTVisitor
    : public clang::RecursiveASTVisitor<RefactoringASTVisitor> {
public:
    void setRefactorers(std::vector<std::unique_ptr<Refactorer>> *Refactorers);
    void setASTContext(clang::ASTContext &ASTContext);

    bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *Decl);
    bool VisitCXXDestructorDecl(clang::CXXDestructorDecl *Decl);
    bool VisitCXXMethodDecl(clang::CXXMethodDecl *Decl);
    bool VisitCXXRecordDecl(clang::CXXRecordDecl *Decl);
    bool VisitDecl(clang::Decl *Decl);
    bool VisitDeclaratorDecl(clang::DeclaratorDecl *Decl);
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
    bool VisitUnresolvedLookupExpr(clang::UnresolvedLookupExpr *Expr);

    bool VisitElaboratedTypeLoc(clang::ElaboratedTypeLoc &TypeLoc);
    bool VisitFunctionProtoTypeLoc(clang::FunctionProtoTypeLoc &TypeLoc);
    bool VisitFunctionTypeLoc(clang::FunctionTypeLoc &TypeLoc);
    bool
    VisitInjectedClassNameTypeLoc(clang::InjectedClassNameTypeLoc &TypeLoc);
    bool VisitMemberPointerTypeLoc(clang::MemberPointerTypeLoc &TypeLoc);
    bool VisitPointerTypeLoc(clang::PointerTypeLoc &TypeLoc);
    bool VisitQualifiedTypeLoc(clang::QualifiedTypeLoc &TypeLoc);
    bool VisitReferenceTypeLoc(clang::ReferenceTypeLoc &TypeLoc);
    bool VisitTagTypeLoc(clang::TagTypeLoc &TypeLoc);
    bool VisitTemplateSpecializationTypeLoc(
        clang::TemplateSpecializationTypeLoc &TypeLoc);
    bool VisitTemplateTypeParmTypeLoc(clang::TemplateTypeParmTypeLoc &TypeLoc);
    bool VisitTypedefTypeLoc(clang::TypedefTypeLoc &TypeLoc);
    bool VisitTypeLoc(clang::TypeLoc &TypeLoc);

private:
    std::vector<std::unique_ptr<Refactorer>> *Refactorers_;
};

#endif /* RF_REFACTORINGASTVISITOR_HPP_ */
