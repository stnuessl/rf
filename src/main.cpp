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

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>

#ifdef __unix__
#include <unistd.h>
#endif

#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>

#include <Refactorers/EnumConstantRefactorer.hpp>
#include <Refactorers/FunctionRefactorer.hpp>
#include <Refactorers/MacroRefactorer.hpp>
#include <Refactorers/NamespaceRefactorer.hpp>
#include <Refactorers/TagRefactorer.hpp>
#include <Refactorers/VariableRefactorer.hpp>

#include <util/commandline.hpp>
#include <util/memory.hpp>
#include <util/string.hpp>
#include <util/yaml.hpp>

#include <RefactoringActionFactory.hpp>
#include <ToolThread.hpp>

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
    "compile-commands",
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

static llvm::cl::opt<unsigned int> NumThreads(
    "num-threads",
    llvm::cl::desc(
        "Set the number of threads to run simultaneously\n"
        "on the translation units. The default value is\n"
        "the number of cores available on the system."
    ),
    llvm::cl::value_desc("int"),
    llvm::cl::cat(ProgramSetupOptions),
    llvm::cl::init(std::thread::hardware_concurrency())
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
getCompilationDatabase(const std::string &Path, std::string &ErrMsg)
{
    using clang::tooling::CompilationDatabase;
    using clang::tooling::JSONCompilationDatabase;

    if (!Path.empty())
        return JSONCompilationDatabase::loadFromFile(Path, ErrMsg);

    llvm::SmallString<64> Buffer;
    auto Error = llvm::sys::fs::current_path(Buffer);
    if (Error) {
        Buffer.clear();
        Buffer.append("./");
    }
    
    auto WorkDir = Buffer.str();

    return CompilationDatabase::autoDetectFromDirectory(WorkDir, ErrMsg);
}

template <typename T>
static void add(std::vector<RefactoringActionFactory> &Factories,
                const std::vector<std::string> &ArgVec)
{
    for (const auto &Arg : ArgVec) {
        auto Index = Arg.find('=');
        if (Index == std::string::npos) {
            std::cerr << util::cl::Error() << "invalid argument \"" << Arg
                      << "\" - argument syntax is \"Victim=Replacement\"\n";
            std::exit(EXIT_FAILURE);
        }

        auto Victim = Arg.substr(0, Index);
        auto Repl = Arg.substr(Index + 1);

        if (Victim == Repl)
            continue;

        for (auto &Factory : Factories) {
            auto Refactorer = std::make_unique<T>();
            Refactorer->setForce(Force);
            Refactorer->setVictimQualifier(Victim);
            Refactorer->setReplacementQualifier(Repl);

            Factory.refactorers().push_back(std::move(Refactorer));
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
    auto OptionCategories = llvm::ArrayRef<llvm::cl::OptionCategory *>({
        &RefactoringOptions, &ProgramSetupOptions,
    });

    llvm::cl::HideUnrelatedOptions(OptionCategories);

    const auto print_version = []() {
        llvm::outs() << "rf version: " << RF_VERSION_INFO << " - "
                     << RF_HOMEPAGE_URL << "\n"
                     << RF_LICENSE_INFO << "\n";
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
        Args.EnumConstants = std::move(EnumConstantArgs);
        Args.Functions = std::move(FunctionArgs);
        Args.Macros = std::move(PPMacroArgs);
        Args.Namespaces = std::move(NamespaceArgs);
        Args.Tags = std::move(TagArgs);
        Args.Variables = std::move(VariableArgs);

        util::yaml::write(llvm::outs(), Args);

        std::exit(EXIT_SUCCESS);
    }

    auto ErrMsg = std::string();
    auto CompileCommands = getCompilationDatabase(CompileCommandsPath, ErrMsg);
    if (!CompileCommands) {
        llvm::errs() << util::cl::Error() << ErrMsg << "\n";
        std::exit(EXIT_FAILURE);
    }

    auto SourceFiles = CompileCommands->getAllFiles();
    if (!InputFiles.empty())
        std::swap(SourceFiles, *&InputFiles);

    if (NumThreads == 0)
        NumThreads = 1;

    std::vector<RefactoringActionFactory> Factories(NumThreads);

    if (!SyntaxOnly) {
        add<EnumConstantRefactorer>(Factories, EnumConstantArgs);
        add<FunctionRefactorer>(Factories, FunctionArgs);
        add<MacroRefactorer>(Factories, PPMacroArgs);
        add<NamespaceRefactorer>(Factories, NamespaceArgs);
        add<TagRefactorer>(Factories, TagArgs);
        add<VariableRefactorer>(Factories, VariableArgs);

        if (!FromFile.empty()) {
            util::yaml::RefactoringArgs Args;
            util::yaml::read(FromFile, Args);

            add<EnumConstantRefactorer>(Factories, Args.EnumConstants);
            add<FunctionRefactorer>(Factories, Args.Functions);
            add<MacroRefactorer>(Factories, Args.Macros);
            add<NamespaceRefactorer>(Factories, Args.Namespaces);
            add<TagRefactorer>(Factories, Args.Tags);
            add<VariableRefactorer>(Factories, Args.Variables);
        }
    }

    auto FilesPerThread = SourceFiles.size() / NumThreads;
    auto RemainingFiles = SourceFiles.size() % NumThreads;
    auto BatchIt = SourceFiles.begin();

    /*
     * This vector is not allowed to resize as currently
     * running threads may try to write to its memory
     */
    std::vector<ToolThread> Threads(Factories.size());

    auto ThreadIt = Threads.begin();
    for (auto &Factory : Factories) {
        auto NumFiles = FilesPerThread;

        if (RemainingFiles > 0) {
            --RemainingFiles;
            ++NumFiles;
        }

        auto BatchBegin = BatchIt;
        auto BatchEnd = std::next(BatchBegin, NumFiles);
        BatchIt = BatchEnd;

        ToolThread::Data Data;
        Data.CompilationDatabase = CompileCommands.get();
        Data.Factory = &Factory;
        Data.Files = llvm::ArrayRef<std::string>(&*BatchBegin, &*BatchEnd);

        ThreadIt->run(Data);
        ++ThreadIt;
    }

    if (BatchIt != SourceFiles.end() || ThreadIt != Threads.end()) {
        /* If the math above is correct this should never happen */
        llvm::errs() << util::cl::Error()
                     << "internal program error - no changes are made\n";
        std::exit(EXIT_FAILURE);
    }
    
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagIds;
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOptions;

    DiagIds = new clang::DiagnosticIDs();
    DiagOptions = new clang::DiagnosticOptions();
    
    clang::TextDiagnosticPrinter Client(llvm::errs(), &*DiagOptions);
    clang::DiagnosticsEngine DiagEngine(DiagIds, &*DiagOptions, &Client, false);

    for (auto &Thread : Threads) {
        Thread.join();

        if (Thread.errorOccured()) {
            llvm::errs() << util::cl::Error()
                         << "encountered syntax error(s) while processing "
                         << "translation units.\n";

            std::exit(EXIT_FAILURE);
        }
    }

    if (SyntaxOnly)
        std::exit(EXIT_SUCCESS);

    clang::tooling::RefactoringTool Tool(*CompileCommands, SourceFiles);
    auto &Replacements = Tool.getReplacements();
    auto InsertIter = std::inserter(Replacements, Replacements.end());

    for (auto &Factory : Factories) {
        for (auto &Refactorer : Factory.refactorers()) {
            auto &Repls = Refactorer->replacements();

            std::move(Repls.begin(), Repls.end(), InsertIter);
        }
    }

    if (Tool.getReplacements().empty()) {
        llvm::errs() << util::cl::Info() << "no replacements were found\n";
        std::exit(EXIT_SUCCESS);
    }

    clang::SourceManager SM(DiagEngine, Tool.getFiles());

    if (Verbose) {
        auto &FileManager = SM.getFileManager();

        for (const auto &Repl : Tool.getReplacements()) {
            auto FileEntry = FileManager.getFile(Repl.getFilePath());
            auto ID = SM.getOrCreateFileID(FileEntry, clang::SrcMgr::C_User);

            auto Offset = Repl.getOffset();
            auto Line = SM.getLineNumber(ID, Offset);
            auto Column = SM.getColumnNumber(ID, Offset);

            llvm::outs() << "\"" << Repl.getReplacementText() << "\" -- "
                         << Repl.getFilePath() << ":" << Line << ":" << Column
                         << "\n";
        }
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
            llvm::errs() << util::cl::Error() << "invalid input \"" << Response
                         << "\" - "
                         << "discarding all replacements\n";

            std::exit(EXIT_FAILURE);
        }
    }

    clang::LangOptions LangOpts;
    clang::Rewriter Rewriter(SM, LangOpts);

    bool ok = Tool.applyAllReplacements(Rewriter);
    if (!ok) {
        llvm::errs() << util::cl::Error() << "failed to apply replacements\n";
        std::exit(EXIT_FAILURE);
    }

    bool err = Rewriter.overwriteChangedFiles();
    if (err) {
        llvm::errs() << util::cl::Error() << "failed to save changes to disk\n";
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
