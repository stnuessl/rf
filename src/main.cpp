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
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>

#include <Refactorers/EnumConstantRefactorer.hpp>
#include <Refactorers/FunctionRefactorer.hpp>
#include <Refactorers/MacroRefactorer.hpp>
#include <Refactorers/NamespaceRefactorer.hpp>
#include <Refactorers/TagRefactorer.hpp>
#include <Refactorers/VariableRefactorer.hpp>

#include <util/CommandLine.hpp>
#include <util/memory.hpp>
#include <util/string.hpp>
#include <util/yaml.hpp>

#include <RefactoringActionFactory.hpp>

static llvm::cl::OptionCategory RefactoringOptions("Code Refactoring Options");
static llvm::cl::OptionCategory ProgramSetupOptions("Program Setup Options");

/* clang-format off */
static llvm::cl::extrahelp HelpText(
    "\n!! Commit your source code to a version control system before "
    "refactoring it !!\n\n"
);

#ifdef __unix__
static llvm::cl::opt<bool> AllowRoot(
    "allow-root",
    llvm::cl::desc(
        "Allow this application to run with root privileges.\n"
    ),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(false)
);
#endif

static llvm::cl::opt<std::string> CompileCommandsPath(
    "commands",
    llvm::cl::desc(
        "Specify the compilation database <file>.\n"
        "Usually this <file> is named \"compile_commands.json\".\n"
        "If not specified rf will automatically search all\n"
        "parent directories for such a file."
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::cat(ProgramSetupOptions)
);

static llvm::cl::opt<bool> DryRun(
    "dry-run",
    llvm::cl::desc(
        "Do not make any changes at all.\n"
        "Useful for debugging, especially when used with \"--verbose\"."
    ),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(false)
);

static llvm::cl::list<std::string> EnumConstantArgs(
    "enum-constant",
    llvm::cl::desc(
        "Rename an enumeration constant."
    ),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated,
    llvm::cl::cat(RefactoringOptions)
);

static llvm::cl::opt<bool> Force(
    "force",
    llvm::cl::desc(
        "Disable safety checks and apply replacements even if they may\n"
        "break the code. No replacements are done if \"--dry-run\"\n"
        "is passed along this option."
    ),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(false)
);

static llvm::cl::opt<std::string> FromFile(
    "from-file",
    llvm::cl::desc(
        "Read additional refactoring options from specified\n"
        "YAML <file>. An exemplary file can be generated\n"
        "with \"--to-yaml\"."
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::cat(ProgramSetupOptions)
);

static llvm::cl::list<std::string> FunctionArgs(
    "function",
    llvm::cl::desc(
        "Rename a function or class method name."
    ),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated,
    llvm::cl::cat(RefactoringOptions)
);

static llvm::cl::opt<bool> Interactive(
    "interactive",
    llvm::cl::desc(
        "Prompt before applying replacements."
    ),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(false)
);

/* 'MacroArgs' is ambiguous if used in namespace 'clang' */
static llvm::cl::list<std::string> PPMacroArgs(
    "macro",
    llvm::cl::desc(
        "Rename a preprocessor macro."
    ),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated,
    llvm::cl::cat(RefactoringOptions)       
);

static llvm::cl::list<std::string> NamespaceArgs(
    "namespace",
    llvm::cl::desc(
        "Rename a namespace."
    ),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated,
    llvm::cl::cat(RefactoringOptions)
);

static llvm::cl::opt<bool> SyntaxOnly(
    "syntax-only",
    llvm::cl::desc(
        "Perform a syntax check and exit.\n"
        "No changes are made even if replacements were specified."
    ),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(false)
);

static llvm::cl::list<std::string> TagArgs(
    "tag",
    llvm::cl::desc(
        "Rename a class, enumeration, structure, or type alias."
    ),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated,
    llvm::cl::cat(RefactoringOptions)
);

static llvm::cl::list<std::string> VariableArgs(
    "variable",
    llvm::cl::desc(
        "Rename the name of a variable or class field."
    ),
    llvm::cl::value_desc("victim=repl"),
    llvm::cl::CommaSeparated,
    llvm::cl::cat(RefactoringOptions)
);

static llvm::cl::opt<bool> Verbose(
    "verbose",
    llvm::cl::desc(
        "Print a line for each replacement to be made."
    ),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(false)
);

static llvm::cl::opt<bool> ToYAML(
    "to-yaml",
    llvm::cl::desc(
        "Convert specified renaming operations into YAML format\n"
        "and print the result to stdout. The program will exit\n"
        "afterwards.\n"
        "This is useful to create an initial YAML file where\n"
        "additional renaming operations can be gathered.\n"
        "This file can be passed with \"--from-file\" to apply\n"
        "a huge bulk of operations at once where specifying\n"
        "every operation on the command-line would not be\n"
        "convenient."
    ),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(false)
);

static llvm::cl::list<std::string> InputFiles(
    llvm::cl::desc("[<file> ...]"),
    llvm::cl::Positional,
    llvm::cl::ZeroOrMore,
    llvm::cl::PositionalEatsArgs
);

/* clang-format on */

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


template<typename T> 
static void add(Refactorers &Refactorers, 
                const std::vector<std::string> &ArgVec,
                clang::tooling::RefactoringTool &Tool)
{
    auto Replacements = &Tool.getReplacements();
    
    for (const auto &Arg : ArgVec) {
        auto Index = Arg.find('=');
        if (Index == std::string::npos) {
            std::cerr << util::cl::Error() 
                      << "invalid argument \"" << Arg 
                      << "\" - argument syntax is \"Victim=Replacement\"\n";
            std::exit(EXIT_FAILURE);
        }
        
        auto Victim = Arg.substr(0, Index);
        auto Repl = Arg.substr(Index + 1);
        
        if (Victim != Repl) {
            auto Refactorer = std::make_unique<T>();
            Refactorer->setReplacements(Replacements);
            Refactorer->setVerbose(Verbose);
            Refactorer->setForce(Force);
            Refactorer->setVictimQualifier(std::move(Victim));
            Refactorer->setReplacementQualifier(std::move(Repl));
            
            Refactorers.push_back(std::move(Refactorer));
        }
    }
}

#define RF_VERSION_MAJOR "1"
#define RF_VERSION_MINOR "1"
#define RF_VERSION_PATCH "0"
#define RF_VERSION_INFO                                                        \
    RF_VERSION_MAJOR "." RF_VERSION_MINOR "." RF_VERSION_PATCH

#define RF_HOMEPAGE_URL "https://github.com/stnuessl/rf"

#define RF_LICENSE_INFO                                                        \
    "License GPLv3+: GNU GPL version 3 or later "                              \
    "<http://www.gnu.org/licenses/gpl.html>\n"                                 \
    "This is free software: you are free to change and redistribute it.\n"     \
    "There is NO WARRANTY, to the extent permitted by law.\n"

int main(int argc, const char **argv) 
{
    using namespace clang;
    using namespace clang::tooling;
    
    auto OptionCategories = llvm::ArrayRef<llvm::cl::OptionCategory *>({ 
        &RefactoringOptions, 
        &ProgramSetupOptions,
    });
    
    llvm::cl::HideUnrelatedOptions(OptionCategories);
    
    const auto print_version = []() {
        llvm::outs() << "rf version: " << RF_VERSION_INFO << " - "
                     << RF_HOMEPAGE_URL << "\n" << RF_LICENSE_INFO << "\n";
    };
    
    llvm::cl::SetVersionPrinter(print_version);
    
    llvm::cl::ParseCommandLineOptions(argc, argv);
    
    /* 
     * Seems like 'ParseCommandLineOptions' has to be called before
     * running this. Otherwise 'PrintHelpMessage' will cause a
     * segmentation fault.
     * Also, 'PrintHelpMessage' will terminate the program.
     */
    if (argc <= 1)
        llvm::cl::PrintHelpMessage(false, true);
    
#ifdef __unix__
    if (!AllowRoot && getuid() == 0) {
        llvm::errs() << util::cl::Error() 
                     << "running on root privileges - aborting...\n";
        std::exit(EXIT_FAILURE);
    }
#endif

    if (ToYAML) {
        auto Args = util::yaml::RefactoringArgs();
        Args.EnumConstants  = std::move(EnumConstantArgs);
        Args.Functions      = std::move(FunctionArgs);
        Args.Macros         = std::move(PPMacroArgs);
        Args.Namespaces     = std::move(NamespaceArgs);
        Args.Tags           = std::move(TagArgs);
        Args.Variables      = std::move(VariableArgs);

        llvm::yaml::Output YAMLOut(llvm::outs());
        YAMLOut << Args;
        
        std::exit(EXIT_SUCCESS);
    }
        
    auto ErrMsg = std::string();
    auto CompilationDB = makeCompilationDatabase(CompileCommandsPath, ErrMsg);
    if (!CompilationDB) {
        llvm::errs() << util::cl::Error() << ErrMsg << "\n";
        std::exit(EXIT_FAILURE);
    }

    auto SourceFiles = CompilationDB->getAllFiles();
    if (!InputFiles.empty())
        std::swap(SourceFiles, *&InputFiles);
    
    if (SyntaxOnly) {
        auto Action = newFrontendActionFactory<clang::SyntaxOnlyAction>();

        ClangTool Tool(*CompilationDB, SourceFiles);
        int err = Tool.run(Action.get());
        
        std::exit((err == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    
    auto Refactorers = ::Refactorers();
    
    RefactoringTool Tool(*CompilationDB, SourceFiles);
    
    add<EnumConstantRefactorer>(Refactorers, EnumConstantArgs, Tool);
    add<FunctionRefactorer>(Refactorers, FunctionArgs, Tool);
    add<MacroRefactorer>(Refactorers, PPMacroArgs, Tool);
    add<NamespaceRefactorer>(Refactorers, NamespaceArgs, Tool);
    add<TagRefactorer>(Refactorers, TagArgs, Tool);
    add<VariableRefactorer>(Refactorers, VariableArgs, Tool);
    
    if (!FromFile.empty()) {
        util::yaml::RefactoringArgs Args;        
        util::yaml::read(FromFile, Args);

        add<EnumConstantRefactorer>(Refactorers, Args.EnumConstants, Tool);
        add<FunctionRefactorer>(Refactorers, Args.Functions, Tool);
        add<MacroRefactorer>(Refactorers, Args.Macros, Tool);
        add<NamespaceRefactorer>(Refactorers, Args.Namespaces, Tool);
        add<TagRefactorer>(Refactorers, Args.Tags, Tool);
        add<VariableRefactorer>(Refactorers, Args.Variables, Tool);
    }

    if (!Refactorers.empty()) {
        auto Factory = std::make_unique<RefactoringActionFactory>();
        Factory->setRefactorers(&Refactorers);
        
        int err = Tool.run(Factory.get());
        if (err) {
            llvm::errs() << util::cl::Error() 
                         << "found syntax error(s) while refactoring\n";
            
            if (!Force) {
                llvm::errs() << util::cl::Info() 
                             << "use \"--force\" to override\n";
                std::exit(EXIT_FAILURE);
            }
        }
    }
    
    if (Tool.getReplacements().empty()) {
        llvm::errs() << util::cl::Info() << "no replacements found - done\n";
        std::exit(EXIT_SUCCESS);
    }
    
    if (DryRun)
        std::exit(EXIT_SUCCESS);
    
    if (Interactive && llvm::outs().is_displayed()) {
        std::string Response;
        
        llvm::outs().changeColor(llvm::raw_ostream::WHITE, true);
        llvm::outs() << ":: Apply all replacements? [y/N]: ";
        llvm::outs().resetColor();
        
        std::getline(std::cin, Response);
        
        util::string::trim(Response);
        util::string::to_lower(Response);
        
        if (Response.empty() || Response[0] == 'n' || Response == "no")
            std::exit(EXIT_SUCCESS);
        
        if (Response[0] != 'y' && Response != "yes") {
            llvm::errs() << util::cl::Error() 
                         << "invalid input \"" << Response << "\" - " 
                         << "discarding all replacements\n";
                         
            std::exit(EXIT_FAILURE);
        }
    }
        
    IntrusiveRefCntPtr<DiagnosticOptions> Opts = new DiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticIDs> Id = new DiagnosticIDs();
    
    TextDiagnosticPrinter Printer(llvm::errs(), &*Opts);
    DiagnosticsEngine Diagnostics(Id, &*Opts, &Printer, false);
    SourceManager SM(Diagnostics, Tool.getFiles());
    LangOptions LangOpts;
    
    Rewriter Rewriter(SM, LangOpts);
    
    bool ok = Tool.applyAllReplacements(Rewriter);
    if (!ok) {
        std::cerr << util::cl::Error() << "failed to apply all replacements\n";
        std::exit(EXIT_FAILURE);
    }
    
    bool err = Rewriter.overwriteChangedFiles();
    if (err) {
        std::cerr << util::cl::Error() << "failed to save changes to disk\n";
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
