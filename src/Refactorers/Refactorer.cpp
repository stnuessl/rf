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


#include <utility>

#include <clang/Tooling/Refactoring.h>
#include <Refactorers/Refactorer.hpp>


Refactorer::Refactorer()
    : _CompilerInstance(nullptr),
      _ASTContext(nullptr),
      _ReplSet(nullptr),
      _DupCount(0),
      _Verbose(false),
      _Force(false)
{
}

void Refactorer::setCompilerInstance(clang::CompilerInstance *CI)
{
    _CompilerInstance = CI;
}

void Refactorer::setASTContext(clang::ASTContext *ASTContext)
{
    _ASTContext = ASTContext;
}

void Refactorer::setReplacements(clang::tooling::Replacements *ReplSet)
{
    _ReplSet = ReplSet;
}

const clang::tooling::Replacements *Refactorer::replacements() const
{
    return _ReplSet;
}

void Refactorer::setVerbose(bool Value)
{
    _Verbose = Value;
}

bool Refactorer::verbose() const
{
    return _Verbose;
}

void Refactorer::setForce(bool Value)
{
    _Force = Value;
}

bool Refactorer::force() const
{
    return _Force;
}

void Refactorer::beforeSourceFileAction(llvm::StringRef File)
{
    (void) File;
}

void Refactorer::afterSourceFileAction()
{
}

void Refactorer::visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitCXXDestructorDecl(const clang::CXXDestructorDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitCXXMethodDecl(const clang::CXXMethodDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitCXXRecordDecl(const clang::CXXRecordDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitEnumConstantDecl(const clang::EnumConstantDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitEnumDecl(const clang::EnumDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitFieldDecl(const clang::FieldDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitFunctionDecl(const clang::FunctionDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitNamespaceDecl(const clang::NamespaceDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitRecordDecl(const clang::RecordDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitUsingDecl(const clang::UsingDecl *Decl)
{
    (void) Decl;
}

void 
Refactorer::visitUsingDirectiveDecl(const clang::UsingDirectiveDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitVarDecl(const clang::VarDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitCallExpr(const clang::CallExpr *Expr)
{
    (void) Expr;
}

void Refactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    (void) Expr;
}

void Refactorer::visitMemberExpr(const clang::MemberExpr *Expr)
{
    (void) Expr;
}

void Refactorer::
visitNestedNameSpecifierLoc(const clang::NestedNameSpecifierLoc &NNSLoc)
{
    (void) NNSLoc;
}

void Refactorer::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::addReplacement(const clang::SourceLocation Loc, 
                                unsigned int Length, 
                                llvm::StringRef ReplText)
{
    auto &SM = _CompilerInstance->getSourceManager();
    
    addReplacement(SM, Loc, Length, ReplText);
}

void Refactorer::addReplacement(const clang::SourceManager &SM, 
                                const clang::SourceLocation Loc, 
                                unsigned int Length, 
                                llvm::StringRef ReplText)
{
    if (Loc.isMacroID() || SM.isInSystemHeader(Loc))
        return;
    
    auto Repl = clang::tooling::Replacement(SM, Loc, Length, ReplText);
    
    auto Ok = _ReplSet->insert(std::move(Repl)).second;
    if (_Verbose && Ok) {
        Loc.dump(SM);
        llvm::errs() << " --> \"" << ReplText << "\"\n";
    }
    
    _DupCount += !Ok;
}
