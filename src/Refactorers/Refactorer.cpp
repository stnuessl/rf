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

#include <Refactorers/Refactorer.hpp>

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

Refactorer::Refactorer()
    : _Finder(),
      _Repls(nullptr),
      _Victim(),
      _ReplName(),
      _ReplSize(),
      _VictimDecl(nullptr),
      _Buffer(),
      _Verbose(false)
{
    _Buffer.reserve(1024);
}

void Refactorer::setVictimName(const std::string &Str)
{
    auto Copy = Str;
    
    setVictimName(std::move(Copy));
}

void Refactorer::setVictimName(std::string &&Str)
{
    /*
     * When refactoring a name it must be specified like
     * "namespace::name". However, not the full qualified name needs
     * a replacement but the last part of the qualifier (here: "name").
     * Thus we need to know the size of that part.
     * '_ReplSize' can be thought of as something like
     *      sizeof("namespace::name") - sizeof("namespace::")
     */
    
    _Victim = std::move(Str);
    _ReplSize = _Victim.size();
    
    auto Pos = _Victim.rfind("::");
    if (Pos != std::string::npos)
        _ReplSize -= Pos + sizeof("::") - 1;
}

const std::string &Refactorer::victimName() const
{
    return _Victim;
}

void Refactorer::setReplacementName(const std::string &Str)
{
    auto Copy = Str;
    
    setReplacementName(std::move(Copy));
}

void Refactorer::setReplacementName(std::string &&Str)
{
    /*
     * Convert "::namespace::class" to just "class" as the namespace is implicit
     * specified by the '_Victim' string.
     */
    
    _ReplName = std::move(Str);
    
    auto Pos = _ReplName.rfind("::");
    if (Pos != std::string::npos)
        _ReplName.erase(0, Pos + sizeof("::") - 1);
    
    exitIfInvalidName(_ReplName);
}

const std::string &Refactorer::replacementName() const
{
    return _ReplName;
}

void Refactorer::setReplacementSet(clang::tooling::Replacements *Repls)
{
    _Repls = Repls;
}

const clang::tooling::Replacements *Refactorer::replacementSet() const
{
    return _Repls;
}

void Refactorer::setVerbose(bool Verbose)
{
    _Verbose = Verbose;
}

bool Refactorer::verbose() const
{
    return _Verbose;
}

clang::ast_matchers::MatchFinder *Refactorer::matchFinder()
{
    return &_Finder;
}

void Refactorer::addReplacement(const MatchResult &Result, 
                                const clang::SourceLocation &Loc)
{
    addReplacement(*Result.SourceManager, Loc);
}

void Refactorer::addReplacement(const clang::SourceManager &SM, 
                                const clang::SourceLocation &Loc)
{
    using namespace clang::tooling;
    
    if (SM.isInSystemHeader(Loc) || !SM.isLocalSourceLocation(Loc))
        return;
    
    auto Repl = Replacement(SM, Loc, _ReplSize, _ReplName);
    
    auto Ok = _Repls->insert(std::move(Repl)).second;
    if (_Verbose && Ok) {
        Loc.dump(SM);
        llvm::errs() << " --> \"" << _ReplName << "\"\n";
    }
}

bool Refactorer::isVictim(const clang::NamedDecl *NamedDecl)
{
    auto Decl = NamedDecl->getCanonicalDecl();
    
    if (_VictimDecl == Decl)
        return true;
    
    bool Match = _Victim == qualifiedName(NamedDecl);
    
    if (Match && !_VictimDecl)
        _VictimDecl = Decl;
        
    return Match;
}

std::string &Refactorer::qualifiedName(const clang::NamedDecl *NamedDecl)
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
