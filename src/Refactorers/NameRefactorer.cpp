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
    : Refactorer(),
      _Victim(),
      _ReplName(),
      _Buffer(),
      _VictimLoc(),
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
                                        unsigned int Line,
                                        unsigned int Column)
{
    auto Copy = Victim;
    
    setVictimQualifier(std::move(Copy), Line, Column);
}

void NameRefactorer::setVictimQualifier(std::string &&Victim)
{
    
    unsigned int Line = 0;
    unsigned int Column = 0;
    
    auto Index = Victim.rfind("::");
    if (Index == std::string::npos) {
        setVictimQualifier(std::move(Victim), Line, Column);
        return;
    }
    
    auto Begin = Victim.begin() + Index + sizeof("::") - 1;
    auto It = Begin;
    auto End = Victim.end();
    
    if (It == End) {
        llvm::errs() << "** ERROR: invalid qualifer \"" << Victim << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    if (It != End && (!!std::isalpha(*It) || *It == '_')) {
        setVictimQualifier(std::move(Victim), Line, Column);
        return;
    }
    
    while (It != End && !!std::isdigit(*It))
        Line = Line * 10 + *It++ - '0';
    
    bool NeedsCol = It != End;
    if (NeedsCol && *It++ != ':') {
        llvm::errs() << "** ERROR: invalid location qualifer \""
                     << std::string(Begin, End) << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    while (It != End && !!std::isdigit(*It))
        Column = Column * 10 + *It++ - '0';
    
    if (It != End || !Line || (NeedsCol && !Column)) {
        llvm::errs() << "** ERROR: invalid location qualifer \""
                     << std::string(Begin, End) << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    Victim.erase(Index);
    
    setVictimQualifier(std::move(Victim), Line, Column);
}

void NameRefactorer::setVictimQualifier(std::string &&Victim,
                                        unsigned int Line,
                                        unsigned int Column)
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
    _Column = Column;
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
    
    return !_Line || isVictimLocation(NamedDecl->getLocation());
}

bool NameRefactorer::isVictim(const clang::Token &MacroName, 
                              const clang::MacroInfo *MacroInfo)
{
    if (_Victim != MacroName.getIdentifierInfo()->getName())
        return false;
    
    return !_Line || isVictimLocation(MacroInfo->getDefinitionLoc());
}

void NameRefactorer::addReplacement(clang::SourceLocation Loc)
{
    Refactorer::addReplacement(Loc, _ReplSize, _ReplName);
}

bool NameRefactorer::isVictimLocation(const clang::SourceLocation Loc)
{
    if (!_Line)
        return true;
    
    if (_VictimLoc.isValid())
        return _VictimLoc == Loc;
    
    auto &SM = _CompilerInstance->getSourceManager();
    bool Invalid;
    
    auto Line = SM.getSpellingLineNumber(Loc, &Invalid);
    if (Invalid) {
        llvm::errs() << "** ERROR: failed to retrieve line number for "
                     << "declaration \"" << _Victim << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    if (_Line != Line)
        return false;
    
    if (!_Column)
        return true;
    
    auto Column = SM.getSpellingColumnNumber(Loc, &Invalid);
    if (Invalid) {
        llvm::errs() << "** ERROR: failed to retrieve column number for "
                     << "declaration \"" << _Victim << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    return _Column == Column;
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



