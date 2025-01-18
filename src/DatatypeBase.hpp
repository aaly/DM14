#ifndef DATATYPEBASE_HPP
#define DATATYPEBASE_HPP

/**
@file             DataType.hpp
@brief            DataType
@details          DataType class, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license
*/

#include "Array.hpp"
#include <string>
#include <vector>

using namespace std;

class funcInfo;

class DatatypeBase {
public:
  DatatypeBase();
  virtual int setCompatibleDatatypes();
  virtual int setOperations();
  int addOperator(string op);
  int addTypeValue(string type);
  int setID(const string &ID);
  Array<string> parents;

  string Value;
  string typeID;
  Array<string> dataTypes;
  Array<string> dataOperators;
  Array<string> CEquivalent;

  bool native;
  bool classType;
  bool enumType;
  bool protoType;
  bool Template;
  Array<string> templateNames;

  std::vector<funcInfo> memberVariables;
  std::vector<funcInfo> memberFunctions;

  bool hasTemplateType(const string &type);

private:
  // virtual string		Serialize();
  // virtual string		DeSerialize();

  std::vector<string> serializeMembers;
  std::vector<string> serializeMembersTypes;
  // template T<T>				Serialize(const string&
  // memberID, const string& memberType); template T<T>
  // DeSerialize(const string& memberID, const string& memberType);
};

#include "ast.hpp"
#include "types.hpp"

#endif // DATATYPEBASE_HPP
