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


#include <refactoring/record_refactorer.hpp>


record_refactorer::record_refactorer()
    : refactorer()
{
    using namespace clang::ast_matchers;
    
    auto record_decl = recordDecl().bind("rdecl");
    auto var_decl = varDecl().bind("vdecl");
    auto func_decl = functionDecl().bind("fdecl");
    auto temp_obj_expr = temporaryObjectExpr().bind("texpr");
    
    _finder.addMatcher(record_decl, this);
    _finder.addMatcher(var_decl, this);
    _finder.addMatcher(func_decl, this);
    _finder.addMatcher(temp_obj_expr, this);
}

void record_refactorer::run(const match_result &result)
{
    run_record_decl(result);
    run_var_decl(result);
    run_function_decl(result);
    run_cxx_temp_object_expr(result);
}

void record_refactorer::run_record_decl(const match_result &result)
{
    auto decl = result.Nodes.getNodeAs<clang::RecordDecl>("rdecl");
    if (!decl || !is_victim(decl))
        return;
    
    add_replacement(result, decl->getLocation());
    
    /* 
     * Check if this record is a CXXRecord and if it is adjust ctors 
     * and dtors accordingly.
     */
    auto record_decl = clang::dyn_cast<clang::CXXRecordDecl>(decl);
    if (!record_decl)
        return;
    
    for (const auto &x : record_decl->ctors())
        add_replacement(result, x->getLocation());
    
    if (!record_decl->hasUserDeclaredDestructor())
        return;
    
    auto destructor = record_decl->getDestructor();
    auto loc = destructor->getLocation().getLocWithOffset(1);
    
    add_replacement(result, loc);
}

void record_refactorer::run_var_decl(const match_result &result)
{
    /* 
     * Example code for a VarDecl
     *   my_class a;
     *   ~~~~~~~^ 
     *
     * Check if 'my_class' must be refactored
     */
    
    auto decl = result.Nodes.getNodeAs<clang::VarDecl>("vdecl");
    if (!decl)
        return;
    
    auto var_template_decl = decl->getDescribedVarTemplate();
    
    if (var_template_decl)
        var_template_decl->dump();
    
    if (!is_victim(decl))
        return;
    
    auto info = decl->getTypeSourceInfo();
    if (!info)
        return;
    
    auto loc = info->getTypeLoc().getEndLoc();
    
    add_replacement(result, loc);
}

void record_refactorer::run_function_decl(const match_result &result)
{
    auto decl = result.Nodes.getNodeAs<clang::FunctionDecl>("fdecl");
    if (!decl || !is_victim(decl))
        return;
    
    auto range = decl->getReturnTypeSourceRange();
    
    add_replacement(result, range.getEnd());
}


void record_refactorer::run_cxx_temp_object_expr(const match_result &result)
{
    auto expr = result.Nodes.getNodeAs<clang::CXXTemporaryObjectExpr>("texpr");
    if (!expr || !is_victim(expr))
        return;
    
    /* 
     * Don't now a proper way to get the Location (yet)
     * Maybe this is good enough...
     */
    
    auto offset = -1 - _repl_size;
    auto loc = expr->getLocEnd().getLocWithOffset(offset);
    
    add_replacement(result, loc);
}

bool record_refactorer::is_victim(const clang::TagDecl *decl)
{
    return qualified_name(decl->getCanonicalDecl()) == _victim;
}

bool record_refactorer::is_victim(const clang::FunctionDecl *decl)
{
    auto tag_decl = decl->getReturnType()->getAsTagDecl();
    
    return (tag_decl) ? is_victim(tag_decl) : false;
}

bool record_refactorer::is_victim(const clang::VarDecl *decl)
{
    auto info = decl->getTypeSourceInfo();
    if (!info)
        return false;
    
    auto tag_decl = info->getType().getTypePtr()->getAsTagDecl();
    
    return (tag_decl) ? is_victim(tag_decl) : false;
}

bool record_refactorer::is_victim(const clang::CXXTemporaryObjectExpr *expr)
{
    auto parent = expr->getConstructor()->getParent();
    
    return is_victim(parent);
}
