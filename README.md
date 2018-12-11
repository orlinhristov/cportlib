cportlib
========

A C++ library that manages parallel task processing in concurrent environment. It provides mechanism for asynchronous execution of multiple task handlers and invocation of callback handlers after task completion.

Visit https://github.com/orlinhristov/cportlib/wiki for more information about the library.

How to compile
==============
Define CPORT_HEADER_ONLY_LIB in the projects that use cportlib. If you use g++ compiler, you will also have to add -pthread option. For example:
:~$ g++ test.cpp -I "cportlib" -D "CPORT_HEADER_ONLY_LIB" -std=c++11 -pthread -Wall -o test
