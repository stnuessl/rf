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

#ifndef _FUNCTIONREFACTORER_HPP_
#define _FUNCTIONREFACTORER_HPP_

#include <Refactorers/NameRefactorer.hpp>

class FunctionRefactorer : public NameRefactorer {
public:
    virtual void visitCallExpr(const clang::CallExpr *Expr) override;
    virtual void visitDeclRefExpr(const clang::DeclRefExpr *Expr) override;
    virtual void visitFunctionDecl(const clang::FunctionDecl *Decl) override;
private:
    bool isVictim(const clang::FunctionDecl *Decl);
    bool overridesVictim(const clang::CXXMethodDecl *Decl);
    bool overridesVictim(const clang::FunctionDecl *Decl);
    
    static bool overrides(const clang::CXXMethodDecl *Decl);
    static bool overrides(const clang::FunctionDecl *Decl);
};

#if 0

#include <Refactorers/Refactorer.hpp>

class FunctionRefactorer : public Refactorer {
public:
    FunctionRefactorer();
    
    virtual void run(const MatchResult &Result) override;
private:
    void runFunctionDecl(const MatchResult &Result);
    void runCallExpr(const MatchResult &Result);
    void runDeclRefExpr(const MatchResult &Result);
    
    bool isVictim(const clang::FunctionDecl *FunctionDecl);
    
    bool overridesVictim(const clang::CXXMethodDecl *CXXMethodDecl);
    
    static bool overrides(const clang::FunctionDecl *FunctionDecl);
    static bool overrides(const clang::CXXMethodDecl *CXXMethodDecl);
};

#endif

#endif /* _FUNCTIONREFACTORER_HPP_ */
