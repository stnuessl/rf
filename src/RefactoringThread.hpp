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

#ifndef RF_REFACTORINGTHREAD_HPP_
#define RF_REFACTORINGTHREAD_HPP_

#include <thread>

#include <clang/Tooling/CompilationDatabase.h>

#include <llvm/ADT/ArrayRef.h>

#include <Refactorers/Base/Refactorer.hpp>

class RefactoringThread {
public:
    RefactoringThread() = default;

    RefactorerVector &refactorers();
    const RefactorerVector &refactorers() const;

    void run(const clang::tooling::CompilationDatabase &CompDB, 
             llvm::ArrayRef<std::string> Files);
    
    void join();
    
private:
    void task(const clang::tooling::CompilationDatabase *CompDB, 
              llvm::ArrayRef<std::string> Files);
    
    std::thread Thread_;
    RefactorerVector Refactorers_;
};

#endif /* RF_REFACTORINGTHREAD_HPP_ */
