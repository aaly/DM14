/**
@file            	types.hpp
@brief         	types
@details      	types, Part of DM14 programming language
@author      	AbdAllah Aly Saad <aaly90@gmail.com>
@date	        	2010-2018
@version		    1.1a
@copyright      See file "license" for bsd license
*/
#ifndef	TYPES_H
#define	TYPES_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "Array.hpp"
#include "DatatypeBase.hpp"
#include "token.hpp" /**< class token */
//class DatatypeBase;
class functionCall;
class funcInfo;

namespace DM14
{
	extern Array<DatatypeBase>					datatypes;
	extern std::vector<pair<string, int>>		Operators;
	extern std::vector<string>						keywords;
	extern fstream											mapsFile;
		
	namespace types	
	{


		using namespace std;

		// type checking functions
		
		int			printTypes();
		bool		isWhiteSpace(const char&);
		bool		isWhite(const char&);
		bool		isNewLine(const char&);
		bool		isNumber(const char&);
		bool		isLetter(const char&);
		bool		isString(const string&);
		bool		isOperator(const string&);
		bool		isDataType(const string&);
		bool		isKeyword(const string&);
		bool		isFunction(const string&);
		bool		isbinOperator(const string&);
		int			matchbinOperator(const string&);
		string		matchbinOperator(const int&);
		bool		isSingleOperator(const string&);
		bool		isCoreOperator(const string&);
		int			matchSingleOperator(const string&);
		string		matchSingleOperator(const int&);
		int			matchDataType(const string&);
		string		matchDataType(const int&);
		//bool		hasOperator(const string&, const int&);
		bool		hasTypeValue(const string&, const string&);
		string		getDataType(const string& type);
		std::pair<bool, DatatypeBase> findDataType(const string& type);
		bool		isClass(const string& type);
		bool		isEnum(const string& type);
		
		bool		classHasMemberFunction(const string& classID, const string& member);
		string		classMemberFunctionType(const string& classID, const string& member);
		std::vector<funcInfo>	getClassMemberVariables(const string& classID);
		
		bool 						isImmediate(const token& tok);
		
		bool		typeHasOperator(const string& op, const string& type);
		bool		classHasMemberFunction(const string& classID, const functionCall& member);
		bool		classHasMemberVariable(const string& classID, const string& member);
		
		std::pair<bool, funcInfo> getClassMemberVariable(const string& classID, const string& member);
		std::pair<bool, funcInfo> getClassMemberFunction(const string& classID, const functionCall& member);
		
		static 		string	Op;
		//static		char	funcbuff;
		static		string	funcname;
		
		string 		toInt(const string&);
		
		
		//static vector<string>		functions;
		int 						intSymbols();
		int							printKeywords();
		int							printOperators();
		int							printDatatypes();
		int							printFunctions();
		
		//static multimap<int, int>			typeOperator; // holds type and its allowed binay operators
		//static multimap<int, int>			typeValue; // holds type and its allowed Types to operate wuth
		//map<string,int>::iterator			bopit;
		
		enum types
		{
			DataType = 1,
			Number,			//2
			Float,			//3
			Char,			//4
			String,			//5
			Bool,			//6
			Gene,			//7
			Operator,		//8
			KeyWord,		//9
			Function,		//10
			Identifier,		//11
			Void,			//12
			USERFUNCTION,	//13
			BUILTINFUNCTION,//14
			NODE,			//15
			DATAMEMBER,		//16
			ENUM			//17
		};
		
		enum binaryOperators
		{
			//BINARYOP=30,
			//Add,		// +
			//Sub,		// -
			//Mul,		// *
			//Div,		// /
			//Equal,		// =
			//EqualE,		// ==
			//NotEqual,	// !=
			//AEqual,		// +=
			//SEqual,		// -=
			//MEqual,		// *=
			//DEqual,		// /=
			//RPar,		// (
			//LPar,		// )
			//GTHAN,		// >
			//LTHAN,		// <
			//GEQUALTHAN, // >=
			//LEQUALTHAN, // <=
			//LOGICALOR,	// ||
			//DOT,		//.
			//BINARYOPLAST,
			//SCOPEOP,	// ::
			//PERCENTAGE,	// %
			GENERALOPERATOR,
			SINGLEOPERATOR,
			BINOPERATOR,
			COREOPERATOR
			
		};
		
		enum uniIoerators
		{
			//UNIOP=BINARYOPLAST+1,
			//DOUBLEADD, //++
			//DOUBLESUB, //--
		};
		
		enum CLASSIFIER
		{
			PUBLIC = 0,
			PRIVATE,
			PROTECTED
		};

	} // namespace types	
} // namespace DM14
#endif // TYPES_H
