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

#ifndef _REFACTORER_HPP_
#define _REFACTORER_HPP_

#include <clang/AST/DeclCXX.h>
#include <clang/Tooling/Refactoring.h>

class Refactorer {
public:
    Refactorer();
    virtual ~Refactorer() = default;
    
    void setASTContext(clang::ASTContext *ASTContext);
    
    void setReplacements(clang::tooling::Replacements *Repls);
    const clang::tooling::Replacements *replacements() const;
    
    void setVerbose(bool Value);
    bool verbose() const;
    
    void setForce(bool Value);
    bool force() const;
    
    virtual void visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl);
    virtual void visitCXXDestructorDecl(const clang::CXXDestructorDecl *Decl);
    virtual void visitCXXMethodDecl(const clang::CXXMethodDecl *Decl);
    virtual void visitCXXRecordDecl(const clang::CXXRecordDecl *Decl);
    virtual void visitEnumDecl(const clang::EnumDecl *Decl);
    virtual void visitFieldDecl(const clang::FieldDecl *Decl);
    virtual void visitFunctionDecl(const clang::FunctionDecl *Decl);
    virtual void visitNamespaceDecl(const clang::NamespaceDecl *Decl);
    virtual void visitRecordDecl(const clang::RecordDecl *Decl);
    virtual void visitUsingDecl(const clang::UsingDecl *Decl);
    virtual void visitUsingDirectiveDecl(const clang::UsingDirectiveDecl *Decl);
    virtual void visitVarDecl(const clang::VarDecl *Decl);
    
    virtual void visitCallExpr(const clang::CallExpr *Expr);
    virtual void visitDeclRefExpr(const clang::DeclRefExpr *Expr);
    virtual void visitMemberExpr(const clang::MemberExpr *Expr);
    
    virtual void 
    visitNestedNameSpecifierLoc(const clang::NestedNameSpecifierLoc &NNSLoc);
    virtual void visitTypeLoc(const clang::TypeLoc &TypeLoc);
    
protected:
    void addReplacement(const clang::SourceLocation &Loc,
                        unsigned int Length,
                        clang::StringRef ReplText);
    void addReplacement(const clang::SourceManager &SM,
                        const clang::SourceLocation &Loc,
                        unsigned int Length,
                        clang::StringRef ReplText);
    
    clang::ASTContext *_ASTContext;
    clang::tooling::Replacements *_ReplSet;
    unsigned int _DupCount;
    bool _Verbose;
    bool _Force;
};

typedef std::vector<std::unique_ptr<Refactorer>> RefactorerVector;

#endif /* _REFACTORER_HPP_ */
