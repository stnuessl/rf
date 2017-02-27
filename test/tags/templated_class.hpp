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

#ifndef TEMPLATED_CLASS_HPP_
#define TEMPLATED_CLASS_HPP_

namespace tags {
    
template <typename T>
class c {
public:
    c();
    c(const c<T> &other);
    template <typename U> c(const c<U> &other);
    c(c<T> &&other);
    ~c() = default;
    
    c &operator=(const c<T> &other);
    c &operator=(c<T> &&other);
};

template <>
class c<bool> { };

template <typename T>
class d;

template <typename T>
class d<const c<T>> { };

/* 
 * Implementation
 */

template<typename T> 
c<T>::c()
{
}

template<typename T>
c<T>::c(const c<T> &other)
{
}

template<typename T> template<typename U>
c<T>::c(const c<U> &other)
{
}

template<typename T>
c<T>::c(c<T> &&other)
{
}


template<typename T> 
c<T> &c<T>::operator=(const c<T> &other)
{
    return *this;
}

template<typename T> 
c<T> &c<T>::operator=(c<T> &&other)
{
    return *this;
}

} /* namespace tags */

#endif /* TEMPLATED_CLASS_HPP_ */
