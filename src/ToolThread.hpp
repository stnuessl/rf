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

#ifndef RF_TOOLTHREAD_HPP_
#define RF_TOOLTHREAD_HPP_

#include <thread>

#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

class ToolThread {
public:
    struct Data {
        llvm::ArrayRef<std::string> Files;
        const clang::tooling::CompilationDatabase *CompilationDatabase;
        clang::tooling::FrontendActionFactory *Factory;
    };
    
    ToolThread() = default;

    void run(ToolThread::Data Data);
    void join();

    bool errorOccured() const;
private:
    class DiagnosticConsumer : public clang::TextDiagnosticPrinter {
    public:
        DiagnosticConsumer(llvm::raw_ostream &OS,
                           clang::DiagnosticOptions *DiagOpts,
                           bool OwnsOutputStream = false);
        
        virtual void HandleDiagnostic(clang::DiagnosticsEngine::Level Level,
                                      const clang::Diagnostic &Info) override;
        
        virtual void BeginSourceFile(const clang::LangOptions &LangOpts,
                                     const clang::Preprocessor *PP) override;                              
        virtual void EndSourceFile() override;
    private:
        static std::atomic<std::thread::id> StreamWriter_;
    };
    
    void work(ToolThread::Data Data);

    std::thread Thread_;
    bool Error_;
};

#endif /* RF_TOOLTHREAD_HPP_ */
