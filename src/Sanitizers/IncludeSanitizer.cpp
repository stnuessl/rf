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

#include <utility>

#include <clang/Lex/Preprocessor.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <Sanitizers/IncludeSanitizer.hpp>
#include <util/memory.hpp>

class PPCallbackHandler : public clang::PPCallbacks {
public:
    explicit PPCallbackHandler(IncludeSanitizer *Sanitizer);
    
    void InclusionDirective(clang::SourceLocation LocBegin, 
                            const clang::Token &Token, 
                            llvm::StringRef FileName, 
                            bool isAngled, 
                            clang::CharSourceRange FileNameRange, 
                            const clang::FileEntry* File, 
                            llvm::StringRef SearchPath, 
                            llvm::StringRef RelativePath, 
                            const clang::Module* Module);
    
private:
    IncludeSanitizer *_Sanitizer;
};

PPCallbackHandler::PPCallbackHandler(IncludeSanitizer *Sanitizer)
    : clang::PPCallbacks(),
      _Sanitizer(Sanitizer)
{
}

void PPCallbackHandler::InclusionDirective(clang::SourceLocation LocBegin, 
                                           const clang::Token &Token, 
                                           llvm::StringRef FileName, 
                                           bool isAngled, 
                                           clang::CharSourceRange FileNameRange, 
                                           const clang::FileEntry* File, 
                                           llvm::StringRef SearchPath, 
                                           llvm::StringRef RelativePath, 
                                           const clang::Module* Module)
{
    (void) Module;
    
    auto &CompilerInstance = _Sanitizer->getCompilerInstance();
    auto &SM = CompilerInstance.getSourceManager();
    auto &IncludeMap = _Sanitizer->includeMap();
    
    if (SM.isInSystemHeader(LocBegin) || !SM.isLocalSourceLocation(LocBegin))
        return;
    
    /*
     * This source range can be visualized as
     * 
     *      #include <memory>
     *      ^~~~~~~~~~~~~~~~^
     */
    auto Range = clang::SourceRange(LocBegin, FileNameRange.getEnd());
    
    IncludeMap[File->getUID()].push_back(std::move(Range));
    
    auto FileID = SM.getFileID(LocBegin);
    auto ThisFile = SM.getFileEntryForID(FileID);
    
    if (ThisFile && ThisFile->isValid()) {
        auto ThisUID = ThisFile->getUID();
        
        llvm::errs() << ThisFile->getName() << " (" << ThisUID << ") includes " 
                     << File->getName() << " (" << File->getUID() << ")\n";
    }
    
//     LocBegin.dump(SM);
//     llvm::errs() << "\nFile " << File->getUID() << " --> " << FileName << "\n";

#if 0
    llvm::errs() << "At: ";
    LocBegin.dump(SM);
    llvm::errs() << " :: included " << FileName << " / " << File->getName() << "\n";
    
    FileNameRange.getBegin().dump(SM);
    llvm::errs() << " < -- > ";
    FileNameRange.getEnd().dump(SM);
    llvm::errs() << "\n\n";
#endif
}

namespace {
class Visitor : public clang::RecursiveASTVisitor<Visitor> {
public:
    Visitor(clang::ASTContext *ASTContext, IncludeSanitizer *Sanitizer);
    
    bool VisitVarDecl(clang::VarDecl *VarDecl);
    bool VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr);
    
    void removeIncluded(clang::SourceLocation Loc);
private:
//     clang::ASTContext *_ASTContext;
    IncludeSanitizer *_Sanitizer;
    clang::SourceManager *_SourceManager;
    IncludeSanitizer::IncludeMap *_IncludeMap;
    unsigned int _LastUID;
};

Visitor::Visitor(clang::ASTContext *ASTContext, IncludeSanitizer *Sanitizer)
    : clang::RecursiveASTVisitor<Visitor>(),
//       _ASTContext(ASTContext),
      _Sanitizer(Sanitizer),
      _SourceManager(&_Sanitizer->getCompilerInstance().getSourceManager()),
      _IncludeMap(&_Sanitizer->includeMap()),
      _LastUID(std::numeric_limits<unsigned int>::max())
{
}

bool Visitor::VisitVarDecl(clang::VarDecl *VarDecl)
{
    auto TagDecl = VarDecl->getType()->getAsTagDecl();
    if (!TagDecl)
        return true;
    
    auto Loc = TagDecl->getCanonicalDecl()->getLocStart();
    
    removeIncluded(Loc);
    
//     VarDecl->dump();

    return !_IncludeMap->empty();

}

bool Visitor::VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr)
{
    auto ValueDecl = DeclRefExpr->getDecl();
    auto Loc = ValueDecl->getCanonicalDecl()->getLocStart();
    
//     DeclRefExpr->getLocation().dump(*_SourceManager);
//     llvm::errs() << "\n";
//     ValueDecl->dump();
//     llvm::errs() << "\n";
    
    
    removeIncluded(Loc);
    
    return !_IncludeMap->empty();
}


void Visitor::removeIncluded(clang::SourceLocation Loc)
{
    while (Loc.isValid() && !_IncludeMap->empty()) {
        auto FileID = _SourceManager->getFileID(Loc);
        auto FileEntry = _SourceManager->getFileEntryForID(FileID);
        
#if 0
        if (FileEntry && FileEntry->isValid())
            _IncludeMap->erase(FileEntry->getUID());
#endif
        if (FileEntry && FileEntry->isValid()) {
            auto UID = FileEntry->getUID();
            
//             llvm::errs() << "Removing: " << UID << " / " << FileEntry->getName() << "\n";
            _IncludeMap->erase(UID);
        }
        
        Loc = _SourceManager->getIncludeLoc(FileID);
    }
}


class Consumer : public clang::ASTConsumer {
public:
    Consumer(clang::ASTContext *ASTContext, IncludeSanitizer *Sanitizer);
    
    virtual void HandleTranslationUnit(clang::ASTContext &ASTContext) override;
private:
    Visitor _Visitor;
};

Consumer::Consumer(clang::ASTContext *ASTContext, IncludeSanitizer *Sanitizer)
    : _Visitor(ASTContext, Sanitizer)
{
}

void Consumer::HandleTranslationUnit(clang::ASTContext &ASTContext)
{
    _Visitor.TraverseDecl(ASTContext.getTranslationUnitDecl());
}

} /* namespace */


IncludeSanitizer::IncludeMap &IncludeSanitizer::includeMap()
{
    return _IncludeMap;
}

const IncludeSanitizer::IncludeMap &IncludeSanitizer::includeMap() const
{
    return _IncludeMap;
}

void IncludeSanitizer::ExecuteAction()
{
    auto &CompilerInstance = getCompilerInstance();
    auto &PP = CompilerInstance.getPreprocessor();
    
    llvm::errs() << "\nExecute Action for \"" << getCurrentFile() << "\"\n";
    
    auto PPCbHandler = std::make_unique<PPCallbackHandler>(this);
    PP.addPPCallbacks(std::move(PPCbHandler));
    
    Sanitizer::ExecuteAction();
}

void IncludeSanitizer::EndSourceFileAction()
{
    clang::FrontendAction::EndSourceFileAction();
    
    auto &SM = getCompilerInstance().getSourceManager();
    
    if (!_IncludeMap.empty())
        llvm::errs() << "\nUnneeded includes:\n";
    
    for (const auto &x : _IncludeMap) {
        for (const auto &y : x.second) {
            llvm::errs() << "  ";
            y.getBegin().dump(SM);
            llvm::errs() << "\n";
        }
    }
    
    _IncludeMap.clear();
}


std::unique_ptr<clang::ASTConsumer> 
IncludeSanitizer::CreateASTConsumer(clang::CompilerInstance &CI, StringRef File)
{
    (void) File;
    
    return std::make_unique<Consumer>(&CI.getASTContext(), this);
}

