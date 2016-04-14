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
    
    virtual void MacroExpands(const clang::Token &Token,
                              const clang::MacroDefinition &Definition,
                              clang::SourceRange Range,
                              const clang::MacroArgs *Args) override;
                              
    
private:
    IncludeSanitizer *_Sanitizer;
};

PPCallbackHandler::PPCallbackHandler(IncludeSanitizer *Sanitizer)
    : clang::PPCallbacks(),
      _Sanitizer(Sanitizer)
{
}

static bool isHeaderFile(const clang::StringRef &Name)
{
    /* Catch standard headers */
    if (Name.rfind('.') == clang::StringRef::npos)
        return true;
    
    if (Name.endswith_lower(".h"))
        return true;
    
    if (Name.endswith_lower(".hpp"))
        return true;
    
    if (Name.endswith_lower(".hxx"))
        return true;

    return false;
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
    
    if (!isHeaderFile(FileName))
        return;
    
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
            
            /* 
             * Given:
             *      main.cpp:
             *          #include <string>
             *          #include <file.hpp>
             *      file.hpp:
             *          #include <string>
             * 
             * The string header is included twice and the second include
             * cannot be checked for usage. Such includes will be removed from
             * testing. However if main.cpp really depends on <string> compile
             * times will not be effected if the #include in file.hpp is
             * redundant. This basically means that in order to remove all 
             * superfluous includes the program has to be run as many times as
             * it finds includes to remove (here two times if string is 
             * redundant in both files).
             * 
             * The next case is a little easier:
             *      main:cpp
             *          #include <string>
             *          #include <string>
             * 
             * Something like that can easily happen if there are a lot files
             * included between those two shown include directives.
             * This results in the following mapping:
             *      (main.cpp, string) --> clang::SourceRange_1
             *                         --> clang::SourceRange_2
             *                         --> ...
             *                         --> clang::SourceRange_N
             * 
             * This means that if we remove an element from the vector here
             * and the vector is not empty it was a redundant include in the
             * same file which can be removed.
             */
             
            if (Vec.empty()) {
                IncludeMap.erase(MapIt);
            } else {
                It->getBegin().dump(SM);
                llvm::errs() << " --> unused #include directive\n";
            }
            
            break;
        }
    }
}


void PPCallbackHandler::MacroExpands(const clang::Token &Token, 
                                     const clang::MacroDefinition &Definition, 
                                     clang::SourceRange Range, 
                                     const clang::MacroArgs *Args)
{
    /* 
     * Handle Macro dependencies to header includes here
     */
    (void) Token;
    (void) Args;
    
    auto &SM = _Sanitizer->getCompilerInstance().getSourceManager();
    auto &IncludeMap = _Sanitizer->includeMap();
    
    if (IncludeMap.empty())
        return;
    
    auto InvLoc = Range.getBegin();
    auto Info = Definition.getMacroInfo();
    auto DefLoc = Info->getDefinitionLoc();
    
    if (SM.isWrittenInSameFile(InvLoc, DefLoc))
        return;
    
    auto InvFileID = SM.getFileID(InvLoc);
    auto InvFile = SM.getFileEntryForID(InvFileID);
    
    if (!InvFile || !InvFile->isValid())
        return;
    
    auto UID1 = InvFile->getUID();
    
    while (DefLoc.isValid() && !IncludeMap.empty()) {
        auto FileID = SM.getFileID(DefLoc);
        auto File = SM.getFileEntryForID(FileID);
        
        if (File && File->isValid()) {
            auto UID2 = File->getUID();
            
            if (UID1 == UID2)
                return;

            IncludeMap.erase({UID1, UID2});
        }
        
        DefLoc = SM.getIncludeLoc(FileID);
    }
}


namespace {
class Visitor : public clang::RecursiveASTVisitor<Visitor> {
public:
    Visitor(clang::ASTContext *ASTContext, IncludeSanitizer *Sanitizer);
    
    bool shouldVisitTemplateInstantiations() const;
    
    bool VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr);
    bool VisitTypeLoc(const clang::TypeLoc &TypeLoc);
    
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

std::size_t 
IncludeSanitizer::UIntPairHash::operator()(const UIntPair &Pair) const
{
    auto Hasher = std::hash<unsigned int>();

    /* 
     * Pairs with 'first' == 'second' should not appear:
     * it would mean that a file includes itself.
     */
    
    return Hasher(Pair.first) ^ Hasher(Pair.second);
}

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

    auto PPCbHandler = std::make_unique<PPCallbackHandler>(this);
    PP.addPPCallbacks(std::move(PPCbHandler));
    
    Sanitizer::ExecuteAction();
}

void IncludeSanitizer::EndSourceFileAction()
{
    clang::FrontendAction::EndSourceFileAction();
    
    auto &SM = getCompilerInstance().getSourceManager();
    
    for (const auto &x : _IncludeMap) {
        for (const auto &y : x.second) {
            y.getBegin().dump(SM);
            llvm::errs() << " --> unused #include directive\n";
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

