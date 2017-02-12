/*
 * Copyright (C) 2017  Steffen Nüssle
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

#include <cctype>

#include <util/string.hpp>

namespace util {
namespace string {

void trim(std::string &str)
{
    auto func = [](char c) { 
        return !!std::isspace(c); 
    };

    trim(str, func);
}

void to_lower(std::string &str)
{
    const auto func = [](char c) {
        return static_cast<char>(std::tolower(c));
    };
    
    transform(str, func);
}

void to_upper(std::string &str)
{
    const auto func = [](char c) {
        return static_cast<char>(std::toupper(c));
    };
    
    transform(str, func);
}

} 
}
