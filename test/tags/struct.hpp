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

#ifndef TAG_STRUCT_HPP_
#define TAG_STRUCT_HPP_

namespace tags {

struct s {
    typedef s*          pointer;
    typedef const s*    const_pointer;
    typedef s&          reference;
    typedef const s&    const_reference;
    
    s();
    s(const_reference other);
    s(s &&other);
    ~s();
    
    reference operator=(const_reference other);
    s &operator=(s &&other);
    
    pointer as_ptr();
    const_pointer as_ptr() const;
    
    void f() const;
    
    static pointer create();
};
    
}

#endif /* TAG_STRUCT_HPP_ */
