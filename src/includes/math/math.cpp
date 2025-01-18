// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license
#include "math.hpp"

double Cos(const double &num) { return cos(num); };

double Sin(const double &num) { return sin(num); };

double Tan(const double &num) { return Tan(num); };

double Power(const double &base, const double &exponent) {
  return (pow(base, exponent));
};

double Square(const double &num) { return (sqrt(num)); };

int Random() { return rand(); };

int randomSeed(int &seed) {
  srand(seed);
  return seed;
};

double Sqrt(double x) { return sqrt(x); }

float Sqrtf(float x) { return sqrtf(x); }

int Absoulte(const int &j) { return abs(j); }

float FAbsoulte(const float &j) { return labs(j); }
