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

#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <Refactorers/Base/Refactorer.hpp>
#include <util/commandline.hpp>

void Refactorer::setCompilerInstance(clang::CompilerInstance *CI)
{
    CompilerInstance_ = CI;
}

void Refactorer::setASTContext(clang::ASTContext *ASTContext)
{
    ASTContext_ = ASTContext;
}

clang::tooling::Replacements &Refactorer::replacements()
{
    return Replacements_;
}

const clang::tooling::Replacements &Refactorer::replacements() const
{
    return Replacements_;
}

void Refactorer::setForce(bool Value)
{
    Force_ = Value;
}

bool Refactorer::force() const
{
    return Force_;
}

void Refactorer::beginSourceFileAction(llvm::StringRef File)
{
    (void) File;
}

void Refactorer::endSourceFileAction()
{
}

void Refactorer::visitCXXConstructorDecl(const clang::CXXConstructorDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitCXXDestructorDecl(const clang::CXXDestructorDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitCXXMethodDecl(const clang::CXXMethodDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitCXXRecordDecl(const clang::CXXRecordDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitDecl(const clang::Decl *Decl)
{
    (void) Decl;
}

void Refactorer::visitEnumConstantDecl(const clang::EnumConstantDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitEnumDecl(const clang::EnumDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitFieldDecl(const clang::FieldDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitFunctionDecl(const clang::FunctionDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitNamespaceAliasDecl(const clang::NamespaceAliasDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitNamespaceDecl(const clang::NamespaceDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitRecordDecl(const clang::RecordDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitTypedefNameDecl(const clang::TypedefNameDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitUsingDecl(const clang::UsingDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitUsingDirectiveDecl(const clang::UsingDirectiveDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitUsingShadowDecl(const clang::UsingShadowDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitVarDecl(const clang::VarDecl *Decl)
{
    (void) Decl;
}

void Refactorer::visitExpr(const clang::Expr *Expr)
{
    (void) Expr;
}

void Refactorer::visitCallExpr(const clang::CallExpr *Expr)
{
    (void) Expr;
}

void Refactorer::visitDeclRefExpr(const clang::DeclRefExpr *Expr)
{
    (void) Expr;
}

void Refactorer::visitMemberExpr(const clang::MemberExpr *Expr)
{
    (void) Expr;
}

void Refactorer::visitUnresolvedLookupExpr(
    const clang::UnresolvedLookupExpr *Expr)
{
    (void) Expr;
}

void Refactorer::visitElaboratedTypeLoc(const clang::ElaboratedTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitFunctionTypeLoc(const clang::FunctionTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitInjectedClassNameTypeLoc(
    const clang::InjectedClassNameTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitMemberPointerTypeLoc(
    const clang::MemberPointerTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitPointerTypeLoc(const clang::PointerTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitQualifiedTypeLoc(const clang::QualifiedTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitReferenceTypeLoc(const clang::ReferenceTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitTagTypeLoc(const clang::TagTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitTemplateSpecializationTypeLoc(
    const clang::TemplateSpecializationTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitTypedefTypeLoc(const clang::TypedefTypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::visitTypeLoc(const clang::TypeLoc &TypeLoc)
{
    (void) TypeLoc;
}

void Refactorer::addReplacement(clang::SourceLocation Loc,
                                unsigned int Length,
                                llvm::StringRef ReplText)
{
    auto &SM = CompilerInstance_->getSourceManager();

    addReplacement(SM, Loc, Length, ReplText);
}

void Refactorer::addReplacement(const clang::SourceManager &SM,
                                clang::SourceLocation Loc,
                                unsigned int Length,
                                llvm::StringRef ReplText)
{
    if (Loc.isInvalid())
        return;

    /*
     * If we land here we basically found a source location which needs
     * refactoring. So this checks if the source location is a result
     * of an macro expansion. If it is we locate the source location from
     * before the macro expansion as written in the source code.
     */

    if (Loc.isMacroID())
        Loc = SM.getSpellingLoc(Loc);

    if (SM.isInSystemHeader(Loc) || SM.isInExternCSystemHeader(Loc))
        return;

    /*
     * Different looking relative paths can specify the same file.
     * Such paths may occur when relative paths are used in inclusion
     * directives. This leads to multiple replacements which effectively
     * refactor the same source location and thus probably breaking the code.
     * Since their paths differ the 'Replacements' container will not
     * detect such duplicates.
     * To avoid this problem we retrieve the realpath (absolute
     * with no './' or '../' segments) for each source location.
     */

    auto File = SM.getFilename(Loc);
    auto Offset = SM.getFileOffset(Loc);

    if (File != LastFile_) {
        LastFile_.assign(File.begin(), File.end());

        PathBuffer_ = File;

        auto Error = llvm::sys::fs::make_absolute(PathBuffer_);
        if (Error) {
            llvm::errs() << util::cl::Error()
                         << "failed to retrieve absolute file path for \""
                         << File << "\" - " << Error.message() << "\n";
            std::exit(EXIT_FAILURE);
        }

        llvm::sys::path::remove_dots(PathBuffer_, true);
    }

    File = PathBuffer_.str();

    auto Repl = clang::tooling::Replacement(File, Offset, Length, ReplText);
    Replacements_.insert(std::move(Repl));
}
