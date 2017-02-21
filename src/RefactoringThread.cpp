/*
 * Copyright (C) 2017  Steffen Nüssle
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

#include <cstdlib>
#include <algorithm>

#include <util/memory.hpp>
#include <util/CommandLine.hpp>

#include <RefactoringActionFactory.hpp>
#include <RefactoringThread.hpp>


RefactorerVector &RefactoringThread::refactorers()
{
    return Refactorers_;
}

const RefactorerVector &RefactoringThread::refactorers() const
{
    return Refactorers_;
}

void RefactoringThread::run(const clang::tooling::CompilationDatabase &CompDB,
                            llvm::ArrayRef<std::string> Files)
{
    Thread_ = std::thread(&RefactoringThread::task, this, &CompDB, Files);
}

void RefactoringThread::join()
{
    Thread_.join();
}

void RefactoringThread::task(const clang::tooling::CompilationDatabase *CompDB, 
                             llvm::ArrayRef<std::string> Files)
{
    if (Refactorers_.empty() || Files.empty())
        return;
    
    clang::tooling::ClangTool Tool(*CompDB, Files);
    
    auto Factory = std::make_unique<RefactoringActionFactory>();
    Factory->setRefactorers(&Refactorers_);
    
    int err = Tool.run(Factory.get());
    if (err) {
        llvm::errs() << util::cl::Error()
                     << "found syntax error(s) while refactoring\n";
        
        std::exit(EXIT_FAILURE);
    }
}


