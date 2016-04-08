# rf - refactoring for C and C++

rf is a command-line tool capable of refactoring C and C++ source code.

## Motivation

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

* Renaming tags, e.g. structs, classes and enums
* Renaming functions including class methods
* Renaming (class) variables 

However, in all cases the corresponding entity should __NOT__ lie in
anonymous namespaces.

### What might be supported in the future?

* Renaming template parameters
* Supporting anonymous namespaces
* One tool run for multiple refactoring procedures, at the moment this is done
with multiple passes. This would be a huge performance improvement.

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

### Arch Linux

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
to understand how to use __rf__. First you need to install __rf__ as described
in section [Installation](https://github.com/stnuessl/rf#Installation).
For rf to be capable of refactoring its own source code you need to create a 
compilation database named compile_commands.json. This can be done with

```
    $ make compile_commands.json
```

That's it. __rf__ should be ready for use in its own source code. 

More will follow.


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
without refactoring _base::run_, if you want to achieve the same program 
behaviour as before.

More to follow.


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
