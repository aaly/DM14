// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license

// add type to fubnctioncall

#ifndef	GRAMMAR_H
#define	GRAMMAR_H

#include <string>
#include <vector>
#include <map>

using namespace std;


class idInfo
{
	public:
		idInfo(const string&, const int&, const int&);
		idInfo();
		string name;
		int scope;
		int type;
		string value;
};


class with
{
	with();
	string* package; // package , like io ( string , ... )
	string* library; // library , like math or just *
};

class statement
{
	public:
		int index;
		int scope;
		int statementType;
		int	type;
		//vector < map<string, map <int, map < int, void*> > >* > vars; // map ( identifier , map ( type, map ( operator, value) )
};


class emptyStatement : public statement
{
	public:
		emptyStatement();
};


class declareStatement : public statement
{
	public:
		declareStatement();
		vector<string>*	identifiers; 		// identifiers
		//int				type;				// type , int float , etc..
		//int				op;					// operator like += or -= so on ... , we really need it ??
		bool				array;
		bool				initilazied;
		int					size;
		statement*			value;				// value to be stored in the variable
};

class operationalStatement : public statement
{
	public:
		operationalStatement();
		
		//int					type;
		int					op;		// operator like += or -= so on ...
		//vector<string>*		left;	// right expression maybe a,b ?
		vector<statement*>*		right;	// termStatements ?
		statement*			left;	// left expression
		int					countLevels; // count how many ( we entered !
};


class termStatment : public statement
{
	public:
		termStatment();
		termStatment(const string&);
		termStatment(const string&, const int&);
		
		string	term; // if its type is Identifier or immediate
		//int		type;
};


class functionCall : public statement
{
	public:
		functionCall();
		
		string			name;			// function to call
		vector<statement*>*	parameters;		// vector of parameters with void , that can be anything
};


class function
{
	public:
		function ();
		int scope; 	
		string name;					// global ? or in a class
		vector<statement*>* body;	// will point to the statments in the statments vector
		vector<idInfo>* parameters;		// 
		//vector<int>* parametersTypes;		// 
		// when user define a function , get its info into the info map and then we can search for its info later
		string returnID;				//return value or variable
		int		returnIDType;
};

class forloop : public statement
{
	public:
		forloop();
		string from;						// the starter value
		string to;							// the end value
		string step;						// the step value
		vector<statement*>* body; 			// will point to the statments in the statments vector
};

class IF : public statement
{
	public:
		IF();
		statement*			condition;
		vector<statement*>*	body; 		// will point to the statments in the statments vector
		vector<statement*>*	ELSE;
};

class CASE : public statement
{
	public:
		CASE();
		statement* condition;
		//vector<statement*>*	cases;		// vector of parameters with void , that can be anything
		//vector<statement*>*	body;		// vector of parameters with void , that can be anything
		map<statement*, vector<statement*> > Body;
};

/*class expression : public statement
{
	public:
	expression();
	statement* expr;
	private:
	
};*/


enum bTypes // block types
{
	
};

enum sTypes // statement types
{
	eStatement = 1, // Empty Statement
	dStatement,		// Declare Statement
	oStatement,		// Operational Statement
	tStatement,		// Term Statement
	fCall,			// Function  Call
	fLoop,			// For Loop
	IFStatement,	// IF statement
	CASEStatement,	// Case statement
	Expr
};

enum tTokens // terminal tokens
{
	// ;
	// ?
	
};

enum fTypes // function types
{
	BUILTINFUNCTION=199,
	USERFUNCTION
};
#endif // GRAMMAR_H
