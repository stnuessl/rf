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

void NamespaceRefactorer::visitNamespaceDecl(const clang::NamespaceDecl *Decl)
{
    if (!isVictim(Decl))
        return;
    
    addReplacement(Decl->getLocation());
}

void NamespaceRefactorer::visitUsingDirectiveDecl(
    const clang::UsingDirectiveDecl *Decl)
{
    /*
     * This function handles statements like
     *      using namespace;
     * Statements like
     *      using namespace::namespace::...;
     * are handled by 'runNestedNameSpecifierLoc()'
     */
    if (!isVictim(Decl->getNominatedNamespace()))
        return;
    
    addReplacement(Decl->getIdentLocation());
}

void NamespaceRefactorer::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    /* 
     * This basically searchs all NestedNameSpecifier's for a reference
     * to the namespace which shall be refactored.
     * The loop proceeds like this:
     *      a::b::c::d 
     *          -> a::b::c 
     *              -> a::b 
     *                  -> a
     * and checks each qualifer for a match.
     */
    auto ElaboratedTypeLoc = TypeLoc.getAs<clang::ElaboratedTypeLoc>();
    if (!ElaboratedTypeLoc)
        return;
    
    auto NNSLoc = ElaboratedTypeLoc.getQualifierLoc();
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
