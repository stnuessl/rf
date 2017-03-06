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

void ToolThread::run(ToolThread::Data &Data)
{
    Thread_ = std::thread(&ToolThread::work, this, Data);
}

void ToolThread::join()
{
    Thread_.join();
}

bool ToolThread::errorOccured() const
{
    return Error_;
}

void ToolThread::work(ToolThread::Data Data)
{
    if (Data.Files.empty())
        return;
    
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOptions;
    DiagOptions = new clang::DiagnosticOptions();
    DiagOptions->Remarks.clear();
    DiagOptions->Warnings.clear();
    DiagOptions->ShowColors = true;

    DiagnosticConsumer DiagConsumer(llvm::errs(), &*DiagOptions);
    
    clang::tooling::ClangTool Tool(*Data.CompilationDatabase, Data.Files);
    Tool.setDiagnosticConsumer(&DiagConsumer);

    Error_ = !!Tool.run(Data.Factory);
}

std::atomic<std::thread::id> ToolThread::DiagnosticConsumer::StreamWriter_;

ToolThread::DiagnosticConsumer::DiagnosticConsumer(
    llvm::raw_ostream &OS, 
    clang::DiagnosticOptions *DiagOpts, 
    bool OwnsOutputStream)
    : clang::TextDiagnosticPrinter(OS, DiagOpts, OwnsOutputStream)
{
}

void ToolThread::DiagnosticConsumer::HandleDiagnostic(
    clang::DiagnosticsEngine::Level Level, 
    const clang::Diagnostic &Info)
{
    if (Level != clang::DiagnosticsEngine::Error)
        return;
    
    auto DefaultId = std::thread::id();
    auto Id = std::this_thread::get_id();
    
    /* 
     * The first thread to set 'Owner' aquires the rights to write to 
     * the output stream. Otherwise all threads would report errors and
     * the diagnostics would get scrambled. As this tool is not primarily
     * a syntax checker one thread reporting errors should be enough.
     */
    if (!StreamWriter_.compare_exchange_strong(DefaultId, Id))
        return;
    
    clang::TextDiagnosticPrinter::HandleDiagnostic(Level, Info);
}

void ToolThread::DiagnosticConsumer::BeginSourceFile(
    const clang::LangOptions &LangOpts, 
    const clang::Preprocessor *PP)
{
    clang::TextDiagnosticPrinter::BeginSourceFile(LangOpts, PP);
    
    /* 
     * Reset these values so they will not propagate between translation
     * units.
     */
    clang::TextDiagnosticPrinter::NumWarnings = 0;
    clang::TextDiagnosticPrinter::NumErrors = 0;
}


void ToolThread::DiagnosticConsumer::EndSourceFile()
{
    clang::TextDiagnosticPrinter::EndSourceFile();
}
