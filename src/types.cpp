/**
@file            	types.cpp
@brief         	types
@details      	types, Part of DM14 programming language
@author      	AbdAllah Aly Saad <aaly90@gmail.com>
@date	        	2010-2018
@version		    1.1a
@copyright      See file "license" for bsd license
*/

#include "types.hpp"

namespace DM14 {
Array<DatatypeBase> datatypes;
std::vector<pair<string, int>> Operators;
std::vector<string> keywords;
fstream mapsFile;

namespace types {
// FIX105 add peek for /r/n for windows ?
//  add all operators to both single or binary ops and then modify functions for
//  searching, to search both lists.
int intSymbols() {
  keywords = {"with",      "use",        "while",     "for",       "case",
              "if",        "class",      "else",      "break",     "continue",
              "return",    "distribute", "reset",     "addparent", "struct",
              "setnode",   "backprop",   "channel",   "nodist",    "noblock",
              "recurrent", "extern",     "endextern", "link",      "slink",
              "global",    "thread"};

  Operators = {
      make_pair("\\", GENERALOPERATOR), make_pair(">", BINOPERATOR),
      make_pair(">>", BINOPERATOR),     make_pair("<", BINOPERATOR),
      make_pair("<<", BINOPERATOR),     make_pair("<-", GENERALOPERATOR),
      make_pair("->", BINOPERATOR),     make_pair(".", BINOPERATOR),
      make_pair("?", GENERALOPERATOR),  make_pair("~", GENERALOPERATOR),
      make_pair(":", GENERALOPERATOR),  make_pair(";", GENERALOPERATOR),
      make_pair(",", GENERALOPERATOR),  make_pair("[", GENERALOPERATOR),
      make_pair("]", GENERALOPERATOR),  make_pair("{", GENERALOPERATOR),
      make_pair("}", GENERALOPERATOR),  make_pair("(", GENERALOPERATOR),
      make_pair(")", GENERALOPERATOR),  make_pair("\'", GENERALOPERATOR),
      make_pair("#", GENERALOPERATOR),  make_pair("+", BINOPERATOR),
      make_pair("-", BINOPERATOR),      make_pair("*", BINOPERATOR),
      make_pair("/", BINOPERATOR),      make_pair("=", BINOPERATOR),
      make_pair("==", BINOPERATOR),     make_pair("!=", BINOPERATOR),
      make_pair("!", SINGLEOPERATOR),   make_pair("+=", BINOPERATOR),
      make_pair("-=", BINOPERATOR),     make_pair("*=", BINOPERATOR),
      make_pair("/=", BINOPERATOR),     make_pair("<", BINOPERATOR),
      make_pair("<=", BINOPERATOR),     make_pair(">", BINOPERATOR),
      make_pair(">=", BINOPERATOR),     make_pair("||", BINOPERATOR),
      make_pair("&&", BINOPERATOR),     make_pair("&", BINOPERATOR),
      make_pair("|", BINOPERATOR),      make_pair(".", BINOPERATOR),
      make_pair("::", BINOPERATOR),     make_pair("%", BINOPERATOR),
      make_pair("++", SINGLEOPERATOR),  make_pair("--", SINGLEOPERATOR),
      make_pair("@", COREOPERATOR)};

  DatatypeBase INTTYPE;
  INTTYPE.setID("int");
  INTTYPE.addOperator("@");
  INTTYPE.addOperator("+");
  INTTYPE.addOperator("++");
  INTTYPE.addOperator("-");
  INTTYPE.addOperator("--");
  INTTYPE.addOperator("/");
  INTTYPE.addOperator("*");
  INTTYPE.addOperator("!");
  INTTYPE.addOperator("*");
  INTTYPE.addOperator("");
  INTTYPE.addOperator("=");
  INTTYPE.addOperator("!=");
  INTTYPE.addOperator("+=");
  INTTYPE.addOperator("-=");
  INTTYPE.addOperator("*=");
  INTTYPE.addOperator("/=");
  INTTYPE.addOperator(">");
  INTTYPE.addOperator("<");
  INTTYPE.addOperator("<=");
  INTTYPE.addOperator(">=");
  INTTYPE.addOperator("&&");
  INTTYPE.addOperator("%");
  INTTYPE.addTypeValue("int");
  INTTYPE.addTypeValue("float");
  INTTYPE.addTypeValue("double");
  INTTYPE.addTypeValue("long");
  INTTYPE.Value = "0";
  datatypes.push_back(INTTYPE);

  DatatypeBase FLOATTYPE;
  FLOATTYPE.setID("float");
  FLOATTYPE.addOperator("@");
  FLOATTYPE.addOperator("/");
  FLOATTYPE.addOperator("*");
  FLOATTYPE.addOperator("+");
  FLOATTYPE.addOperator("++");
  FLOATTYPE.addOperator("-");
  FLOATTYPE.addOperator("--");
  FLOATTYPE.addOperator("*");
  FLOATTYPE.addOperator("!");
  FLOATTYPE.addOperator("");
  FLOATTYPE.addOperator("=");
  FLOATTYPE.addOperator("!=");
  FLOATTYPE.addOperator("+=");
  FLOATTYPE.addOperator("-=");
  FLOATTYPE.addOperator("*=");
  FLOATTYPE.addOperator("/=");
  FLOATTYPE.addOperator(">");
  FLOATTYPE.addOperator("<");
  FLOATTYPE.addOperator("<=");
  FLOATTYPE.addOperator(">=");
  FLOATTYPE.addOperator("&&");
  FLOATTYPE.addOperator("%");
  FLOATTYPE.addTypeValue("int");
  FLOATTYPE.addTypeValue("float");
  FLOATTYPE.addTypeValue("double");
  FLOATTYPE.addTypeValue("long");
  FLOATTYPE.CEquivalent.push_back("double");
  FLOATTYPE.CEquivalent.push_back("long");
  FLOATTYPE.Value = "0";
  datatypes.push_back(FLOATTYPE);

  DatatypeBase CHARTYPE;
  CHARTYPE.setID("char");
  CHARTYPE.addOperator("@");
  CHARTYPE.addOperator("=");
  CHARTYPE.addOperator("==");
  CHARTYPE.addOperator("!=");
  CHARTYPE.addOperator(">");
  CHARTYPE.addOperator("<");
  CHARTYPE.addOperator("<=");
  CHARTYPE.addOperator(">=");
  CHARTYPE.addTypeValue("int");
  CHARTYPE.addTypeValue("char");
  CHARTYPE.Value = "\\0";
  datatypes.push_back(CHARTYPE);

  DatatypeBase STRINGTYPE;
  STRINGTYPE.setID("string");
  STRINGTYPE.addOperator("=");
  STRINGTYPE.addOperator("+");
  STRINGTYPE.addOperator("+=");
  STRINGTYPE.addOperator("==");
  STRINGTYPE.addOperator("@");
  STRINGTYPE.addTypeValue("string");
  STRINGTYPE.addTypeValue("char");
  datatypes.push_back(STRINGTYPE);

  DatatypeBase GENETYPE;
  GENETYPE.setID("gene");
  GENETYPE.addOperator("=");
  GENETYPE.addOperator("==");
  GENETYPE.addTypeValue("int");
  GENETYPE.addTypeValue("float");
  GENETYPE.addTypeValue("string");
  datatypes.push_back(GENETYPE);

  DatatypeBase BOOLTYPE;
  BOOLTYPE.setID("bool");
  BOOLTYPE.addOperator("=");
  BOOLTYPE.addOperator("!");
  BOOLTYPE.addOperator("==");
  BOOLTYPE.addOperator("||");
  BOOLTYPE.addOperator(">=");
  BOOLTYPE.addOperator("<=");
  BOOLTYPE.addOperator(">");
  BOOLTYPE.addOperator("<");
  BOOLTYPE.addOperator("&&");
  BOOLTYPE.addOperator("@");
  BOOLTYPE.addTypeValue("bool");
  BOOLTYPE.addTypeValue("int");
  BOOLTYPE.addTypeValue("float");
  BOOLTYPE.Value = "0";
  datatypes.push_back(BOOLTYPE);

  // DatatypeBase NODETYPE;
  // NODETYPE.setID("node");
  // NODETYPE.addOperator("=");
  // NODETYPE.addOperator("==");
  // NODETYPE.addTypeValue("node");
  // datatypes.push_back(NODETYPE);

  return 0;
}

int printOperators() {
  for (uint32_t i = 0; i < Operators.size(); i++) {
    cout << "operator[" << i << "] = " << Operators.at(i).first << endl;
  }
  return 0;
};

int printKeywords() {
  for (uint32_t i = 0; i < keywords.size(); i++) {
    cout << "keyword[" << i << "] = " << keywords.at(i) << endl;
  }
  return (0);
};

int printDatatypes() {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    cout << "datatypes[" << i << "] = " << datatypes.at(i).typeID << endl;
  }
  return datatypes.size();
};

/*int printFunctions()
{
       for ( uint32_t i =0; i< functions.size(); i++ )
       {
               cout << "functions[" << i << "] = "<< functions.at(i) << endl;
       }
       return (0);
};*/

bool isWhiteSpace(const char &ch) {
  return ((ch == ' ' || ch == '\t') ? true : false);
};

bool isNewLine(const char &ch) {
  // FIX105 add peek for /r/n for windows ?
  return ((ch == '\n' || ch == '\r') ? true : false);
};

bool isWhite(const char &ch) {
  return (isWhiteSpace(ch) || isNewLine(ch) || ((int)ch == 10));
};

bool isNumber(const char &ch) {
  return ((ch >= '0') && (ch <= '9') ? true : false);
};

bool isLetter(const char &ch) {
  Op.clear();
  Op = ch;
  return (
      (!isNewLine(ch) && !isWhiteSpace(ch) && !isNumber(ch) && !isOperator(Op))
          ? true
          : false);
};

bool isString(const string &str) {

  return ((!isOperator(str) && !isKeyword(str) && !isDataType(str)) ? true
                                                                    : false);
};

bool isKeyword(const string &str) {
  for (uint32_t i = 0; i < keywords.size(); i++) {
    if (str == keywords.at(i)) {
      return true;
    }
  }

  return false;
};

bool isDataType(const string &type) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == type) {
      return true;
    }

    for (uint32_t k = 0; k < datatypes.at(i).CEquivalent.size(); k++) {
      if (datatypes.at(i).CEquivalent.at(k) == type) {
        return true;
      }
    }
  }
  return false;
};

/*bool isFunction(const string& str)
{

        for ( uint32_t i = 0; i<functions.size(); i++ )
        {
                if ( str == functions.at(i) )
                {
                        return (true);
                }
        }
        return (false);
};*/

bool isbinOperator(const string &str) {
  for (uint32_t i = 0; i < Operators.size(); i++) {
    if (str == Operators.at(i).first) {
      if (Operators.at(i).second == BINOPERATOR) {
        return true;
      }
    }
  }
  return false;
};

bool isSingleOperator(const string &str) {
  for (uint32_t i = 0; i < Operators.size(); i++) {
    if (str == Operators.at(i).first) {
      if (Operators.at(i).second == SINGLEOPERATOR) {
        return true;
      }
    }
  }
  return false;
};

bool isCoreOperator(const string &str) {
  for (uint32_t i = 0; i < Operators.size(); i++) {
    if (str == Operators.at(i).first) {
      if (Operators.at(i).second == COREOPERATOR) {
        return true;
      }
    }
  }
  return false;
};

/*int	matchDataType(const string& str)
{
        if ( str == "int" )
        {
                return Number;
        }
        else if ( str == "float" )
        {
                return Float;
        }
        else if ( str == "char" )
        {
                return Char;
        }
        else if ( str == "bool" )
        {
                return Bool;
        }
        else if ( str == "string" )
        {
                return String;
        }
        else if ( str == "node" )
        {
                return NODE;
        }

        return (-1);
}

string	matchDataType(const int& type)
{
        if ( type == Number)
        {
                return "int";
        }
        else if ( type == Float )
        {
                return "double";
        }
        else if ( type ==  Char )
        {
                return "char";
        }
        else if ( type == Bool )
        {
                return "bool";
        }
        else if ( type == String )
        {
                return "string";
        }
                else if ( type == NODE )
        {
                return "node";
        }

        return ("NIL");

};*/

/*bool hasOperator(const string& type, const string& op)
{
        for (uint32_t i =0;i < datatypes.size(); i++)
        {
                if ( datatypes.at(i).typeID == type )
                {
                        for (uint32_t k =0;k <
datatypes.at(i).dataOperators.size(); k++)
                        {
                                if ( datatypes.at(i).dataOperators.at(k) == op )
                                {
                                        return true;
                                }
                        }
                }
        }
        return false;
}*/

bool isOperator(const string &str) {
  for (uint32_t i = 0; i < Operators.size(); i++) {
    if (str == Operators.at(i).first) {
      return true;
    }
  }

  return false;
};

bool hasTypeValue(const string &type, const string &oppositeType) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == type) {
      for (uint32_t k = 0; k < datatypes.at(i).dataTypes.size(); k++) {
        if (datatypes.at(i).dataTypes.at(k) == oppositeType) {
          return true;
        }
      }
    }
  }
  return false;
}

string getDataType(const string &type) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {

    if (datatypes.at(i).typeID == type) {
      return datatypes.at(i).typeID;
    }

    for (uint32_t k = 0; k < datatypes.at(i).CEquivalent.size(); k++) {
      if (type == datatypes.at(i).CEquivalent.at(k)) {
        return datatypes.at(i).typeID;
      }
    }
  }
  return "NIL";
}

std::pair<bool, DatatypeBase> findDataType(const string &type) {
  std::pair<bool, DatatypeBase> result(false, DatatypeBase());
  for (uint32_t i = 0; i < datatypes.size() && result.first == false; i++) {
    if (datatypes.at(i).typeID == type) {
      result.first = true;
      result.second = datatypes.at(i);
      break;
    }

    for (uint32_t k = 0; k < datatypes.at(i).CEquivalent.size(); k++) {
      if (type == datatypes.at(i).CEquivalent.at(k)) {
        result.first = true;
        result.second = datatypes.at(i);
        break;
      }
    }
  }
  return result;
}

bool isClass(const string &type) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == type && datatypes.at(i).classType) {
      return true;
    }
  }
  return false;
}

bool isEnum(const string &type) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == type && datatypes.at(i).enumType) {
      return true;
    }
  }
  return false;
}

bool classHasMemberFunction(const string &classID, const string &member) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == classID && datatypes.at(i).classType) {
      for (uint32_t k = 0; k < datatypes.at(i).memberFunctions.size(); k++) {
        if (datatypes.at(i).memberFunctions.at(k).name == member) {
          return true;
        }
      }
    }
  }
  return false;
}

string classMemberFunctionType(const string &classID, const string &member) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == classID && datatypes.at(i).classType) {
      for (uint32_t k = 0; k < datatypes.at(i).memberFunctions.size(); k++) {
        if (datatypes.at(i).memberFunctions.at(k).name == member) {
          return datatypes.at(i).memberFunctions.at(k).returnType;
        }
      }
    }
  }
  return "NIL";
}

bool classHasMemberFunction(const string &classID, const functionCall &member) {
  /*for(uint32_t i =0; i < datatypes.size(); i++)
  {
          if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
          {
                  for (uint32_t k =0; k <
  datatypes.at(i).memberFunctions.size(); k++)
                  {
                          if(datatypes.at(i).memberFunctions.at(k).name ==
  member.name)
                          {
                                  if(*(datatypes.at(i).memberFunctions.at(k).size())
  == *member.size())
                                  {
                                          return true;
                                  }
                          }
                  }
          }
  }
  return false;*/
  return false;
}

std::pair<bool, funcInfo> getClassMemberFunction(const string &classID,
                                                 const functionCall &member) {
  std::pair<bool, funcInfo> result(false, funcInfo());
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == classID && datatypes.at(i).classType) {
      for (uint32_t k = 0; k < datatypes.at(i).memberFunctions.size(); k++) {
        if (datatypes.at(i).memberFunctions.at(k).name == member.name) {
          result.first = true;
          result.second = datatypes.at(i).memberFunctions.at(k);
          break;
          /*if(*(datatypes.at(i).memberFunctions.at(k).size()) ==
          *member.size())
          {
                  return true;
          }*/
        }
      }
    }
  }
  return result;
}

bool classHasMemberVariable(const string &classID, const string &member) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == classID &&
        (datatypes.at(i).classType || datatypes.at(i).enumType)) {
      for (uint32_t k = 0; k < datatypes.at(i).memberVariables.size(); k++) {
        if (datatypes.at(i).memberVariables.at(k).name == member) {
          return true;
        }
      }
    }
  }
  return false;
}

std::pair<bool, funcInfo> getClassMemberVariable(const string &classID,
                                                 const string &member) {
  std::pair<bool, funcInfo> result(false, funcInfo());
  for (uint32_t i = 0; i < datatypes.size() && result.first == false; i++) {
    if (datatypes.at(i).typeID == classID && datatypes.at(i).classType) {
      for (uint32_t k = 0; k < datatypes.at(i).memberVariables.size(); k++) {
        if (datatypes.at(i).memberVariables.at(k).name == member) {
          result.first = true;
          result.second = datatypes.at(i).memberVariables.at(k);
          break;
        }
      }
    }
  }
  return result;
}

std::vector<funcInfo> getClassMemberVariables(const string &classID) {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == classID && datatypes.at(i).classType) {
      return datatypes.at(i).memberVariables;
    }
  }
  return std::vector<funcInfo>();
}

int printTypes() {
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    cout << datatypes.at(i).typeID << endl;

    if (datatypes.at(i).typeID == "string") {
      for (uint32_t k = 0; k < datatypes.at(i).dataOperators.size(); k++) {
        cout << ":::" << datatypes.at(i).dataOperators.at(k) << endl;
      }
    }
    for (uint32_t k = 0; k < datatypes.at(i).CEquivalent.size(); k++) {
      cout << "::" << datatypes.at(i).CEquivalent.at(k) << endl;
    }
  }

  return 0;
}

bool typeHasOperator(const string &op, const string &type) {
  int typeFound = false;
  for (uint32_t i = 0; i < datatypes.size(); i++) {
    if (datatypes.at(i).typeID == type) {
      typeFound = true;
    }

    for (uint32_t k = 0; k < datatypes.at(i).CEquivalent.size(); k++) {
      if (datatypes.at(i).CEquivalent.at(k) == type) {
        typeFound = true;
      }
    }

    if (typeFound) {
      for (uint32_t k = 0; k < datatypes.at(i).dataOperators.size(); k++) {
        if (datatypes.at(i).dataOperators.at(k) == op) {
          return true;
        }
      }
      break;
    }
  }

  return false;
}

bool isImmediate(const token &tok) {
  /*if(! (isOperator(immediate) || isKeyword(immediate) ||
                  DM14::types::isDataType(immediate) || isFunction(immediate,
     true) || isIdentifier(immediate)))*/
  if (tok.type == "float" || tok.type == "int" || tok.type == "string" ||
      tok.type == "char" || tok.type == "bool") {
    return true;
  }
  return false;
};

} // namespace types

} // namespace DM14
