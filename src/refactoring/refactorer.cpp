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

#include <algorithm>

#include <refactoring/refactorer.hpp>

static void rcopy(std::string &str, const clang::StringRef &ref)
{
    auto n = ref.size();
    
    while (n--)
        str += ref[n];
}

refactorer::refactorer()
    : _finder(),
      _repls(nullptr),
      _victim(),
      _repl_str(),
      _repl_size(0),
      _buffer()
{
    _buffer.reserve(1024);
}

void refactorer::set_victim(const std::string &str)
{
    /*
     * When refactoring a name it must be specified like
     * "namespace::name". However, not the full qualified name needs
     * a replacement but the last part of the qualifier (here: "name").
     * Thus we need to know the size of that part.
     * '_repl_size' can be thought of as something like
     *      sizeof("namespace::name") - sizeof("namespace::")
     */
    _victim = str;
    _repl_size = _victim.size();
    
    auto i = _victim.rfind("::");
    if (i != std::string::npos)
        _repl_size -= i + sizeof("::") - 1;
}

void refactorer::set_repl_str(const std::string &str)
{
    /*
     * Convert "::namespace::class" to just "class" as the namespace is implicit
     * specified by the '_victim' string.
     */
    auto i = str.rfind("::");
    if (i != std::string::npos)
        _repl_str = str.substr(i + sizeof("::") - 1);
    else
        _repl_str = str;
}

void refactorer::set_replacements(clang::tooling::Replacements *repls)
{
    _repls = repls;
}

clang::ast_matchers::MatchFinder *refactorer::match_finder()
{
    return &_finder;
}

void refactorer::add_replacement(const match_result &result, 
                                 const clang::SourceLocation &loc)
{
    add_replacement(*result.SourceManager, loc);
}


void refactorer::add_replacement(const clang::SourceManager &sm, 
                                 const clang::SourceLocation &loc)
{
    using namespace clang::tooling;
    
    _repls->insert(Replacement(sm, loc, _repl_size, _repl_str));
}

std::string &refactorer::qualified_name(const clang::NamedDecl *decl)
{
    /*
     * The DeclContext order for e.g. "namespace::class"
     * is class -> namespace, meaning we first visit the context for 'class' 
     * and then the context for 'namespace' by using 'getParent()'
     * To avoid using some sort of container we write everything reversed into
     * '_buffer' giving "ssalc::ecapseman"
     * Before returning we just have to reverse '_buffer' giving the correct
     * qualified name "namespace::class"
     */
    
    _buffer.clear();
    
    rcopy(_buffer, decl->getName());
    _buffer += "::";
    
    auto context = decl->getDeclContext();
    
    while (context) {
        auto ndecl = clang::dyn_cast<clang::NamedDecl>(context);
        if (ndecl) {
            rcopy(_buffer, ndecl->getName());
            _buffer += "::";
        }
        
        context = context->getParent();
    }
    
    while (!_buffer.empty() && _buffer.back() == ':')
        _buffer.pop_back();
    
    std::reverse(_buffer.begin(), _buffer.end());
    
    return _buffer;
}
