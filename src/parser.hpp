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
			mapcode(const string&, std::shared_ptr<Array<ast_function>> const, const Array<includePath>&, const bool&, const int&, const int&);
			~mapcode();
			string					getFileName();
			std::shared_ptr<Array<ast_function>>		getFunctions();
			//multimap<string,string> getIncludes();
			Array<includePath> getIncludes();
			bool					isHeader();
			bool					setHeader(bool);
			int32_t						nodesCount;
			int32_t						dVariablesCount;
			std::shared_ptr<Array<string>>			ExternCodes;
			std::shared_ptr<Array<std::shared_ptr<Statement>>>		linkLibs;
			int32_t						Print();
			Array<std::shared_ptr<Statement>>		globalDeclarations;
			Array<std::shared_ptr<Statement>>		globalDefinitions;
			Array<DatatypeBase>		dataTypes;
			uint32_t				addInclude(includePath);
		
		private:
			string						fileName;
			std::shared_ptr<Array<ast_function>>		functions;
			//multimap<string,string>	includes;
			Array<includePath>	includes;
			bool						Header;
	};



	class Parser
	{
		public:
		
			 Parser(std::shared_ptr<Array<token>>, const string&, const bool);
			~Parser();
			
			void						setActiveStack(EBNF::callstack_t& stack);
			EBNF::callstack_t&			activeStack();

			std::shared_ptr<Array<ast_function>>		getFunctions();
			std::shared_ptr<Array<string>>				getExternCodes();
			//multimap<string,string>		getIncludes();
			//Array<pair<string,string> > getIncludes();
			Array<includePath>			getIncludes();
			std::shared_ptr<Array<mapcode>>				getMapCodes();
			// parse functions
			int32_t							parse();
			std::shared_ptr<Statement>					parseIncludes();
			std::shared_ptr<Statement>					parseFunction();
			std::shared_ptr<Statement>					parseStruct();
			std::shared_ptr<Statement>					parseDeclaration();
			std::shared_ptr<Statement>					parseDeclarationInternal();
			std::shared_ptr<Statement>					parseFunctionCall();
			std::shared_ptr<Statement>					parseFunctionCallInternal(bool, const string& returnType = "", const string& classID = "");
			//Statement*					parseConditionalExpression(Statement* stmt);
			std::shared_ptr<Statement>					parseForloop();
			std::shared_ptr<Statement>					parseIf();
			std::shared_ptr<Statement>					parseWhile();
			std::shared_ptr<Statement>					parseCase();
			std::shared_ptr<Statement>					parseMatrixIndex();
			std::shared_ptr<Statement>					parseArrayIndex();
			//DatatypeBase				parseClass();
			std::shared_ptr<Statement>					parseExtern();
			std::shared_ptr<Statement>					parseLink();
			std::shared_ptr<Statement>					parseDistribute();
			std::shared_ptr<Statement>					parseReset();
			std::shared_ptr<Statement>					parseSetNode();
			std::shared_ptr<Statement>					parseContinue();
			std::shared_ptr<Statement>					parseBreak();
			std::shared_ptr<Statement>					parseAddParent();
			std::shared_ptr<Statement>					parseThread();
			std::shared_ptr<Statement>					parseExpressionStatement();
			std::shared_ptr<Statement>					parseNOPStatement();
			std::shared_ptr<Statement>					parseOpStatement(int32_t, int32_t, const string&, const int32_t&, std::shared_ptr<Statement>, std::shared_ptr<idInfo> parent = nullptr, const string& parentOp = "");
			int32_t							increaseScope(std::shared_ptr<Statement>);
			int32_t							decreaseScope();
			int32_t							parseIncludesInsider(const string&, const string&, const includePath::sourceFileType);
			std::shared_ptr<Statement>	parseStatement(const std::string starting_rule, EBNF::parser_callback custom_callback = nullptr);
			//string 						getDataType(const string&);
			
			// helper functions
			int32_t							reachToken(const string&,const bool&, const bool&, const bool&, const bool&, const bool&);
			bool						peekToken(const string&);
			token						peekToken(const int32_t&);
			std::string 				getOpStatementType(std::string stmtType, const std::string& classID);
			
			int32_t extractSplitStatements(std::shared_ptr<Array<std::shared_ptr<Statement>>> array, std::shared_ptr<Array<std::shared_ptr<Statement>>> splitStatements);
			
			// including stuff
			int32_t							mapFunctions(const string&, const string&);
			
			std::shared_ptr<Statement>	findTreeNode(std::shared_ptr<Statement> opStatement, int32_t StatementType);
			int32_t							searchVariables(std::shared_ptr<Statement> opStatement, int32_t depencyType, string op="");
			
			// identifiers
			std::shared_ptr<distStatement>				distStatementTemp;
			std::shared_ptr<Array<idInfo>>				distModifiedGlobal;
			
			bool						isIdentifier(const string&);
			//bool						isClass(const string&);
			int32_t							findIDInfo(const idInfo&, const int32_t&);
			string						findIDType(const idInfo& ID, const string& classID = "");
			
			idInfo						findID(const string& ID, const string& parentID="");
			string						getType(const int32_t&);
			// Functions
			bool						isFunction(const string&, bool, const string& classID = "");
			bool						isUserFunction(const string&, bool,const string& classID = "");
			bool 						isBuiltinFunction(const string&);
			//int32_t						findFUNCInfo(const string&, const int32_t&);
			funcInfo					getFunc(const string&, const string& classID = "");
			funcInfo					getFunc(const string&, Array<string>*, const string& returnType, const string& classID);
			
			
			
			bool						checkToken(int32_t, string, bool);
			bool						checkToken(string, string, bool);
			bool						RequireType(string, string, bool);
			bool						RequireValue(string, string, bool);
			
			int32_t							distributedVariablesCount;
			int32_t							distributedNodesCount;
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
		
			int32_t addIncludePath(string path);
			
			std::shared_ptr<Statement>		bad_program();
			string							fName;
			
		private:
				std::shared_ptr<Array<token>> tokens;
				DM14::EBNF::EBNF ebnf;
				Array<EBNF::callstack_t> stackList; /// the stack nodes produced the the EBNF Parser
				
				token current_token; 				/** current poped token */
				Array<token>* working_tokens = nullptr;

				Array<DatatypeBase>	mapcodeDatatypes;
				
				std::shared_ptr<Statement> parseReturn();
				uint32_t getLevelOfEBNFRule(const std::string rule, const std::string start);
				
				

				std::shared_ptr<idInfo>					getTopParent(std::shared_ptr<idInfo>);
				std::shared_ptr<Array<idInfo>>			identifiers;
			
				bool							globalNoDist;
				Array<std::shared_ptr<Statement>>	globalDeclarations;
				Array<std::shared_ptr<Statement>>	globalDefinitions;
				
				int32_t								scope;
				int32_t								distributedScope;
				bool							Header;
				/** map for dividing files */
				std::shared_ptr<Array<mapcode>>			mapCodes;
				
				/** FUNCTIONS */
				
				std::shared_ptr<Array<ast_function>>	functions;
				std::shared_ptr<Array<funcInfo>>			functionsInfo;
				std::shared_ptr<Array<string>>				ExternCodes;

				//Array<pair<string,string>>		includes;
				Array<includePath>				includes;
				Array<string>			includePaths;
				string						package;
				string						library;
				
				int32_t							functionStatementsCount;
				int32_t							pushModified(const string&, idInfo&);
				int32_t							pushDependency(idInfo&);
				int32_t							pushModifiedGlobal(idInfo&);
				
				Array<pair<idInfo,int32_t>>		modifiedVariablesList;						
				Array<pair<idInfo,int32_t>>		dependenciesVariablesList;
				ast_function					currentFunction;
				
				bool							tmpScope;
				bool							insider;
				
				std::shared_ptr<Statement>		currentStatement;
				std::shared_ptr<Statement>		parentStatement;
				
				int32_t 							addStatementDistributingVariables(std::shared_ptr<Statement> stmt);
				std::shared_ptr<Array<std::shared_ptr<distributingVariablesStatement>>> dvList;
				std::shared_ptr<Array<std::shared_ptr<Statement>>>				linkLibs;
				
				long parseCClass(std::shared_ptr<scanner> scner, uint32_t start, const Array<string>& templateName);
				funcInfo parseCFunction(std::shared_ptr<scanner> scner, uint32_t start, const DatatypeBase& parentClass);

				bool restore(std::shared_ptr<std::vector<token>>);
				std::shared_ptr<std::vector<token>> extract(int32_t from, int32_t to);
				
				
				
				EBNF::callstack_t m_activeStack;
				//std::vector<token>* extract_temp_vector;
				
				
				/*token popToken();
				token popToken(const uint32_t index);
				token getToken();
				token getToken(const uint32_t index);
				int32_t 			index = 0;
				int32_t *input_tokens_index = nullptr;
				Statement* parseStatementInternal(Statement* output, const std::string starting_rule = "", EBNF::parser_callback custom_callback = nullptr);*/
	};
	
} //namespace DM14::parser

#endif // PARSER_H

