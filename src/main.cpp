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


#include <refactoring/record_refactorer.hpp>
#include <refactoring/function_refactorer.hpp>
#include <util/memory.hpp>


using namespace clang;
using namespace clang::tooling;
using namespace llvm;

// static cl::OptionCategory refactoring_options("rf options");
// static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::list<std::string> record_vec(
    "record", 
    cl::desc("Refactor a struct or a class name."),
    cl::value_desc("victim=repl"),
    cl::CommaSeparated
);

static cl::list<std::string> function_vec(
    "function",
    cl::desc("Refactor a function or class method name."),
    cl::value_desc("victim=repl"),
    cl::CommaSeparated
);

static cl::opt<std::string> comp_db_path(
    "comp-db",
    cl::desc("Specify the <path> to the compilation database."),
    cl::value_desc("path")
);

static cl::opt<bool> dry_run(
    "dry-run",
    cl::desc("do not make any changes at all. Useful for debugging."),
    cl::init(false)
);


static std::unique_ptr<CompilationDatabase> 
compilation_database(const std::string &path, std::string &err_msg)
{
    if (!path.empty())
        return JSONCompilationDatabase::loadFromFile(path, err_msg);
    
    const char *cwd = std::getenv("PWD");
    if (!cwd)
        cwd = "./";
    
    return CompilationDatabase::autoDetectFromDirectory(cwd, err_msg);
}

template<typename T>
void add_refactorers(const std::vector<std::string> &args, 
                     std::vector<std::unique_ptr<refactorer>> &rf_vec)
{
    for (const auto &x : args) {
        auto pos = x.find('=');
        if (pos == std::string::npos) {
            std::cerr << "** ERROR: invalid argument '" << x << "' - have a "
                      << "look at the help message" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        
        auto victim = x.substr(0, pos);
        auto repl_str = x.substr(pos + sizeof(char));

        auto rf = std::make_unique<T>();
        rf->set_victim(std::move(victim));
        rf->set_repl_str(std::move(repl_str));
        
        rf_vec.push_back(std::move(rf));
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
    
    auto rf_vec = std::vector<std::unique_ptr<refactorer>>();
    
    add_refactorers<record_refactorer>(record_vec, rf_vec);
    add_refactorers<function_refactorer>(function_vec, rf_vec);
    
    auto err_msg = std::string();
    auto comp_db = compilation_database(comp_db_path, err_msg);
    if (!comp_db) {
        std::cerr << "** ERROR: " << err_msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    auto tool = RefactoringTool(*comp_db, comp_db->getAllFiles());

    for (auto &x : rf_vec) {
        int err;
        
        x->set_replacements(&tool.getReplacements());
        
        auto action = newFrontendActionFactory(x->match_finder());
        
        if (dry_run)
            err = tool.run(action.get());
        else
            err = tool.runAndSave(action.get());
        
        if (err != 0) {
            std::cerr << "** CRITICAL: a refactoring run failed - source files "
                      << "may be corrupted" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

        
//     llvm::outs() << "Replacements collected by the tool:\n";
//     for (auto &r : tool.getReplacements()) {
//         llvm::outs() << r.toString() << "\n";
//     }
    
    return 0;
}
