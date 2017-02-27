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

#include "functions/function.hpp"
#include "tags/struct.hpp"
#include "tags/templated_class.hpp"

/* clang-format off */




namespace p {
    int x = 0;
    
namespace p {
    int x = 0;
    
}
}

void test_tags()
{
    tags::s s1;
    tags::s s2 = tags::s();
    tags::s s3 = tags::s(s1);
    tags::s s4 = *tags::s().as_ptr();
    tags::s *s5 = tags::s::create();
    
    tags::s::reference s6 = s1;
    tags::s::pointer s7 = s6.as_ptr();
    
    typedef tags::s t;
    
    t t1 = *s7;
    auto t2 = t();
    
    (void) t2;
    
    delete s5;
    
    tags::c<struct tags::s> c1;
    tags::c<class tags::c<int>> c2;
    tags::c<int> c3 = tags::c<int>();
    tags::c<double> c4 = tags::c<double>();
    tags::c<bool> c5;
    
    (void) c5;
    
    auto b1 = c1 == c2;
    auto b2 = functions::f(c3, c4);
    
    using functions::f;
    auto b3 = f(c1, c3);
    
    (void) b1;
    (void) b2;
    (void) b3;
}

void test_namespaces()
{
    
}

void test_variables()
{
    
}

void test_macro()
{
#define M 100
#if defined(M)
    #define N (M)
#else
    #define N 100
#endif
    
#ifdef M
    auto s = tags::s();
    (void) s;
#endif
    
#undef M
#define M 1000
    auto c1 = tags::c<decltype(M)>();
    auto c2 = tags::c<decltype(N)>();
    
    (void) c1;
    (void) c2;
#undef M
}

void test_functions()
{
    auto f1 = functions::g;
    int (*f2)(int, int) = f1;
    
    (void) f1(1, 2);
    (void) f2(1, 2);
    (void) functions::g(1, 2);
    
    using functions::g;
    
    f1 = g;
    f2 = g;
    
    (void) f1(1, 2);
    (void) f2(1, 2);
    (void) g(1, 2);
    
    auto f3 = functions::f<int, int>;
    (void) f3(tags::c<int>(), tags::c<int>());
    
    tags::s().f();
    
    auto s = tags::s();
    s.f();
    
    void (tags::s::*f4)() const = &tags::s::f;
    
    (s.*f4)();
    (s.as_ptr()->*f4)();
    (*s.as_ptr().*f4)();
    
}

int main(void)
{
    test_functions();
    test_macro();
    test_namespaces();
    test_tags();
    test_variables();
    
//     namespace o = n;
//     using namespace o;

    
//     p::x = 42;
//     p::p::x = 42;
//     
//     namespace qq = p::p;
//     using qq::x;
//     x = 0;
    
    return 0;
}

/* clang-format on */

