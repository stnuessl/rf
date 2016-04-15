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
#include <cctype>

#include <clang/Tooling/Refactoring.h>
#include <Refactorers/Refactorer.hpp>


RefactorerNew::RefactorerNew()
    : _ASTContext(nullptr),
      _ReplSet(nullptr),
      _DupCount(0),
      _Verbose(false),
      _Force(false)
{
}

void RefactorerNew::setASTContext(clang::ASTContext *ASTContext)
{
    _ASTContext = ASTContext;
}

void RefactorerNew::setReplacements(clang::tooling::Replacements *ReplSet)
{
    _ReplSet = ReplSet;
}

const clang::tooling::Replacements *RefactorerNew::replacements() const
{
    return _ReplSet;
}

void RefactorerNew::setVerbose(bool Value)
{
    _Verbose = Value;
}

bool RefactorerNew::verbose() const
{
    return _Verbose;
}

void RefactorerNew::setForce(bool Value)
{
    _Force = Value;
}

bool RefactorerNew::force() const
{
    return _Force;
}

void RefactorerNew::visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitCXXDestructorDecl(const clang::CXXDestructorDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitCXXMethodDecl(const clang::CXXMethodDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitCXXRecordDecl(const clang::CXXRecordDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitEnumDecl(const clang::EnumDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitFieldDecl(const clang::FieldDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitFunctionDecl(const clang::FunctionDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitNamespaceDecl(const clang::NamespaceDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitRecordDecl(const clang::RecordDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitUsingDecl(const clang::UsingDecl *Decl)
{
    (void) Decl;
}

void 
RefactorerNew::visitUsingDirectiveDecl(const clang::UsingDirectiveDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitVarDecl(const clang::VarDecl *Decl)
{
    (void) Decl;
}

void RefactorerNew::visitCallExpr(const clang::CallExpr *Expr)
{
    (void) Expr;
}

void RefactorerNew::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    (void) Expr;
}

void RefactorerNew::visitMemberExpr(const clang::MemberExpr *Expr)
{
    (void) Expr;
}

void RefactorerNew::
visitNestedNameSpecifierLoc(const clang::NestedNameSpecifierLoc &NNSLoc)
{
    (void) NNSLoc;
}


void RefactorerNew::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    (void) TypeLoc;
}


void RefactorerNew::addReplacement(const clang::SourceLocation &Loc, 
                                   unsigned int Length, 
                                   StringRef ReplText)
{
    addReplacement(_ASTContext->getSourceManager(), Loc, Length, ReplText);
}


void RefactorerNew::addReplacement(const clang::SourceManager &SM, 
                                   const clang::SourceLocation &Loc, 
                                   unsigned int Length, 
                                   StringRef ReplText)
{
    auto Repl = clang::tooling::Replacement(SM, Loc, Length, ReplText);
    
    auto Ok = _ReplSet->insert(std::move(Repl)).second;
    if (_Verbose && Ok) {
        Loc.dump(SM);
        llvm::errs() << " --> \"" << ReplText << "\"\n";
    }
    
    _DupCount += !Ok;
}
