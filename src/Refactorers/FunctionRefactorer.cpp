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

#include <util/CommandLine.hpp>

static bool overrides(const clang::CXXMethodDecl *Decl)
{
    return !!Decl->size_overridden_methods();
}

static bool overrides(const clang::FunctionDecl *Decl)
{
    auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    return (MethodDecl) ? overrides(MethodDecl) : false;
}

void FunctionRefactorer::visitCallExpr(const clang::CallExpr *Expr)
{
    /*
     * Only calls to class methods have to be handled here.
     * Normal function calls are handled with 'visitDeclRefExpr()'
     */
    
    auto FuncDecl = Expr->getDirectCallee();
    if (!FuncDecl)
        return;
    
    if (isVictim(FuncDecl))
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
    
    if (isVictim(FuncDecl))
        addReplacement(Expr->getNameInfo().getLoc());
}

void FunctionRefactorer::visitFunctionDecl(const clang::FunctionDecl *Decl)
{
    if (isVictim(Decl))
        addReplacement(Decl->getLocation());
}

bool FunctionRefactorer::isVictim(const clang::FunctionDecl *Decl)
{
    /* 
     * We need to roll our own 'isVictim()' function here as we need to
     * automatically detect derived and overridden methods to change
     * them accordingly. Also, we want to avoid refactoring if the current 
     * function is a victim and overrides another function as this may alter
     * the behaviour of the program.
     */
    
    bool isVictimDecl = NameRefactorer::isVictim(Decl);
    if (isVictimDecl && overrides(Decl) && !Force_) {
        auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
        
        auto Begin = MethodDecl->begin_overridden_methods();
        auto QualifiedName = (*Begin)->getQualifiedNameAsString();
        
        llvm::errs() << util::cl::Error() 
                     << "refactoring overriding class method \""
                     << victimQualifier() << "\" - aborting\n"
                     << util::cl::Info()
                     << "consider refactoring \"" << QualifiedName
                     << "\" instead or override with \"--force\"\n";
        
        std::exit(EXIT_FAILURE);
    }
    
    /*
     * Basically, if '_Force' is true the user can refactor methods which
     * override another function. In this case we don't want to refactor
     * methods further down the class hierarchy.
     */
    return isVictimDecl || (!Force_ && overridesVictim(Decl));
}

bool FunctionRefactorer::overridesVictim(const clang::CXXMethodDecl *Decl)
{
    /* 
     * Check if 'Decl' is the victim. If not walk up the (possible) class 
     * hierarchy and check each overridden method if it is the  victim.
     * This is needed to ensure that all cases like in the following 
     * example are detected properly.
     * 
     *      class a { virtual void run() {} };
     *                             ^(1)
     *      class b : public a { virtual void run() override {} };
     *                                        ^(2)
     *      class c : public b { virtual void run() override {} };
     *                                        ^(3)
     *      ...
     *      
     * Command: $ rf --function a::run=process
     */
    
    auto Begin = Decl->begin_overridden_methods();
    auto End = Decl->end_overridden_methods();

    const auto isVictimFunc = [this](const clang::CXXMethodDecl *Decl) {
        return this->isVictim(Decl) || this->overridesVictim(Decl);
    };
    
    return std::any_of(Begin, End, isVictimFunc);
}

bool FunctionRefactorer::overridesVictim(const clang::FunctionDecl *Decl)
{
    auto MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    return (MethodDecl) ? overridesVictim(MethodDecl) : false;
}

