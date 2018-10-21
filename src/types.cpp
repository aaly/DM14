// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license

#include "types.hpp"

Array<DatatypeBase>	datatypes;
std::vector<pair<string, int> >		Operators;
std::vector<string>		keywords;
fstream				mapsFile;

namespace DM14::types
{
//FIX105 add peek for /r/n for windows ?

// add all operators to both single or binary ops and then modify functions for searching, to search both lists.
int intSymbols()
{
keywords.push_back("with");
keywords.push_back("use");
keywords.push_back("class");
//keywords.push_back("func");
keywords.push_back("while");
keywords.push_back("for");
keywords.push_back("case");
keywords.push_back("if");
keywords.push_back("break");
keywords.push_back("continue");
keywords.push_back("return");
keywords.push_back("distribute");
keywords.push_back("reset");
keywords.push_back("addparent");
keywords.push_back("setnode");
keywords.push_back("backprop");
keywords.push_back("channel");
keywords.push_back("nodist");
keywords.push_back("noblock");
keywords.push_back("recurrent");
keywords.push_back("extern");
keywords.push_back("endextern");
keywords.push_back("link");
keywords.push_back("slink");
keywords.push_back("thread");
keywords.push_back("global");



Operators.push_back(make_pair("\\",GENERALOPERATOR));
Operators.push_back(make_pair(">",BINOPERATOR));
Operators.push_back(make_pair(">>",GENERALOPERATOR));
Operators.push_back(make_pair("<",BINOPERATOR));
Operators.push_back(make_pair("<<",GENERALOPERATOR));
Operators.push_back(make_pair("<-",GENERALOPERATOR));
Operators.push_back(make_pair("->",GENERALOPERATOR));
Operators.push_back(make_pair(".",GENERALOPERATOR));
Operators.push_back(make_pair("?",GENERALOPERATOR));
Operators.push_back(make_pair("~",GENERALOPERATOR));
Operators.push_back(make_pair(":",GENERALOPERATOR));
Operators.push_back(make_pair(";",GENERALOPERATOR));
Operators.push_back(make_pair(",",GENERALOPERATOR));
Operators.push_back(make_pair("[",GENERALOPERATOR));
Operators.push_back(make_pair("]",GENERALOPERATOR));
Operators.push_back(make_pair("{",GENERALOPERATOR));
Operators.push_back(make_pair("}",GENERALOPERATOR));
Operators.push_back(make_pair("(",GENERALOPERATOR));
Operators.push_back(make_pair(")",GENERALOPERATOR));
Operators.push_back(make_pair("\'",GENERALOPERATOR));
Operators.push_back(make_pair("#",GENERALOPERATOR));
Operators.push_back(make_pair("+",BINOPERATOR));
Operators.push_back(make_pair("-",BINOPERATOR));
Operators.push_back(make_pair("*",BINOPERATOR));
Operators.push_back(make_pair("/",BINOPERATOR));
Operators.push_back(make_pair("=",BINOPERATOR));
Operators.push_back(make_pair("==",BINOPERATOR));
Operators.push_back(make_pair("!=",BINOPERATOR));
Operators.push_back(make_pair("!",SINGLEOPERATOR));
Operators.push_back(make_pair("+=",BINOPERATOR));
Operators.push_back(make_pair("-=",BINOPERATOR));
Operators.push_back(make_pair("*=",BINOPERATOR));
Operators.push_back(make_pair("/=",BINOPERATOR));
Operators.push_back(make_pair("<",BINOPERATOR));
Operators.push_back(make_pair("<=",BINOPERATOR));
Operators.push_back(make_pair(">",BINOPERATOR));
Operators.push_back(make_pair(">=",BINOPERATOR));
Operators.push_back(make_pair("||",BINOPERATOR));
Operators.push_back(make_pair("&&",BINOPERATOR));
Operators.push_back(make_pair("&",BINOPERATOR));
Operators.push_back(make_pair("|",BINOPERATOR));
Operators.push_back(make_pair(".",BINOPERATOR));
Operators.push_back(make_pair("::",BINOPERATOR));
Operators.push_back(make_pair("%",BINOPERATOR));
Operators.push_back(make_pair("++",SINGLEOPERATOR));
Operators.push_back(make_pair("--",SINGLEOPERATOR));
//Operators.push_back(make_pair("@",SINGLEOPERATOR));
//Operators.push_back(make_pair("@",BINOPERATOR));
Operators.push_back(make_pair("@",COREOPERATOR));
//Operators.push_back(make_pair("@",GENERALOPERATOR));



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


//DatatypeBase NODETYPE;
//NODETYPE.setID("node");
//NODETYPE.addOperator("=");
//NODETYPE.addOperator("==");
//NODETYPE.addTypeValue("node");
//datatypes.push_back(NODETYPE);


return 0;
}

 int printOperators()
{
	for ( int unsigned i =0; i< Operators.size(); i++ )
	{
		cout << "operator[" << i << "] = "<< Operators.at(i).first << endl;
	}
	return 0;
};



 int printKeywords()
{
	for ( int unsigned i =0; i< keywords.size(); i++ )
	{
		cout << "keyword[" << i << "] = "<< keywords.at(i) << endl;
	}
	return (0);
};

 int printDatatypes()
{
	for ( int unsigned i =0; i< datatypes.size(); i++ )
	{
		cout << "datatypes[" << i << "] = "<< datatypes.at(i).typeID << endl;
	}
	return datatypes.size();
};

 /*int printFunctions()
{
	for ( int unsigned i =0; i< functions.size(); i++ )
	{
		cout << "functions[" << i << "] = "<< functions.at(i) << endl;
	}
	return (0);
};*/

bool isWhiteSpace(const char& ch)
{
	return ( (ch == ' ' || ch == '\t') ? true : false );
};


bool isNewLine(const char& ch)
{
	//FIX105 add peek for /r/n for windows ?
	return ( (ch == '\n' || ch == '\r') ? true : false );
};

bool isWhite(const char& ch)
{
	return (isWhiteSpace(ch) || isNewLine(ch) || ((int)ch == 10));
};

bool isNumber(const char& ch)
{
	return ( ( ch >= '0' ) &&  ( ch <= '9' ) ? true : false);
};

bool isLetter(const char& ch)
{
	Op.clear();
	Op = ch;
	return ( ( !isNewLine(ch) && !isWhiteSpace(ch) && !isNumber(ch) && !isOperator(Op)) ? true : false);
};

bool isString(const string& str)
{
	
	return ( ( !isOperator(str) && !isKeyword(str) && !isDataType(str) ) ? true : false);
};

bool isKeyword(const string& str)
{
	for ( unsigned int i =0; i<keywords.size(); i++ )
	{
		if ( str == keywords.at(i) )
		{
			return true;
		}
	}
	
	return false;
};

bool isDataType(const string& type)
{
	for ( unsigned int i = 0; i<datatypes.size(); i++ )
	{
		if (datatypes.at(i).typeID == type)
		{
			return true;
		}
		
		for (unsigned k =0; k < datatypes.at(i).CEquivalent.size(); k++)
		{
			if( datatypes.at(i).CEquivalent.at(k) == type)
			{
				return true;
			}
		}
	}
	return false;
};

/*bool isFunction(const string& str)
{
	
	for ( unsigned int i = 0; i<functions.size(); i++ )
	{
		if ( str == functions.at(i) )
		{
			return (true);
		}
	}
	return (false);
};*/

bool isbinOperator(const string& str)
{
	for ( unsigned int i =0; i < Operators.size(); i++ )
	{
		if ( str == Operators.at(i).first)
		{
			if(Operators.at(i).second == BINOPERATOR)
			{
				return true;
			}
		}
	}
	return false;
};

bool isSingleOperator(const string& str)
{
	for ( unsigned int i =0; i < Operators.size(); i++ )
	{
		if ( str == Operators.at(i).first )
		{
			if(Operators.at(i).second == SINGLEOPERATOR)
			{
				return true;
			}
		}
	}
	return false;
};


bool isCoreOperator(const string& str)
{
	for ( unsigned int i =0; i < Operators.size(); i++ )
	{
		if ( str == Operators.at(i).first )
		{
			if(Operators.at(i).second == COREOPERATOR)
			{
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
	for (unsigned int i =0;i < datatypes.size(); i++)
	{
		if ( datatypes.at(i).typeID == type )
		{
			for (unsigned int k =0;k < datatypes.at(i).dataOperators.size(); k++)
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

bool isOperator(const string& str)
{
	for ( unsigned int i =0; i < Operators.size(); i++ )
	{
		if ( str == Operators.at(i).first )
		{
			return true;
		}
	}
	
	return false;
};


bool hasTypeValue(const string& type, const string& oppositeType)
{
	for (unsigned int i =0;i < datatypes.size(); i++)
	{
		if ( datatypes.at(i).typeID == type )
		{		
			for (unsigned int k =0;k < datatypes.at(i).dataTypes.size(); k++)
			{
				if ( datatypes.at(i).dataTypes.at(k) == oppositeType )
				{
					return true;
				}
			}
		}
	}
	return false;
}


string getDataType(const string& type)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		
		if(datatypes.at(i).typeID == type)
		{
			return datatypes.at(i).typeID;
		}
		
		for (unsigned k =0; k < datatypes.at(i).CEquivalent.size(); k++)
		{
			if(type ==  datatypes.at(i).CEquivalent.at(k))
			{
				return datatypes.at(i).typeID;
			}
		}
	}
	return "NIL";
}



DatatypeBase findDataType(const string& type)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == type)
		{
			return datatypes.at(i);
		}
		
		for (unsigned k =0; k < datatypes.at(i).CEquivalent.size(); k++)
		{
			if(type ==  datatypes.at(i).CEquivalent.at(k))
			{
				return datatypes.at(i);
			}
		}
	}
	return DatatypeBase();
}

bool isClass(const string& type)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == type && datatypes.at(i).classType)
		{
			return true;
		}
	}
	return false;
}

bool isEnum(const string& type)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == type && datatypes.at(i).enumType)
		{
			return true;
		}
	}
	return false;
}

bool classHasMemberFunction(const string& classID, const string& member)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
		{
			for (unsigned int k =0; k < datatypes.at(i).memberFunctions.size(); k++)
			{
				if(datatypes.at(i).memberFunctions.at(k).name == member)
				{
					return true;
				}
			}
		}
	}
	return false;
}


string classMemberFunctionType(const string& classID, const string& member)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
		{
			for (unsigned int k =0; k < datatypes.at(i).memberFunctions.size(); k++)
			{
				if(datatypes.at(i).memberFunctions.at(k).name == member)
				{
					return datatypes.at(i).memberFunctions.at(k).returnType;
				}
			}
		}
	}
	return "NIL";
}



bool classHasMemberFunction(const string& classID, const functionCall& member)
{
	/*for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
		{
			for (unsigned int k =0; k < datatypes.at(i).memberFunctions.size(); k++)
			{
				if(datatypes.at(i).memberFunctions.at(k).name == member.name)
				{
					if(*(datatypes.at(i).memberFunctions.at(k).size()) == *member.size())
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


funcInfo getClassMemberFunction(const string& classID, const functionCall& member)
{
	/*for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
		{
			for (unsigned int k =0; k < datatypes.at(i).memberFunctions.size(); k++)
			{
				if(datatypes.at(i).memberFunctions.at(k).name == member.name)
				{
					if(*(datatypes.at(i).memberFunctions.at(k).size()) == *member.size())
					{
						return true;
					}
				}
			}
		}
	}
	return false;*/
	return funcInfo();
}


bool classHasMemberVariable(const string& classID, const string& member)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == classID && (datatypes.at(i).classType || datatypes.at(i).enumType))
		{
			for (unsigned int k =0; k < datatypes.at(i).memberVariables.size(); k++)
			{
				if(datatypes.at(i).memberVariables.at(k).name == member)
				{
					return true;
				}
			}
		}
	}
	return false;
}

funcInfo getClassMemberVariable(const string& classID, const string& member)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
		{
			for (unsigned int k =0; k < datatypes.at(i).memberVariables.size(); k++)
			{
				if(datatypes.at(i).memberVariables.at(k).name == member)
				{
					return datatypes.at(i).memberVariables.at(k);
				}
			}
		}
	}
	return funcInfo();
}


std::vector<funcInfo> getClassMemberVariables(const string& classID)
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
		{
			return datatypes.at(i).memberVariables;
		}
	}
	return std::vector<funcInfo>();
}

int printTypes()
{
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		cout << datatypes.at(i).typeID <<endl;
		
		if(datatypes.at(i).typeID == "string")
		{
			for (unsigned k =0; k < datatypes.at(i).dataOperators.size(); k++)
			{
				cout << ":::" << datatypes.at(i).dataOperators.at(k) << endl;
			}
		}
		for (unsigned k =0; k < datatypes.at(i).CEquivalent.size(); k++)
		{
			cout << "::" << datatypes.at(i).CEquivalent.at(k) << endl;
		}
	}
	
	return 0;
}

bool typeHasOperator(const string& op, const string& type)
{
	int typeFound = false;
	for(unsigned int i =0; i < datatypes.size(); i++)
	{
		if(datatypes.at(i).typeID == type)
		{
			typeFound = true;
		}
		
		for (unsigned k =0; k < datatypes.at(i).CEquivalent.size(); k++)
		{
			if(datatypes.at(i).CEquivalent.at(k) == type)
			{
				typeFound = true;
			}
		}
		
		if(typeFound)
		{
			for (unsigned k =0; k < datatypes.at(i).dataOperators.size(); k++)
			{
				if(datatypes.at(i).dataOperators.at(k) == op)
				{
					return true;
				}
			}
			break;
		}
	}
	
	return false;
}

} // namespace DM14::types

