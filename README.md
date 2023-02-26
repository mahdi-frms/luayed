# Luayed

A Lua interpreter written in C++.

## Build

Build requirements:

+ CMake
+ GCC
+ Lua

After cloning the repo:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Usage

Luayed can run as REPL:

```
$ ./build/luayed
```

To run a file:

```
$ ./build/luayed script.lua arg1 arg2 ...
```

## Running The Tests

```
$ ./build/luaytest
```

## Disclaimer

Luayed is still under development, and currently misses the following features:

+ Variable attributes
+ Standard library
+ Debug info & debugger
+ A documented C++ API
+ Metatables

Any contribution and reporting of issues is welcome.