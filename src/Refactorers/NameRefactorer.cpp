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

#include <algorithm>

#include <Refactorers/NameRefactorer.hpp>

static void rCopy(std::string &Str, const clang::StringRef &Ref)
{
    auto n = Ref.size();
    
    while (n--)
        Str += Ref[n];
}

static void exitIfInvalidName(const std::string &Str)
{
    auto it = Str.begin();
    auto end = Str.end();
    
    if (it == end)
        goto fail;
    
    if (!std::isalpha(*it) && *it != '_')
        goto fail;
    
    while (++it != end) {
        if (!std::isalnum(*it) && *it != '_')
            goto fail;
    }
    
    return;
    
fail:
    llvm::errs() << "** ERROR: invalid name \"" << Str << "\" - aborting...\n"; 
    std::exit(EXIT_FAILURE);
}

NameRefactorer::NameRefactorer()
    : RefactorerNew(),
      _Victim(),
      _ReplName(),
      _Buffer(),
      _LineLoc(),
      _ReplSize(0),
      _Line(0)
      
{
    _Buffer.reserve(1024);
}

void NameRefactorer::setVictimQualifier(const std::string &Victim)
{
    auto Copy = Victim;
    
    setVictimQualifier(std::move(Copy));
}

void NameRefactorer::setVictimQualifier(const std::string &Victim, 
                                        unsigned int Line)
{
    auto Copy = Victim;
    
    setVictimQualifier(std::move(Copy), Line);
}

void NameRefactorer::setVictimQualifier(std::string &&Victim)
{
    unsigned int Line = 0;
    
    /* Check if 'Str' is a fully qualified name */
    auto Pos = Victim.rfind("::");
    if (Pos != std::string::npos) {
        auto It = Victim.begin() + Pos + sizeof("::") - 1;
        auto End = Victim.end();
        
        auto Pred = [](const char c) { return !!std::isdigit(c); };
        
        /* Does the last qualifier specify a line number ? */
        if (std::all_of(It, End, Pred)) {
            /* 
             * std::stoi() is just dumb, it really should work on 
             * string iterators 
             */
            while (It != End)
                Line = Line * 10 + *It++ - '0';
            
            if (!Line) {
                llvm::errs() << "** ERROR: invalid line number in \""
                             << Victim << "\" - aborting...\n";
                std::exit(EXIT_FAILURE);
            }
            
            /* safe without check: see rfind above */
            Victim.erase(Pos + 1);
        }
    }
    
    setVictimQualifier(std::move(Victim), Line);
}

void NameRefactorer::setVictimQualifier(std::string &&Victim,
                                        unsigned int Line)
{
    /*
     * When refactoring a name it must be specified like
     * "namespace::name". However, not the full qualified name needs
     * a replacement but the last part of the qualifier (here: "name").
     * Thus we need to know the size of that part.
     * '_ReplSize' can be thought of as something like
     *      sizeof("namespace::name") - sizeof("namespace::")
     */
    
    _Victim = std::move(Victim);
    _ReplSize = _Victim.size();
    
    auto Pos = _Victim.rfind("::");
    if (Pos != std::string::npos)
        _ReplSize -= Pos + sizeof("::") - 1;
    
    _Line = Line;
}

const std::string &NameRefactorer::victimQualifier() const
{
    return _Victim;
}

void NameRefactorer::setReplacementQualifier(const std::string &Repl)
{
    auto Copy = Repl;
    
    setReplacementQualifier(std::move(Copy));
}

void NameRefactorer::setReplacementQualifier(std::string &&Repl)
{
    /*
     * Convert "::namespace::class" to just "class" as the namespace is implicit
     * specified by the '_Victim' string.
     */
    
    _ReplName = std::move(Repl);
    
    auto Pos = _ReplName.rfind("::");
    if (Pos != std::string::npos)
        _ReplName.erase(0, Pos + sizeof("::") - 1);
    
    if (!_Force)
        exitIfInvalidName(_ReplName);
}

const std::string NameRefactorer::replacementQualifier() const
{
    return _ReplName;
}

bool NameRefactorer::isVictim(const clang::NamedDecl *NamedDecl)
{
    if (_Victim != qualifiedName(NamedDecl))
        return false;
    
    if (!_Line)
        return true;
    
    auto Loc = NamedDecl->getLocation();
    
    if (_LineLoc.isValid())
        return _LineLoc == Loc;
    
    auto &SM = _ASTContext->getSourceManager();
    bool Invalid;
    
    auto Line = SM.getSpellingLineNumber(Loc, &Invalid);
    if (Invalid) {
        llvm::errs() << "** ERROR: failed to retrieve line number for "
                     << "declaration \"" << _Victim << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    auto Equal = _Line == Line;
    if (Equal)
        _LineLoc = Loc;
    
    return Equal;
}

void NameRefactorer::addReplacement(const clang::SourceLocation &Loc)
{
    RefactorerNew::addReplacement(Loc, _ReplSize, _ReplName);
}

const std::string &
NameRefactorer::qualifiedName(const clang::NamedDecl *NamedDecl)
{
    /*
     * The DeclContext order for e.g. "namespace::class"
     * is class -> namespace, meaning we first visit the context for 'class' 
     * and then the context for 'namespace' by using 'getParent()'
     * To avoid using some sort of container we write everything reversed into
     * '_Buffer' giving "ssalc::ecapseman"
     * Before returning we just have to reverse '_Buffer' giving the correct
     * qualified name "namespace::class"
     */
    
    _Buffer.clear();
    
    rCopy(_Buffer, NamedDecl->getName());
    _Buffer += "::";
    
    auto Context = NamedDecl->getDeclContext();
    
    while (Context) {
        NamedDecl = clang::dyn_cast<clang::NamedDecl>(Context);
        if (NamedDecl) {
            rCopy(_Buffer, NamedDecl->getName());
            _Buffer += "::";
        }
        
        Context = Context->getParent();
    }
    
    while (!_Buffer.empty() && _Buffer.back() == ':')
        _Buffer.pop_back();
    
    std::reverse(_Buffer.begin(), _Buffer.end());
    
    return _Buffer;
}



