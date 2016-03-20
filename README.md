# rf - refactoring for C and C++

rf is a command-line tool that is able to refactor names C and C++ source files.

## Motivation

## Project Status

The project is in development and not much more than a simple prototype.
I don't expect it the produce a correct program after refactoring.

## Installation

### Dependencies

### Compiling

## Usage

## Code Breakage

// TODO

The following example illustrates a program which will crash after a refactoring
run with __rf__.
Consider the following valid program:
```cpp
class a {};
a b() { return a(); }
int main() { b(); }
```
Running __rf__ with the following command will produce a program which will
crash on __runtime__:
```
$ rf --tag a=b
```
Refactored (crashing) program:
```cpp
class b {};
b b() { return b(); }
int main() { b(); }
```
The code refactoring produced a recursive function b that lacks a stopping 
condition and will therefore overflow the runtime stack. However, I assume
that this problem should be __very rare__ and won't occur in most 
real world programs.
