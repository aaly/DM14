// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#ifndef STRING_HPP
#define STRING_HPP

#include <string>
#include <sstream>

using namespace std;

//some string operationgs !

int		str_size(const string&);
char	str_at(const string&, const unsigned int&);
bool	str_isEmpty(const string&);
string	str_sub(const string&, const int&, const int&);

string	numberToStr(int);
string  floatToStr(float);
int		strToNumber(const string&);
float	strToFloat(const string&);


string	getToken(const string&, const char&, const int&);
int		tokenCount(const string&, const char&);

#endif // STRING_HPP
