// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#ifndef TESSERACT_HPP
#define TESSERACT_HPP

#include <iostream>
#include <leptonica/allheaders.h>
#include <string>
#include <tesseract/baseapi.h>

// using namespace std;

using std::cerr;
using std::cout;
using std::endl;

typedef tesseract::TessBaseAPI TBAPI;

class Tesseract {
public:
  Tesseract();
  ~Tesseract();
  string readImage(const string &);
  int setLanguage(const string &);

private:
  TBAPI *api;
};

#endif // TESSERACT_HPP
