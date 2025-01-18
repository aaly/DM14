/**
@file             token.hpp
@brief            token
@details          token class, represents the structure of a token produced by
the scanner to be consumed by the parser and other later phases
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-present
@version          1.1a
@copyright        See file "license" for bsd license
*/

#ifndef DM14_TOKEN_HPP
#define DM14_TOKEN_HPP

#include <iostream>
#include <stdlib.h>

namespace DM14 {
using namespace std;

class token {
public:
  token() {
    columnNumber = -1;
    lineNumber = -1;
  };

  void print() {
    cerr << "Type : " << type << endl;
    cerr << "Value : " << value << endl;
  }

  int columnNumber; // Number of character on the current line
  int lineNumber;   // Number of the current line
  string value;     // Value of the token
  string type;      // Value type
};
} // namespace DM14

#endif // DM14_TOKEN_HPP
