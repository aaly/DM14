/**
@file             DataType.cpp
@brief            DataType
@details          DataType class, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license
*/

#include "DatatypeBase.hpp"

DatatypeBase::DatatypeBase() {
  native = false;
  classType = false;
  protoType = false;
  Template = false;
  enumType = false;
}

/*
string DatatypeBase::Serialize()
{
        return "";
}

string DatatypeBase::DeSerialize()
{
        return "";
}*/

int DatatypeBase::setID(const string &ID) {
  if (ID.size() == 0) {
    return 1;
  }

  typeID = ID;
  return 0;
}

int DatatypeBase::setCompatibleDatatypes() { return 0; }

int DatatypeBase::setOperations() { return 0; }
int DatatypeBase::addOperator(string op) {
  if (op == "!" && typeID == "string") {
    cout << "ERRRRRRRRRRRRRRRRRRRRRRRRRRR" << endl;
  }
  dataOperators.push_back(op);
  return 0;
}

int DatatypeBase::addTypeValue(string type) {
  dataTypes.push_back(type);
  return 0;
}

bool DatatypeBase::hasTemplateType(const string &type) {
  for (unsigned int i = 0; i < templateNames.size(); i++) {
    if (type == templateNames.at(i)) {
      return true;
    }
  }
  return false;
}

/*
int DatatypeBase::Serialize(const string& memberID, const string& memberType)
{
        return 0;
}

int DatatypeBase::DeSerialize(const string& memberID, const string& memberType)
{
        return 0;
}*/
