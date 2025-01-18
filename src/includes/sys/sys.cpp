// Copyright (c) 2010, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license
#include "sys.hpp"

int Fork() {
  pid_t pid = fork();
  return pid;
};

int Execute(const string &path, const string &args) {
  return execl(path.c_str(), args.c_str());
};

int System(const string &path, const string &args) {
  return system((path + " \"" + args + "\"").c_str());
};
