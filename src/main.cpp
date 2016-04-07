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

#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include <llvm/Support/CommandLine.h>


#include <Refactorers/TagRefactorer.hpp>
#include <Refactorers/FunctionRefactorer.hpp>
#include <Refactorers/NamespaceRefactorer.hpp>
#include <Refactorers/VariableRefactorer.hpp>
#include <util/memory.hpp>


// static cl::OptionCategory refactoring_options("rf options");
// static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static llvm::cl::list<std::string> TagVec(
    "tag", 
    llvm::cl::desc("Refactor an enumeration, structure, or class."),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated
);

static llvm::cl::list<std::string> FunctionVec(
    "function",
    llvm::cl::desc("Refactor a function or class method."),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated
);

static llvm::cl::list<std::string> NamespaceVec(
    "namespace",
    llvm::cl::desc("Refactor a namespace."),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated
);

static llvm::cl::list<std::string> VarVec(
    "variable",
    llvm::cl::desc("Change the name of a variable."),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated
);

static llvm::cl::opt<std::string> CompDBPath(
    "comp-db",
    llvm::cl::desc("Specify the <path> to the compilation database."),
    llvm::cl::value_desc("path")
);

static llvm::cl::opt<bool> DryRun(
    "dry-run",
    llvm::cl::desc(
        "Do not make any changes at all.\n"
        "Useful for debugging, especially when used with \"--verbose\"."
    ),
    llvm::cl::init(false)
);

static llvm::cl::opt<bool> Verbose(
    "verbose",
    llvm::cl::desc(
        "Increase verbosity: Prints a line for each replacment made."
    ),
    llvm::cl::init(false)
);

static llvm::cl::opt<bool> Force(
    "force",
    llvm::cl::desc(
        "Disable safety checks and apply replacements even if they may\n"
        "break the code. No replacements are done if \"--dry-run\"\n"
        "is passed along this option."
    ),
    llvm::cl::init(false)
);

static llvm::cl::opt<bool> SyntaxOnly(
    "syntax-only",
    llvm::cl::desc(
        "Perform a syntax check and exit.\n"
        "No changes are made even if replacements were specified."
    ),
    llvm::cl::init(false)
);

#ifdef __unix__
static llvm::cl::opt<bool> AllowRoot(
    "allow-root",
    llvm::cl::desc(
        "Allow this application to run with root privileges.\n"
    ),
    llvm::cl::init(false)
);
#endif

static std::unique_ptr<clang::tooling::CompilationDatabase> 
makeCompilationDatabase(const std::string &Path, std::string &ErrMsg)
{
    using namespace clang::tooling;
    
    if (!Path.empty())
        return JSONCompilationDatabase::loadFromFile(Path, ErrMsg);
    
    const char *WorkDir = std::getenv("PWD");
    if (!WorkDir)
        WorkDir = "./";
    
    return CompilationDatabase::autoDetectFromDirectory(WorkDir, ErrMsg);
}

template<typename T> static 
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
        Refactorer->setVictimQualifier(std::move(VictimName));
        Refactorer->setReplacementQualifier(std::move(ReplName));
        
        RefactorerVec.push_back(std::move(Refactorer));
    }
}

int main(int argc, const char **argv) 
{
    using namespace clang;
    using namespace clang::tooling;
    
    llvm::cl::ParseCommandLineOptions(argc, argv, "");
    
#ifdef __unix__
    if (!AllowRoot && getuid() == 0) {
        std::cerr << "** ERROR: running on root privileges - aborting...\n";
        std::exit(EXIT_FAILURE);
    }
#endif
    
    auto RefactorerVec = std::vector<std::unique_ptr<Refactorer>>();
    
    addRefactorers<TagRefactorer>(TagVec, RefactorerVec);
    addRefactorers<FunctionRefactorer>(FunctionVec, RefactorerVec);
    addRefactorers<NamespaceRefactorer>(NamespaceVec, RefactorerVec);
    addRefactorers<VariableRefactorer>(VarVec, RefactorerVec);
    
    auto ErrMsg = std::string();
    auto CompilationDB = makeCompilationDatabase(CompDBPath, ErrMsg);
    if (!CompilationDB) {
        std::cerr << "** ERROR: " << ErrMsg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    auto SourceFiles = CompilationDB->getAllFiles();
    
    if (SyntaxOnly) {
        auto Action = newFrontendActionFactory<clang::SyntaxOnlyAction>();
        
        int err = ClangTool(*CompilationDB, SourceFiles).run(Action.get());
        
        std::exit((err == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    
    auto Tool = RefactoringTool(*CompilationDB, SourceFiles);
    
    for (auto &Refactorer : RefactorerVec) {
        Refactorer->setReplacementSet(&Tool.getReplacements());
        Refactorer->setVerbose(Verbose);
        Refactorer->setForce(Force);

        auto Action = newFrontendActionFactory(Refactorer->matchFinder());
        
        /* 
         * Do _NOT_ use runAndSave() here: We want to support multiple
         * (non-conflicting) refactoring runs. This means we have to
         * collect all replacements first or different runs can interact
         * in unpredicted ways because the underlying files have been changed.
         */
        int err = Tool.run(Action.get());
        if (err != 0) {
            std::cerr << "** ERROR: a refactoring run failed\n";
            std::exit(EXIT_FAILURE);
        }
    }
    
    if (Tool.getReplacements().empty()) {
        std::cerr << "** Info: no code replacements to make - done\n";
    } else if (!DryRun) {
        IntrusiveRefCntPtr<DiagnosticOptions> Opts = new DiagnosticOptions();
        IntrusiveRefCntPtr<DiagnosticIDs> Id = new DiagnosticIDs();
        
        TextDiagnosticPrinter Printer(llvm::errs(), &*Opts);
        DiagnosticsEngine Diagnostics(Id, &*Opts, &Printer, false);
        SourceManager SM(Diagnostics, Tool.getFiles());
        
        Rewriter Rewriter(SM, LangOptions());
        
        bool ok = Tool.applyAllReplacements(Rewriter);
        if (!ok) {
            std::cerr << "** ERROR: failed to apply all code replacements\n";
            std::exit(EXIT_FAILURE);
        }
        
        bool err = Rewriter.overwriteChangedFiles();
        if (err) {
            std::cerr << "** ERROR: failed to save changes to disk\n";
            std::exit(EXIT_FAILURE);
        }
    }
    
    return 0;
}
