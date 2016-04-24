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

#ifndef _INCLUDEREFACTORER_HPP_
#define _INCLUDEREFACTORER_HPP_

#include <unordered_map>
#include <vector>

#include <clang/Lex/Token.h>
#include <clang/Lex/MacroInfo.h>

#include <Refactorers/Refactorer.hpp>


class IncludeRefactorer : public Refactorer {
public:
    virtual void afterSourceFileAction() override;
    
    virtual void InclusionDirective(clang::SourceLocation LocBegin, 
                                    const clang::Token &Token, 
                                    llvm::StringRef FileName, 
                                    bool isAngled, 
                                    clang::CharSourceRange NameRange, 
                                    const clang::FileEntry* File, 
                                    llvm::StringRef SearchPath, 
                                    llvm::StringRef RelativePath, 
                                    const clang::Module* Module) override;
                            
    virtual void FileSkipped(const clang::FileEntry &SkippedFile,
                             const clang::Token &FileNameToken,
                             clang::SrcMgr::CharacteristicKind Kind) override;
    
    virtual void MacroExpands(const clang::Token &Token,
                              const clang::MacroDefinition &MacroDef,
                              clang::SourceRange Range,
                              const clang::MacroArgs *Args) override;

    virtual void visitCallExpr(const clang::CallExpr *Expr) override;
    virtual void visitDeclRefExpr(const clang::DeclRefExpr *Expr) override;
    virtual void visitTypeLoc(const clang::TypeLoc &TypeLoc) override;
private:
    typedef std::pair<unsigned int, unsigned int> UIntPair;
    typedef std::vector<clang::SourceRange> SourceRangeVector;
    struct UIntPairHash {
        std::size_t operator()(const UIntPair &Pair) const;
    };
    
    std::pair<unsigned int, bool> getFileUID(clang::SourceLocation Loc) const;
    
    bool isInSystemLibraryHeader(clang::SourceLocation Loc);
    void run(clang::SourceLocation IncludingLoc, const clang::Decl *Decl);
    void removeUsedIncludes(clang::SourceLocation IncludingLoc,
                            clang::SourceLocation IncludedLoc);
    void removeUsedIncludes(unsigned int IncludingUID,
                            clang::SourceLocation IncludedLoc);
    
    bool isHeaderFile(const clang::StringRef &FileName) const;
    
    void addReplacement(const clang::SourceRange Range);
    void addReplacement(const clang::SourceLocation Loc, unsigned int Length);
    
    std::unordered_map<UIntPair, SourceRangeVector, UIntPairHash> _IncludeMap;
};

#endif /* _INCLUDEREFACTORER_HPP_ */