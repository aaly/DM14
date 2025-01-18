#ifndef SYS_HPP
#define SYS_HPP

// Copyright (c) 2010, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license

#include <stdio.h>  // printf, perror
#include <stdlib.h> // exit
#include <string>
#include <sys/types.h> // pid_t
#include <sys/wait.h>  // waitpid
#include <unistd.h>    // _exit, fork
using namespace std;

int Fork();
int Execute(const string &, const string &);
int System(const string &, const string &);
#endif
