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
    
    virtual void InclusionDirective(clang::SourceLocation LocBegin, 
                                    const clang::Token &Token, 
                                    llvm::StringRef FileName, 
                                    bool isAngled, 
                                    clang::CharSourceRange FileNameRange, 
                                    const clang::FileEntry* File, 
                                    llvm::StringRef SearchPath, 
                                    llvm::StringRef RelativePath, 
                                    const clang::Module* Module) override;
                            
    virtual void FileSkipped(const clang::FileEntry &SkippedFile,
                             const clang::Token &FileNameToken,
                             clang::SrcMgr::CharacteristicKind Kind) override;
    
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
    
    auto FileID = SM.getFileID(LocBegin);
    auto IncludingFile = SM.getFileEntryForID(FileID);
    
    if (!IncludingFile || !IncludingFile->isValid())
        return;
    
    auto UID1 = IncludingFile->getUID();
    auto UID2 = File->getUID();
    
    /*
     * This source range can be visualized as
     * 
     *      #include <memory>
     *      ^~~~~~~~~~~~~~~~^
     */
    auto Range = clang::SourceRange(LocBegin, FileNameRange.getEnd());
    
    /* Same file could be included several times */
    IncludeMap[{UID1, UID2}].push_back(std::move(Range));
    
//     llvm::errs() << IncludingFile->getName() << ", " << File->getName() << "\n";
//     llvm::errs() << "Adding: (" << UID1 << ", " << UID2 << ")\n";
}

void PPCallbackHandler::FileSkipped(const clang::FileEntry &SkippedFile, 
                                    const clang::Token &FileNameToken, 
                                    clang::SrcMgr::CharacteristicKind Kind)
{
    /*
     * Remove Source Locations which cannot possible achieve a hit
     * in the Vistitor code below since they were included twice
     * (only the first include will be hitted).
     */
    (void) Kind;
    
    auto &SM = _Sanitizer->getCompilerInstance().getSourceManager();
    
    auto Loc = FileNameToken.getEndLoc();
    auto FileID = SM.getFileID(Loc);
    auto IncludingFile = SM.getFileEntryForID(FileID);
    
    if (!IncludingFile || !IncludingFile->isValid())
        return;
    
    auto IncludingUID = IncludingFile->getUID();
    auto SkippedUID = SkippedFile.getUID();
    
    auto &IncludeMap = _Sanitizer->includeMap();
    
    auto MapIt = IncludeMap.find({IncludingUID, SkippedUID});
    if (MapIt == IncludeMap.end())
        return;
    
    auto &Vec = MapIt->second;
    
    for (auto It = Vec.begin(), End = Vec.end(); It != End; ++It) {
        if (Loc == It->getEnd()) {
            Vec.erase(It);
            
            if (Vec.empty())
                IncludeMap.erase(MapIt);
            
            break;
        }
    }
}


namespace {
class Visitor : public clang::RecursiveASTVisitor<Visitor> {
public:
    Visitor(clang::ASTContext *ASTContext, IncludeSanitizer *Sanitizer);
    
    bool shouldVisitTemplateInstantiations() const;
    
    bool VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr);
    bool VisitTypeLoc(const clang::TypeLoc &TypeLoc);
    bool VisitTypedefNameDecl(clang::TypedefNameDecl *TypedefNamedDecl);
    
    void run(clang::SourceLocation Loc, unsigned int IncluderUID);
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

bool Visitor::shouldVisitTemplateInstantiations() const
{
    return true;
}

bool Visitor::VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr)
{
    auto DeclRefExprLoc = DeclRefExpr->getLocStart();
    
    if (_SourceManager->isInSystemHeader(DeclRefExprLoc))
        return true;
    
    auto FileID = _SourceManager->getFileID(DeclRefExprLoc);
    auto File = _SourceManager->getFileEntryForID(FileID);
    
    if (!File || !File->isValid())
        return true;
    
    auto UID = File->getUID();
    auto ValueDecl = DeclRefExpr->getFoundDecl();
    auto Loc = ValueDecl->getLocStart();

    if (_SourceManager->isWrittenInSameFile(DeclRefExprLoc, Loc))
        return true;
    
    run(Loc, UID);
    
    return !_IncludeMap->empty();
}

bool Visitor::VisitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    auto Type = TypeLoc.getType();
    
    if (Type->isBuiltinType())
        return true;
    
    auto Loc = TypeLoc.getLocStart();
    
    if (_SourceManager->isInSystemHeader(Loc))
        return true;
    
    auto FileID = _SourceManager->getFileID(Loc);
    auto File = _SourceManager->getFileEntryForID(FileID);
    
    if (!File || !File->isValid())
        return true;
    
    auto UID = File->getUID();
    
    auto TemplateSpecType = Type->getAs<clang::TemplateSpecializationType>();    
    if (TemplateSpecType) {
        auto TemplateName = TemplateSpecType->getTemplateName();
        auto TemplateDecl = TemplateName.getAsTemplateDecl();
        
        run(TemplateDecl->getLocStart(), UID);
        
        return !_IncludeMap->empty();
    }
    
    auto TypedefType = Type->getAs<clang::TypedefType>();
    if (TypedefType) {
        auto TypedefNameDecl = TypedefType->getDecl();
        
        run(TypedefNameDecl->getLocStart(), UID);
        return !_IncludeMap->empty();
    }
    
    auto TagDecl = TypeLoc.getType()->getAsTagDecl();
    if (!TagDecl)
        return true;
    
    run(TagDecl->getLocStart(), UID);
    
    return !_IncludeMap->empty();
}

bool Visitor::VisitTypedefNameDecl(clang::TypedefNameDecl *TypedefNamedDecl)
{
//     llvm::errs() << "TypedefDecl: " << TypedefNamedDecl->getNameAsString() << "\n";
//     auto UID = getFileUID(TypedefNamedDecl->getLocStart());
//     
//     auto TagDecl = TypedefNamedDecl->getAnonDeclWithTypedefName();
//     if (!TagDecl)
//         return true;
//     
//     if (_SourceManager->isInMainFile(TypedefNamedDecl->getLocStart())) {
//         TypedefNamedDecl->dump();
//     }
//     
//     llvm::errs() << "TypedefNameDecl\n";
//     run(TagDecl->getLocStart(), UID);
//     
    return !_IncludeMap->empty();
}


void Visitor::run(clang::SourceLocation Loc, unsigned int IncluderUID)
{
    while (Loc.isValid() && !_IncludeMap->empty()) {
        auto FileID = _SourceManager->getFileID(Loc);
        auto FileEntry = _SourceManager->getFileEntryForID(FileID);
        
        if (FileEntry && FileEntry->isValid()) {
            auto IncludedUID = FileEntry->getUID();
            
            if (IncluderUID == IncludedUID)
                return;
            
            _IncludeMap->erase({IncluderUID, IncludedUID});
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
            llvm::errs() << "  (" << x.first.first << ", " << x.first.second
                         << ") --->";
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

