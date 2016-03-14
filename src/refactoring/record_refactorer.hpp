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

#ifndef _RECORD_REFACTORER_HPP_
#define _RECORD_REFACTORER_HPP_

#include <refactoring/refactorer.hpp>

class record_refactorer : public refactorer {
public:
    record_refactorer();
    
    virtual void run(const match_result &result) override;

private:
    void run_record_decl(const match_result &result);
    void run_var_decl(const match_result &result);
    void run_function_decl(const match_result &result);
    void run_cxx_temp_object_expr(const match_result &result);
    
    bool is_victim(const clang::TagDecl *decl);
    bool is_victim(const clang::FunctionDecl *decl);
    bool is_victim(const clang::VarDecl *decl);
    bool is_victim(const clang::CXXTemporaryObjectExpr *expr);
};

#endif /* _RECORD_REFACTORER_HPP_ */
