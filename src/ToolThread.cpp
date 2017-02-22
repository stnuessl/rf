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

#include <ToolThread.hpp>

void ToolThread::run(const clang::tooling::CompilationDatabase &CompDB,
                     llvm::ArrayRef<std::string> Files,
                     clang::tooling::FrontendActionFactory &Factory)
{
    auto DBRef = std::cref(CompDB);
    auto FRef = std::ref(Factory);
    
    Thread_ = std::thread(&ToolThread::work, this, DBRef, Files, FRef);
}

void ToolThread::join()
{
    Thread_.join();
}

bool ToolThread::errorOccured() const
{
    return Error_;
}

void ToolThread::work(const clang::tooling::CompilationDatabase &CompDB,
                      llvm::ArrayRef<std::string> Files,
                      clang::tooling::FrontendActionFactory &Factory)
{
    if (Files.empty())
        return;
    
    clang::tooling::ClangTool Tool(CompDB, Files);
    
    Error_ = !!Tool.run(&Factory);
}
