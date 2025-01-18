/**
@file             ast.hpp
@brief            ast
@details          Abstract Syntax Tree classes, Part of DM14 programming
language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2020
@version          1.1a
@copyright        See file "license" for bsd license
*/
#include "ast.hpp"

funcInfo::funcInfo() {
  parameters = std::make_shared<Array<pair<string, bool>>>();

  noAutism = false;
  protoType = false;
  classifier = DM14::types::CLASSIFIER::PUBLIC;
};

int funcInfo::clear() {
  // parameters = new Array<int>;
  name.clear();
  returnType = "NIL";
  // parameters->clear();
  // delete parameters;
  // parameters = new Array<int>;
  classConstructor = false;
  return 0;
};

with::with() {
  package = std::make_shared<std::string>();
  library = std::make_shared<std::string>();
}

with::~with() {}

breakStatement::breakStatement() { StatementType = BREAKSTATEMENT; }

continueStatement::continueStatement() { StatementType = CONTINUESTATEMENT; }

NOPStatement::NOPStatement() { StatementType = NOPSTATEMENT; }

NOPStatement::~NOPStatement() {}

/*
Class::Class()
{
}

Class::~Class()
{
}*/

threadStatement::threadStatement() {
  functioncall = nullptr;
  ID = "";
  classMember = false;
  StatementType = THREADStatement;
};
threadStatement::~threadStatement() {};

Link::Link() {
  Static = false;
  libs = " ";
}

Link::~Link() {}

idInfo::idInfo() {
  array = false;
  tmpScope = false;
  pointer = false;
  internal = false;
  distributed = true;
  backProp = false;
  recurrent = false;
  channel = false;
  noblock = false;
  // parent = "";
  parent = nullptr;
  functionParent = "";
  global = false;
  shared = false;
  requestAddress = false;
  // name = "";
  type = "";
  // value = "";
};

idInfo::idInfo(const string &ID, const int &IDscope, const string &IDtype,
               std::shared_ptr<Statement> aIndex) {
  idInfo();
  name = ID;
  scope = IDscope;
  type = IDtype;
  arrayIndex = aIndex;
  parent = nullptr;
};

idInfo::idInfo(const string &ID, const int &IDscope, const string &IDtype,
               const string &Value, std::shared_ptr<Statement> aIndex) {
  name = ID;
  scope = IDscope;
  type = IDtype;
  value = Value;
  arrayIndex = aIndex;
  parent = nullptr;
  idInfo();
};

// idInfo* idInfo::getParent()
//{
//	return parent;
// };

int idInfo::setParent(const string &parentID) {
  // parent = parentID;
  return 0;
};

Statement::Statement() {
  scopeLevel = 0;
  // distStatements = new
  // Array<distributingVariablesstd::shared_ptr<Statement>>();

  line = 0;
  scope = 0;
  StatementType = 0;
  type = "";
};

void Statement::absorbDistStatements(std::shared_ptr<Statement> arg) {
  if (arg == nullptr) {
    return;
  }

  for (uint32_t i = 0; i < arg->distStatements->size(); i++) {
    distStatements->push_back(arg->distStatements->at(i));
  }

  arg->distStatements->clear();
}

emptyStatement::emptyStatement() { StatementType = eStatement; };

returnStatement::returnStatement() {
  StatementType = rStatement;
  retValue = nullptr;
};

declareStatement::declareStatement() {
  StatementType = dStatement;
  value = nullptr;
  identifiers = std::make_shared<Array<idInfo>>();
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

declareStatement::~declareStatement() {};

operationalStatement::operationalStatement() {
  StatementType = oStatement;
  // right = std::make_shared<Array<std::shared_ptr<Statement>>>();
  right = nullptr;
  left = nullptr;
  op = "";
};

operationalStatement::~operationalStatement() {};

termStatement::termStatement() {
  StatementType = tStatement;
  arrayIndex = nullptr;
  identifier = false;
  size = 0;
};

distStatement::distStatement() {
  StatementType = DISTStatement;
  // bigOrder++;
  // order=distStatement::bigOrder;
  dependenciesVariables = std::make_shared<Array<idInfo>>();
  modifiedVariables = std::make_shared<Array<idInfo>>();
};

distStatement::~distStatement() {

};

resetStatement::resetStatement() {
  count = nullptr;
  StatementType = RESETStatement;
};

resetStatement::~resetStatement() {

};

parentAddStatement::parentAddStatement() {
  socket = nullptr;
  ip = nullptr;
  StatementType = PAStatement;
};

parentAddStatement::~parentAddStatement() {

};

setNodeStatement::setNodeStatement() {
  node = nullptr;
  StatementType = SNStatement;
};

setNodeStatement::~setNodeStatement() {

};

termStatement::termStatement(const string &value) {
  StatementType = tStatement;
  term = value; // immediate or identifier :D
  arrayIndex = nullptr;
  id = nullptr;
  size = 0;
};

termStatement::termStatement(const string &value, const string &Stype) {
  StatementType = tStatement;
  term = value; // immediate or identifier :D
  type = Stype;
  arrayIndex = nullptr;
  id = nullptr;
  size = 0;
};

functionCall::functionCall() {
  StatementType = fCall;
  parameters = std::make_shared<Array<std::shared_ptr<Statement>>>();
};

functionCall::~functionCall() {};

forloop::forloop() {
  StatementType = fLoop;
  fromCondition = std::make_shared<Array<std::shared_ptr<Statement>>>();
  toCondition = std::make_shared<Array<std::shared_ptr<Statement>>>();
  stepCondition = std::make_shared<Array<std::shared_ptr<Statement>>>();
  body = std::make_shared<Array<std::shared_ptr<Statement>>>();
};

forloop::~forloop() {

};

whileloop::whileloop() {
  StatementType = wLoop;
  condition = std::make_shared<Statement>();
  body = std::make_shared<Array<std::shared_ptr<Statement>>>();
};

whileloop::~whileloop() {};

IF::IF() {
  StatementType = IFStatement;
  condition = std::make_shared<Statement>();
  body = std::make_shared<Array<std::shared_ptr<Statement>>>();
  ELSE = std::make_shared<Array<std::shared_ptr<Statement>>>();
  elseIF = std::make_shared<Array<std::shared_ptr<Statement>>>();
}

IF::~IF() {}

CASE::CASE() {
  //	body = new map<expression,Statement>;
  StatementType = CASEStatement;
  condition = std::make_shared<Statement>();
}

CASE::~CASE() {}

ast_function::ast_function() {
  body = make_shared<Array<std::shared_ptr<Statement>>>();
  parameters = make_shared<Array<idInfo>>();
  distributed = false;
}

distributingVariablesStatement::distributingVariablesStatement() {
  StatementType = dvStatement;
  dependencyNode = -1;
};

EXTERN::EXTERN() {
  StatementType = EXTERNStatement;
  dependenciesVariables = std::make_shared<Array<idInfo>>();
  modifiedVariables = std::make_shared<Array<idInfo>>();
};

EXTERN::~EXTERN() {

};

ast_function::~ast_function() {
  // delete body;
  // delete parameters;
}
