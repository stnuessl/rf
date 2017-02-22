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

#ifndef RF_REFACTORER_HPP_
#define RF_REFACTORER_HPP_

#include <mutex>

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Tooling/Refactoring.h>

/* 
 * Inheriting from PPCallbacks saves a lot of ugly boilerplate code.
 * Those PPCallbacks function are not directly called from the clang
 * preprocessor but from PPCallbackDispatcher. This enables to serve multiple
 * PPCallbacks clients at once.
 */

class Refactorer : public clang::PPCallbacks {
public:
    Refactorer() = default;
    virtual ~Refactorer() = default;
    
    void setCompilerInstance(clang::CompilerInstance *CI);
    void setASTContext(clang::ASTContext *ASTContext);
    
    const clang::tooling::Replacements &replacements() const;
    
    void setForce(bool Value);
    bool force() const;
    
    virtual void beforeSourceFileAction(llvm::StringRef File);
    virtual void afterSourceFileAction();
    
    virtual void visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl);
    virtual void visitCXXDestructorDecl(const clang::CXXDestructorDecl *Decl);
    virtual void visitCXXMethodDecl(const clang::CXXMethodDecl *Decl);
    virtual void visitCXXRecordDecl(const clang::CXXRecordDecl *Decl);
    virtual void visitDecl(const clang::Decl *Decl);
    virtual void visitEnumConstantDecl(const clang::EnumConstantDecl *Decl);
    virtual void visitEnumDecl(const clang::EnumDecl *Decl);
    virtual void visitFieldDecl(const clang::FieldDecl *Decl);
    virtual void visitFunctionDecl(const clang::FunctionDecl *Decl);
    virtual void visitNamespaceAliasDecl(const clang::NamespaceAliasDecl *Decl);
    virtual void visitNamespaceDecl(const clang::NamespaceDecl *Decl);
    virtual void visitRecordDecl(const clang::RecordDecl *Decl);
    virtual void visitTypedefNameDecl(const clang::TypedefNameDecl *Decl);
    virtual void visitUsingDecl(const clang::UsingDecl *Decl);
    virtual void visitUsingDirectiveDecl(const clang::UsingDirectiveDecl *Decl);
    virtual void visitUsingShadowDecl(const clang::UsingShadowDecl *Decl);
    virtual void visitVarDecl(const clang::VarDecl *Decl);
    
    virtual void visitExpr(const clang::Expr *Expr);
    virtual void visitCallExpr(const clang::CallExpr *Expr);
    virtual void visitDeclRefExpr(const clang::DeclRefExpr *Expr);
    virtual void visitMemberExpr(const clang::MemberExpr *Expr);
    
    virtual void visitTypeLoc(const clang::TypeLoc &TypeLoc);
    
protected:
    void addReplacement(clang::SourceLocation Loc,
                        unsigned int Length,
                        llvm::StringRef ReplText);
    void addReplacement(const clang::SourceManager &SM,
                        clang::SourceLocation Loc,
                        unsigned int Length,
                        llvm::StringRef ReplText);
    
    clang::CompilerInstance *CompilerInstance_;
    clang::ASTContext *ASTContext_;
    clang::tooling::Replacements Replacements_;
    bool Force_;
};

typedef std::vector<std::unique_ptr<Refactorer>> RefactorerVector;

#endif /* RF_REFACTORER_HPP_ */
