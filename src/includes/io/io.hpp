// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license

#ifndef IO_HPP
#define IO_HPP


#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <stdlib.h>
using namespace std;

int spill (const string&); 	// for strings
int	nspill(const string&);	// for new line ( if parameter is empty , then just print new line ) , same as above but with endl
int spill (const int&); 	
int	nspill(const int&);		
int spill (const double&);
int	nspill(const double&);
int spill (const char&);
int	nspill(const char&);

// we could use template ofcourse but we will have to add that to the parser , meh :D
int get(int&);
int get(char&);
int get(double&);
int get(string&);


int	dm14about();
int	dm14version();
int	dm14license();

int	Exit(int);

#endif // IO_HPP
