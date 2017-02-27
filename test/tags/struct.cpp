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

#include "struct.hpp"

namespace tags {
    
s::s()
{
}

s::s(const_reference other)
{
}

s::s(s &&other)
{
}

s::~s()
{
}

s::reference s::operator=(const_reference other)
{
    return *this;
}

auto s::operator=(s &&other) -> reference
{
    return *this;
}

s::pointer s::as_ptr()
{
    return this;
}

auto s::as_ptr() const -> const_pointer
{
    return this;
}

void s::f() const
{
}

} /* namespace tags */

tags::s::pointer tags::s::create()
{
    return new tags::s();
}
