/*
 * Copyright (C) 2017  Steffen Nüssle
 * rf - refactor
 *
 * This file is part of rf.
 *
 * This program is free software: you cn redistribute it and/or modify
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

#ifndef FUNCTION_HPP_
#define FUNCTION_HPP_

#include "../tags/templated_class.hpp"

namespace tags {
    
template <typename T, typename U>
bool operator==(const class c<T> &lhs, const class c<U> &rhs)
{ 
    return true;
}
    
} /* namespace tags */

namespace functions {

template <typename T, typename U>
bool f(const tags::c<T> &lhs, const tags::c<U> &rhs)
{ 
    return lhs == rhs;
}

int g(int a, int b);

} /* namespace function */



#endif /* FUNCTION_HPP_ */
