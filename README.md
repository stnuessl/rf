# rf - refactoring for C and C++

rf is a command-line tool capable of refactoring C and C++ source code.

## Motivation

## Project Status

The project is in development and not much more than a simple prototype.
I don't expect it the produce a correct program after refactoring.

### What's supported right now?

* Renaming Tags, e.g. structs, classes and enums
* Renaming Functions including class methods

However, in all cases the corresponding entity should __NOT__ lie in
anonymous namespaces.

### What might be supported in the future?

* Renaming variables names
* Renaming template parameters
* Supporting anonymous namespaces

## Installation

* g++/clang++ supporting at least C++11
* make

### Dependencies

* [llvm](http://llvm.org/) and [clang](http://clang.llvm.org/) 3.7.1

### Compiling

```
    $ git clone https://github.com/stnuessl/rf
    $ cd rf/
    $ make
    $ su -c 'make install'
```

## Usage

## Code Breakage

This section covers examples of source code which should __not__ be 
refactored with certain __rf__ invocations. Most if not all of those examples
should be very rare in real world programs.


The following example illustrates a program which will crash after a refactoring
run with __rf__.
Consider the following valid program:
```cpp
class a {};
a b() { return a(); }
int main() { b(); }
```
Running __$ rf --tag a=b__ with the following command will produce a 
program which will crash on __runtime__:
```cpp
class b {};
b b() { return b(); }
int main() { b(); }
```
The code refactoring produced a recursive function b that lacks a stopping 
condition and will therefore overflow the runtime stack.

The following two pieces of code illustrate a similiar case which is even more 
subtle since it won't crash the program. However, it still can alter its 
behaviour in unintended ways.
```cpp
class a {};
class a b() { class a a; return a; }
int main() { class a var = b(); }
```

Running __$ rf --tag a=b__ will produce:

```cpp
class b {};
class b b() { class b a; return a; }
}
int main() { class b var = b(); }
```

This is a valid program which will compile and unlike the example above it won't
crash but the behaviour may be very well different than before.
The function "b()" now overshadows the constructor call from "class b".
So before refactoring the main function calls the constructor of class b.
After refactoring the main function will call the function "b()".
