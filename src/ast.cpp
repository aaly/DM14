// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license

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


NOPStatement::NOPStatement()
{
	statementType = NOPSTATEMENT;
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
	statementType = THREADStatement;
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


idInfo::idInfo(const string& ID, const int& IDscope, const string& IDtype, statement* aIndex)
{
	idInfo();
	name = ID;
	scope = IDscope;
	type = IDtype;
	arrayIndex = aIndex;
	parent = NULL;
	
};

idInfo::idInfo(const string& ID, const int& IDscope, const string& IDtype, const string& Value,statement* aIndex)
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

statement::statement()
{
	scopeLevel = 0;
	//distStatements = new Array<distributingVariablesStatement*>();
	
	line = 0;
	scope = 0;
	statementType = 0;
	type = "";
};


emptyStatement::emptyStatement()
{
	statementType = eStatement;
};

returnStatement::returnStatement()
{
	statementType = rStatement;
	retValue = NULL;
};

declareStatement::declareStatement()
{
	statementType = dStatement;
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
	statementType = oStatement;
	//right = new Array<statement*>;
	right = NULL;
	left  = NULL;
	op = "";
};

operationalStatement::~operationalStatement()
{
	delete right;
	delete left;
};


termStatment::termStatment()
{
	statementType = tStatement;
	arrayIndex = NULL;
	identifier = false;
	size = 0;
};

distStatement::distStatement()
{
	statementType = DISTStatement;
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
	statementType = RESETStatement;
};

resetStatement::~resetStatement()
{
	
};


parentAddStatement::parentAddStatement()
{
	socket = NULL;
	ip = NULL;
	statementType = PAStatement;
};

parentAddStatement::~parentAddStatement()
{
	
};

setNodeStatement::setNodeStatement()
{
	node = NULL;
	statementType = SNStatement;
};

setNodeStatement::~setNodeStatement()
{
	
};



termStatment::termStatment(const string& value)
{
	statementType = tStatement;
	term = value; // immediate or identifier :D
	arrayIndex = NULL;
	id = NULL;
	size = 0;
};

termStatment::termStatment(const string& value, const string& Stype)
{
	statementType = tStatement;
	term = value; // immediate or identifier :D
	type = Stype;
	arrayIndex = NULL;
	id = NULL;
	size = 0;
};

functionCall::functionCall()
{
	statementType = fCall;
	parameters = new Array<statement*>;
};


functionCall::~functionCall()
{
	delete parameters;
};


forloop::forloop()
{
	statementType = fLoop;
	fromCondition = new Array<statement*>;
	toCondition = new Array<statement*>;
	stepCondition = new Array<statement*>;
	body = new Array<statement*>;
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
	statementType = wLoop;
	condition = new statement();
	body = new Array<statement*>;
};


whileloop::~whileloop()
{
	delete condition;
	delete body;
};


IF::IF()
{
	statementType = IFStatement;
	condition = new statement;
	body = new Array<statement*>;
	ELSE = new Array<statement*>;
	elseIF = new Array<statement*>;
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
//	body = new map<expression,statement>;
	statementType = CASEStatement;
	condition = NULL;
}

CASE::~CASE()
{
	delete condition;
}

ast_function::ast_function()
{
	body = new Array<statement*>;
	parameters = new Array<idInfo>;
	distributed = false;
}

distributingVariablesStatement::distributingVariablesStatement()
{
	statementType = dvStatement;
	dependencyNode = -1;
};



EXTERN::EXTERN()
{
	statementType = EXTERNStatement;
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
