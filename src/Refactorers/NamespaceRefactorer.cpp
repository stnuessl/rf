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

NamespaceRefactorer::NamespaceRefactorer()
    : Refactorer()
{
    using namespace clang::ast_matchers;
    
    auto NamespaceDeclMatcher = namespaceDecl().bind("NSDecl");
    auto NNSLocMatcher = nestedNameSpecifierLoc().bind("NNSLoc");
    auto UsingDDeclMatcher = usingDirectiveDecl().bind("UDDecl");
    
    _Finder.addMatcher(NamespaceDeclMatcher, this);
    _Finder.addMatcher(NNSLocMatcher, this);
    _Finder.addMatcher(UsingDDeclMatcher, this);
}

void NamespaceRefactorer::run(const MatchResult &Result)
{
    runNamespaceDecl(Result);
    runNestedNameSpecifierLoc(Result);
    runUsingDecl(Result);
}

void NamespaceRefactorer::runNamespaceDecl(const MatchResult &Result)
{
    auto NamespaceDecl = Result.Nodes.getNodeAs<clang::NamespaceDecl>("NSDecl");
    if (!NamespaceDecl || !isVictim(NamespaceDecl))
        return;
    
    addReplacement(Result, NamespaceDecl->getLocation());
}

void NamespaceRefactorer::runNestedNameSpecifierLoc(const MatchResult &Result)
{
    auto NLoc = Result.Nodes.getNodeAs<clang::NestedNameSpecifierLoc>("NNSLoc");
    if (!NLoc)
        return;
    
    auto NNSLoc = *NLoc;
    
    while (NNSLoc) {
        auto NestedNameSpecifer = NNSLoc.getNestedNameSpecifier();
        if (!NestedNameSpecifer)
            continue;
        
        auto NamespaceDecl = NestedNameSpecifer->getAsNamespace();
        if (NamespaceDecl && isVictim(NamespaceDecl)) {
            addReplacement(Result, NNSLoc.getLocalBeginLoc());
            break;
        }
        
        NNSLoc = NNSLoc.getPrefix();
    }
}

void NamespaceRefactorer::runUsingDecl(const MatchResult &Result)
{
    /*
     * This function handles statements like
     *      using namespace;
     * Statements like
     *      using namespace::namespace::...;
     * are handled by 'runNestedNameSpecifierLoc()'
     */
    auto UDDecl = Result.Nodes.getNodeAs<clang::UsingDirectiveDecl>("UDDecl");
    if (!UDDecl || !isVictim(UDDecl->getNominatedNamespaceAsWritten()))
        return;

    addReplacement(Result, UDDecl->getIdentLocation());
}
