#ifndef	PARSER_H
#define	PARSER_H

/**
@file             parser.hpp
@brief            parser
@details          parser, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license
*/

#include <regex> /**<  grammar_token_type::REGEX_TOKEN */
#include <iostream> /**< std::cerr, std::cout, std::endl */
#include <string> /**< std::string */
#include "types.hpp"
#include "ast.hpp"
#include "scanner.hpp" /**< class token */
#include <unordered_map> /**< EBNF */
#include <sstream> 



extern Array<DatatypeBase>	datatypes;
static Array<DatatypeBase>	mapcodeDatatypes;

namespace DM14::parser
{
using namespace std;


class includePath
{
	public:

		enum class sourceFileType
		{
			FILE_DM14 = 1,
			FILE_C,
			LIBRARY
		};

		std::string package;
		std::string library;
		sourceFileType includeType;
		
		includePath()
		{

		}

		includePath(const std::string& package, const std::string& library, const includePath::sourceFileType includeType)
		{
			this->package = package;
			this->library = library;
			this->includeType = includeType;
		}

		
};

class mapcode
{
	public:
		mapcode();
		mapcode(const string&, Array<ast_function>* const, const Array<includePath>&, const bool&, const int&, const int&);
		~mapcode();
		string					getFileName();
		Array<ast_function>*		getFunctions();
		//multimap<string,string> getIncludes();
		Array<includePath> getIncludes();
		bool					isHeader();
		bool					setHeader(bool);
		int						nodesCount;
		int						dVariablesCount;
		Array<string>*			ExternCodes;
		Array<statement*>*		linkLibs;
		int						Print();
		Array<statement*>		globalDeclarations;
		Array<statement*>		globalDefinitions;
		Array<DatatypeBase>		dataTypes;
		uint32_t				addInclude(includePath);
	
	private:
		string						fileName;
		Array<ast_function>*		functions;
		//multimap<string,string>	includes;
		Array<includePath>	includes;
		bool						Header;
};


typedef enum
{
	DATATTYPE_TOKEN=0,
	TERMINAL_TOKEN,
	REGEX_TOKEN,
	EXPANSION_TOKEN,
	SINGLE_OP_TOKEN,
	CORE_OP_TOKEN,
	BINARY_OP_TOKEN,
	IMMEDIATE_TOKEN,
	KEYWORD_TOKEN,
}
grammar_token_type;

class parser;

typedef statement* (parser::*parser_callback)();

//typedef struct


class EBNF_token_t
{
	public:
	
    EBNF_token_t() = default;
    EBNF_token_t(const EBNF_token_t&) = default;
	string expansion;
	grammar_token_type tokenType; // type or terminal or regex
	parser_callback callback;
};

typedef enum
{
	GRAMMAR_TOKEN_OR_ARRAY=0,
	GRAMMAR_TOKEN_AND_ARRAY,
	GRAMMAR_TOKEN_ZERO_MORE_ARRAY,
	GRAMMAR_TOKEN_ONE_MORE_ARRAY,
	GRAMMAR_TOKEN_ONLY_ONE_ARRAY,
}
grammar_rule_type_t;


typedef struct
{
	grammar_rule_type_t type;
	std::vector<EBNF_token_t> tokens;
}
grammar_rule_t;

typedef std::unordered_map <std::string, std::vector<grammar_rule_t> > EBNF_map_t;

typedef std::pair<int, statement*> ebnfResult;


class parser
{
	public:
	
		ebnfResult parseEBNF(Array<token>* input_tokens, std::string start_map_index, Array<token>* output_tokens, const bool verify_only = false);

		Array<token>* working_tokens = NULL;
		//int *working_tokens_index = NULL;
		Array<token>* input_tokens = NULL;
		int *input_tokens_index = NULL;
		bool pushStatements = true;
		Array<statement*> statements_stack;
		 parser(Array<token>*, const string&, const bool);
		~parser();

		int							nextIndex();
		Array<ast_function>*			getFunctions();
		Array<string>*				getExternCodes();
		//multimap<string,string>		getIncludes();
		//Array<pair<string,string> > getIncludes();
		Array<includePath>			 getIncludes();
		Array<mapcode>*				getMapCodes();
		// parse functions
		int							parse();
		statement*					parseIncludes();
		statement*					parseFunction();
		statement*					parseStruct();
		statement*					parseDeclaration();
		statement*					parseDeclarationInternal(const string& terminal = ";");
		statement*					parseFunctionCall();
		statement*					parseFunctionCallInternal(bool, const string& returnType = "", const string& classID = "");
		statement*					parseConditionalExpression(statement* stmt);
		statement*					parseForloop();
		statement*					parseIF();
		statement*					parseWhile();
		statement*					parseCase();
		//DatatypeBase				parseClass();
		statement*					parseExtern();
		statement*					parseLink();
		statement*					parseStatement(const std::string starting_rule);
		statement*					parseDistribute();
		statement*					parseReset();
		statement*					parseSetNode();
		statement*					parseAddParent();
		statement*					parseThread();
		statement*					parseExpressionStatement();
		statement*					parseNOPStatement();
		statement*					parseOpStatement(int32_t, int32_t, const string&, const int&, statement*, idInfo* parent = NULL, const string& parentOp = "");
		int							increaseScope(statement*);
		int							decreaseScope();
		int							parseIncludesInsider(const string&, const string&, const includePath::sourceFileType);
		//string 						getDataType(const string&);
		
		// helper functions
		int							reachToken(const string&,const bool&, const bool&, const bool&, const bool&, const bool&);
		bool						peekToken(const string&);
		token						peekToken(const int&);
		
		int extractSplitStatements(Array<statement*>* array, Array<statement*>* splitStatements);
		
		// including stuff
		int							mapFunctions(const string&, const string&);
		
		statement*					findTreeNode(statement* opstatement, int statementType);
		int							searchVariables(statement* opstatement, int depencyType, string op="");
		
		// identifiers
		distStatement*				diststatementTemp;
		Array<idInfo>*				distModifiedGlobal;
		
		bool						isIdentifier(const string&);
		//bool						isClass(const string&);
		int							findIDInfo(const idInfo&, const int&);
		string						findIDType(const idInfo& ID, const string& classID = "");
		
		idInfo						findID(const string& ID, const string& parentID="");
		string						getType(const int&);
		// Functions
		bool						isFunction(const string&, bool, const string& classID = "");
		bool						isUserFunction(const string&, bool,const string& classID = "");
		bool 						isBuiltinFunction(const string&);
		//int						findFUNCInfo(const string&, const int&);
		funcInfo					getFunc(const string&, const string& classID = "");
		funcInfo					getFunc(const string&, Array<string>*, const string& returnType, const string& classID);
		

		//bool						isImmediate(const string&);
		bool 						isImmediate(const token& tok);
		
		bool						checkToken(int, string, bool);
		bool						checkToken(string, string, bool);
		bool						RequireType(string, string, bool);
		bool						RequireValue(string, string, bool);
		
		int							distributedVariablesCount;
		int							distributedNodesCount;
		enum  returnTypes
		{
			SCOPE =1,
			TYPE,
			VALUE,
			RETURNTYPE,
			PARANETERTYPE,
			INITIALIZED,
			ARRAY,
			ARRAYSIZE,
			TMPSCOPE,
			DISTRIBUTED,
			BACKPROB,
			RECURRENT,
			CHANNEL,
			NOBLOCK,
			CLASS,
			GLOBAL
		};

		
		
		
		/*enum  classClassifier
		{
			PRIVATE=1,
			PUBLIC=2,
			PROTECTED=3
		};*/
		

		/** @details print the Extended BNF grammar */
		int printEBNF();

		int addIncludePath(string path);
		private:
		
			Array<string> ebnf_verification_list;

			token current_token;
			int token_index = -1;
			int pushToken(token tok);
			int removeToken();
			Array<token> tokens_stack;
			token popToken();
			token getToken();
			token getToken(const uint32_t index);

			
			statement* parseReturn();
			EBNF_map_t EBNF;
			uint32_t getLevelOfEBNFRule(const std::string rule, const std::string start);
			
			int deadvance_EBNFindex(uint16_t steps = 1);
			int advance_EBNFindex(uint16_t steps = 1);
			statement* empty_file();

			idInfo*						getTopParent(idInfo*);
			Array<idInfo>*				identifiers;
		
			bool						globalNoDist;
			Array<statement*>			globalDeclarations;
			Array<statement*>			globalDefinitions;
			string						fName;
			Array<token>*				tokens;
			int							index;
			int							scope;
			int							distributedScope;
			
			bool						Header;
			// map for dividing files
			Array<mapcode>*			mapCodes;
			
			// FUNCTIONS
			
			Array<ast_function>*			functions;
			Array<funcInfo>*			functionsInfo;
			
			Array<string>*				ExternCodes;

			//Array<pair<string,string>>		includes;
			Array<includePath>				includes;
			Array<string>			includePaths;
			string						package;
			string						library;
			
			int							functionStatementsCount;
			int							pushModified(const string&, idInfo&);
			int							pushDependency(idInfo&);
			int							pushModifiedGlobal(idInfo&);
			
			Array < pair<idInfo,int> >		modifiedVariablesList;						
			Array < pair<idInfo,int> >		dependenciesVariablesList;
			ast_function						currentFunction;
			
			bool							tmpScope;
			bool							insider;
			
			statement*						currentStatement;
			statement*						parentStatement;
			
			int 							addStatementDistributingVariables(statement* stmt);
			Array<distributingVariablesStatement*>* dvList;
			Array<statement*>*				linkLibs;
			
			long parseCClass(scanner* scner, uint32_t start, const Array<string>& templateName);
			funcInfo parseCFunction(scanner* scner, uint32_t start, const DatatypeBase& parentClass);


};

} //namespace DM14::parser

#endif // PARSER_H

