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
#include <utility>
#include <cctype>

#include <Refactorers/VariableRefactorer.hpp>


VariableRefactorer::VariableRefactorer()
    : Refactorer(),
      _VictimLoc(),
      _LineNum(0)
{
    using namespace clang::ast_matchers;
    
    auto VarDeclMatcher = varDecl().bind("VarDecl");
    auto FieldDeclMatcher = fieldDecl().bind("FieldDecl");
    auto DeclRefExprMatcher = declRefExpr().bind("DRefExpr");
    
    _Finder.addMatcher(VarDeclMatcher, this);
    _Finder.addMatcher(FieldDeclMatcher, this);
    _Finder.addMatcher(DeclRefExprMatcher, this);
}

void VariableRefactorer::setVictimName(const std::string &Str)
{
    auto Copy = Str;
    
    setVictimName(std::move(Copy));
}

void VariableRefactorer::setVictimName(std::string &&Str)
{
    /* Check if 'Str' is a fully qualified name */
    auto Pos = Str.rfind("::");
    if (Pos != std::string::npos) {
        auto It = Str.begin() + Pos + sizeof("::") - 1;
        auto End = Str.end();
        
        auto Pred = [](const char c) { return !!std::isdigit(c); };
        
        /* Does the last qualifier specify a line number ? */
        if (std::all_of(It, End, Pred)) {
            Refactorer::setVictimName(Str.substr(0, Pos));
            
            /* 
             * std::stoi() is just dumb, it really should work on 
             * string iterators 
             */
            _LineNum = 0;
            while (It != End)
                _LineNum = _LineNum * 10 + *It++ - '0';

            if (!_LineNum) {
                llvm::errs() << "** ERROR: invalid line number in \""
                             << Str << "\" - aborting...\n";
                std::exit(EXIT_FAILURE);
            }
                
            return;
        }
    }

    Refactorer::setVictimName(std::move(Str));
}

void VariableRefactorer::run(const MatchResult &Result)
{
    runVarDecl(Result);
    runFieldDecl(Result);
    runDeclRefExpr(Result);
}

void VariableRefactorer::runVarDecl(const MatchResult &Result)
{
    auto VarDecl = Result.Nodes.getNodeAs<clang::VarDecl>("VarDecl");
    if (!VarDecl || !isVictim(VarDecl))
        return;
    
    auto Loc = VarDecl->getLocation();
    
    if (!isVictimLine(Loc, *Result.SourceManager))
        return;

    addReplacement(Result, Loc);
}

void VariableRefactorer::runFieldDecl(const Refactorer::MatchResult &Result)
{
    auto FieldDecl = Result.Nodes.getNodeAs<clang::FieldDecl>("FieldDecl");
    if (!FieldDecl || !isVictim(FieldDecl))
        return;
    
    addReplacement(Result, FieldDecl->getLocation());
}


void VariableRefactorer::runDeclRefExpr(const MatchResult &Result)
{
    auto DeclRefExpr = Result.Nodes.getNodeAs<clang::DeclRefExpr>("DRefExpr");
    if (!DeclRefExpr)
        return;
    
    auto Decl = DeclRefExpr->getFoundDecl();
    
    if (!clang::dyn_cast<clang::FieldDecl>(Decl))
        return;
    
    llvm::errs() << "DeclRefExpr: " << qualifiedName(Decl) << "\n";

    if (!isVictim(Decl))
        return;
    
    if (!isVictimLine(Decl->getLocation(), *Result.SourceManager))
        return;
    
    auto Loc = DeclRefExpr->getNameInfo().getLoc();
    addReplacement(Result, Loc);
}

bool VariableRefactorer::isVictimLine(const clang::SourceLocation &Loc, 
                                      const clang::SourceManager &SM)
{
    if (!_LineNum)
        return true;
        
    if (_VictimLoc.isValid())
        return _VictimLoc == Loc;
    
    bool Invalid;
    
    auto Info = SM.getDecomposedLoc(Loc);
    auto Line = SM.getLineNumber(Info.first, Info.second, &Invalid);
    
    if (Invalid) {
        llvm::errs() << "** ERROR: failed to retrieve line number\n";
        std::exit(EXIT_FAILURE);
    }
    
    auto Equal = _LineNum == Line;
    if (Equal)
        _VictimLoc = Loc;
    
    return Equal;
}
