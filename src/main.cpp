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

#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <cstdlib>

#ifdef __unix__
#include <unistd.h>
#endif

#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include <llvm/Support/CommandLine.h>


#include <Refactorers/TagRefactorer.hpp>
#include <Refactorers/FunctionRefactorer.hpp>
#include <util/memory.hpp>


using namespace clang;
using namespace clang::tooling;
using namespace llvm;

// static cl::OptionCategory refactoring_options("rf options");
// static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::list<std::string> TagVec(
    "tag", 
    cl::desc("Refactor a enumeration, structure, or class name."),
    cl::value_desc("victim=repl"),
    cl::CommaSeparated
);

static cl::list<std::string> FunctionVec(
    "function",
    cl::desc("Refactor a function or class method name."),
    cl::value_desc("victim=repl"),
    cl::CommaSeparated
);

static cl::opt<std::string> CompDBPath (
    "comp-db",
    cl::desc("Specify the <path> to the compilation database."),
    cl::value_desc("path")
);

static cl::opt<bool> DryRun(
    "dry-run",
    cl::desc("Do not make any changes at all. Useful for debugging."),
    cl::init(false)
);

static cl::opt<bool> Verbose(
    "verbose",
    cl::desc("Increase verbosity."),
    cl::init(false)
);

static cl::opt<bool> SyntaxOnly(
    "syntax-only",
    cl::desc("Do not make any changes and perform just syntax check."),
    cl::init(false)
);

static std::unique_ptr<CompilationDatabase> 
makeCompilationDatabase(const std::string &Path, std::string &ErrMsg)
{
    if (!Path.empty())
        return JSONCompilationDatabase::loadFromFile(Path, ErrMsg);
    
    const char *WorkDir = std::getenv("PWD");
    if (!WorkDir)
        WorkDir = "./";
    
    return CompilationDatabase::autoDetectFromDirectory(WorkDir, ErrMsg);
}

template<typename T>
void addRefactorers(const std::vector<std::string> &ArgVec, 
                    std::vector<std::unique_ptr<Refactorer>> &RefactorerVec)
{
    for (const auto &Str : ArgVec) {
        auto Pos = Str.find('=');
        if (Pos == std::string::npos) {
            std::cerr << "** ERROR: invalid argument '" << Str << "' - have a "
                      << "look at the help message" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        
        auto VictimName = Str.substr(0, Pos);
        auto ReplName = Str.substr(Pos + sizeof(char));

        auto Refactorer = std::make_unique<T>();
        Refactorer->setVictimName(std::move(VictimName));
        Refactorer->setReplacementName(std::move(ReplName));
        
        RefactorerVec.push_back(std::move(Refactorer));
    }
}

int main(int argc, const char **argv) 
{
#ifdef __unix__
    if (getuid() == 0) {
        std::cerr << "** ERROR: running on root privileges - aborting...\n";
        std::exit(EXIT_FAILURE);
    }
#endif
    
    cl::ParseCommandLineOptions(argc, argv, "");
    
    auto RefactorerVec = std::vector<std::unique_ptr<Refactorer>>();
    
    addRefactorers<TagRefactorer>(TagVec, RefactorerVec);
    addRefactorers<FunctionRefactorer>(FunctionVec, RefactorerVec);
    
    auto ErrMsg = std::string();
    auto CompilationDB = makeCompilationDatabase(CompDBPath, ErrMsg);
    if (!CompilationDB) {
        std::cerr << "** ERROR: " << ErrMsg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    auto SourceFiles = CompilationDB->getAllFiles();
    
    if (SyntaxOnly) {
        auto Action = newFrontendActionFactory<clang::SyntaxOnlyAction>();
        
        ClangTool(*CompilationDB, SourceFiles).run(Action.get());
        
        std::exit(EXIT_SUCCESS);
    }
    
    auto Tool = RefactoringTool(*CompilationDB, SourceFiles);

    for (auto &Refactorer : RefactorerVec) {
        int err;
        
        Refactorer->setReplacementSet(&Tool.getReplacements());
        Refactorer->setVerbose(Verbose);
        
        auto Action = newFrontendActionFactory(Refactorer->matchFinder());
        
        if (DryRun)
            err = Tool.run(Action.get());
        else
            err = Tool.runAndSave(Action.get());
        
        if (err != 0) {
            std::cerr << "** CRITICAL: a refactoring run failed - source files "
                      << "may be corrupted" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    
    return 0;
}
