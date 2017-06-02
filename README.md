# rf - refactoring for C and C++

rf is a command-line tool capable of refactoring C and C++ source code.

## Overview
* [rf - refactoring for C and C++](README.md#rf---refactoring-for-c-and-c)
    * [Overview](README.md#overview)
    * [Motivation](README.md#motivation)
    * [Advantages](README.md#advantages)
    * [Disadvantages](README.md#disadvantages)
    * [Project Status](README.md#project-status)
        * [What's supported right now?](README.md#whats-supported-right-now)
        * [What is not supported?](README.md#what-is-not-supported)
            * [Copy constructor of a templated record with elaborated type specifier](README.md#copy-constructor-of-a-templated-record-with-elaborated-type-specifier)
            * [Non self-contained macros](README.md#non-self-contained-macros)
    * [Installation](README.md#installation)
        * [Dependencies](README.md#dependencies)
            * [Arch Linux](README.md#arch-linux)
        * [Compiling](README.md#compiling)
    * [Usage](README.md#usage)
        * [Attention](README.md#attention)
        * [Setting up rf for a project](README.md#setting-up-rf-for-a-project)
        * [Refactoring rf's own source code](README.md#refactoring-rfs-own-source-code)
            * [Refactoring Tags](README.md#refactoring-tags)
            * [Refactoring Functions](README.md#refactoring-functions)
            * [Refactoring Variables](README.md#refactoring-variables)
            * [Refactoring include paths](README.md#refactoring-include-paths)
        * [Further Refactoring examples](README.md#further-refactoring-examples)
            * [Inherited functions](README.md#inherited-functions)
            * [Overridden functions](README.md#overridden-functions)
            * [Overlapping qualifiers](README.md#overlapping-qualifiers)
            * [Overshadowing declarations](README.md#overshadowing-declarations)
            * [Common prefixes](README.md#common-prefixes)
            * [Batching](README.md#batching)
        * [Creating a Compilation Database using CMake](README.md#creating-a-compilation-database-using-cmake)
        * [Creating a Compilation Database using Make](README.md#creating-a-compilation-database-using-make)
    * [Bugs and Bug Reports](README.md#bugs-and-bug-reports)

## Motivation

My favorite IDE lacks a good refactoring tool which more often then not breaks
my code resulting in me going manually through the sources and fixing stuff
which should not be broken. That is the reason why I want to try to build a
reliable refactoring tool which just works. By avoiding any graphical user 
interface this program may help other developers experiencing similiar issues
with their IDE or editor. Also, this enables the user the specify 
multiple entities for refactoring while running only a single tool invocation.
This usually can't be done with refactoring tools provided by most IDEs.

## Advantages

* No graphical user interface --> works with any IDE / code editor
* Capable of running multiple refactorings with one invocation
* Fully parses the source code and makes sure it is correct before refactoring

## Disadvantages 

* Getting a working compilation database can be tricky
* Slow, especially for big C++ projects

## Project Status

Most of the features I wanted are implemented and are working quite well. 
Feel free to grab __rf__ and try it out yourself.

### What's supported right now?

__rf__ can refactor the following things:

* classes, enums, structs and type aliases
* functions and class methods
* variables and class variables
* enum constants
* namespaces
* macros

### What is not supported?

This section describes some of the known scenarios where __rf__ will fail
to correctly refactor the source code. 

#### Copy constructor of a templated record with elaborated type specifier

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
only the one at (1). However, the more common cases should work:

```cpp
    template <typename T> class a {
    public:
        a(const a &other);
    };
```
```cpp
    template <typename T> class a {
    public:
        a(const a<T> &other);
    };
```
#### Non self-contained macros

Given the command __rf --variable main::a=var__,
I really think it is quite unfeasible to support something like the following:

```cpp
#define PREINC ++a
int main() {
    int a = 0;
    PREINC;
    PREINC;
}
```
The preprocessor cannot possible know if _++a_ inside _PREINC_ has to be 
refactored. Later on, while traversing the AST it is quite hard to know for sure
that _++a_ is exactly meant for the variable _a_ in _main_. Consider the 
possibility of another function existing with a variable _a_ and the macro 
_PREINC_ being used. How would one detect such a scenario?
However, the more sane case is supported:

```cpp
#define PREINC(_a) ++_a
int main() {
    int a = 0;
    PREINC(a);
    PREINC(a);
}
```

## Installation

This section describes the installation process for rf. 

### Dependencies

__rf__ has only two real dependencies:

* [llvm](http://llvm.org/) and [clang](http://clang.llvm.org/) 3.9.0

However, since you have to install and compile the program from source you will 
need some other tools too.

* g++ / clang++ supporting at least C++11
* make
* git

#### Arch Linux

The above listed dependencies can be installed with the following _pacman_
invocation:

```
    # pacman -Syu llvm clang gcc make git
```

### Compiling

First you need to download the source code from this repository and 
change to the project directory. Run:
```
    $ git clone https://github.com/stnuessl/rf
    $ cd rf/
```
After that you need to compile the source code. This can be done with:
```
    $ make
```
If you want to compile the program using _clang++_ change the command to
```
    $ make CXX=clang++
```

The last command installs the __bash-completion__ and the __rf__ binary on your
system:
```
    $ make install
```

## Usage

You can always have a look at the help message. Use:
```
    $ rf --help
```
If you forgot the name of the flag you can also just type __rf --[tab][tab]__
and the bash completion will suggest all available options.

### Attention

Before invoking __rf__ make absolutely sure that the new name does not conflict
with an already existing one. The refactoring will succeed but the resulting
program won't compile or its behaviour may have been altered. 
E.g. consider the following program:

```cpp
    class a {};
    a b() { return a(); }
    int main() { b(); }
```
Running __$ rf --tag a=b__ with the following command will produce:

```cpp
    class b {};
    b b() { return b(); }
    int main() { b(); }
```

The function _b()_ is now a recursive function without a stopping condition
and the program will crash at runtime.
There are no safety guards implemented to avoid __rf__ invocations which
could produce such behaviour. 

Also, having your code under version control is recommended before invoking 
__rf__.

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

[llvm](https://github.com/llvm-mirror/llvm)
needs to generate some files which are then included from source 
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
there is a script _fix-jcdb.py_ located in _rf/utils_. 
The following command will fix the __compile_commands.json__ file.

```
    python fix-jcdb.py -i --add I/usr/lib/clang/$(llvm-config --version)/include -- compile_commands.json
```

The next command will remove some warnings 
(unknown warning flag and language extension) generated by the frontend. This is
just a cosmetic change but helps to minimize visual clutter when running __rf__.
```
    python fix-jcdb.py -i --discard pedantic Wno-maybe-uninitialized -- compile_commands.json
```

Notice how the leading dashes for these flags were omitted. _fix-jcdb.py_ will
automatically add them. Otherwise the python _ArgumentParser_ would not be able 
to differentiate between all the passed flags.

__rf__ should be usable now for refactoring the 
[llvm](https://github.com/llvm-mirror/llvm) project.

```
    $ rf --tag llvm::IntrusiveRefCntPtr=intrusive_ref_cnt_ptr
```

Note that this project is fairly large and a refactoring run with __rf__
may take a very long time (about __30__ min).

### Refactoring rf's own source code

This section shows how to refactor __rf's__ own source code. This shall help
to understand how to use __rf__ as all the examples are expected to work
as hands on examples. First you need to install __rf__ as described
in section [Installation](https://github.com/stnuessl/rf#Installation).
For rf to be capable of refactoring its own source code you need to create a 
compilation database named _compile_commands.json_. This can be done with

```
    $ make compile-commands
```

That's it. __rf__ should be ready for use with its own source code.

#### Refactoring Tags

The following command will change the name of the two classes _Refactorer_
and _NameRefactorer_.

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
A normal __rf__ invocation could be:

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

Let's make this example straight forward again. The class _Refactorer_
contains a class variable named _CompilerInstance_. To change its name one 
could run

```
    $ rf --variable Refactorer::CompilerInstance_=MyNewVarName
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

#### Refactoring include paths

Sometimes it is necessary to change the name of a file leaving a lot of include
directives invalid. If one knows of such a file name change beforehand __rf__
can be used to automatically adjust the include directives.
The following include directive can be changed in the following ways:

```
    #include <header.h>
```

* with __$ tq --include "header.h=new-header.h"__ to 
```
    #include <new-header.h>
```
* _or_ with __$ tq --include "<header.h>=\"new-header.h\""__ to 
```
    #include "new-header.h"
```

Of course, the following can also be done:
```
    #include "header.h"
```
* with __$ tq --include "header.h=new-header.h"__ to 
```
    #include "new-header.h"
```

* _or_ with __$ tq --include "\"header.h\"=<new-header.h>"__ to 
```
    #include <new-header.h>
```

### Further Refactoring examples

This subsection shows various examples on how to refactor certain code parts
to achieve the desired results. 

#### Inherited functions

Consider the following piece of code.
```cpp
    struct base { void work() { } };
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
    struct base { virtual void run() { } };
    struct derived : public base { virtual void run() override { } };
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

* __$ tq --function f=ff__
```cpp
1 |     void ff(int value) { }
2 |     void ff(double value) { }
3 |     int main() { ff(0); ff(0.0); }
```
* __$ tq --function f::1=ff__
```cpp
1 |     void ff(int value) { }
2 |     void f(double value) { }
3 |     int main() { ff(0); f(0.0); }
```
* __$ tq --function f::2=ff__
```cpp
1 |     void f(int value) { }
2 |     void ff(double value) { }
3 |     int main() { f(0); ff(0.0); }
```
* __$ tq --variable f::value=val__
```cpp
1 |     void f(int val) { }
2 |     void f(double val) { }
3 |     int main() { f(0); f(0.0); }
```
* __$ tq --variable f::value::1=val__
```cpp
1 |     void f(int val) { }
2 |     void f(double value) { }
3 |     int main() { f(0); f(0.0); }
```
* __$ tq --variable f::value::2=val__
```cpp
1 |     void f(int value) { }
2 |     void f(double val) { }
3 |     int main() { f(0); f(0.0); }
```

#### Overshadowing declarations

The previous example showed how one can deal with the same qualified names
on different lines of code. This example demonstrates how to deal with
this if the qualified names where on the same line of code.


Given the following line of code.

```cpp
243 |    int i = 0; if (<expr>) { int i = 42; f(i); }
             ^    ^    ^    ^    ^    ^    ^    ^
     ...     5    10   15   20   25   30   35   40          <--- Column Numbers
```
there are three possbile ways to refactor this (surrounding qualifiers are 
omitted):

* __$ tq --variable i=ii__
```cpp
243 |    int ii = 0; if (<expr>) { int ii = 42; f(ii); }
```

* __$ tq --variable i::243:5=ii__
```cpp
243 |    int ii = 0; if (<expr>) { int i = 42; f(i); }
```

* __$ tq --variable i::243:30=ii__
```cpp
243 |    int i = 0; if (<expr>) { int ii = 42; f(ii); }
```

#### Common prefixes

In C code the following is a common pattern:
```cpp
    struct a {};
    void a_init(struct a *a);
    void a_run(struct a *a);
    void a_destroy(struct a *a);
```
If you want to refactor the struct _a_ it only makes sense to also refactor the
functions to adapt to the change. This can easily be done by 
running:

```
    $ rf --tag a=b --function a_*=b
```
This command uses the '*' character to signal __rf__ that one does not care what
follows after the '_a_' prefix in a function, effectively refactoring all '_a_'
prefixes to '_b_' prefixes. Prefix refactoring works with any mode 
(tag, function, variable, etc.) available in __rf__.

With that said, the above command will produce the following piece of code:

```cpp
    struct b {};
    void b_init(struct b *a);
    void b_run(struct b *a);
    void b_destroy(struct b *a);
```

#### Batching

Specifying a lot of replacements proves unfeasible at some point. __rf__ allows
you to batch your replacements in a simple yaml file which can be read in.
Here is a simple template which can be used:

```
    ---
    Functions:       
        - 'f=ff'
    Macros:          
        - 'M=MM'
    Tags:            
        - 's=ss'
        - 'c=cc'
    Variables:
        - 'v=vv'
    Includes:
        - 'i=ii'
    Namespaces:
        - 'n=nn'
    ...
```

Alternatively, one can create such a file with:

```
    $ rf --to-yaml --function f=ff --macro M=MM --tag = s=ss,c=cc [...] > my-replacements.yaml
```

Additional replacements can be easily added later on.
The file can be read in with:

```
    $ rf --from-file my-replacements.yaml
```

### Creating a Compilation Database using CMake

To create a _compile_commands.json_ with CMake simply run:
    
```
    $ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../
```
Unfortunately this compilation database does not work out of the box with 
__rf__ on my system. Have a look at this 
[section](https://github.com/stnuessl/rf#setting-up-rf-for-a-project) 
to resolve the issue.


### Creating a Compilation Database using Make

You might want to look at this project 
[bear - Build EAR](https://github.com/rizsotto/Bear).


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
    "directory": "/path/to/dir/",
    "command": "g++ -std=c++11 -o test -I/usr/lib/clang/3.9.0/include /path/to/dir/main.cpp"
}]
```

If you succeed in providing these requirements I will try hard to fix the 
experienced bug.
