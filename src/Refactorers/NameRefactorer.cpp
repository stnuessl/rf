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

#include <util/CommandLine.hpp>

template <typename Iter>
static bool isNumber(Iter Begin, Iter End)
{
    return std::all_of(Begin, End, [](char c) { return !!std::isdigit(c); });
}

template <typename Iter>
static bool isValidLocation(Iter Begin, Iter End)
{
    auto It = std::find(Begin, End, ':');
    if (It == End)
        return isNumber(Begin, End);
    
    return isNumber(Begin, It) && isNumber(It + 1, End);
}

template <typename Iter>
static bool isValidName(Iter Begin, Iter End)
{
    if (Begin == End || (!std::isalpha(*Begin) && *Begin != '_'))
        return false;
    
    for (auto It = ++Begin; It != End; ++It) {
        if (!std::isalnum(*It) && *It != '_')
            return false;
    }
    
    return true;
}

template <typename Iter>
static bool isValidQualifierEnd(Iter Begin, Iter End)
{
    if (Begin == End)
        return false;
    
    if (End[-1] == '*')
        --End;
    
    if (!!std::isalpha(*Begin) || *Begin == '_')
        return isValidName(Begin, End);
    
    if (!!std::isdigit(*Begin))
        return isValidLocation(Begin, End);
    
    return false;
}

NameRefactorer::NameRefactorer()
    : Refactorer(),
      Victim_(),
      ReplName_(),
      Buffer_(),
      VictimLoc_(),
      ReplSize_(0),
      Line_(0),
      _IsEqualFunc(std::mem_fn(&NameRefactorer::isEqualToVictim))
{
    Buffer_.reserve(1024);
}

void NameRefactorer::setVictimQualifier(const std::string &Victim)
{
    auto Copy = Victim;
    
    setVictimQualifier(std::move(Copy));
}

void NameRefactorer::setVictimQualifier(std::string &&Victim)
{
    auto Begin = Victim.begin();
    auto End = Victim.end();
    
    while (Begin != End) {
        auto It = std::search_n(Begin, End, 2, ':');
        if (It == End) {
            /*
             * That's the last part of the qualifier name.
             * It can be a variable name (e.g. "name"), 
             * a pattern (e.g. "name*"), or * a source location (e.g. 31:5).
             */
            setVictimQualifier(std::move(Victim), Begin, End);
            return;
        }
        
        /*
         * Given a qualified name like:
         *      namespace::namespace::class::variable
         *      ^(1)       ^(2)       ^(3)
         * 
         * Check that the positions (1),(2) and (3) are valid 
         * qualifiers in C / C++
         */
        if (!isValidName(Begin, It)) {
            llvm::errs() << util::cl::Error() 
                         << "invalid qualifier section \"" 
                         << std::string(Begin, It) << "\" in \"" 
                         << Victim << "\"\n";
                         
            std::exit(EXIT_FAILURE);
        }
        
        /* 
         * We have to set / update '_Replsize' here in case the last 
         * section is a * source location specifier
         */
        ReplSize_ = std::distance(Begin, It);
        Begin = It + 2;
    }
    
    llvm::errs() << util::cl::Error() 
                 << "invalid victim qualifier \"" << Victim << "\"\n";

    std::exit(EXIT_FAILURE);
}

const std::string &NameRefactorer::victimQualifier() const
{
    return Victim_;
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

    ReplName_ = std::move(Repl);
    
    auto Pos = ReplName_.rfind("::");
    if (Pos != std::string::npos)
        ReplName_.erase(0, Pos + sizeof("::") - 1);
    
    if (!Force_ && !isValidName(ReplName_.begin(), ReplName_.end())) {
        llvm::errs() << util::cl::Error() 
                     << "invalid replacement \"" << ReplName_ << "\"\n"
                     << util::cl::Info() 
                     << "override with \"--force\"\n";
        std::exit(EXIT_FAILURE);
    }
}

const std::string &NameRefactorer::replacementQualifier() const
{
    return ReplName_;
}

bool NameRefactorer::isVictim(const clang::NamedDecl *NamedDecl)
{
    if (!_IsEqualFunc(*this, qualifiedName(NamedDecl)))
        return false;
    
    return !Line_ || isVictimLocation(NamedDecl->getLocation());
}

bool NameRefactorer::isVictim(const clang::Token &MacroName, 
                              const clang::MacroInfo *MacroInfo)
{
    if (!_IsEqualFunc(*this, MacroName.getIdentifierInfo()->getName()))
        return false;

    return !Line_ || isVictimLocation(MacroInfo->getDefinitionLoc());
}

void NameRefactorer::addReplacement(clang::SourceLocation Loc)
{
    Refactorer::addReplacement(Loc, ReplSize_, ReplName_);
}

bool NameRefactorer::isEqualToVictim(const std::string &Name) const
{
    return Victim_ == Name;
}

bool NameRefactorer::isEqualToVictimPrefix(const std::string &Name) const
{
    if (Name.size() < Victim_.size())
        return false;
    
    return std::equal(Victim_.begin(), Victim_.end(), Name.begin());
}


void NameRefactorer::setVictimQualifier(std::string &&Victim, 
                                        std::string::iterator Begin, 
                                        std::string::iterator End)
{
    if (Begin == End) {
        llvm::errs() << util::cl::Error() 
                     << "no last section in victim qualifier \"" 
                     << Victim << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    auto It = Begin;
    if (!!std::isalpha(*It) || *It == '_') {
        /*
         * Here the last section has to be a name or a pattern, e.g.
         *      namespace::class::member_variable
         *      namespace::class::member*
         */
        auto Last = (End[-1] == '*') ? End - 1 : End;
        
        if (!isValidName(It, Last)) {
            llvm::errs() << util::cl::Error() 
                         << "invalid last section \"" << std::string(Begin, End)
                         << "\" in \"" << Victim << "\"\n";
            std::exit(EXIT_FAILURE);
        }

        ReplSize_ = std::distance(Begin, Last);
        Victim_ = std::move(Victim);
        Line_ = 0;
        Column_ = 0;
        _IsEqualFunc = std::mem_fn(&NameRefactorer::isEqualToVictim);
        
        if (Last < End) {
            /*
             * Pattern detected, transform
             *      "namespace::class::member*" to
             *      "namespace::class::member"
             * and update '_IsEqualFunc'.
             */
            Victim_.pop_back();
            _IsEqualFunc = std::mem_fn(&NameRefactorer::isEqualToVictimPrefix);
        }
        
        return;
    } else if (!!std::isdigit(*It)) {
        /* Here the last section specifies a source location, e.g. "285:9" */
        unsigned int Line = 0;
        unsigned int Column = 0;
        
        while (It != End && std::isdigit(*It))
            Line = Line * 10 + *It++ - '0';
        
        auto ExpectingColumn = It != End;
        if (ExpectingColumn && *It++ != ':') {
            llvm::errs() << util::cl::Error() 
                         << "invalid source location delimiter \"" << It[-1] 
                         << "\" in \"" << Victim << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        
        while (It != End && std::isdigit(*It))
            Column = Column * 10 + *It++ - '0';
        
        if (It != End) {
            llvm::errs() << util::cl::Error()
                         << "invalid remaining char sequence \"" 
                         << std::string(It, End) << "\" in \"" << Victim 
                         << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        
        if (!Line) {
            llvm::errs() << util::cl::Error()
                         << "invalid line number \"0\" in \"" << Victim 
                         << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        
        if (ExpectingColumn && !Column) {
            llvm::errs() << util::cl::Error() 
                         << "invalid column number \"0\" in \"" << Victim 
                         << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        
        Victim.erase(Begin, End);
        
        /* 
         * Given a victim qualifier like:
         *      namespace::class::32:1
         *                        ^(1)
         * 
         * The above 'erase()' removed "32:1" leaving:
         *      namespace::class::
         * 
         * Remove the trailing '::'
         */
        Victim.pop_back();
        Victim.pop_back();
        
        Victim_ = std::move(Victim);
        
        /* 
         * '_ReplSize' must have been set in the public 
         * 'setVictimQualifier()' function
         */
        Line_ = Line;
        Column_ = Column;
        _IsEqualFunc = std::mem_fn(&NameRefactorer::isEqualToVictim);
        
        return;
    }
    
    llvm::errs() << util::cl::Error() 
                 << "invalid last victim qualifier section \"" 
                 << std::string(Begin, End) << "\" in \"" << Victim << "\"\n";
    std::exit(EXIT_FAILURE);
}


bool NameRefactorer::isVictimLocation(const clang::SourceLocation Loc)
{
    if (!Line_)
        return true;
    
    if (VictimLoc_.isValid())
        return VictimLoc_ == Loc;
    
    const auto &SM = CompilerInstance_->getSourceManager();
    auto FullLoc = clang::FullSourceLoc(Loc, SM);
    bool Invalid;
    
    auto Line = FullLoc.getSpellingLineNumber(&Invalid);
    if (Invalid) {
        llvm::errs() << util::cl::Error()
                     << "failed to retrieve line number for declaration \"" 
                     << Victim_ << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    if (Line_ != Line)
        return false;
    
    if (!Column_)
        return true;
    
    auto Column = FullLoc.getSpellingColumnNumber(&Invalid);
    if (Invalid) {
        llvm::errs() << util::cl::Error() 
                     << "failed to retrieve column number for declaration \"" 
                     << Victim_ << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    
    return Column_ == Column;
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
    
    Buffer_.clear();
    auto dest = std::back_inserter(Buffer_);
    auto Name = NamedDecl->getName();
    
    std::reverse_copy(Name.begin(), Name.end(), dest);
    Buffer_ += "::";
    
    auto Context = NamedDecl->getDeclContext();
    while (Context) {
        NamedDecl = clang::dyn_cast<clang::NamedDecl>(Context);
        if (NamedDecl) {
            Name = NamedDecl->getName();
            std::reverse_copy(Name.begin(), Name.end(), dest);
            Buffer_ += "::";
        }
        
        Context = Context->getParent();
    }
    
    while (!Buffer_.empty() && Buffer_.back() == ':')
        Buffer_.pop_back();
    
    std::reverse(Buffer_.begin(), Buffer_.end());
    
    return Buffer_;
}



