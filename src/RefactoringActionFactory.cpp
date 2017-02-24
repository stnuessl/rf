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

#include <clang/Lex/Preprocessor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PTHManager.h>

#include <PPCallbackDispatcher.hpp>
#include <RefactoringASTConsumer.hpp>
#include <RefactoringActionFactory.hpp>

#include <util/memory.hpp>

void RefactoringAction::setRefactorers(
    std::vector<std::unique_ptr<Refactorer>> *Refactorers)
{
    Refactorers_ = Refactorers;
}

bool RefactoringAction::BeginInvocation(clang::CompilerInstance &CI)
{
    return clang::ASTFrontendAction::BeginInvocation(CI);
}

bool RefactoringAction::BeginSourceFileAction(clang::CompilerInstance &CI, 
                                              llvm::StringRef File)
{
    auto Dispatcher = std::make_unique<PPCallbackDispatcher>();
    Dispatcher->setRefactorers(Refactorers_);
    
    CI.getPreprocessor().addPPCallbacks(std::move(Dispatcher));

    for (auto &Refactorer : *Refactorers_) {
        Refactorer->setCompilerInstance(&CI);
        Refactorer->beginSourceFileAction(File);
    }
    
    return true;
}

void RefactoringAction::EndSourceFileAction()
{
    for (auto &Refactorer : *Refactorers_)
        Refactorer->endSourceFileAction();
}

void RefactoringAction::ExecuteAction()
{
    clang::ASTFrontendAction::ExecuteAction();
}

std::unique_ptr<clang::ASTConsumer> 
RefactoringAction::CreateASTConsumer(clang::CompilerInstance &CI, 
                                     llvm::StringRef File)
{
    (void) CI;
    (void) File;
    
    auto Consumer = std::make_unique<RefactoringASTConsumer>();
    Consumer->setRefactorers(Refactorers_);

    return Consumer;
}

std::vector<std::unique_ptr<Refactorer>> &
RefactoringActionFactory::refactorers()
{
    return Refactorers_;
}

const std::vector<std::unique_ptr<Refactorer>> &
RefactoringActionFactory::refactorers() const
{
    return Refactorers_;
}

clang::FrontendAction *RefactoringActionFactory::create()
{
    if (Refactorers_.empty())
        return new clang::SyntaxOnlyAction();
        
    auto Action = new RefactoringAction();
    Action->setRefactorers(&Refactorers_);
    
    return Action;
}
