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

#ifndef _REFACTORINGACTIONFACTORY_HPP_
#define _REFACTORINGACTIONFACTORY_HPP_

#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>

#include <Refactorers/Refactorer.hpp>

class RefactoringAction : public clang::ASTFrontendAction {
public:
    void setRefactorers(Refactorers *Refactorers);
    
    virtual bool BeginSourceFileAction(clang::CompilerInstance &CI,
                                       llvm::StringRef File) override;
    virtual void EndSourceFileAction() override;
                                 
    virtual std::unique_ptr<clang::ASTConsumer> 
    CreateASTConsumer(clang::CompilerInstance &CI, 
                      llvm::StringRef File) override;
private:
    Refactorers *_Refactorers;
};

class RefactoringActionFactory : public clang::tooling::FrontendActionFactory {
public:
    void setRefactorers(Refactorers *Refactorers);
    
    virtual clang::FrontendAction *create() override;
private:
    Refactorers *_Refactorers;
};

#endif /* RF_REFACTORINGACTIONFACTORY_HPP_ */
