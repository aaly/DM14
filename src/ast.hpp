/**
@file             ast.hpp
@brief            ast
@details          Abstract Syntax Tree classes, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license
*/

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

class Statement
{
	public:
		void absorbDistStatements(Statement*);
		Statement();
		int line;
		int scope;
		int StatementType;
		string	type;
		int	scopeLevel; // count how many ( we entered !
		//Array < map<string, map <int, map < int, void*> > >* > vars; // map ( identifier , map ( type, map ( operator, value) )
		Array<distributingVariablesStatement*> distStatements;
		Array<Statement*> splitStatements;
};

class funcInfo
{
	public:
		//idInfo(const string&, const int&, const int&);
		funcInfo();
		int				clear();
		string 			name;
		string 			returnType;
		Array<pair<string, bool>>*	parameters;
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
		idInfo(const string&, const int&, const string&, Statement*);
		idInfo(const string&, const int&, const string&, const string&, Statement*);
		idInfo();
		string		name;
		int			scope;
		string		type;
		string		value;
		int			index;
		bool		initialized;
		int			distributedScope;
		bool		distributed;
		Statement*	arrayIndex;
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

class returnStatement : public Statement
{
	public:
		returnStatement();
		Statement* retValue;
};


class threadStatement : public Statement
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
		Statement*		functioncall;
		//Array<Statement*>*	parameters;
		//funcInfo			funcinfo;
		
};

class emptyStatement : public Statement
{
	public:
		emptyStatement();
};

class breakStatement : public Statement
{
	public:
		breakStatement();
};

class continueStatement : public Statement
{
	public:
		continueStatement();
};


class distributingVariablesStatement : public Statement
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
class Class : public Statement
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

class Link : public Statement
{
	public:
		Link();
		~Link();
		string		libs;
		bool		Static;
	private:
		
};

class declareStatement : public Statement
{
	public:
		declareStatement();
		~declareStatement();
		Array<idInfo>*	identifiers; 		// identifiers
		//int				type;				// type , int float , etc..
		//int				op;					// operator like += or -= so on ... , we really need it ??
		
		bool				array;
		int					size;
		Statement*			value;				// value to be stored in the variable
		Array<Statement*>	values;
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

class resetStatement : public Statement
{
	public:
		resetStatement();
		~resetStatement();	
		
		Statement* count;
};

class parentAddStatement : public Statement
{
	public:
		parentAddStatement();
		~parentAddStatement();	
		
		Statement* socket;
		Statement* ip;
};

class NOPStatement : public Statement
{
	public:
		NOPStatement();
		~NOPStatement();	
};

class setNodeStatement : public Statement
{
	public:
		setNodeStatement();
		~setNodeStatement();	
		
		Statement* node;
};


class distStatement : public Statement
{
	public:
		distStatement();
		~distStatement();
		
		Array<idInfo>*		modifiedVariables;
		Array<idInfo>*		dependenciesVariables;
		//static int		bigOrder;
		int					order;
	
};

class operationalStatement : public Statement
{
	public:
		operationalStatement();
		~operationalStatement();
		//int					type;
		string						op;		// operator like += or -= so on ...
		//Array<string>*		left;	// right expression maybe a,b ?
		//Array<Statement*>*		right;	// termStatements ?
		Statement*			right;	// left expression
		Statement*			left;	// left expression
};


class termStatement : public Statement
{
	public:
		termStatement();
		termStatement(const string&);
		termStatement(const string&, const string&);		
		string			term; // if its type is Identifier or immediate
		idInfo*			id;
		bool			identifier;
		Statement*		arrayIndex;
		unsigned int	size;
};


class functionCall : public Statement
{
	public:
		functionCall();
		~functionCall();
		string			name;				// function to call
		Array<Statement*>*	parameters;		// Array of parameters with void , that can be anything
		int				functionType;
};

class forloop : public Statement
{
	public:
		forloop();
		~forloop();
		//string from;						// the starter value
		Array<Statement*>* fromCondition;
		//string to;							// the end value
		Array<Statement*>* toCondition;
		//string step;						// the step value
		Array<Statement*>* stepCondition;
		Array<Statement*>* body; 			// will point to the Statements in the Statements Array
};

class whileloop : public Statement
{
	public:
		whileloop();
		~whileloop();
		Statement* condition;						// the starter value
		Array<Statement*>* body; 			// will point to the Statements in the Statements Array
};

/*class adressingStatement : public Statement
{
	public:
		adressingStatement();
		~adressingStatement();
		Statement* condition;						// the starter value
		Array<Statement*>* body; 			// will point to the Statements in the Statements Array
};*/


class IF : public Statement
{
	public:
		IF();
		~IF();
		Statement*			condition;
		Array<Statement*>*	body; 		// will point to the Statements in the Statements Array
		Array<Statement*>*	ELSE;
		Array<Statement*>*	elseIF;
};

class CASE : public Statement
{
	public:
		CASE();
		~CASE();
		Statement* condition;
		//Array<Statement*>*	cases;		// Array of parameters with void , that can be anything
		//Array<Statement*>*	body;		// Array of parameters with void , that can be anything
		map<Statement*, Array<Statement*> > Body;
};

class EXTERN : public Statement
{
	public:
		EXTERN();
		~EXTERN();
		string			body;
		
		Array<idInfo>*		modifiedVariables;
		Array<idInfo>*		dependenciesVariables;
		
};

/*class expression : public Statement
{
	public:
	expression();
	Statement* expr;
	private:
	
};*/


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
	IFStatement,			/**< IF Statement */
	CASEStatement,			/**< Case Statement */
	DISTStatement,			/**< distribute Statement */
	adressingStatement,		/**< Node addressing Statement */
	Expr,					/**< Expression Statement */
	dvStatement,			/**< distributingVariablesStatement */
	RESETStatement,			/**< Reset Statement */
	PAStatement,			/**< Parent Add Statement */
	SNStatement,			/**< Set Node Statement */
	EXTERNStatement, 		/**< Extern Statement */
	THREADStatement, 		/**< Thread Statement */
	NOPSTATEMENT,			/**< nop Statement */
	BREAKSTATEMENT,			/**< break Statement */
	CONTINUESTATEMENT,		/**< continue Statement */

};

class ast_function
{
	public:
		ast_function();
		~ast_function();
		int						scope;				//
		string					name;				// global ? or in a class
		Array<Statement*>*		body;				// will point to the Statements in the Statements Array
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
