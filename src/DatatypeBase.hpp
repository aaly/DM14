#ifndef	DATATYPEBASE_HPP
#define	DATATYPEBASE_HPP

#include <vector>
#include <string>
#include "Array.hpp"

using namespace std;

class funcInfo;

class DatatypeBase
{
	public :
		DatatypeBase();
		virtual int		setCompatibleDatatypes();
		virtual int		setOperations();
		int				addOperator (string op);
		int				addTypeValue (string type);
		int				setID(const string& ID);
		Array<string> 	parents;
	
		string			Value;
		string			typeID;
		Array<string>	dataTypes;
		Array<string>	dataOperators;
		Array<string>	CEquivalent;
		
		bool				native;
		bool				classType;
		bool				enumType;
		bool				protoType;
		bool				Template;
		Array<string>		templateNames;
		
		std::vector<funcInfo>	memberVariables;
		std::vector<funcInfo>	memberFunctions;
		
		bool				hasTemplateType (const string& type);
	
	private :
	
		//virtual string		Serialize();
		//virtual string		DeSerialize();
		
		std::vector<string>	serializeMembers;
		std::vector<string>	serializeMembersTypes;
		//template T<T>				Serialize(const string& memberID, const string& memberType);
		//template T<T>				DeSerialize(const string& memberID, const string& memberType);
};

#include "types.hpp"

#include "ast.hpp"

#endif	//DATATYPEBASE_HPP
