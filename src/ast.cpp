/**
@file             ast.hpp
@brief            ast
@details          Abstract Syntax Tree classes, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license
*/
#include "ast.hpp"

funcInfo::funcInfo()
{
	parameters = new Array<pair<string, bool> >;
	noAutism = false;
	protoType = false;
	classifier = DM14::types::CLASSIFIER::PUBLIC;
	
};

int funcInfo::clear()
{
	//parameters = new Array<int>;
	name.clear();
	returnType = "NIL";
	//parameters->clear();
	//delete parameters;
	//parameters = new Array<int>;
	classConstructor = false;
	return(0);
};

with::with()
{
	package = new string;
	library = new string;
}

with::~with()
{
	delete package;
	delete library;
}


breakStatement::breakStatement()
{
	StatementType = BREAKSTATEMENT;
}

continueStatement::continueStatement()
{
	StatementType = CONTINUESTATEMENT;
}

NOPStatement::NOPStatement()
{
	StatementType = NOPSTATEMENT;
}

NOPStatement::~NOPStatement()
{
}


/*
Class::Class()
{
}

Class::~Class()
{
}*/

threadStatement::threadStatement()
{
	functioncall = NULL;
	ID = "";
	classMember = false;
	StatementType = THREADStatement;
};
threadStatement::~threadStatement()
{
	if(functioncall)
	{
		delete functioncall;
	}
};


Link::Link()
{
	Static = false;
	libs = " ";
}

Link::~Link()
{
}

idInfo::idInfo()
{
	array=false;
	tmpScope=false;
	pointer =false;
	internal = false;
	distributed = true;
	backProp = false;
	recurrent = false;
	channel = false;
	noblock = false;
	//parent = "";
	parent = NULL;
	functionParent = "";
	global = false;
	shared = false;
	requestAddress = false;
	//name = "";
	type = "";
	//value = "";
};


idInfo::idInfo(const string& ID, const int& IDscope, const string& IDtype, Statement* aIndex)
{
	idInfo();
	name = ID;
	scope = IDscope;
	type = IDtype;
	arrayIndex = aIndex;
	parent = NULL;
	
};

idInfo::idInfo(const string& ID, const int& IDscope, const string& IDtype, const string& Value,Statement* aIndex)
{
	name = ID;
	scope = IDscope;
	type = IDtype;
	value = Value;
	arrayIndex = aIndex;
	parent = NULL;
	idInfo();
};

//idInfo* idInfo::getParent()
//{
//	return parent;
//};

int	idInfo::setParent(const string& parentID)
{
	//parent = parentID;
	return 0;
};

Statement::Statement()
{
	scopeLevel = 0;
	//distStatements = new Array<distributingVariablesStatement*>();
	
	line = 0;
	scope = 0;
	StatementType = 0;
	type = "";
};

void Statement::absorbDistStatements(Statement* arg)
{
	if(arg == NULL)
	{
		return;
	}

	for(uint32_t i = 0; i < arg->distStatements.size(); i++)
	{
		distStatements.push_back(arg->distStatements.at(i));
	}

	arg->distStatements.clear();
}

emptyStatement::emptyStatement()
{
	StatementType = eStatement;
};

returnStatement::returnStatement()
{
	StatementType = rStatement;
	retValue = NULL;
};

declareStatement::declareStatement()
{
	StatementType = dStatement;
	value = NULL;
	identifiers = new Array<idInfo>;
	array = false;
	tmpScope = false;
	backProp = false;
	recurrent = false;
	channel = false;
	noblock = false;
	classtype = false;
	global = false;
	shared = false;
	distributed = true;
};

declareStatement::~declareStatement()
{
	delete value;
	delete identifiers;
};

operationalStatement::operationalStatement()
{
	StatementType = oStatement;
	//right = new Array<Statement*>;
	right = NULL;
	left  = NULL;
	op = "";
};

operationalStatement::~operationalStatement()
{
	delete right;
	delete left;
};


termStatement::termStatement()
{
	StatementType = tStatement;
	arrayIndex = NULL;
	identifier = false;
	size = 0;
};

distStatement::distStatement()
{
	StatementType = DISTStatement;
	//bigOrder++;
	//order=distStatement::bigOrder;
	dependenciesVariables	= new Array<idInfo>;
	modifiedVariables		= new Array<idInfo>;
	
};

distStatement::~distStatement()
{
	delete dependenciesVariables;
	delete modifiedVariables;
	
};

resetStatement::resetStatement()
{
	count = NULL;
	StatementType = RESETStatement;
};

resetStatement::~resetStatement()
{
	
};


parentAddStatement::parentAddStatement()
{
	socket = NULL;
	ip = NULL;
	StatementType = PAStatement;
};

parentAddStatement::~parentAddStatement()
{
	
};

setNodeStatement::setNodeStatement()
{
	node = NULL;
	StatementType = SNStatement;
};

setNodeStatement::~setNodeStatement()
{
	
};



termStatement::termStatement(const string& value)
{
	StatementType = tStatement;
	term = value; // immediate or identifier :D
	arrayIndex = NULL;
	id = NULL;
	size = 0;
};

termStatement::termStatement(const string& value, const string& Stype)
{
	StatementType = tStatement;
	term = value; // immediate or identifier :D
	type = Stype;
	arrayIndex = NULL;
	id = NULL;
	size = 0;
};

functionCall::functionCall()
{
	StatementType = fCall;
	parameters = new Array<Statement*>;
};


functionCall::~functionCall()
{
	delete parameters;
};


forloop::forloop()
{
	StatementType = fLoop;
	fromCondition = new Array<Statement*>;
	toCondition = new Array<Statement*>;
	stepCondition = new Array<Statement*>;
	body = new Array<Statement*>;
};

forloop::~forloop()
{
	delete fromCondition;
	delete toCondition;
	delete stepCondition;
	delete body;
};


whileloop::whileloop()
{
	StatementType = wLoop;
	condition = new Statement();
	body = new Array<Statement*>;
};


whileloop::~whileloop()
{
	delete condition;
	delete body;
};


IF::IF()
{
	StatementType = IFStatement;
	condition = new Statement;
	body = new Array<Statement*>;
	ELSE = new Array<Statement*>;
	elseIF = new Array<Statement*>;
}

IF::~IF()
{
	delete condition;
	delete body;
	delete ELSE;
	delete elseIF;
}

CASE::CASE()
{
//	body = new map<expression,Statement>;
	StatementType = CASEStatement;
	condition = NULL;
}

CASE::~CASE()
{
	delete condition;
}

ast_function::ast_function()
{
	body = new Array<Statement*>;
	parameters = new Array<idInfo>;
	distributed = false;
}

distributingVariablesStatement::distributingVariablesStatement()
{
	StatementType = dvStatement;
	dependencyNode = -1;
};



EXTERN::EXTERN()
{
	StatementType = EXTERNStatement;
	dependenciesVariables	= new Array<idInfo>;
	modifiedVariables		= new Array<idInfo>;
	
};

EXTERN::~EXTERN()
{
	delete dependenciesVariables;
	delete modifiedVariables;
	
};

ast_function::~ast_function()
{
	//delete body;
	//delete parameters;
}
