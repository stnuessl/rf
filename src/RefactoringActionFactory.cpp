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

#include <RefactoringASTConsumer.hpp>
#include <RefactoringActionFactory.hpp>

#include <util/memory.hpp>

void RefactoringAction::setRefactorers(RefactorerVector *Refactorers)
{
    _Refactorers = Refactorers;
}

bool RefactoringAction::usesPreprocessorOnly() const
{
    return false;
}

std::unique_ptr<clang::ASTConsumer> 
RefactoringAction::CreateASTConsumer(clang::CompilerInstance &CI, 
                                     llvm::StringRef File)
{
    (void) CI;
    (void) File;
    
    auto Consumer = std::make_unique<RefactoringASTConsumer>();
    Consumer->setRefactorers(_Refactorers);
    
    /* This 'std::move()' removes an error when running "--syntax-only" */
    return std::move(Consumer);
}


void RefactoringActionFactory::setRefactorers(RefactorerVector *Refactorers)
{
    _Refactorers = Refactorers;
}


clang::FrontendAction *RefactoringActionFactory::create()
{
    auto Action = new RefactoringAction();
    Action->setRefactorers(_Refactorers);
    
    return Action;
}
