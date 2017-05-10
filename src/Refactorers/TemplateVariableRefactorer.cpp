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

#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Type.h>

#include <Refactorers/TemplateVariableRefactorer.hpp>


void TemplateVariableRefactorer::visitTemplateTypeParmTypeLoc(
    const clang::TemplateTypeParmTypeLoc &TypeLoc)
{
    
    auto TemplateTypeParmDecl = TypeLoc.getDecl();
    if (isVictim(TemplateTypeParmDecl)) {
        auto Loc = TypeLoc.getLocStart();
        addReplacement(Loc);
    }
//     else {
// 
//         TypeLoc.getLocStart().dump(CompilerInstance_->getSourceManager());
//         llvm::errs() << "\n";
//         TypeLoc.getType()->dump();
//         
//         auto Context = TemplateTypeParmDecl->getDeclContext();
//         
//         if (Context->getOuterLexicalRecordContext())
//             Context->getOuterLexicalRecordContext()->dump();
//         if (Context->isRecord()) {
//             Context->getOuterLexicalRecordContext()->dump();
//         }
//         
//         
//         while (Context) {
//             auto NamedDecl = clang::dyn_cast<clang::NamedDecl>(Context);
//             if (NamedDecl) {
//                 NamedDecl->dump();
//             }
//             
//             Context = Context->getParent();
//         }
        
//         llvm::errs() << "--------------------------------------------------\n";
        
//         auto Context = TemplateTypeParmDecl->getParentFunctionOrMethod();
//         if (Context) {
//             auto FunctionDecl = clang::dyn_cast<clang::FunctionDecl>(Context);
//             
//             if (FunctionDecl)
//                 FunctionDecl->dump();
//         }
//     }
    

//     llvm::errs() << " :: " << qualifiedName(TemplateTypeParmDecl) << "\n";
//     
//     TemplateTypeParmDecl->dump();
//     TemplateTypeParmDecl->getTypeForDecl()->dump();
    
//     auto Type = TypeLoc.getType()->getAs<clang::TemplateTypeParmType>();
//     if (Type) {
//         Type = Type->desugar()->getAs<clang::TemplateTypeParmType>();
//         TypeLoc.getLocStart().dump(CompilerInstance_->getSourceManager());
//         llvm::errs() << "  ";
//         Type->dump();
//         
//         auto TemplateTypeParmDecl = TypeLoc.getDecl();
//         if (TemplateTypeParmDecl) {
//         } else {
//             llvm::errs() << "NO TYPEPARMDECL\n";
//         }
        
//         auto TagDecl = TypeLoc.getAsTagDecl();
//         if (TagDecl) {
//         } else {
//             llvm::errs() << "NO TAGDECL\n";
//         }
//         
//         auto PointeeCXX = TypeLoc.getPointeeCXXRecordDecl();
//         if (PointeeCXX) {
//             
//         } else {
//             llvm::errs() << "NO POINTEE\n";
//         }
//     }
}

void TemplateVariableRefactorer::visitTemplateSpecializationTypeLoc(
    const clang::TemplateSpecializationTypeLoc &TypeLoc)
{
    /*
     * Handle template variables in classes and structs.
     */

    auto Type = TypeLoc.getType()->getAs<clang::TemplateSpecializationType>();
    if (Type) {
        auto TemplateDecl = Type->getTemplateName().getAsTemplateDecl();
        auto TemplateParameters = TemplateDecl->getTemplateParameters();

        for (const auto &Decl : TemplateParameters->asArray()) {
            if (isVictim(Decl)) {
                auto Loc = Decl->getLocation();
                addReplacement(Loc);
            }
        }
    }
}
