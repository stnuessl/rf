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

#ifndef RF_STRING_HPP_
#define RF_STRING_HPP_

#include <algorithm>
#include <string>

namespace util {
namespace string {

template <typename Predicate>
void trim_front(std::string &str, Predicate pred)
{
    auto it = std::find_if_not(str.begin(), str.end(), pred);
    str.erase(str.begin(), it);
}

template <typename Predicate>
void trim_back(std::string &str, Predicate pred)
{
    auto it = std::find_if_not(str.rbegin(), str.rend(), pred);
    str.erase(it.base(), str.rbegin().base());
}

template <typename Predicate>
void trim(std::string &str, Predicate pred)
{
    trim_front(str, pred);
    trim_back(str, pred);
}

void trim(std::string &str);

template <typename Transform>
void transform(std::string &str, Transform transform)
{
    std::for_each(str.begin(), str.end(), transform);
}

void to_lower(std::string &str);

void to_upper(std::string &str);
}
}

#endif /* RF_STRING_HPP_ */
