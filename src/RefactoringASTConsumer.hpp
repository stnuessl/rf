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

#ifndef RF_REFACTORINGASTCONSUMER_HPP_
#define RF_REFACTORINGASTCONSUMER_HPP_

#include <clang/AST/ASTConsumer.h>

#include <Refactorers/Base/Refactorer.hpp>
#include <RefactoringASTVisitor.hpp>

class RefactoringASTConsumer : public clang::ASTConsumer {
public:
    void setRefactorers(std::vector<std::unique_ptr<Refactorer>> *Refactorers);
    
    virtual void HandleTranslationUnit(clang::ASTContext &ASTContext) override;
private:
    RefactoringASTVisitor Visitor_;
};

#endif /* RF_REFACTORINGASTCONSUMER_HPP_ */
