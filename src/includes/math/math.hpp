#ifndef MATH_HPP
#define MATH_HPP
// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

double Cos(const double &);
double Sin(const double &);
double Tan(const double &);

double Power(const double &, const double &);

double Square(const double &);
float Sqrtf(float x);

int Random();
int randomSeed(int &);

int Absoulte(const int &j);
float FAbsoulte(const float &j);
#endif
