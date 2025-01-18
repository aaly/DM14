// Copyright (c) 2017, <Abdallah Aly> <aaly90 [@] gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license

#include "io.hpp"

int spill(const string &str) {
  // check for ?VARNAME? , if true , then cout << original string till '?' <<
  // VARNAME << rest of string ... ;
  /*for ( unsigned int i =0; i < str.size(); i++)
  {
          if (str.at(i) == '?')
          {
                  if ( i == ( str.size() - 1 ) )
                  {
                          cout << str << flush;
                          break;
                  }
                  else
                  {
                          if ( str.at(i-1) == '\\' )
                          {
                                  continue;
                          }
                          i++;
                  }
          }
  }*/

  cout << str << flush;
  return 0;
};

int nspill(const string &str) {
  cout << str << endl << flush;
  return 0;
};

int spill(const int &str) {
  cout << str << flush;
  return 0;
}

int nspill(const int &str) {
  cout << str << endl << flush;
  return 0;
};

int spill(const double &str) {
  cout << str << flush;
  return 0;
}

int nspill(const double &str) {
  cout << str << endl << flush;
  return 0;
};

int spill(const char &str) {
  cout << str << flush;
  return 0;
}

int nspill(const char &str) {
  cout << str << endl << flush;
  return 0;
};

int get(int &Int) {
  cin >> Int;

  if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }

  return 0;
};

int get(char &Char) {
  cin >> Char;
  if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }
  return 0;
};

int get(double &Double) {
  cin >> Double;
  if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }

  return 0;
};

int get(string &String) {
  cin >> String;
  if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }
  return 0;
};

int dm14about() {
  cout << "Mission14 Programming Language :" << endl;
  cout << "\t a nice and easy programming language" << endl;
  cout << "\t syntax is very similar to C/C++" << endl;
  cout << "Programmed by : Abdallah Aly" << endl;
  cout << "See license file" << endl;
  return 0;
};

int dm14version() {
  cout << "M14 Version : 0.01 Pre-alpha" << endl;
  return 0;
};

int dm14license() {
  /*cout << "oops , should print the license xD" << endl;
  cout << "too many things todo,laters :D" << endl;*/
  cout
      << " Copyright (c) 2017, <Abdallah Aly> <aaly90 [@] gmail.com>" << endl
      << "All rights reserved." << endl
      << "Redistribution and use in source and binary forms, with or without"
      << endl
      << "modification, are permitted provided that the following conditions "
         "are met:"
      << endl
      << "* Redistributions of source code must retain the above copyright"
      << endl
      << "notice, this list of conditions and the following disclaimer." << endl
      << "* Redistributions in binary form must reproduce the above copyright"
      << endl
      << "notice, this list of conditions and the following disclaimer in the"
      << endl
      << "documentation and/or other materials provided with the distribution."
      << endl
      << "* Neither the name of the <organization> nor the" << endl
      << "names of its contributors may be used to endorse or promote products"
      << endl
      << "derived from this software without specific prior written permission."
      << endl
      << "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "
         "\"AS IS\" AND"
      << endl
      << "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, "
         "THE IMPLIED"
      << endl
      << "WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE "
         "ARE"
      << endl
      << "DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY"
      << endl
      << "DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL "
         "DAMAGES"
      << endl
      << "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR "
         "SERVICES;"
      << endl
      << "LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER "
         "CAUSED AND"
      << endl
      << "ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, "
         "OR TORT"
      << endl
      << "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE "
         "USE OF THIS"
      << endl
      << "SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE." << endl
      << endl
      << flush;
  return 0;
}

int Exit(int code) { exit(code); };
