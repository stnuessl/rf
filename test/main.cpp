/*
 * Copyright (C) 2016  Steffen Nüssle
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

#define M 100
#define N (M)

namespace n {
struct a {
    a() {};
    a(const a &other) {};
    a(a &&other) {};
    ~a() {};
};
} // namespace n

namespace {

auto a0 = n::a();
    
} // namespace

template <typename T>
struct b {
    b() {};
    b(const b<T> &other) {};
    b(b<T> &&other) {};
#if 0
    // clang error? Unable to retrieve correct source location
    b(const class b &other) {};
    b(class b &&other) {};
#endif
    ~b() {};
    
    void run() const {};
    template <typename U> b(const b<U> &other) { run(); };
};

template <typename T> void f(b<T> b) {};

template <typename T, typename U>
bool operator==(const class b<T> &lhs, const class b<U> &rhs) { return true; };

template <typename T, typename U>
bool f(const b<T> &lhs, const b<U> &rhs) { return lhs == rhs; };

template <typename T> struct c;
template <typename T> struct c<const b<T>> {};


int main(void)
{
    b<struct n::a> v;
    b<struct b<int>> u;
    
    struct n::a a1 = n::a();
    n::a a2 = n::a(a0);
    
    using namespace n;
    a a3 = a(a());
    
    b<int> b1;
    struct b<double> b2;
    
    (void) f(b1, b2);
    
#undef M
#define M 1000
    f(b<decltype(M)>(), b<decltype(N)>());
#undef M
    
    return 0;
}
