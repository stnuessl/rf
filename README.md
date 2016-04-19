# rf - refactoring for C and C++

rf is a command-line tool capable of refactoring C and C++ source code.

# Overview
* [rf - refactoring for C and C++](https://github.com/stnuessl/rf#rf---refactoring-for-c-and-c)
* [Overview](https://github.com/stnuessl/rf#overview)
    * [Motivation](https://github.com/stnuessl/rf#motivation)
    * [Advantages](https://github.com/stnuessl/rf#advantages)
    * [Disadvantages](https://github.com/stnuessl/rf#disadvantages)
    * [Project Status](https://github.com/stnuessl/rf#project-status)
        * [What's supported right now?](https://github.com/stnuessl/rf#whats-supported-right-now)
        * [What might be supported in the future?](https://github.com/stnuessl/rf#what-might-be-supported-in-the-future)
        * [What is __not__ supported?](https://github.com/stnuessl/rf#what-is-not-supported)
    * [Installation](https://github.com/stnuessl/rf#installation)
        * [Dependencies](https://github.com/stnuessl/rf#dependencies)
            * [Arch Linux](https://github.com/stnuessl/rf#arch-linux)
        * [Compiling](https://github.com/stnuessl/rf#compiling)
    * [Usage](https://github.com/stnuessl/rf#usage)
        * [Refactoring rf's own source code](https://github.com/stnuessl/rf#refactoring-rfs-own-source-code)
            * [Refactoring Tags](https://github.com/stnuessl/rf#refactoring-tags)
            * [Refactoring Functions](https://github.com/stnuessl/rf#refactoring-functions)
            * [Refactoring Variables](https://github.com/stnuessl/rf#refactoring-variables)
        * [Refactoring examples](https://github.com/stnuessl/rf#refactoring-examples)
            * [Inherited functions](https://github.com/stnuessl/rf#inherited-functions)
            * [Overridden functions](https://github.com/stnuessl/rf#overridden-functions)
            * [Overlapping qualifiers](https://github.com/stnuessl/rf#overlapping-qualifiers)
        * [Setting up rf for a project](https://github.com/stnuessl/rf#setting-up-rf-for-a-project)
        * [Creating a Compilation Database using CMake](https://github.com/stnuessl/rf#creating-a-compilation-database-using-cmake)
    * [Code Breakage](https://github.com/stnuessl/rf#code-breakage)
    * [Bugs and Bug Reports](https://github.com/stnuessl/rf#bugs-and-bug-reports)

## Motivation

My favorite IDE lacks a good refactoring tool which more often then not breaks
my code resulting in me going manually through the sources and fixing stuff
which should not be broken. That is the reason why I want to try to build a
reliable refactoring tool which just works. By avoiding any graphical user 
interface this program may help other developers experiencing similiar issues
with their IDE or editor. This also enables the user the specify more than
one entity to be refactored at the same time which can't be usually be done
with refactoring tools provided by IDEs. 

## Advantages

* no graphical user interface needed
* capable of running multiple refactorings with one invocation
* fully parses the source code and makes sure it is correct before refactoring

## Disadvantages 

* getting a working compilation database can be tricky
* slow, especially for big C++ projects

## Project Status

The project is in development and not much more than a simple prototype.
I don't expect it the produce a correct program after refactoring.

### What's supported right now?

* Renaming tags, e.g. structs, classes, and enums
* Renaming functions including class methods
* Renaming (class) variables 
* Renaming namespaces
* Renaming macros
* Removing unused include directives

However, in all cases the corresponding entity should __NOT__ lie in
anonymous namespaces.

### What might be supported in the future?

* Supporting anonymous namespaces
* Adding a column specifier for victim qualifiers
* Refactoring typedef's

### What is __not__ supported?

Constructs where a scoped variable overshadows another variable and both
corresponding declarations happen on the __same__ line, e.g:

```cpp
{ int a = 0; if (...) { int a = 1; function(a); } }
```

If the declarations take place on different lines you can specifiy the line
number of the declaration you want to be refactored:

```
    $ rf --variable MyFunction::MyVar::42=MyNewVar
```

The problm above could easily be fixed with an optional column specification. 
This may be added in the future.


__$ rf --tag a=b__ will produce an incorrect program for something like
the following copy constructor:

```cpp
    template <typename T> class a {
    public:
        a(const class a &other);
                ^(1)  ^(2)
    };
```

For some reason I cannot retrieve the correct source location at (2) but
only the one at (1). However, the more common case should work:

```cpp
    template <typename T> class a {
    public:
        a(const a &other);
    };
```

## Installation

This section describes the installation process for rf. 

### Dependencies

* g++/clang++ supporting at least C++11
* make
* git
* [llvm](http://llvm.org/) and [clang](http://clang.llvm.org/) 3.7.1

#### Arch Linux

```
    # pacman -Syu llvm clang gcc make git
```

### Compiling

```
    $ git clone https://github.com/stnuessl/rf
    $ cd rf/
    $ make
    $ su -c 'make install'
```

## Usage

```
    $ rf --help
```

### Refactoring rf's own source code

This section shows how to refactor __rf's__ own source code. This shall help
to understand how to use __rf__ as all the examples are expected to work
as hands on examples. First you need to install __rf__ as described
in section [Installation](https://github.com/stnuessl/rf#Installation).
For rf to be capable of refactoring its own source code you need to create a 
compilation database named _compile_commands.json_. This can be done with

```
    $ make compile_commands.json
```

That's it. __rf__ should be ready for use with its own source code.

#### Refactoring Tags

The following command will change the name of the two classes _Refactorer_
and _NameRefactore_.

```
    $ rf --tag Refactorer=MyNewName,NameRefactorer=MyOtherNewName
```

#### Refactoring Functions

Basically, renaming functions and methods is the same as refactoring tags:
just exchange the _--tag_ argument with _--function_. So lets make this
example a little more interesting.

The file _src/Refactorers/NameRefactorer.hpp_ contains the following lines of 
code (line numbers may vary):

```cpp
    class NameRefactorer : ... {
    ...
47 |    void setReplacementQualifier(const std::string &Repl);
48 |    void setReplacementQualifier(std::string &&Repl);
    ...
    };
```
A normal _rf_ invocation could be:

```
    $ rf --function NameRefactorer::setReplacementQualifier=MyNewFunctionName
```

which, of course, will produce the correct lines of code

```cpp
    class NameRefactorer : ... {
    ...
47 |    void MyNewFunctionName(const std::string &Repl);
48 |    void MyNewFunctionName(std::string &&Repl);
    ...
    };
```

However, __rf__ can distinguish between the two function declarations, so the
following could also be done:

```
    $ rf --function NameRefactorer::setReplacementQualifier::47=MyNewFunctionName
```
which will produce (also the correct lines of code):
```cpp
    class NameRefactorer : ... {
    ...
47 |    void MyNewFunctionName(const std::string &Repl);
48 |    void setReplacementQualifier(std::string &&Repl);
    ...
    };
```

This might be useful if one want to remove an overloaded function in favor of
a new function with a more expressive name.


#### Refactoring Variables

Let's make this example straight forward again. The class _IncludeRefactorer_
contains a class variable named _IncludeMap_. To change its name one could run

```
    $ rf --variable IncludeRefactorer::_IncludeMap=MyNewVarName
```

What if you want to rename the name of a local variable inside a function?
The file _src/Refactorers/NameRefactorer.cpp_ contains a function _rCopy_
with a local variable _n_. To refactor it, run:
```
    $ rf --variable rCopy::n=MyNewLocalVarName
```
And as a major performance improvement, since you 100% know that this variable
cannot be referenced in another file, you can tell __rf__ to only consider
_src/Refactorers/NameRefactorer.cpp_ for refactoring.

```
    $ rf --variable rCopy::n=MyNewLocalVarName src/Refactorers/NameRefactorer.cpp
```


### Refactoring examples

This subsection shows various examples on how to refactor certain code parts
to achieve the desired results. 

#### Inherited functions

Consider the following piece of code.
```cpp
    struct base { void work() { }; };
    struct derived : public base { };
    int main() { derived().work(); }
```
Further assume you want to refactor the function "work" so the _main_ reads
```cpp
    int main() { derived().run(); }
```
Running __rf --function derived::work=run__ won't find anything to refactor
because the function is inherited from _base_. The correct way of refactoring 
this is by running __rf --function base::work=run__.

#### Overridden functions

Consider the following piece of code.
```cpp
    struct base { virtual void run() { }; };
    struct derived : public base { virtual void run() override { ; }; };
    int main() {
        base *x = new derived();
        x->run();
    }
```
Further assume you want to refactor _derived::run_.
Again, as above, you will have to run __tq --function base::run=work__ since
_derived::run_ is an overriding function and you can't refactor _derived::run_
without refactoring _base::run_.

#### Overlapping qualifiers

Consider the following piece of code.
```cpp
1 |     void f(int value) { }
2 |     void f(double value) { }
3 |     int main() { f(0); f(0.0); }
```

The are two overlapping qualifiers: f and f::value.
There are multiple ways to refactor this which are shown in the following.
The resulting program is shown immediatley after the invoked __rf__ command.

1. __$ tq --function f=ff__
```cpp
1 |     void ff(int value) { }
2 |     void ff(double value) { }
3 |     int main() { ff(0); ff(0.0); }
```
2. __$ tq --function f::1=ff__
```cpp
1 |     void ff(int value) { }
2 |     void f(double value) { }
3 |     int main() { ff(0); f(0.0); }
```
3. __$ tq --function f::2=ff__
```cpp
1 |     void f(int value) { }
2 |     void ff(double value) { }
3 |     int main() { f(0); ff(0.0); }
```
4. __$ tq --variable f::value=val__
```cpp
1 |     void f(int val) { }
2 |     void f(double val) { }
3 |     int main() { f(0); ff(0.0); }
```
5. __$ tq --variable f::value::1=val__
```cpp
1 |     void f(int val) { }
2 |     void f(double value) { }
3 |     int main() { f(0); ff(0.0); }
```
6. __$ tq --variable f::value::2=val__
```cpp
1 |     void f(int value) { }
2 |     void f(double val) { }
3 |     int main() { f(0); ff(0.0); }
```



### Setting up rf for a project

This section shows how to set up __rf__ for use from within a project. 
[llvm](https://github.com/llvm-mirror/llvm) will be used as a real world 
example.

```
    $ git clone https://github.com/llvm-mirror/llvm
    $ mkdir llvm/build
    $ cd llvm/build
    $ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../
    $ cp compile_commands.json ../
```

llvm needs to generate some files which are then included from source 
files. This can probably be done more efficient but this will do the job.

```
    $ make -j4
```

Note that for some reason the Opts.inc is still missing in an include.
Good thing it was created, meaning a quick fix for this problem is to run
```
    $ cp ./unittests/Option/Opts.inc ../unittests/Option/
```

The __compile_commands.json__ needs (at least on my machine) to be adjusted.
The clang frontend has its own 
[builtin includes](http://clang.llvm.org/docs/FAQ.html#i-get-errors-about-some-headers-being-missing-stddef-h-stdarg-h) 
which are not located in _/usr/include/_ but instead in 
_/usr/lib/clang/$(llvm-config --version)/include_. To help with this task 
there is a script located in _rf/utils_. The following command will fix the 
__compile_commands.json__ file.

```
    python fixdb.py --pretty -f /path/to/llvm/compile_commands.json -- -I/usr/lib/clang/$(llvm-config --version)/include
```

The next command will remove some warnings 
(unknown warning flag and language extension) generated by the frontend. This is
just a cosmetic change but helps to minimize visual clutter when running __rf__.
```
    python fixdb.py --pretty -f /path/to/llvm/compile_commands.json --remove -- -pedantic -Wno-maybe-initialized
```

__rf__ should be usable now for refactoring the 
[llvm](https://github.com/llvm-mirror/llvm) project.

```
    $ rf --tag llvm::IntrusiveRefCntPtr=intrusive_ref_cnt_ptr
```

### Creating a Compilation Database using CMake
    
```
    $ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../
```

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
    int main() { class b var = b(); }
```

This is a valid program which will compile and unlike the example above it won't
crash but the behaviour may be very well different than before.
The function "b()" now overshadows the constructor call from "class b".
So before refactoring the main function calls the constructor of class b.
After refactoring the main function will call the function "b()".

## Bugs and Bug Reports

You've found a bug in __rf__? That's actually great (at least for me).
I really want to build a reliable tool and I don't expect things to go smoothly
in the short- and midterm. This means I really appreciate any bug reports
as long as they contain the following information:

* The __rf__ invocation which does __not__ produce the desired result.
* A __minimal__ working program (the shorter the better) 
which the provided command fails to refactor.
* The correctly refactored program as it was intended
* A trivial and working _compile_commands.json_, like:
```
[{
        "file": "main.cpp",
        "directory": "/path/to/directory/",
        "command": "g++ -std=c++11 -o test -I/usr/lib/clang/3.7.1/include /path/to/directory/main.cpp"
}]
```

If you succeed in providing these requirements I will try hard to fix the 
experienced bug.