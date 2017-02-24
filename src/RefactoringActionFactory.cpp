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

#include <algorithm>
#include <iterator>
#include <utility>

#include <clang/Lex/Preprocessor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PTHManager.h>

#include <PPCallbackDispatcher.hpp>
#include <RefactoringASTConsumer.hpp>
#include <RefactoringActionFactory.hpp>

#include <util/commandline.hpp>
#include <util/filesystem.hpp>
#include <util/memory.hpp>

void RefactoringAction::setRefactorers(
    std::vector<std::unique_ptr<Refactorer>> *Refactorers)
{
    Refactorers_ = Refactorers;
}

void RefactoringAction::setProjectPath(llvm::StringRef Path)
{
    ProjectPath_ = Path.str();
}

bool RefactoringAction::BeginInvocation(clang::CompilerInstance &CI)
{
    uint64_t FileSize;
    
    bool Ok = clang::ASTFrontendAction::BeginInvocation(CI);
    if (!Ok)
        return false;
    
    if (ProjectPath_.empty())
        return true;
    
    auto File = getMappedPTHFile(getCurrentFile());
    
    auto ErrCode = llvm::sys::fs::file_size(File, FileSize);
    
    /*
     * Empty source files will create a minimal PTH file which will 
     * throw an error while parsing. This check is supposed to filter
     * out such files.
     */
    if (!ErrCode && FileSize > 1024)
        CI.getPreprocessorOpts().TokenCache = std::move(File);

    return true;
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
    
    if (ProjectPath_.empty())
        return;
    
    auto File = getMappedPTHFile(getCurrentFile());
    
    bool Exists = llvm::sys::fs::exists(File);
    if (!Exists && !util::fs::createDirectoriesForFile(File)) {
        auto &CI = getCompilerInstance();
        
        auto OutFile = CI.createDefaultOutputFile(true, File, "pth");
        if (!OutFile)
            return;
        
        clang::CacheTokens(CI.getPreprocessor(), OutFile.get());
    }
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

std::string RefactoringAction::getMappedPTHFile(llvm::StringRef File) const
{
    /*
     * Basically, given the project directory "/home/user/project/",
     * this is the mapping we want to achieve:
     * 
     *      "/home/user/project/src/main.cpp"
     *          --> "/home/user/project/.rf/src/main.cpp"
     * 
     *      "/home/user/project/src/lib/file.cpp"
     *          --> "/home/user/project/.rf/src/lib/file.cpp"
     */
    
    auto MappedPTHFile = File.str();
    
    auto Error = util::fs::realpath(MappedPTHFile);
    if (Error) {
        llvm::errs() << util::cl::Error()
                     << "failed to retrieve the realpath for \""
                     << File << "\" - " << Error.message() << "\n";
        std::exit(EXIT_FAILURE);
    }
    
    auto MinSize = std::min(ProjectPath_.size(), MappedPTHFile.size());
    
    auto Begin = MappedPTHFile.begin();
    auto End   = std::next(Begin, MinSize);
    auto It = std::mismatch(Begin, End, ProjectPath_.begin()).first;
    auto Index = std::distance(Begin, It);
    
    MappedPTHFile.insert(Index, "/.rf/pths");
    
    /* 
     * Cut off the file extension, e.g.:
     *      src/main.cpp    -->     src/main
     */
    auto Extension = llvm::sys::path::extension(MappedPTHFile);
    
    auto NewSize = MappedPTHFile.size() - Extension.size();
    MappedPTHFile.resize(NewSize);

    /* New file extension for the pre tokenized header */
    MappedPTHFile += ".pth";

    return MappedPTHFile;
}

std::string &RefactoringActionFactory::projectPath()
{
    return ProjectPath_;
}

const std::string &RefactoringActionFactory::projectPath() const
{
    return ProjectPath_;
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
    Action->setProjectPath(ProjectPath_);
    Action->setRefactorers(&Refactorers_);
    
    return Action;
}
