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

#include <Refactorers/NamespaceRefactorer.hpp>

void NamespaceRefactorer::visitNamespaceAliasDecl(
    const clang::NamespaceAliasDecl *Decl)
{
    //         auto &SM = _CompilerInstance->getSourceManager();
    //
    //         if (SM.isInSystemHeader(Decl->getLocation()))
    //             return;
    //
    //         Decl->dump();
    //         Decl->getNamespace()->dump();
    //         Decl->getAliasedNamespace()->dump();
    /*
     * This part handles cases like:
     *      namespace n { ... }
     *
     *      void f() { using m = n; ... }
     *                       ^(1)
     */
    if (isVictim(Decl)) {
        addReplacement(Decl->getLocation());
        return;
    }

    /*
     * This part handles cases like:
     *      namespace n { namespace m { int x; }}
     *
     *      void f() { using nm = n::m; nm::x = 0; }
     *                               ^(1)
     */
    if (isVictim(Decl->getNamespace())) {
        addReplacement(Decl->getTargetNameLoc());
        return;
    }

    /*
     * This part handles cases like:
     *      namespace n { namespace m { int x; }}
     *
     *      void f() { using nm = n::m; nm::x = 0; }
     *                            ^(1)
     */
    auto NNSLoc = Decl->getQualifierLoc();
    if (!NNSLoc)
        return;

    traverse(NNSLoc);
}

void NamespaceRefactorer::visitNamespaceDecl(const clang::NamespaceDecl *Decl)
{
    if (!isVictim(Decl))
        return;

    addReplacement(Decl->getLocation());
}

void NamespaceRefactorer::visitUsingDecl(const clang::UsingDecl *Decl)
{
    /*
     * This function handles cases like:
     *      namespace n { int x; }
     *
     *      void f() { using n::x; x = 0; }
     */
    auto NSSLoc = Decl->getQualifierLoc();
    if (!NSSLoc)
        return;

    traverse(NSSLoc);
}

void NamespaceRefactorer::visitUsingDirectiveDecl(
    const clang::UsingDirectiveDecl *Decl)
{
    /*
     * This function handles statements like:
     *      namespace n {}
     *      using namespace n;
     *                      ^(1)
     * The 'getNominatedNamespaceAsWritten() ensures that
     * constructs like.
     *      namespace o = n;
     *      using namespace o;
     * will work.
     */

    if (!isVictim(Decl->getNominatedNamespaceAsWritten()))
        return;

    addReplacement(Decl->getLocation());
}

void NamespaceRefactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    /* This function handles the following case:
     *      namespace n { int x; }
     *
     *      void f() { n::x = 0; }
     *                 ^(1)
     */
    auto NNSLoc = Expr->getQualifierLoc();
    if (!NNSLoc)
        return;

    traverse(NNSLoc);
}

void NamespaceRefactorer::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    /*
     * This function handles the following case:
     *      namespace n { class c {}; }
     *
     *      void f() { auto var = n::c(); }
     *                            ^(1)
     */

    auto ElaboratedTypeLoc = TypeLoc.getAs<clang::ElaboratedTypeLoc>();
    if (!ElaboratedTypeLoc)
        return;

    auto NNSLoc = ElaboratedTypeLoc.getQualifierLoc();
    if (!NNSLoc)
        return;

    traverse(NNSLoc);
}

void NamespaceRefactorer::traverse(clang::NestedNameSpecifierLoc NNSLoc)
{
    /*
     * This basically searchs all NestedNameSpecifier's for a reference
     * to the namespace which shall be refactored.
     * The loop proceeds like this:
     *      a::b::c::d
     *          -> a::b::c
     *              -> a::b
     *                  -> a
     * and checks each qualifier for a match.
     */

    while (NNSLoc) {
        auto NNSpecifier = NNSLoc.getNestedNameSpecifier();

        auto NamespaceDecl = NNSpecifier->getAsNamespace();
        if (NamespaceDecl && isVictim(NamespaceDecl)) {
            addReplacement(NNSLoc.getLocalBeginLoc());
            break;
        }

        NNSLoc = NNSLoc.getPrefix();
    }
}
