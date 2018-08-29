// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license

#ifndef	AST_H
#define	AST_H

#include <string>
#include "common.hpp"
#include "types.hpp"
#include <map>

using namespace std;

class idInfo;

class with
{
	with();
	~with();
	string* package; // package , like io ( string , ... )
	string* library; // library , like math or just *
};



class distributingVariablesStatement;
class funcInfo;

class statement
{
	public:
		statement();
		int line;
		int scope;
		int statementType;
		string	type;
		int	scopeLevel; // count how many ( we entered !
		//Array < map<string, map <int, map < int, void*> > >* > vars; // map ( identifier , map ( type, map ( operator, value) )
		Array<distributingVariablesStatement*> distStatements;
		Array<statement*> splitStatements;
};

class funcInfo
{
	public:
		//idInfo(const string&, const int&, const int&);
		funcInfo();
		int				clear();
		string 			name;
		string 			returnType;
		Array< pair<string, bool> >*	parameters;
		bool			protoType;
		bool			noAutism;
		int				type;
		int				classifier;
		bool			classConstructor;
};

//class idInfo;

class idInfo
{
	public:
		idInfo(const string&, const int&, const string&, statement*);
		idInfo(const string&, const int&, const string&, const string&, statement*);
		idInfo();
		string		name;
		int			scope;
		string		type;
		string		value;
		int			index;
		bool		initialized;
		int			distributedScope;
		bool		distributed;
		statement*	arrayIndex;
		bool		array;
		bool		pointer;
		int			size;
		bool		tmpScope;
		bool		internal;
		bool		backProp;
		bool		recurrent;
		bool		channel;
		bool		noblock;
		bool		global;
		bool		shared;
		string		requestAddress;
		string		functionParent;
		
		//string		parent;
		//string		getParent();
		idInfo*			parent;
		//idInfo* 		getParent();
		int			setParent(const string& parentID);
		
};

class returnStatement : public statement
{
	public:
		returnStatement();
		statement* retValue;
};


class threadStatement : public statement
{
	public:
		threadStatement();
		~threadStatement();
		string			ID;
		string			classID;
		string			parentID;
		string			returnType;
		string			Identifier;
		
		bool			classMember;
		statement*		functioncall;
		//Array<statement*>*	parameters;
		//funcInfo			funcinfo;
		
};

class emptyStatement : public statement
{
	public:
		emptyStatement();
};

class distributingVariablesStatement : public statement
{
	public:
		enum dvStatementType
		{
			MODS = 128,
			DEPS
		};
		distributingVariablesStatement();
		//Array<idInfo>	variables;
		idInfo			variable;
		int				type; // Mods or Deps ?
		int				dependencyNode;
};

/*
class Class : public statement
{
	public:
		Class();
		~Class();
		string name;
		Array<string> parents;
		Array<funcInfo> publicFuctions;
		Array<funcInfo> privateFuctions;
		Array<funcInfo> protectedFuctions;
		
		Array<funcInfo> publicMembers;
		Array<funcInfo> privateMembers;
		Array<funcInfo> protectedMembers;
		
	
};
*/

class Link : public statement
{
	public:
		Link();
		~Link();
		string		libs;
		bool		Static;
	private:
		
};

class declareStatement : public statement
{
	public:
		declareStatement();
		~declareStatement();
		Array<idInfo>*	identifiers; 		// identifiers
		//int				type;				// type , int float , etc..
		//int				op;					// operator like += or -= so on ... , we really need it ??
		
		bool				array;
		int					size;
		statement*			value;				// value to be stored in the variable
		Array<statement*>	values;
		bool			Initilazed;
		bool			distributed;
		bool			tmpScope;
		bool			backProp;
		bool			recurrent;
		bool			channel;
		bool			noblock;
		bool			classtype;
		bool			global;
		bool			shared;
};

class resetStatement : public statement
{
	public:
		resetStatement();
		~resetStatement();	
		
		statement* count;
};

class parentAddStatement : public statement
{
	public:
		parentAddStatement();
		~parentAddStatement();	
		
		statement* socket;
		statement* ip;
};

class NOPStatement : public statement
{
	public:
		NOPStatement();
		~NOPStatement();	
};

class setNodeStatement : public statement
{
	public:
		setNodeStatement();
		~setNodeStatement();	
		
		statement* node;
};


class distStatement : public statement
{
	public:
		distStatement();
		~distStatement();
		
		Array<idInfo>*		modifiedVariables;
		Array<idInfo>*		dependenciesVariables;
		//static int		bigOrder;
		int					order;
	
};

class operationalStatement : public statement
{
	public:
		operationalStatement();
		~operationalStatement();
		//int					type;
		string						op;		// operator like += or -= so on ...
		//Array<string>*		left;	// right expression maybe a,b ?
		//Array<statement*>*		right;	// termStatements ?
		statement*			right;	// left expression
		statement*			left;	// left expression
};


class termStatment : public statement
{
	public:
		termStatment();
		termStatment(const string&);
		termStatment(const string&, const string&);		
		string			term; // if its type is Identifier or immediate
		idInfo*			id;
		bool			identifier;
		statement*		arrayIndex;
		unsigned int	size;
};


class functionCall : public statement
{
	public:
		functionCall();
		~functionCall();
		string			name;				// function to call
		Array<statement*>*	parameters;		// Array of parameters with void , that can be anything
		int				functionType;
};

class forloop : public statement
{
	public:
		forloop();
		~forloop();
		//string from;						// the starter value
		Array<statement*>* fromCondition;
		//string to;							// the end value
		Array<statement*>* toCondition;
		//string step;						// the step value
		Array<statement*>* stepCondition;
		Array<statement*>* body; 			// will point to the statments in the statments Array
};

class whileloop : public statement
{
	public:
		whileloop();
		~whileloop();
		statement* condition;						// the starter value
		Array<statement*>* body; 			// will point to the statments in the statments Array
};

/*class adressingStatement : public statement
{
	public:
		adressingStatement();
		~adressingStatement();
		statement* condition;						// the starter value
		Array<statement*>* body; 			// will point to the statments in the statments Array
};*/


class IF : public statement
{
	public:
		IF();
		~IF();
		statement*			condition;
		Array<statement*>*	body; 		// will point to the statments in the statments Array
		Array<statement*>*	ELSE;
		Array<statement*>*	elseIF;
};

class CASE : public statement
{
	public:
		CASE();
		~CASE();
		statement* condition;
		//Array<statement*>*	cases;		// Array of parameters with void , that can be anything
		//Array<statement*>*	body;		// Array of parameters with void , that can be anything
		map<statement*, Array<statement*> > Body;
};

class EXTERN : public statement
{
	public:
		EXTERN();
		~EXTERN();
		string			body;
		
		Array<idInfo>*		modifiedVariables;
		Array<idInfo>*		dependenciesVariables;
		
};

/*class expression : public statement
{
	public:
	expression();
	statement* expr;
	private:
	
};*/


enum grammarTerminals
{
	
};

enum grammarNonTerminals
{
	iStatement = 1,			/**< Include Statement */
	eStatement, 			/**< Empty Statement */
	rStatement,				/**< return Statement */
	dStatement,				/**< Declare Statement */
	oStatement,				/**< Operational Statement */
	tStatement,				/**< Term Statement */
	fCall,					/**< Function  Call */
	fLoop,					/**< For Loop */
	wLoop,					/**< while loop */
	IFStatement,			/**< IF statement */
	CASEStatement,			/**< Case statement */
	DISTStatement,			/**< distribute statement */
	adressingStatement,		/**< Node addressing Statement */
	Expr,					/**< Expression statement */
	dvStatement,			/**< distributingVariablesStatement */
	RESETStatement,			/**< Reset statement */
	PAStatement,			/**< Parent Add statement */
	SNStatement,			/**< Set Node statement */
	EXTERNStatement, 		/**< Extern Statement */
	THREADStatement, 		/**< Thread Statement */
	NOPSTATEMENT			/**< nop statement */
};

class ast_function
{
	public:
		ast_function();
		~ast_function();
		int						scope;				//
		string					name;				// global ? or in a class
		Array<statement*>*		body;				// will point to the statments in the statments Array
		Array<idInfo>*			parameters;			// 
		//Array<int>*			parametersTypes;	// 
		// when user define a function , get its info into the info map and then we can search for its info later
		string					returnID;			//return value or variable
		string					returnIDType;		//
		int						Type;				// userfunction or builtinfunction
		Array<string>			functionNodes;		//
		bool 					distributed;
};



#endif // AST_H
