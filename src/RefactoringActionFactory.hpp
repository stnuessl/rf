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

#ifndef RF_REFACTORINGACTIONFACTORY_HPP_
#define RF_REFACTORINGACTIONFACTORY_HPP_

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>

#include "Refactorers/Base/Refactorer.hpp"

class RefactoringAction : public clang::ASTFrontendAction {
public:
    void setRefactorers(std::vector<std::unique_ptr<Refactorer>> *Refactorers);

    bool BeginInvocation(clang::CompilerInstance &CI) override;

    bool BeginSourceFileAction(clang::CompilerInstance &CI) override;
    void EndSourceFileAction() override;

    void ExecuteAction() override;

    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef File) override;

private:
    std::vector<std::unique_ptr<Refactorer>> *Refactorers_;
};

class RefactoringActionFactory : public clang::tooling::FrontendActionFactory {
public:
    RefactoringActionFactory() = default;

    std::vector<std::unique_ptr<Refactorer>> &refactorers();
    const std::vector<std::unique_ptr<Refactorer>> &refactorers() const;

    std::unique_ptr<clang::FrontendAction> create() override;

private:
    std::vector<std::unique_ptr<Refactorer>> Refactorers_;
};

#endif /* RF_REFACTORINGACTIONFACTORY_HPP_ */
