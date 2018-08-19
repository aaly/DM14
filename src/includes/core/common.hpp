// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license
#ifndef	COMMON_H
#define	COMMON_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include "Array.hpp"

using namespace std;

int displayDebug(const string& message);
int displayInfo(const string& message);

int displayInfo(const string& fName, const int& line, const int& column, const string& message);
int displayInfo(const string& message);

int displayWarning(const string&, const int&,const int&,const string&);
int displayWarning(const string&);

int displayError(const string& message);
int displayError(const string&, const int&,const int&,const string&, bool Exit = true);



#endif	// COMMON_H
