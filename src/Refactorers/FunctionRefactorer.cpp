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

#include <Refactorers/FunctionRefactorer.hpp>

void FunctionRefactorer::visitCallExpr(const clang::CallExpr *Expr)
{
    /*
     * Only calls to class methods have to be handled here.
     * Normal function calls are handled with 'visitDeclRefExpr()'
     */
    
    auto FuncDecl = Expr->getDirectCallee();
    if (isVictim(FuncDecl) || overridesVictim(FuncDecl))
        addReplacement(Expr->getCallee()->getExprLoc());
}

void FunctionRefactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    /*
     * Handle replacements for expressions like:
     * 
     *      function_name();
     *      namespace_name::function_name();
     *      void (*callback)() = &victim;
     *      void (class_name::*callback)() = &class_name::function_name();
     */
    
    auto ValueDecl = Expr->getDecl();
    
    auto FuncDecl = clang::dyn_cast<clang::FunctionDecl>(ValueDecl);
    if (!FuncDecl)
        return;
    
    if (isVictim(FuncDecl) || overridesVictim(FuncDecl))
        addReplacement(Expr->getNameInfo().getLoc());
}

void FunctionRefactorer::visitFunctionDecl(const clang::FunctionDecl *Decl)
{
    if (isVictim(Decl) || overridesVictim(Decl))
        addReplacement(Decl->getLocation());
}

bool FunctionRefactorer::isVictim(const clang::FunctionDecl *Decl)
{
    if (!NameRefactorer::isVictim(Decl))
        return false;
    
    if (overrides(Decl) && !_Force) {
        auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
        
        auto Begin = MethodDecl->begin_overridden_methods();
        auto QualifiedName = (*Begin)->getQualifiedNameAsString();
        
        llvm::errs() << "** ERROR: refactoring overriding class method \""
                     << victimQualifier() << "\" - aborting\n"
                     << "** INFO: consider refactoring \"" << QualifiedName
                     << "\" instead or use the \"--force\" option\n";
        
        std::exit(EXIT_FAILURE);
    }
    
    return true;
}

bool FunctionRefactorer::overridesVictim(const clang::CXXMethodDecl *Decl)
{
    auto Begin = Decl->begin_overridden_methods();
    auto End = Decl->end_overridden_methods();
    
    for (auto It = Begin; It != End; ++It) {
        if (isVictim(*It))
            return true;
    }
    
    return false;
}

bool FunctionRefactorer::overridesVictim(const clang::FunctionDecl *Decl)
{
    auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    return (MethodDecl) ? overridesVictim(MethodDecl) : false;
}


bool FunctionRefactorer::overrides(const clang::CXXMethodDecl *Decl)
{
    return !!Decl->size_overridden_methods();
}

bool FunctionRefactorer::overrides(const clang::FunctionDecl *Decl)
{
    auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    return (MethodDecl) ? overrides(MethodDecl) : false;
}


#if 0

#include <Refactorers/FunctionRefactorer.hpp>


FunctionRefactorer::FunctionRefactorer()
    : Refactorer()
{
    using namespace clang::ast_matchers;
    
    auto FunctionDeclMatcher = functionDecl().bind("FuncDecl");
    auto CallExprMatcher = callExpr().bind("CallExpr");
    auto DeclRefExpr = declRefExpr().bind("DRefExpr");
    
    _Finder.addMatcher(FunctionDeclMatcher, this);
    _Finder.addMatcher(CallExprMatcher, this);
    _Finder.addMatcher(DeclRefExpr, this);
}

void FunctionRefactorer::run(const MatchResult &Result)
{
    runFunctionDecl(Result);
    runCallExpr(Result);
    runDeclRefExpr(Result);
}

void FunctionRefactorer::runFunctionDecl(const MatchResult &Result)
{
    auto FunctionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("FuncDecl");
    if (!FunctionDecl)
        return;
    
    if (isVictim(FunctionDecl))
        addReplacement(Result, FunctionDecl->getLocation());

    auto CXXMethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(FunctionDecl);
    if (!CXXMethodDecl || !overridesVictim(CXXMethodDecl))
        return;
    
    addReplacement(Result, CXXMethodDecl->getLocation());
}

void FunctionRefactorer::runCallExpr(const MatchResult &Result)
{
    auto CallExpr = Result.Nodes.getNodeAs<clang::CallExpr>("CallExpr");
    if (!CallExpr)
        return;
    
    auto FunctionDecl = CallExpr->getDirectCallee();
    if (!FunctionDecl)
        return;
    
    /*
     * Only calls to class methods have to be handled here.
     * Normal function calls are handled with 'runDeclRefExpr()'
     */
    
    if (!isVictim(FunctionDecl)) {
        auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(FunctionDecl);
        if (!MethodDecl || !overridesVictim(MethodDecl))
            return;
    }

    auto Loc = CallExpr->getCallee()->getExprLoc();
    addReplacement(Result, Loc);
}

void FunctionRefactorer::runDeclRefExpr(const MatchResult &Result)
{
    /*
     * Handle replacements for expressions like:
     * 
     *      function_name();
     *      namespace_name::function_name();
     *      void (*callback)() = &victim;
     *      void (class_name::*callback)() = &class_name::function_name();
     */
    auto DeclRefExpr = Result.Nodes.getNodeAs<clang::DeclRefExpr>("DRefExpr");
    if (!DeclRefExpr)
        return;
    
    auto ValueDecl = DeclRefExpr->getDecl();
    if (!ValueDecl)
        return;

    auto FunctionDecl = clang::dyn_cast<clang::FunctionDecl>(ValueDecl);
    if (!FunctionDecl)
        return;
    
    if (!isVictim(FunctionDecl)) {
        auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(FunctionDecl);
        if (!MethodDecl || !overridesVictim(MethodDecl))
            return;
    }
    
    auto Loc = DeclRefExpr->getNameInfo().getLoc();
    addReplacement(Result, Loc);
}

bool FunctionRefactorer::isVictim(const clang::FunctionDecl *FunctionDecl)
{
    if (!Refactorer::isVictim(FunctionDecl))
        return false;
    
    if (overrides(FunctionDecl) && !force()) {
        auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(FunctionDecl);
        
        auto Begin = MethodDecl->begin_overridden_methods();
        auto QualifiedName = (*Begin)->getQualifiedNameAsString();
        
        llvm::errs() << "** ERROR: refactoring overriding class method \""
                     << _Victim << "\" - aborting\n"
                     << "** INFO: consider refactoring \"" << QualifiedName
                     << "\" instead or use the \"--force\" option\n";
     
        std::exit(EXIT_FAILURE);
    }

    return true;
}

bool FunctionRefactorer::overridesVictim(const clang::CXXMethodDecl *CXXMDecl) 
{
    auto Begin = CXXMDecl->begin_overridden_methods();
    auto End = CXXMDecl->end_overridden_methods();
    
    for (auto It = Begin; It != End; ++It) {
        if (isVictim(*It))
            return true;
    }
    
    return false;
}

bool FunctionRefactorer::overrides(const clang::FunctionDecl *FunctionDecl)
{
    auto CXXMethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(FunctionDecl);
    return (CXXMethodDecl) ? overrides(CXXMethodDecl) : false;
}

bool FunctionRefactorer::overrides(const clang::CXXMethodDecl* CXXMethodDecl)
{
    return !!CXXMethodDecl->size_overridden_methods();
}

#endif
