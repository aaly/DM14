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
#include "EBNF.hpp" /** EBNF Grammar parser */
#include "ast.hpp"
#include "token.hpp" /**< class token */
#include "scanner.hpp" /**< class token */
#include <unordered_map> /**< EBNF */
#include <sstream> 

namespace DM14
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
			
			includePath(){}

			includePath(const std::string& package, const std::string& library, const includePath::sourceFileType includeType)
			{
				this->package = package;
				this->library = library;
				this->includeType = includeType;
			}

			bool operator==(const includePath& other) const
			{
				if(package == other.package &&
					library == other.library)
				{
					return true;
				}
				return false;
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
			Array<Statement*>*		linkLibs;
			int						Print();
			Array<Statement*>		globalDeclarations;
			Array<Statement*>		globalDefinitions;
			Array<DatatypeBase>		dataTypes;
			uint32_t				addInclude(includePath);
		
		private:
			string						fileName;
			Array<ast_function>*		functions;
			//multimap<string,string>	includes;
			Array<includePath>	includes;
			bool						Header;
	};



	class parser
	{
		public:
		
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
			Statement*					parseIncludes();
			Statement*					parseFunction();
			Statement*					parseStruct();
			Statement*					parseDeclaration();
			Statement*					parseDeclarationInternal();
			Statement*					parseFunctionCall();
			Statement*					parseFunctionCallInternal(bool, const string& returnType = "", const string& classID = "");
			//Statement*					parseConditionalExpression(Statement* stmt);
			Statement*					parseForloop();
			Statement*					parseIf();
			Statement*					parseWhile();
			Statement*					parseCase();
			Statement*					parseMatrixIndex();
			Statement*					parseArrayIndex();
			//DatatypeBase				parseClass();
			Statement*					parseExtern();
			Statement*					parseLink();
			Statement*					parseStatement(const std::string starting_rule,  EBNF::parser_callback = nullptr);
			Statement*					parseDistribute();
			Statement*					parseReset();
			Statement*					parseSetNode();
			Statement*					parseContinue();
			Statement*					parseBreak();
			Statement*					parseAddParent();
			Statement*					parseThread();
			Statement*					parseExpressionStatement();
			Statement*					parseNOPStatement();
			Statement*					parseOpStatement(int32_t, int32_t, const string&, const int&, Statement*, idInfo* parent = NULL, const string& parentOp = "");
			int							increaseScope(Statement*);
			int							decreaseScope();
			int							parseIncludesInsider(const string&, const string&, const includePath::sourceFileType);
			//string 						getDataType(const string&);
			
			// helper functions
			int							reachToken(const string&,const bool&, const bool&, const bool&, const bool&, const bool&);
			bool						peekToken(const string&);
			token						peekToken(const int&);
			std::string 				getOpStatementType(std::string stmtType, const std::string& classID);
			
			int extractSplitStatements(Array<Statement*>* array, Array<Statement*>* splitStatements);
			
			// including stuff
			int							mapFunctions(const string&, const string&);
			
			Statement*					findTreeNode(Statement* opStatement, int StatementType);
			int							searchVariables(Statement* opStatement, int depencyType, string op="");
			
			// identifiers
			distStatement*				distStatementTemp;
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
		
			int addIncludePath(string path);
			
			Statement*					bad_program();
			private:

				DM14::EBNF::EBNF ebnf;

				Array<DatatypeBase>	mapcodeDatatypes;
				
				Statement* parseReturn();
				uint32_t getLevelOfEBNFRule(const std::string rule, const std::string start);
				
				

				idInfo*							getTopParent(idInfo*);
				Array<idInfo>*			identifiers;
			
				bool							globalNoDist;
				Array<Statement*>	globalDeclarations;
				Array<Statement*>	globalDefinitions;
				string							fName;
				Array<token>*			tokens;
				int								scope;
				int								distributedScope;
				bool							Header;
				/** map for dividing files */
				Array<mapcode>*			mapCodes;
				
				/** FUNCTIONS */
				
				Array<ast_function>*	functions;
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
				
				Statement*						currentStatement;
				Statement*						parentStatement;
				
				int 							addStatementDistributingVariables(Statement* stmt);
				Array<distributingVariablesStatement*>* dvList;
				Array<Statement*>*				linkLibs;
				
				long parseCClass(scanner* scner, uint32_t start, const Array<string>& templateName);
				funcInfo parseCFunction(scanner* scner, uint32_t start, const DatatypeBase& parentClass);

				bool restore(std::vector<token>*);
				std::vector<token>* extract(int32_t from, int32_t to);
				//std::vector<token>* extract_temp_vector;
	};
	
} //namespace DM14::parser

#endif // PARSER_H

