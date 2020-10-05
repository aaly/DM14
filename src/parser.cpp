/**
@file             parser.cpp
@brief            parser
@details          parser, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2020
@version          1.1a
@copyright        See file "license" for bsd license

/page  Parser the DM14 Parser

/section EBNF

the extended back-naur form grammar is possible to express using the provided functions such as \ref parseEBNF() , however it is tricky
to accomplish some goals.

@TODO: the parenthed expressions should be kept and added to the EBNF too
@TODO: xa["x"] = 10; if xa is a map only .... and map key type is "x"
*/

#include "parser.hpp"

namespace DM14
{
	std::shared_ptr<Statement> Parser::bad_program()
	{		
		displayError(fName, -1,0,"unable to proceed file, error follows :", false);
		//old_successful_depth.print();
		ebnf.old_depth.print();
		std::cerr  << "i can no recognize token[" << ebnf.old_depth.input_tokens_index << "] : " << ebnf.input_tokens->at(ebnf.old_depth.input_tokens_index).value <<  std::endl;
		std::cerr  << "on this line : " <<  std::endl;

		int errorLine = ebnf.input_tokens->at(ebnf.old_depth.input_tokens_index).lineNumber;

		for(uint32_t i =0; i < tokens->size(); i++)
		{
			if(tokens->at(i).lineNumber == errorLine)
			{
				std::cerr << tokens->at(i).value;
			}
		}

		std::cerr << std::endl;
		exit(1);
		return nullptr;
	}

	//bool EBNF_is_index_frozen = false; /** boolean used by freeze_EBNFindex() and unfreeze_EBNFindex() */
	//int EBNF_frozen_index = -1; /** last index before freezing the EBNF index using freeze_EBNFindex(), to be used for restoration using unfreeze_EBNFindex()*/

	Parser::Parser(std::shared_ptr<Array<token>> gtokens, const string& filename, const bool insider = true)
	{
		if(gtokens == nullptr || gtokens->size() == 0)
		{
			displayError(filename, 0,0,"Internal parser error !!!, no tokens , maybe empty source file ?");
		}
		tokens = gtokens;	
		ebnf.tokens				=	gtokens.get();
		fName				=	filename;
		
		functions			=	std::make_shared<Array<ast_function>>();
		identifiers			=	std::make_shared<Array<idInfo>>();
		functionsInfo		=	std::make_shared<Array<funcInfo>>();
		ExternCodes			=	std::make_shared<Array<string>>();
		mapCodes 			=	std::make_shared<Array<mapcode>>();
		scope				=	0;
		Header				=	false;
		distStatementTemp	=	std::make_shared<distStatement>();
		distributedVariablesCount = 0;
		distributedNodesCount = 0;
		tmpScope = false;
		globalNoDist = false;
		
		ebnf.input_tokens = tokens.get();
		
		dvList				=	std::make_shared<Array<std::shared_ptr<distributingVariablesStatement>>>();
		linkLibs			=	std::make_shared<Array<std::shared_ptr<Statement>>>();

		this->insider = insider;
		
		distModifiedGlobal	= std::make_shared<Array<idInfo>>();
		distStatementTemp	= std::make_shared<distStatement>();
		modifiedVariablesList = Array<pair<idInfo,int>>();
		increaseScope(nullptr);
		distributedScope = 0;
		functionStatementsCount = 0;
		
		ebnf.setParserInstance(this);
		
	
		
		ebnf.grammar["program"] = {{EBNF::GRAMMAR_TOKEN_OR,
														{{"function-list", EBNF::EXPANSION_TOKEN, &DM14::Parser::parseFunction},
														//TODO {"global-Statement",EXPANSION_TOKEN}, add all allowed Statements in the global scope !?
														{"Statement", EBNF::EXPANSION_TOKEN},
														{"unknown-list", EBNF::EXPANSION_TOKEN, &DM14::Parser::bad_program}}}};
		
		ebnf.grammar["unknown-list"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{".*",EBNF::REGEX_TOKEN}}}};
		ebnf.grammar["identifier"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"[a-zA-Z]+([a-zA-Z_0-9])*",EBNF::REGEX_TOKEN}}}};
		ebnf.grammar["Statement-list"] = {{EBNF::GRAMMAR_TOKEN_ZERO_MORE ,{{"Statement",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["Statement"] = {{EBNF::GRAMMAR_TOKEN_OR,{
							{"include-Statement",EBNF::EXPANSION_TOKEN,&DM14::Parser::parseIncludes},
								{"declaration-full-Statement",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseDeclaration},
								{"for-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseForloop}, /** for(from -> to : step) { Statements; } */
								//{"extern-Statement", EBNF::EXPANSION_TOKEN, &DM14::Parser::parseExtern}, /** extern  { c/c++ code } endextern */
								{"link-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseLink},
								{"struct-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseStruct},
								{"if-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseIf}, /** if [expr] {} else if[] {} else {} */
								{"distribute",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseDistribute},
								{"reset-Statement",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseReset},
								{"setnode-Statement",EBNF::EXPANSION_TOKEN},
								{"while-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseWhile}, /** while [ cond ] { Statements } */
								//{"case-list",EBNF::EXPANSION_TOKEN}, /** case [ID/expr] in { 1) ; 2) ; *) ;}   body is like map<condition,Statements> */
								//{"addparent-Statement",EBNF::EXPANSION_TOKEN},
								{"thread-Statement",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseThread},
								{"function-call-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseFunctionCall},
								{"return-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseReturn},
								{"break-Statement",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseBreak},
								{"continue-Statement",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseContinue},
								{"nop-Statement",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseNOPStatement},
								{"expression-Statement", EBNF::EXPANSION_TOKEN, &DM14::Parser::parseExpressionStatement}}}};

		/// the with Statement
		ebnf.grammar["include-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"with",EBNF::KEYWORD_TOKEN}, {"include-Statement-body",EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["include-Statement-body"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"include-Statement-package",EBNF::EXPANSION_TOKEN}, {"include-Statement-file",EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["include-Statement-package"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"[a-zA-Z0-9]+",EBNF::REGEX_TOKEN}, {"use",EBNF::KEYWORD_TOKEN}, {"[a-zA-Z0-9]+",EBNF::REGEX_TOKEN}}}};
		ebnf.grammar["include-Statement-file"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"\"[a-zA-Z0-9]+[\\.]?[[a-zA-Z0-9]+]?\"",EBNF::REGEX_TOKEN}}}};
		
		/// the extern Statement
		ebnf.grammar["extern-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"extern",EBNF::KEYWORD_TOKEN},
									{"string",EBNF::DATATYPE_TOKEN},
									{"endextern",EBNF::KEYWORD_TOKEN}}}};

		/// the link Statement
		ebnf.grammar["link-list"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"link-Statement",EBNF::EXPANSION_TOKEN}, {"slink-Statement",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["link-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"link",EBNF::KEYWORD_TOKEN}, {"string",EBNF::DATATYPE_TOKEN}}}};
		ebnf.grammar["slink-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"slink",EBNF::KEYWORD_TOKEN}, {"string",EBNF::DATATYPE_TOKEN}}}};

		/// the distribute Statement
		ebnf.grammar["distribute"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"distribute",EBNF::KEYWORD_TOKEN}, {";",EBNF::TERMINAL_TOKEN}}}};

		/// the break Statement
		ebnf.grammar["break-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"break",EBNF::KEYWORD_TOKEN}, {";",EBNF::TERMINAL_TOKEN}}}};

		/// the continue Statement 
		ebnf.grammar["continue-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"continue",EBNF::KEYWORD_TOKEN}, {";",EBNF::TERMINAL_TOKEN}}}};
													
		/// the reset Statement
		ebnf.grammar["reset-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"reset",EBNF::EXPANSION_TOKEN}, {";",EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["reset"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"reset",EBNF::KEYWORD_TOKEN}, {"full-expression-list",EBNF::EXPANSION_TOKEN}}},
						{EBNF::GRAMMAR_TOKEN_AND,{{"reset",EBNF::KEYWORD_TOKEN}}}};
		
		/// the setnode Statement
		ebnf.grammar["setnode-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"setnode-expr",EBNF::EXPANSION_TOKEN},
															 {";",EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["setnode-expr"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"setnode",EBNF::KEYWORD_TOKEN},
												   {"full-expression-list",EBNF::EXPANSION_TOKEN}}},
						{EBNF::GRAMMAR_TOKEN_AND,{{"setnode",EBNF::KEYWORD_TOKEN}}}};
		/// the struct Statement
		ebnf.grammar["struct-list"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"struct",EBNF::KEYWORD_TOKEN},
													{"[a-zA-Z]+([a-zA-Z_0-9])*",EBNF::REGEX_TOKEN},
													{"struct-body",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["struct-body"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{";",EBNF::TERMINAL_TOKEN},
															   {"struct-definition",EBNF::EXPANSION_TOKEN}}}};											
		ebnf.grammar["struct-definition"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"{",EBNF::TERMINAL_TOKEN},
																{"struct-declaration-list",EBNF::EXPANSION_TOKEN},
																{"}",EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["struct-declaration-list"] = {{EBNF::GRAMMAR_TOKEN_ONE_MORE ,{{"declaration-full-Statement",EBNF::EXPANSION_TOKEN}}}};
		
		/// the for loop Statement
		
		ebnf.grammar["for-list"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"for",EBNF::KEYWORD_TOKEN},
													  {"[",EBNF::TERMINAL_TOKEN},
													  {"loop-expression-declarator",EBNF::EXPANSION_TOKEN},
													  {"loop-expression-condition",EBNF::EXPANSION_TOKEN},
													  {"loop-expression-step-list",EBNF::EXPANSION_TOKEN},
													  {"]",EBNF::TERMINAL_TOKEN},
													  {"compound-Statement",EBNF::EXPANSION_TOKEN}}}};
																		
		ebnf.grammar["loop-expression-declarator"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"declaration-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseDeclaration},
																		{";",EBNF::TERMINAL_TOKEN, &DM14::Parser::parseNOPStatement}}}};
																		
		ebnf.grammar["loop-expression-condition"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"expression-Statement",EBNF::EXPANSION_TOKEN,  &DM14::Parser::parseExpressionStatement},
																	   {";",EBNF::TERMINAL_TOKEN, &DM14::Parser::parseNOPStatement}}}};
		
		ebnf.grammar["logical-expression-list"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"logical-expression",EBNF::EXPANSION_TOKEN},
																	  {";",EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["logical-expression"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"logical-expression-term",EBNF::EXPANSION_TOKEN},
																 {".*",EBNF::BINARY_OP_TOKEN},
																 {"logical-expression-term",EBNF::EXPANSION_TOKEN}}},
									  {EBNF::GRAMMAR_TOKEN_AND ,{{"logical-expression-term",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["logical-expression-term"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",EBNF::REGEX_TOKEN},
																	 {".*",EBNF::IMMEDIATE_TOKEN}}}};

		ebnf.grammar["loop-expression-step-list"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"expression-Statement",EBNF::EXPANSION_TOKEN,  &DM14::Parser::parseExpressionStatement},
																	   {";",EBNF::TERMINAL_TOKEN, &DM14::Parser::parseNOPStatement}}}};

		/// the if Statement
		ebnf.grammar["if-list"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"if",EBNF::KEYWORD_TOKEN},
													  {"[",EBNF::TERMINAL_TOKEN},
													  {"full-expression-list",EBNF::EXPANSION_TOKEN},
													  {"]",EBNF::TERMINAL_TOKEN},
													  {"compound-Statement",EBNF::EXPANSION_TOKEN},
													  {"elseif-list",EBNF::EXPANSION_TOKEN},
													  {"else-list",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["elseif-list"] = {{EBNF::GRAMMAR_TOKEN_ZERO_MORE,{{"elseif-Statement",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["elseif-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"else",EBNF::KEYWORD_TOKEN},
															{"if-list",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["else-list"] = {{EBNF::GRAMMAR_TOKEN_ZERO_MORE,{{"else-Statement",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["else-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"else",EBNF::KEYWORD_TOKEN},
															{"compound-Statement",EBNF::EXPANSION_TOKEN}}}};
		/// the while loop Statement
		ebnf.grammar["while-list"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"while",EBNF::KEYWORD_TOKEN},
													  {"[",EBNF::TERMINAL_TOKEN},
													  //{"logical-expression",EBNF::EXPANSION_TOKEN},
													  {"full-expression-list",EBNF::EXPANSION_TOKEN},
													  {"]",EBNF::TERMINAL_TOKEN},
													  {"compound-Statement",EBNF::EXPANSION_TOKEN}}}};
		
		/// variable
		ebnf.grammar["variable"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",EBNF::REGEX_TOKEN},
													   {"declaration-index-list",EBNF::EXPANSION_TOKEN}}}};

		/// the variables declaration list
		ebnf.grammar["declaration-list"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"declaration-full-Statement",EBNF::EXPANSION_TOKEN},
															{"declaration-Statement",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["declaration-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",EBNF::REGEX_TOKEN},
															   //{"declaration-dataflow-specifier",EBNF::EXPANSION_TOKEN},
															   //{"declaration-dist-specifiers-list",EBNF::EXPANSION_TOKEN},
															   //{"declaration-global-specifier",EBNF::EXPANSION_TOKEN},
															   {"declaration-specifiers-list", EBNF::EXPANSION_TOKEN},
															   {"declaration-datatype-list", EBNF::EXPANSION_TOKEN},
															   {"declaration-index-list", EBNF::EXPANSION_TOKEN},
															   {"declaration-value-list", EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["declaration-specifiers-list"] = {{EBNF::GRAMMAR_TOKEN_ZERO_MORE, {{"declaration-specifiers", EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["declaration-specifiers"] = {{EBNF::GRAMMAR_TOKEN_OR, {{"global",EBNF::KEYWORD_TOKEN},
						{"nodist", EBNF::KEYWORD_TOKEN},
						{"channel", EBNF::KEYWORD_TOKEN},
						{"backprop", EBNF::KEYWORD_TOKEN},
						{"recurrent", EBNF::KEYWORD_TOKEN},
						{"noblock", EBNF::KEYWORD_TOKEN}}}};

		ebnf.grammar["declaration-full-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"declaration-Statement",EBNF::EXPANSION_TOKEN},
									{";",EBNF::TERMINAL_TOKEN}}}};

		//ebnf.grammar["declaration-dataflow-specifier"] = {{EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"backprop",EBNF::KEYWORD_TOKEN}}}}; // should not be ok with nodist...

		//ebnf.grammar["declaration-dist-specifiers-list"] = {{EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"channel",EBNF::KEYWORD_TOKEN}}},
		//											{EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"recurrent",EBNF::KEYWORD_TOKEN}}},
		//											{EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"nodist",EBNF::KEYWORD_TOKEN}}}};

		//ebnf.grammar["declaration-global-specifier"] = {{EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"global",EBNF::KEYWORD_TOKEN}}}};
		
		ebnf.grammar["declaration-datatype-list"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"declaration-datatype",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["declaration-datatype"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+[a-zA-Z_0-9]*",EBNF::REGEX_TOKEN}}}};
		

		ebnf.grammar["declaration-index-list"] = {{EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"declaration-index",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["declaration-index"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{"matrix-index",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseMatrixIndex},
								 {"array-index",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseArrayIndex}}}};

		
		ebnf.grammar["matrix-index"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"array-index",EBNF::EXPANSION_TOKEN},
							   {"array-index",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["array-index"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"[",EBNF::TERMINAL_TOKEN},
							  {"[0-9]+",EBNF::REGEX_TOKEN},
							  {"]",EBNF::TERMINAL_TOKEN}}},
				{EBNF::GRAMMAR_TOKEN_AND ,{{"[",EBNF::TERMINAL_TOKEN},
						  {"variable",EBNF::EXPANSION_TOKEN},
						  {"]",EBNF::TERMINAL_TOKEN}}}};

		
		ebnf.grammar["declaration-value-list"] = {{EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"declaration-value",EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["declaration-value"] = {{EBNF::GRAMMAR_TOKEN_AND , {{"=",EBNF::TERMINAL_TOKEN}, 
								   {"expression-list",EBNF::EXPANSION_TOKEN}}}};

		/// the function call Statement
		
		ebnf.grammar["function-call-list"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"function-call",EBNF::EXPANSION_TOKEN}, {";",EBNF::TERMINAL_TOKEN}}}};
														  
		//ebnf.grammar["function-call"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"[a-zA-Z]+([a-zA-Z_0-9])*",EBNF::REGEX_TOKEN},
		ebnf.grammar["function-call"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"identifier",EBNF::EXPANSION_TOKEN},
																   {"(",EBNF::TERMINAL_TOKEN},
																   {"function-call-arguments-list",EBNF::EXPANSION_TOKEN},
																   {")",EBNF::TERMINAL_TOKEN}}}};
		
		ebnf.grammar["function-call-arguments-list"] = {{EBNF::GRAMMAR_TOKEN_ZERO_MORE,{{"function-call-arguments",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["function-call-arguments"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"full-expression-list",EBNF::EXPANSION_TOKEN},
																	 {",",EBNF::TERMINAL_TOKEN}}},
										   {EBNF::GRAMMAR_TOKEN_AND,{{"full-expression-list",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["expression-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"expression-list",EBNF::EXPANSION_TOKEN},
														{";",EBNF::TERMINAL_TOKEN, &DM14::Parser::parseExpressionStatement}}}};

		ebnf.grammar["expression-list"] = {{EBNF::GRAMMAR_TOKEN_ONE_MORE ,{{"full-expression-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseExpressionStatement}}}};

		ebnf.grammar["full-expression-list"] = {{EBNF::GRAMMAR_TOKEN_OR,{{"big-expression-list",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseExpressionStatement},
														{"expression",EBNF::EXPANSION_TOKEN, &DM14::Parser::parseExpressionStatement}}}};
														
		ebnf.grammar["big-expression-list"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"expression-types",EBNF::EXPANSION_TOKEN},
														{"BIN_OP",EBNF::BINARY_OP_TOKEN},
														{"expression-types",EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["expression-types"] = {{EBNF::GRAMMAR_TOKEN_OR,{{"big-expression",EBNF::EXPANSION_TOKEN},
														{"expression",EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["big-expression"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"(",EBNF::OP_TOKEN},
														{"big-expression-list",EBNF::EXPANSION_TOKEN},
														{")",EBNF::OP_TOKEN}}}};
		ebnf.grammar["expression"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"(",EBNF::OP_TOKEN},
														{"expression",EBNF::EXPANSION_TOKEN},
														{")",EBNF::OP_TOKEN}}},
								{EBNF::GRAMMAR_TOKEN_AND,{{"expression-extend",EBNF::EXPANSION_TOKEN},
														{"BIN_OP",EBNF::BINARY_OP_TOKEN},
														{"expression-types",EBNF::EXPANSION_TOKEN}}},
								{EBNF::GRAMMAR_TOKEN_AND,{{"SING_OP",EBNF::SINGLE_OP_TOKEN},
														{"expression-extend",EBNF::EXPANSION_TOKEN}}},
								{EBNF::GRAMMAR_TOKEN_AND,{{"CORE_OP",EBNF::CORE_OP_TOKEN},
														{"expression-extend",EBNF::EXPANSION_TOKEN}}},
								{EBNF::GRAMMAR_TOKEN_AND,{{"expression-extend",EBNF::EXPANSION_TOKEN},
														{"SING_OP",EBNF::SINGLE_OP_TOKEN}}},
								{EBNF::GRAMMAR_TOKEN_AND,{{"expression-extend",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["expression-extend"] = {{EBNF::GRAMMAR_TOKEN_OR,{{"function-call",EBNF::EXPANSION_TOKEN},
															 {"variable",EBNF::EXPANSION_TOKEN},
															  {"IMMEDIATE",EBNF::IMMEDIATE_TOKEN}}}};
		/// the thread Statement

		ebnf.grammar["thread-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"thread-list",EBNF::EXPANSION_TOKEN},
														 {";",EBNF::TERMINAL_TOKEN}}}};

		ebnf.grammar["thread-list"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"thread",EBNF::KEYWORD_TOKEN},
														 {"function-call",EBNF::EXPANSION_TOKEN}}}};
		
		/// the function Statement
		ebnf.grammar["function-list"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"function-prototype",EBNF::EXPANSION_TOKEN},
														   {"function-definition-list",EBNF::EXPANSION_TOKEN}}}};
		
		
		ebnf.grammar["function-definition-list"] = {{EBNF::GRAMMAR_TOKEN_OR ,{{";",EBNF::TERMINAL_TOKEN},
																		{"compound-Statement",EBNF::EXPANSION_TOKEN}}}};
																 
		ebnf.grammar["function-prototype"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"identifier",EBNF::EXPANSION_TOKEN},
																{"(",EBNF::TERMINAL_TOKEN},
																{"function-parameter-list",EBNF::EXPANSION_TOKEN},
																{"->",EBNF::TERMINAL_TOKEN},
																{"function-return",EBNF::EXPANSION_TOKEN},
																{")",EBNF::TERMINAL_TOKEN}}}};

		ebnf.grammar["function-parameter-list"] = {{EBNF::GRAMMAR_TOKEN_ZERO_MORE ,{{"function-parameters",EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["function-parameters"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"function-parameter",EBNF::EXPANSION_TOKEN}}},
										{EBNF::GRAMMAR_TOKEN_AND ,{{"function-extra-parameter",EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["function-extra-parameter"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{",",EBNF::TERMINAL_TOKEN},
																	  {"function-parameter",EBNF::EXPANSION_TOKEN}}}};															  
		
		ebnf.grammar["function-parameter"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"identifier",EBNF::EXPANSION_TOKEN},
																{"declaration-datatype-list",EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["function-return"] = {{EBNF::GRAMMAR_TOKEN_ZERO_MORE ,{{"function-parameter",EBNF::EXPANSION_TOKEN}}}};
																	
		ebnf.grammar["compound-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND ,{{"{",EBNF::TERMINAL_TOKEN},
																{"Statement-list",EBNF::EXPANSION_TOKEN},
																{"}",EBNF::TERMINAL_TOKEN}}}};
		/// return Statement
		
		ebnf.grammar["return-list"] = {{EBNF::GRAMMAR_TOKEN_AND,{{"return",EBNF::KEYWORD_TOKEN},
														{"expression-Statement",EBNF::EXPANSION_TOKEN}}},
							   {EBNF::GRAMMAR_TOKEN_AND,{{"return",EBNF::KEYWORD_TOKEN},
														{";",EBNF::TERMINAL_TOKEN}}}};
		/// nop Statement
		ebnf.grammar["nop-Statement"] = {{EBNF::GRAMMAR_TOKEN_AND,{{";",EBNF::TERMINAL_TOKEN}}}};
		
		
		ebnf.groupBy({"program", "Statement",
					 "include-Statement", "include-Statement-body",
					 "function-list", "function-prototype", "function-definition-list", 
					 "declaration-full-Statement", 
					 "for-list", 
					 "extern-Statement",
					 "link-list",
					 "struct-list",
					 "if-list",
					 "distribute",
					 "reset-Statement",
					 "setnode-Statement", 
					 "while-list",
					 "case-list", 
					 "addparent-Statement", 
					 "thread-Statement", 
					 "function-call-list", 
					 "return-list",
					 "break-Statement",
					 "continue-Statement",
					 "nop-Statement",
					 "expression-Statement",
					 "compound-Statement",
					 "Statement-list"});
		
	};



	Parser::~Parser()
	{
		//delete distIdentifiers;
		//delete ExternCodes;
	};

	std::shared_ptr<Statement> Parser::parseContinue()
	{
		return std::shared_ptr<Statement>(new continueStatement());
	}

	std::shared_ptr<Statement> Parser::parseBreak()
	{
		return std::shared_ptr<Statement>(new breakStatement());
	}

	int Parser::parse()
	{
		if(!insider)
		{
			parseIncludesInsider("core/DM14GLOBAL.m14", "", includePath::sourceFileType::FILE_DM14);
		}


		displayInfo(fName, -1, -1, "Veryifying & Filling in the EBNF gaps!");
		while(ebnf.getIndex() < tokens->size()-1)
		{
			
			EBNF::ebnfResult ebnf_parse_result = ebnf.parse("program");
			if(ebnf_parse_result.status != EBNF::ebnfResultType::SUCCESS)
			{
				bad_program();
			}
			
			ebnf_parse_result.stack.Print();
			stackList.push_back(ebnf_parse_result.stack);
		}
		
		displayInfo(fName, -1, -1, "Doing the parsing ;)");
		
		//for(auto&& stack : stackList)
		{
			//(this->*stack.callback)();
		}

		// check for the main function 
		// errr no need for file include , the compiler is the one that should that
		if(!Header)
		{
			if(!isUserFunction("main", true))
			{
				displayError(fName, -1, -1, "no main function defined!");
			}
			funcInfo fun = getFunc("main");
			
			if(fun.returnType != "int")
			{
				displayError(fName, -1, -1, "Main function must return int");
			}
		}
			
			
		mapcode mapCode(fName, getFunctions(), getIncludes(), false, distributedNodesCount, distributedVariablesCount);
			
		mapCode.ExternCodes = std::make_shared<Array<string>>();
		*mapCode.ExternCodes = *ExternCodes;
		mapCode.ExternCodes = ExternCodes;
		mapCode.linkLibs = linkLibs;
		mapCode.globalDeclarations = globalDeclarations;
		mapCode.globalDefinitions = globalDefinitions;
		mapCode.dataTypes = mapcodeDatatypes;
		mapCodes->push_back(mapCode);
		//mapCode.Print();
		//FIX, need to clear structs and classes declared only in this mapCode??
		mapcodeDatatypes.clear();
		
		//exit(1);
		
		return 0;
	};

	/** in includes folder , there is folders named after packages , 
	 * and then cpps and hpps named after libraries , so when calling mapFunctions , 
	 * pass package AND library , add the File including too , call parser on it */
	std::shared_ptr<Statement> Parser::parseIncludes() 
	{	
		activeStack().popToken();
		activeStack().popToken();
		string package = "";
		string library = "";
		includePath::sourceFileType includeType = includePath::sourceFileType::LIBRARY;
		if(activeStack().getToken().type != "string" && activeStack().getToken().type != "identifier" && !DM14::types::isDataType(activeStack().getToken().value))//tokens->at(activeStack().getIndex()).type != "datatype")
		{
			displayError(fName, activeStack().getToken().lineNumber,activeStack().getToken().columnNumber,"Expected Package name and not " + activeStack().getToken().value );
		}

		if(activeStack().getToken().type == "string")
		{
			
			package = activeStack().getToken().value.substr(1,(activeStack().getToken().value.size() - 2 ));
			if(package.substr(package.size()-5) == ".m14")
			{
				includeType = includePath::sourceFileType::FILE_DM14;
			}
			else if(package.substr(package.size()-5) == ".cpp" || package.substr(package.size()-4) == ".cc")
			{
				includeType = includePath::sourceFileType::FILE_C;
			}
			else
			{
				displayError("Unknown file include type");
			}
			library = "";
		}
		else
		{
			package = activeStack().getToken().value;
			includeType = includePath::sourceFileType::LIBRARY;
			activeStack().popToken();
			if(activeStack().getToken().value == "use")
			{
				RequireValue("use", "Expected \"Use\" and not ", true);
				activeStack().popToken();
				
				if(activeStack().getToken().value != "*" && activeStack().getToken().type != "identifier" && !DM14::types::isDataType(activeStack().getToken().value))// tokens->at(activeStack().getIndex()).type != "datatype")
				{
					displayError(fName, activeStack().getToken().lineNumber,activeStack().getToken().columnNumber,"Expected Package name");	
				}
				
				if(activeStack().getToken().value == "*" )
				{
					library = package;
				}
				else
				{
					library = activeStack().getToken().value;
				}
				
				activeStack().popToken();
			}
			else
			{
				library = package;
			}
		}
		
		parseIncludesInsider(package, library, includeType);
			
		return NULL;
	};

	int Parser::addIncludePath(string path)
	{
		includePaths.push_back(path);
		return includePaths.size();
	}

	int Parser::parseIncludesInsider(const string& package, const string& library, const includePath::sourceFileType includeType)
	{
		includePath include;
		include.package = package;
		include.library = library;
		include.includeType = includeType;

		//includes.push_back(pair<string,string>(package,library));
		includes.push_back(include);
		if(includeType == includePath::sourceFileType::FILE_DM14 || includeType == includePath::sourceFileType::FILE_C)
		{
			string fullPath;
			for(uint32_t i = 0; i < includePaths.size(); i++)
			{
				ifstream ifs;			
				ifs.open(includePaths.at(i)+"/"+package);
				displayInfo(" searching   ... [" + includePaths.at(i)+"/"+package + "]");
				if(ifs.is_open())
				{
					
					fullPath = includePaths.at(i)+"/"+package;
					ifs.close();
					break;
				}
			}

			displayInfo(" Scanning   ... [" + package + "/" + library + "]");
			DM14::scanner Scanner(fullPath);
			Scanner.setShortComment("~~");
			Scanner.setLongComment("~*", "*~");
			Scanner.scan();
			Scanner.printTokens();
			
			displayInfo(" Parsing   ... [" + package + "/" + library + "]");
			Parser parser(Scanner.getTokens(),package);
			for(uint32_t i = 0; i < includePaths.size(); i++)
			{
				parser.addIncludePath(includePaths.at(i));
			}
			parser.Header = true;
			parser.parse();
			
			for(uint32_t i =0; i < parser.getMapCodes()->size(); i++)
			{
				parser.getMapCodes()->at(i).setHeader(true);
				parser.getMapCodes()->at(i).ExternCodes = parser.ExternCodes;
				
				/*for(unsigned k =0; k < parser.getMapCodes()->at(i).globalDefinitions.size(); k++)
				{
					globalDefinitions.push_back(parser.getMapCodes()->at(i).globalDefinitions.at(k));
				}*/
				
				
				for(const auto& identifier : *(parser.identifiers))
				{
					if(identifier.global)
					{
						identifiers->push_back(identifier);
					}
				}

				/*for(const auto& globalDeclaration : parser.globalDeclarations)
				{
					//globalDeclarations.push_back(globalDeclaration);
					for(const auto& identifier : globalDeclaration)
					{
						indetifiers->push_back(identifier)
					}
				}*/

				//parser.getMapCodes()->at(i).globalDeclarations = Array<Statement*>();
				mapCodes->push_back(parser.getMapCodes()->at(i));
			//mapCodes->at(mapCodes->size()-1).Print();
			}
			
			for(uint32_t i =0; i< parser.linkLibs->size(); i++)
			{
				linkLibs->push_back(parser.linkLibs->at(i));
			}
		
			for(uint32_t i =0; i< parser.getIncludes().size(); i++)
			{
				bool push = true;
				
				for(uint32_t k =0; k< includes.size(); k++)
				{
					if(parser.getIncludes().at(i).package == includes.at(k).package &&
						parser.getIncludes().at(i).library == includes.at(k).library)
					{
						push = false;
					}
				}
				
				if(push)
				{
					includes.push_back(parser.getIncludes().at(i));
				}
			}
			
			// push new stuff

			for(uint32_t i =0; i< parser.functionsInfo->size(); i++)
			{
				functionsInfo->push_back(parser.functionsInfo->at(i));
			}
			
			
			scope += parser.scope;
		}
		else
		{
			mapFunctions(package, library);
		}
		
		return 0;
	}

	std::shared_ptr<Statement> Parser::parseLink()
	{
		std::shared_ptr<Link> stmt(new Link());
		
		activeStack().popToken();
		if(activeStack().getToken().value == "slink")
		{
			stmt->Static = true;
		}
		
		activeStack().popToken();
			
		if(activeStack().getToken().type != "string")
		{
			RequireType("string", "expected library identifier and not : ", true);
		}
		
		if(activeStack().getToken().value.size() > 2)
		{
			stmt->libs = activeStack().getToken().value.substr(1, activeStack().getToken().value.size()-2);
		}
		
		linkLibs->push_back(stmt);
		
		return stmt;
	}


	std::shared_ptr<Statement> Parser::parseReturn()
	{
		activeStack().popToken();
		std::shared_ptr<returnStatement> statement(new returnStatement());
		statement->line = activeStack().getToken().lineNumber;
		statement->scope = scope;
		
		if(!peekToken(";"))
		{
			//returnStatement->retValue = parseOpStatement(0, reachToken(";", false, true, true, true, false)-1, currentFunction.returnIDType, 0, returnStatement);
			statement->retValue = parseStatement("Statement");
		}
		else
		{
			activeStack().popToken();
			RequireValue(";","expected ; and not:", true);
		}

		return statement;
	}

	std::shared_ptr<Statement> Parser::parseNOPStatement()
	{
		activeStack().popToken();
		std::shared_ptr<NOPStatement> result(new NOPStatement());
		result->line = activeStack().getToken().lineNumber;
		result->scope = scope;
		result->scopeLevel = scope;
		
		return result;
	};

	std::shared_ptr<Statement> Parser::parseMatrixIndex()
	{
		//@TODO: need to handle 2nd or multiple diemnsions , have to implement in the AST first then parseDeclraration and ParseOpStatement...
		activeStack().popToken();
		RequireValue("[", "Expected [ and not : ", true);
		int from = 0;
		int to = reachToken("]", false, true, false, true, true)-1;
		std::shared_ptr<Statement> result = parseOpStatement(from, to, "-2", 0, currentStatement);

		result->line = activeStack().getToken().lineNumber;
		activeStack().popToken();
		return result;
	}

	std::shared_ptr<Statement> Parser::parseArrayIndex()
	{
		activeStack().popToken();
		RequireValue("[", "Expected [ and not : ", true);
		int from = 0;
		int to = reachToken("]", false, true, false, true, true)-1;
		std::shared_ptr<Statement>  result = parseOpStatement(from, to, "-2", 0, currentStatement);
		result->line = activeStack().getToken().lineNumber;
		activeStack().popToken();
		return result;
	};


	std::shared_ptr<Statement> Parser::parseAddParent()
	{
		std::shared_ptr<parentAddStatement> ps(new parentAddStatement());
		activeStack().nextIndex();
		
		if((tokens->at(activeStack().getIndex())).value == "(")
		{
			int cindex = activeStack().getIndex();
			ps->ip = parseOpStatement(cindex, reachToken(")", false, true, true, true, true ), "string", 0, ps);
		}
		else
		{
			ps->ip = parseOpStatement(activeStack().getIndex(), activeStack().getIndex(), "string", 0, ps);
		}
		
		activeStack().nextIndex();
		if((tokens->at(activeStack().getIndex())).value == "(")
		{
			int cindex = activeStack().getIndex();
			ps->socket = parseOpStatement(cindex, reachToken(")", false, true, true, true, true ), "int", 0, ps);
		}
		else
		{
			ps->socket = parseOpStatement(activeStack().getIndex(), activeStack().getIndex(), "int", 0, ps);
		}
		return ps;
	};

	std::shared_ptr<Statement>	Parser::parseStatement(const std::string starting_rule, EBNF::parser_callback custom_callback)
	{
		std::shared_ptr<Statement> result = nullptr;
		increaseScope(result);
		result = std::shared_ptr<Statement>(ebnf.parseStatement(result, starting_rule, custom_callback));
		decreaseScope();
		return result;
	}
	
	std::shared_ptr<Statement> Parser::parseSetNode()
	{
		std::shared_ptr<setNodeStatement> sn(new setNodeStatement());
		//sn->node = 
		return sn;
	};

	std::shared_ptr<Statement> Parser::parseReset()
	{
		std::shared_ptr<resetStatement> rs(new resetStatement());
		activeStack().popToken(); // reset
		if(!peekToken(";"))
		{
			activeStack().popToken();
			int cindex = activeStack().getIndex();
			rs->count = parseOpStatement(cindex,(reachToken(";", true, true, true, false, false) - 1), getType(activeStack().getIndex()-1), 0, rs);
		}
		return rs;
	};

	std::shared_ptr<Statement> Parser::parseDistribute()
	{
		currentFunction.distributed = true;
		
		stringstream SS;
		
		SS << activeStack().getToken().lineNumber;
		currentFunction.functionNodes.push_back(currentFunction.name+SS.str());
		SS.str("");
		
		//distStatement* distStatement = new distStatement();
		distStatementTemp->line = activeStack().getToken().lineNumber;
		distStatementTemp->scope = scope;
		//*distStatement->variables = *distIdentifiers;
		//fix105
		distributedNodesCount++;
		std::shared_ptr<distStatement> stmt(distStatementTemp);
		distStatementTemp = std::shared_ptr<distStatement>(new distStatement());
		distributedScope++;
		return stmt;
	};

	/*
	Statement* Parser::parseTempDeclaration()
	{
		Statement* result = NULL;
		
		if(tmpScope)
		{
			result = parseDeclarationInternal(";");
		}
		else
		{
			tmpScope = true;
			result = parseDeclarationInternal(";");
			tmpScope = false;
		}
		return result;
	}*/

	int Parser::pushDependency(idInfo& id)
	{
		if(globalNoDist)
		{
			return 1;
		}
		
		
		if(id.name == currentFunction.returnID)
		{
			return 1;
		}
		
		for(uint32_t i = 0; i < currentFunction.parameters->size(); i++)
		{
			if(currentFunction.parameters->at(i).name == id.name &&
				currentFunction.parameters->at(i).parent == id.parent)
			{
				return 1;
			}
		}	
		
		if(distributedScope == 0 && !id.backProp && !id.channel)
		{
			return 1;
		}
		
		
		string idName;
		
		
		if(id.parent)
		{
			idName = id.parent->name;
		}
		else
		{
			idName = id.name;
		}
		
		
		if(findIDInfo(id, SCOPE)-1 >  currentFunction.scope)
		{
			return 2;
		}
		
		if(!id.distributed)
		{
			return 3;
		}
		
		string parentName = "";
		if(id.parent)
		{
			parentName = id.parent->name;
		}
		
		// FIX109 a modified varaible in current scope, we dont need to receive it then.
		for(uint32_t i =0; i<distModifiedGlobal->size(); i++)
		{
			string listParentName;
			
			if(distModifiedGlobal->at(i).parent)
			{

				listParentName = distModifiedGlobal->at(i).parent->name;
			}
		
			if(distModifiedGlobal->at(i).name == id.name &&
				listParentName == parentName &&
				distModifiedGlobal->at(i).distributedScope == id.distributedScope &&
				distModifiedGlobal->at(i).scope <= id.scope)
			{		
				if(!id.array)
				{
					if(id.arrayIndex == NULL && !id.channel)
					{
						return 1;
					}
				}
			}
		}
		
		// we modified it before in this scope!
		for(uint32_t i =0; i<distStatementTemp->modifiedVariables->size(); i++)
		{
			string listParentName;
			
			if(distStatementTemp->modifiedVariables->at(i).parent)
			{
				listParentName = distStatementTemp->modifiedVariables->at(i).parent->name;
			}
			
			if(distStatementTemp->modifiedVariables->at(i).name == id.name &&
				listParentName == parentName &&
				distStatementTemp->modifiedVariables->at(i).scope <= id.scope)
			{
				if(!id.array)
				{
					if(id.arrayIndex == NULL && !id.channel && !id.backProp && !id.noblock)
					{
						return 1;
					}
				}
			}
		}
		
		// we requested it before !
		for(uint32_t i =0; i<distStatementTemp->dependenciesVariables->size(); i++)
		{
			string listParentName;
			
			if(distStatementTemp->dependenciesVariables->at(i).parent)
			{
				listParentName = distStatementTemp->dependenciesVariables->at(i).parent->name;
			}
			
			if(distStatementTemp->dependenciesVariables->at(i).name == id.name &&
				listParentName == parentName &&
				distStatementTemp->dependenciesVariables->at(i).scope <= id.scope)
			{
				if(!id.array)
				{
					if(id.arrayIndex == NULL && !id.channel && !id.backProp && !id.noblock)
					{
						return 1;
					}
				}
			}
		}
		
		// we already requested it before this Statement !
		for(uint32_t i =0; i<currentStatement->distStatements->size(); i++)
		{
			string listParentName;
			
			if(currentStatement->distStatements->at(i)->variable.parent)
			{
				listParentName = currentStatement->distStatements->at(i)->variable.parent->name;
			}
			
			if(currentStatement->distStatements->at(i)->variable.name == id.name &&
				listParentName== parentName &&
				currentStatement->distStatements->at(i)->variable.scope <= id.scope)
				//&& !id.requestAddress)
			{
				if(!id.array)
				{
					return 1;
				}
			}
		}
		
		
		if(DM14::types::isClass(id.type))
		{
			auto datatype = DM14::types::findDataType(id.type);
			for(uint32_t i =0; i < datatype.second.memberVariables.size(); i++)
			{
				idInfo id2(id);
				id2.name = datatype.second.memberVariables.at(i).name;
				id2.type = datatype.second.memberVariables.at(i).returnType;
				id2.arrayIndex = NULL;
				id2.array = false;
				id2.parent = std::shared_ptr<idInfo>(&id);
				pushDependency(id2);
			}
			
			return 0;
		}
		
		std::shared_ptr<distributingVariablesStatement> dvStatement(new distributingVariablesStatement());
		dvStatement->variable = id;
		dvStatement->type = distributingVariablesStatement::DEPS;
		
		if(id.channel)
		{
			dvStatement->dependencyNode = -2;
		}
		else
		{
			for(int32_t i= distModifiedGlobal->size()-1; i >= 0; i--)
			{
				
				string listParentName = "";
				
				if(distModifiedGlobal->at(i).parent)
				{
					listParentName = distModifiedGlobal->at(i).parent->name;
				}
				
				//cout << "Name : " << distModifiedGlobal->at(i).name << ":" << id.name << endl;
				//cout << "Parents : " << listParentName << ":" << parentName << endl;
				//cout << "Parents : " << distModifiedGlobal->at(i).distributedScope << endl;
				if(distModifiedGlobal->at(i).name == id.name &&
					listParentName == parentName)
				{
					if(id.backProp && distModifiedGlobal->at(i).distributedScope <= id.distributedScope)
					{
						dvStatement->dependencyNode = -3;
						break;
					}
					else if(distModifiedGlobal->at(i).distributedScope != id.distributedScope)
					{
						dvStatement->dependencyNode = distModifiedGlobal->at(i).distributedScope;
						break;
					}
				}
			}
			
			if(dvStatement->dependencyNode == -3)
			{
				dvStatement->dependencyNode = id.distributedScope;
			}
		}
		
		//cout << id.name << ":" << dvStatement->dependencyNode << endl;
		//cout << "\n \n \n \n "<< endl;
		currentStatement->distStatements->push_back(dvStatement);
		dvList->push_back(dvStatement);
		distStatementTemp->dependenciesVariables->push_back(id);
		return 0;
	};



	std::shared_ptr<idInfo> Parser::getTopParent(std::shared_ptr<idInfo> id)
	{
		if(!id->parent)
		{
			return id;
		}
		return getTopParent(id->parent);
	};

	int Parser::pushModified(const string& op, idInfo& id)
	{
		if(globalNoDist)
		{
			return 1;
		}
		
		
		if(id.name == currentFunction.returnID || getTopParent(std::shared_ptr<idInfo>(&id))->name == currentFunction.returnID)
		{
			return 1;
		}
		
		for(uint32_t i = 0; i < currentFunction.parameters->size(); i++)
		{
			if(currentFunction.parameters->at(i).name == id.name &&
				currentFunction.parameters->at(i).parent == id.parent)
			{
				return 1;
			}
		}
		
		string idName;
		
		if(id.parent)
		{
			idName = id.parent->name;
		}
		else
		{
			idName = id.name;
		}
		
		
		
		if(findIDInfo(id, SCOPE)-1 >  currentFunction.scope)
		{
			return 2;
		}
		
		if(!id.distributed)
		{
			return 3;
		}
		
		//cout << "MOD:::::" << id.name << ":" << op << endl;
		
		if(op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "++" || op == "--")
		{
			
			// also make sure it is not modified twice in the same op Statement like : A = B + C(A), where C() will modify A
			// use same code like below(for array index ? )
			for(uint32_t i =0; i< dvList->size(); i++)
			{
				if(id.channel || id.recurrent)
				{
					break;
				}
				
				if (((dvList->at(i))->type == distributingVariablesStatement::MODS) &&
					((dvList->at(i))->variable.name == id.name) &&
					((dvList->at(i))->variable.parent == id.parent) &&
					((dvList->at(i))->variable.arrayIndex == NULL))
				{
					if(!id.backProp &&((dvList->at(i))->variable.distributedScope != distributedScope))
						
					{
						dvList->at(i)->type = -1;
						dvList->remove(i);
						break;
					}
				}
			}
			
			std::shared_ptr<distributingVariablesStatement> dvStatement(new distributingVariablesStatement());
			dvStatement->variable = id;
			dvStatement->type = distributingVariablesStatement::MODS;
			
			currentStatement->distStatements->push_back(dvStatement);
			dvList->push_back(dvStatement);
			
			if(op == "++" || op == "--")
			{
				pushDependency(id);
			}
			
			for(uint32_t i =0; i<distStatementTemp->dependenciesVariables->size(); i++)
			{
				
				int sameParentArrayIndex = false;
				if(id.parent && distStatementTemp->dependenciesVariables->at(i).parent)
				{
					if(id.parent->arrayIndex == distStatementTemp->dependenciesVariables->at(i).parent->arrayIndex)
					{
						sameParentArrayIndex = true;
					}
				}
				
				string parentName;
				string listParentName;
				
				if(id.parent)
				{
					parentName = id.parent->name;
				}
				
				
				if(distStatementTemp->dependenciesVariables->at(i).parent)
				{
					listParentName = distStatementTemp->dependenciesVariables->at(i).parent->name;
				}
					
				if(distStatementTemp->dependenciesVariables->at(i).name == id.name
	//				&&distStatementTemp->dependenciesVariables->at(i).parent == id.parent)
					&& parentName == listParentName
					&& sameParentArrayIndex
					&& distStatementTemp->dependenciesVariables->at(i).arrayIndex == id.arrayIndex)
				{
					distStatementTemp->dependenciesVariables->erase(distStatementTemp->dependenciesVariables->begin()+i);
				}
			}
			
			distStatementTemp->modifiedVariables->push_back(id);
			pushModifiedGlobal(id);
		}
		else
		{
			return 1;
		}
		
		//cout << "MOD : " << id.parent << "." << id.name << endl;
		return 0;
	};


	int Parser::pushModifiedGlobal(idInfo& id)
	{
		//if exists, remove it and out the newest/ just to save memory ?
		distModifiedGlobal->push_back(id);
		return 0;
	};

	std::shared_ptr<Statement> Parser::parseFunctionCall()
	{
		std::shared_ptr<Statement> result(parseFunctionCallInternal(true, "", ""));
		return result;
	}

	std::shared_ptr<Statement> Parser::parseFunctionCallInternal(bool terminated,const string& returnType, const string& classID)
	{
		activeStack().popToken();
		std::shared_ptr<functionCall> funcCall(new functionCall); // for every comma , call parseOP
		funcCall->line = activeStack().getToken().lineNumber;
		funcCall->scope = scope;
		funcCall->name = activeStack().getToken().value;
		
		
		if(isUserFunction(activeStack().getToken().value, true))
		{
			funcCall->functionType = DM14::types::types::USERFUNCTION;
		}
		else
		{
			funcCall->functionType = DM14::types::types::BUILTINFUNCTION;
		}
		
		activeStack().popToken();
		RequireValue("(", "Expected \"(\" and not ", true);
		
		/*loop through parameters*/
		Array<string>* parameters = new Array<string>();
		if(peekToken(")"))
		{
			activeStack().popToken();
		}
		else /** we have parameters ! */
		{
			currentStatement = funcCall;
			while(!peekToken(")"))
			{
				std::shared_ptr<Statement> parameter = parseStatement("full-expression-list");
				funcCall->absorbDistStatements(parameter);
				funcCall->parameters->push_back(parameter);
				parameters->push_back(parameter->type);
				if(peekToken(","))
				{
					activeStack().popToken();
				}
			}

			activeStack().popToken();
		}
		
		bool error = true;
		
		if(funcCall->name == currentFunction.name)
		{
			for(uint32_t i =0; i < currentFunction.parameters->size(); i++)
			{
				if(parameters->size()-1 <= i)
				{
					if(parameters->at(i) == currentFunction.parameters->at(i).type)
					{
						if(i == parameters->size()-1)
						{
							error = false;
						}
						continue;
					}
					break;
				}
				else
				{
					break;
				}
			}
			if(error)
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"1Parameters error for function call : " + funcCall->name);
			}
			funcCall->type = currentFunction.returnIDType;
		}
		else
		{
			if(!getFunc(funcCall->name, parameters, returnType, classID).name.size())
			{
				if(getFunc(funcCall->name, classID).name.size())
				{
					for(uint32_t i =0; i < parameters->size(); i++)
					{
						cerr << "parameter: " << parameters->at(0) << endl;
					}
					cerr << "SIZE :" << parameters->size() << endl;
					cerr << "Return type :" << returnType << endl;
					
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"2Parameters error for function call : " + funcCall->name);
				}
				else
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"return type error [" + returnType + "] for function call : [" + funcCall->name + "] expected : " + getFunc(funcCall->name, classID).returnType);
				}
			}
			funcCall->type = getFunc(funcCall->name, parameters, returnType, classID).returnType;
		}
		
		if(terminated)
		{
			activeStack().popToken();
			RequireValue(";", "Expected ; and not ", true);
		}
		
		return funcCall;
	};

	std::shared_ptr<Statement> Parser::parseForloop()
	{
		activeStack().popToken();
		
		// for [decl;cond;stmt] {stmts}
		std::shared_ptr<forloop> floop(new forloop);
		floop->line = activeStack().getToken().lineNumber;
		floop->scope = scope;
		
		increaseScope(floop);
		
		tmpScope = true; 
		
		activeStack().popToken();
		RequireValue("[", "Expected [ and not ", true);

		if(!peekToken(";"))
		{
			//Statement* stmt =  parseDeclarationInternal(";");
			std::shared_ptr<Statement> stmt = parseStatement("loop-expression-declarator");
			cerr << "declarator :" << stmt << endl;
			stmt->line = activeStack().getToken().lineNumber;
			if(stmt->StatementType != dStatement && stmt->StatementType != eStatement)
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber, "Expected declaration Statement");
			}
			
			if(stmt->StatementType == dStatement)
			{
				floop->fromCondition->push_back(stmt);
			}
		
			addStatementDistributingVariables(stmt);
		}
		
		if(!peekToken(";"))
		{
			std::shared_ptr<Statement> stmt= parseStatement("loop-expression-condition");
			stmt->line = activeStack().getToken().lineNumber;
			if(stmt->StatementType != oStatement && stmt->StatementType != eStatement)
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber, "Expected conditional Statement");
			}
			
			if(stmt->StatementType == oStatement)
			{
				floop->toCondition->push_back(stmt);
			}
			
			addStatementDistributingVariables(stmt);
		}

		
		
		if(!peekToken("]"))
		{
			std::shared_ptr<Statement> stmt= parseStatement("loop-expression-step-list");
			stmt->line = activeStack().getToken().lineNumber;
			if(stmt->StatementType != oStatement && stmt->StatementType != eStatement)
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber, "Expected operational Statement");
			}
			
			if(stmt->StatementType == oStatement)
			{
				floop->stepCondition->push_back(stmt);
			}
			
			addStatementDistributingVariables(stmt);
		}
		
		
		activeStack().popToken();
		RequireValue("]", "Expected ] and not ", true);
		
		tmpScope = false;
		activeStack().popToken();
		RequireValue("{", "Expected { and not ", true);
		

		while(!peekToken("}"))
		{
			std::shared_ptr<Statement> stmt = parseStatement("Statement");
			addStatementDistributingVariables(stmt);
			floop->body->push_back(stmt);
		}
		
		activeStack().popToken();
		decreaseScope();
			
		return floop;
	};

	int Parser::addStatementDistributingVariables(std::shared_ptr<Statement> stmt)
	{
		
		for(uint32_t i=0; i < currentFunction.body->appendBeforeList.size(); i++)
		{
			
			
			if(std::static_pointer_cast<termStatement>(std::static_pointer_cast<distributingVariablesStatement>(currentFunction.body->appendBeforeList.at(i))->variable.arrayIndex) != NULL)
			{
				cerr << currentFunction.body->appendBeforeList.at(i) << endl << flush;
				stmt->distStatements->push_back(std::static_pointer_cast<distributingVariablesStatement>(currentFunction.body->appendBeforeList.at(i)));
				//currentFunction.body->append_after(dvStatement);
				currentFunction.body->appendBeforeList.erase(currentFunction.body->appendBeforeList.begin()+i);
				i--;
			}
		}
		for(uint32_t i=0; i < currentFunction.body->appendAfterList.size(); i++)
		{
			if(std::static_pointer_cast<termStatement>(std::static_pointer_cast<distributingVariablesStatement>(currentFunction.body->appendAfterList.at(i))->variable.arrayIndex) != NULL)
			{
				stmt->distStatements->push_back(std::static_pointer_cast<distributingVariablesStatement>(currentFunction.body->appendAfterList.at(i)));
				//currentFunction.body->append_after(dvStatement);
				currentFunction.body->appendAfterList.erase(currentFunction.body->appendAfterList.begin()+i);
				i--;
			}
		}
		
		return 0;
	}

	std::shared_ptr<Statement> Parser::parseFunction() // add functions prototypes to userFunctions Array too :)
	{
		// always keep global ids
		std::shared_ptr<Array<idInfo>> tmpIdentifiers = identifiers;
		identifiers			= std::shared_ptr<Array<idInfo>>(new Array<idInfo>);
		globalNoDist = true;
		for(uint32_t i =0; i < tmpIdentifiers->size(); i++)
		{
			if(tmpIdentifiers->at(i).global)
			{
				identifiers->push_back(tmpIdentifiers->at(i));
			}
		}
		tmpIdentifiers.reset();

		
		activeStack().popToken();
		RequireType("identifier", "Invalid function declaration", false);
		
		funcInfo Funcinfo;
		Funcinfo.protoType = false;
		ast_function Func;
		currentFunction.scope = scope;
		
		////checkToken(USERFUNCTION, "can not re-define a builtin function : ", true);
		////checkToken(BUILTINFUNCTION, "Predefined function : ", true);
		
		Funcinfo.name = activeStack().getToken().value;
		Funcinfo.type = DM14::types::types::USERFUNCTION;
		
		currentFunction.name = activeStack().getToken().value;
			
		activeStack().popToken();
		RequireValue("(", "Expected \"(\" and not "+ activeStack().getToken().value + " after function definition ", false);

		if(!peekToken("->"))
		{
			while(activeStack().getToken().value != "->")
			{
				//TODO: FIX 102
				// should make a Array of declaration Statements and add the Statements to it, for the compiler to parse them
				// with their initialized values ?
				//declareStatement* stmt =(declareStatement*)parseDeclarationInternal("->");
				//declareStatement* stmt =(declareStatement*)parseStatement("Statement");
				
				std::shared_ptr<declareStatement> stmt = std::static_pointer_cast<declareStatement>(parseStatement("declaration-Statement", &Parser::parseDeclarationInternal));
				
				stmt->line = activeStack().getToken().lineNumber;
				for(uint32_t i = 0; i < stmt->identifiers->size(); i++ )
				{
					idInfo Id;
					Id.name =(stmt->identifiers->at(i)).name;
					Id.parent =(stmt->identifiers->at(i)).parent;
					Id.type =(stmt->identifiers->at(i)).type;
					Id.type = stmt->type;
					Id.scope = stmt->scope;
					Id.index = activeStack().getIndex();
					currentFunction.parameters->push_back(Id);
					Funcinfo.parameters->push_back(pair<string,bool>(Id.type,stmt->value));
				}
				
				if(peekToken(","))
				{
					activeStack().popToken();
				}
				else if(peekToken("->"))
				{
					activeStack().popToken();
				}
			}
		}
		else
		{
			activeStack().popToken(); // ->
		}
		
		RequireValue("->", "Expected \"->\" and not "+activeStack().getToken().value + " after function definition ", false);
		
		if(peekToken(")"))
		{
			activeStack().popToken();
			currentFunction.returnIDType = "NIL";
			currentFunction.returnID = "NIL";
		}
		else
		{	
			while(activeStack().getToken().value != ")")
			{
				//declareStatement* stmt =(declareStatement*)parseDeclarationInternal();
				std::shared_ptr<declareStatement> stmt = std::static_pointer_cast<declareStatement>(parseStatement("declaration-Statement", &Parser::parseDeclarationInternal));
				activeStack().popToken();
				RequireValue(")", "Expected ) and not "+activeStack().getToken().value + " after function definition ", false);
				stmt->line = activeStack().getToken().lineNumber;
				for(uint32_t i = 0; i < stmt->identifiers->size(); i++ )
				{
					stmt->line = activeStack().getToken().lineNumber;
					currentFunction.returnIDType = stmt->type;
					Funcinfo.returnType = stmt->type;
					currentFunction.returnID = stmt->identifiers->at(0).name;
					if(stmt->identifiers->size() > 1)
					{
						displayWarning(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Function : " + Funcinfo.name + " : more than one variable for return, only the first will be used");
						break;
					}
				}
			}
		}
		
		globalNoDist = false;
		// continue to body !
		activeStack().popToken();
		if(activeStack().getToken().value == ";")
		{
			//check if it already exists ??
			// different functions with same name should exist
			// check if same parameters, then check for prototype
			Funcinfo.protoType = true;
			if(isFunction(Funcinfo.name, false))
			{
				for(uint32_t i =0; i < functionsInfo->size(); i++)
				{
					//FIX21
					if(functionsInfo->at(i).name == Funcinfo.name)
					{
						if(*functionsInfo->at(i).parameters == *Funcinfo.parameters)
						{
							if(functionsInfo->at(i).protoType)
							{
								displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Pre-defined function prototype : " +Funcinfo.name);
							}
							else // if(function prototype already exists)
							{
								displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Pre-defined function, no need for prototype : "+Funcinfo.name);
							}
						}
					}
				}
			}
			functionsInfo->push_back(Funcinfo);
		}
		else if(activeStack().getToken().value == "{")
		{
			globalNoDist = false;
			Funcinfo.protoType = false;

			//FIX , isFucntion again ?
			if(isFunction(Funcinfo.name, false))
			{
				for(uint32_t i =0; i < functionsInfo->size(); i++)
				{
					if(functionsInfo->at(i).name == Funcinfo.name &&
						*functionsInfo->at(i).parameters == *Funcinfo.parameters)
					{
						if(!functionsInfo->at(i).protoType)
						{
							displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Pre-defined function : "+Funcinfo.name);
						}
						else
						{
							distributedVariablesCount -= functionsInfo->at(i).parameters->size()+1;
							functionsInfo->erase(functionsInfo->begin()+i);
							functionsInfo->push_back(Funcinfo);
							break;
						}
					}
				}
			}
					
			std::shared_ptr<Array<std::shared_ptr<Statement>>> declarations(new Array<std::shared_ptr<Statement>>());
			
			//while(activeStack().popToken().value != "}")
			while(!peekToken("}"))
			{
				std::shared_ptr<Statement> stmt = parseStatement("Statement");

				if(stmt == NULL)
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"error parsing function : "+Funcinfo.name);
				}

				if(stmt->StatementType == dStatement)
				{
					if(std::static_pointer_cast<declareStatement>(stmt)->Initilazed && !std::static_pointer_cast<declareStatement>(stmt)->global)
					{
						std::shared_ptr<declareStatement> decStatement(std::static_pointer_cast<declareStatement>(stmt));
						std::shared_ptr<Statement> value(decStatement->value);
						decStatement->value = NULL;
						//FIX1001 if function is distributed , else put it in pushModified(os->op, id); currentFunction.body
						declarations->push_back(decStatement);
						//functionStatementsCount++;
						for(uint32_t i=0 ; i < decStatement->identifiers->size(); i++)
						{
							//fix add type and scope ....
							std::shared_ptr<operationalStatement> os(new operationalStatement());
							os->left = std::shared_ptr<Statement>(new termStatement(decStatement->identifiers->at(i).name));
							os->op = "=";
							os->right = value;
							os->type = stmt->type;
							os->scope = std::static_pointer_cast<declareStatement>(decStatement)->scope;

							idInfo id(decStatement->identifiers->at(i).name, os->scope, os->type, NULL);
							id.distributedScope = distributedScope;
							id.backProp =(bool)findIDInfo(id, BACKPROB);
							id.recurrent =(bool)findIDInfo(id, RECURRENT);
							id.channel =(bool)findIDInfo(id, CHANNEL);
							id.array =(bool)findIDInfo(id, ARRAY);
							id.noblock =(bool)findIDInfo(id, NOBLOCK);
							id.distributed =(bool)findIDInfo(id, DISTRIBUTED);
							id.type =(bool)findIDInfo(id, TYPE);
							currentStatement = os;
							pushModified(os->op, id);

							currentFunction.body->push_back(os);
							functionStatementsCount++;
						}
						
						std::shared_ptr<Statement> stmt2 (currentFunction.body->at(currentFunction.body->size()-(decStatement->identifiers->size())));
						stmt2->distStatements = stmt->distStatements;
						stmt->distStatements->clear();
					}
					else
					{
						declarations->push_back(stmt);
					}
				}
				else
				{
					if(stmt->StatementType == rStatement)
					{
						currentFunction.body->clearQueue();
					}
					currentFunction.body->push_back(stmt);
					functionStatementsCount++;
				}
			}

			//let's fix all backprob variables :)
			for(uint32_t i =0; i< dvList->size(); i++ )
			{
				if((dvList->at(i))->type == distributingVariablesStatement::DEPS &&
					(dvList->at(i))->variable.backProp  )
				{
					for(uint32_t k =i+1; k< dvList->size(); k++ )
					{
						if((dvList->at(k))->type != distributingVariablesStatement::MODS ||
						 ((dvList->at(k))->variable.distributedScope ==(dvList->at(i))->variable.distributedScope) ||
						 ((dvList->at(k))->variable.name !=(dvList->at(i))->variable.name) )
						  {
							  continue;
						  }
						  dvList->at(i)->dependencyNode =(dvList->at(k))->variable.distributedScope;
						  break;
					}
				}
			}

			//TODO: here :D fix all functions to fill the type field , and add type to functionInfo too
			functionsInfo->push_back(Funcinfo);
			
			//TODO:before this, loop through origianl body and push the modfifyNotify Statements at their right position i
			
			//TODO:fix what if , two initiated variables are pushed at the same place ?

			std::shared_ptr<termStatement> statement(new termStatement("DM14FUNCTIONBEGIN", "NIL" ));
			statement->scope = scope;
			//termStatement->scopeLevel = scopeLevel;
			//termStatement->arrayIndex = aIndex;
			//termStatement->identifier = true;
			declarations->push_back(statement);
			for(uint32_t i=0; i < currentFunction.body->size(); i++)
			{
				declarations->push_back(currentFunction.body->at(i));
			}

			currentFunction.body = declarations;
			functions->push_back(currentFunction);

			activeStack().popToken();
			RequireValue("}", "Expected } and not : ", true);
		}
		else
		{
			RequireValue("{", "Expected { or ; and not : ", true);
		}
		

		currentFunction = ast_function();
		decreaseScope();
		
		distModifiedGlobal	= std::shared_ptr<Array<idInfo>>(new Array<idInfo>);
		//distIdentifiers	= new Array<idInfo>;
		distStatementTemp	= std::shared_ptr<distStatement>(new distStatement());
		modifiedVariablesList = Array < pair<idInfo,int> >();
		//FIX only increment on if conditions, loops and case... ?
		increaseScope(NULL);
		distributedScope = 0;
		functionStatementsCount = 0;
		return nullptr;
	};


	bool Parser::isIdentifier(const string& ID)
	{
		for(uint32_t i =0; i< identifiers->size(); i++ )
		{
			if(ID ==(identifiers->at(i)).name )
			{
				return true;
			}
		}
		
		return false;
	};




	int Parser::findIDInfo(const idInfo& ID, const int& returnType)
	{	
		for(uint32_t i =0; i< identifiers->size(); i++ )
		{
			//if(ID.name ==(identifiers->at(i)).name && ID.parent && ID.parent->name ==(identifiers->at(i)).parent->name)
			
			string parentName;
			string identifiersParentName;
			
			if(ID.parent)
			{
				parentName = ID.parent->name;
			}
			
			if((identifiers->at(i)).parent)
			{
				identifiersParentName =(identifiers->at(i)).parent->name;
			}
			
			if(ID.name ==(identifiers->at(i)).name && parentName == identifiersParentName)
			{
				if(returnType == SCOPE)
				{
					return(identifiers->at(i)).scope;
				}
				else if(returnType == INITIALIZED)
				{
					return(identifiers->at(i)).initialized;
				}
				else if(returnType == ARRAY)
				{
					return(identifiers->at(i)).array;
				}
				else if(returnType == ARRAYSIZE)
				{
					return(identifiers->at(i)).size;
				}
				else if(returnType == TMPSCOPE)
				{
					return(identifiers->at(i)).tmpScope;
				}
				else if(returnType == DISTRIBUTED)
				{
					return(identifiers->at(i)).distributed;
				}
				else if(returnType == BACKPROB)
				{
					return(identifiers->at(i)).backProp;
				}
				else if(returnType == RECURRENT)
				{
					return(identifiers->at(i)).recurrent;
				}
				else if(returnType == CHANNEL)
				{
					return(identifiers->at(i)).channel;
				}
				else if(returnType == NOBLOCK)
				{
					return(identifiers->at(i)).noblock;
				}
				else if(returnType == GLOBAL)
				{
					return(identifiers->at(i)).global;
				}
				else
				{
					return 0;
				}
			}
		}
		
		
		if(returnType == SCOPE)
		{
			return -1;
		}
		else if(returnType == ARRAY)
		{
			return -1;
		}
		else if(returnType == ARRAYSIZE)
		{
			return -1;
		}
		
		return 0;
	};

	string Parser::findIDType(const idInfo& ID, const string& classID)
	{
		if(classID.size())
		{
			for(uint32_t i =0; i < datatypes.size(); i++)
			{
				if(datatypes.at(i).typeID == classID &&(datatypes.at(i).classType || datatypes.at(i).enumType))
				{
					for(uint32_t k =0; k < datatypes.at(i).memberVariables.size(); k++)
					{
						if(datatypes.at(i).memberVariables.at(k).name == ID.name)
						{
							return datatypes.at(i).memberVariables.at(k).returnType;
						}
					}
				}
			}
		}
		else
		{
			for(uint32_t i =0; i< identifiers->size(); i++ )
			{
				//if(ID.name == identifiers->at(i).name && !identifiers->at(i).parent.size())
				if(ID.name == identifiers->at(i).name && !identifiers->at(i).parent)
				{
					return(identifiers->at(i)).type;
				}
			}
		}
		return "NIL";
	};


	bool Parser::isFunction(const string& func, bool recursiveCurrent,const string& classID )
	{
		if(isUserFunction(func, recursiveCurrent, classID) || isBuiltinFunction(func))
		{
			return true;
		}
		
		return false;
	};

	bool Parser::isBuiltinFunction(const string& func)
	{
		for(uint32_t i = 0; i < functionsInfo->size(); i++)
		{	
			if(func ==(functionsInfo->at(i)).name)
			{
				if((functionsInfo->at(i)).type == DM14::types::types::BUILTINFUNCTION)
				{
					return true;
				}
			}
		}
		return false;
	};


	bool Parser::isUserFunction(const string& func, bool recursiveCurrent,const string& classID)
	{
		if(!classID.size())
		{
			if(recursiveCurrent)
			{
				if(func == currentFunction.name)
				{
					return true;
				}
			}
		
			for(uint32_t i = 0; i < functionsInfo->size(); i++)
			{	
				std::cerr << "function name : " << (functionsInfo->at(i)).name << std::endl;
				if(func ==(functionsInfo->at(i)).name)
				{
					if((functionsInfo->at(i)).type == DM14::types::types::USERFUNCTION)
					{
						return true;
					}
				}
			}
		}
		
		return DM14::types::classHasMemberFunction(classID, func);
	};


	// should return a vector of all matching leafs ?
	std::shared_ptr<Statement> Parser::findTreeNode(std::shared_ptr<Statement> opStatement, int StatementType) 
	{
		if(!opStatement)
		{
			return nullptr;
		}
		
		if(opStatement->StatementType == StatementType)
		{
			return opStatement;
		}
		
		std::shared_ptr<Statement> result = nullptr;
		
		if(opStatement->StatementType == oStatement)
		{
			
			if(std::static_pointer_cast<operationalStatement>(opStatement)->left)
			{
				result = findTreeNode(std::static_pointer_cast<operationalStatement>(opStatement)->left, StatementType);
			}
				
			if(!result && std::static_pointer_cast<operationalStatement>(opStatement)->right)
			{
				result = findTreeNode(std::static_pointer_cast<operationalStatement>(opStatement)->right, StatementType);
			}
		}
		
		return result;
	}



	int32_t Parser::searchVariables(std::shared_ptr<Statement> opStatement, int depencyType, string op)
	{
		
		//distributingVariablesStatement::DEPS
		if(!opStatement)
		{
			return 1;
		}
		
		if(opStatement->StatementType == tStatement)
		{
			
			std::shared_ptr<idInfo> id = std::static_pointer_cast<termStatement>(opStatement)->id;
			if(id)
			{
				if(depencyType == distributingVariablesStatement::MODS)
				{
					pushModified(op, *id);
				}
				else
				{
					pushDependency(*id);
				}
			}
			return 0;
		}
		
			
		if(opStatement->StatementType == oStatement)
		{
			if(std::static_pointer_cast<operationalStatement>(opStatement)->left)
			{
				if(std::static_pointer_cast<operationalStatement>(opStatement)->op == "." || std::static_pointer_cast<operationalStatement>(opStatement)->op == "::")
				{
					if(std::static_pointer_cast<operationalStatement>(opStatement)->left->StatementType == oStatement)
					{
						std::shared_ptr<Statement> leftLeft = std::static_pointer_cast<operationalStatement>(std::static_pointer_cast<operationalStatement>(opStatement)->left)->left;
						searchVariables(leftLeft, depencyType, op);
					}
				}
				else
				{
					searchVariables(std::static_pointer_cast<operationalStatement>(opStatement)->left, depencyType, op);
				}
			}
				
			if(std::static_pointer_cast<operationalStatement>(opStatement)->right)
			{
				searchVariables(std::static_pointer_cast<operationalStatement>(opStatement)->right, depencyType, op);
			}
		}
		
		return 0;
	}


	std::shared_ptr<Statement> Parser::parseExpressionStatement()
	{
		std::shared_ptr<Statement> result = nullptr;
		
		if(activeStack().Tokens()->at(activeStack().Tokens()->size()-1).value == ";")
		{
			result = parseOpStatement(0, activeStack().Tokens()->size()-2, "-2", scope, parentStatement);
		}
		else
		{
			result = parseOpStatement(0, activeStack().Tokens()->size()-1, "-2", scope, parentStatement);
		}
		return result;
	}


	std::string Parser::getOpStatementType(std::string stmtType, const std::string& classID)
	{
		std::string type;

		if(stmtType == "NIL")
		{
			displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Wrong type variable : " + activeStack().getToken().value);
		}
		else if(stmtType == "-2")
		{
			if(isFunction(activeStack().getToken(0).value, true, classID)) /// function, set to it's return type
			{
				type  = getFunc(activeStack().getToken(0).value, classID).returnType;
			}
			else if(DM14::types::isDataType(activeStack().getToken(0).value)) /// if it is  datatype, use it as the Statement type
			{
				type  = activeStack().getToken(0).value;
			}		
			else if(activeStack().getToken(0).type == "identifier") // if identifier, then use it's type
			{
				type  = findIDType(idInfo(activeStack().getToken(0).value, 0, "", NULL), classID);
			}
			else if(DM14::types::isImmediate(activeStack().getToken(0)) ) /// use immediate type
			{
				type  = activeStack().getToken(0).type;
			}
			/*else if((DM14::types::isSingleOperator(activeStack().getToken(0).value) || activeStack().getToken(0).value == "@") && 0 < to)
			{
				if(isImmediate(activeStack().getToken(0+1)))
				{
					type  = activeStack().getToken(0+1).type;
				}
				else if(activeStack().getToken(0+1).type == "identifier" )
				{
					type  = findIDType(idInfo(activeStack().getToken(0+1).value, 0, "", NULL), classID);
				}
				else
				{
					displayError(fName, activeStack().getToken(0).lineNumber,activeStack().getToken(0).columnNumber,"Did not expect "+ activeStack().getToken(0).value);
				}
			}
			else
			{
				displayError(fName, activeStack().getToken(0).lineNumber, activeStack().getToken(0).columnNumber,"Did not expect "+ activeStack().getToken(0).value);
			}*/
		}
		else
		{
			type  = stmtType;
		}

		return type;
	}

	bool Parser::restore(std::shared_ptr<std::vector<token>> extract_temp_vector)
	{
		bool result = true;

		for(uint32_t i =0; i < extract_temp_vector->size(); i++)
		{
			ebnf.pushToken(extract_temp_vector->at(i));
		}
		extract_temp_vector.reset();
		return result;
	}

	std::shared_ptr<std::vector<token>> Parser::extract(int32_t from, int32_t to)
	{
		std::shared_ptr<std::vector<token>>  extract_temp_vector(new std::vector<token>());
		for(uint32_t i = 0; i < from; i++)
		{
			extract_temp_vector->push_back(activeStack().popToken());
		}

		for(uint32_t i = to+1, current_size = activeStack().Tokens()->size(); i < current_size; i++)
		{
			extract_temp_vector->push_back(activeStack().popToken(to+1));
		}
		return extract_temp_vector;
	}

	std::shared_ptr<Statement> Parser::parseOpStatement(int32_t from, int32_t to, 
										const string& stmtType, const int& scopeLevel, std::shared_ptr<Statement> caller, 
										std::shared_ptr<idInfo> parent, const string& parentOp)
	{
		int plevel =0; // precedence levels
		
		string classID = "";
		
		if(parent)
		{	
			classID = parent->type;
		}
		
		bool removeBigParenthes = true;
		
		// [ overall check for(s and )s
		for(int32_t i= from; i <= to ; i++ )
		{
			if(activeStack().getToken(i).value == "(" )
			{
				++plevel;
			}
			else if(activeStack().getToken(i).value == ")" )
			{
				--plevel;
			}

			if(plevel == 0)
			{
				if(i < to)
				{
					removeBigParenthes=false;
				}
			}
		}
		
		if(plevel > 0 )
		{
			displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber," missing \")\" ");
		}
		else if(plevel < 0 )
		{
			displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber," missing \"(\" ");
		}
		
		// big(..) Statement
		if(activeStack().getToken(from).value == "("  && activeStack().getToken(to).value == ")" && removeBigParenthes)
		{
			activeStack().popToken();
			activeStack().Tokens()->remove(activeStack().Tokens()->size()-1);
			return parseOpStatement(0, to-2, stmtType, scopeLevel+1, caller);
		}
		// ]
		
		int origiIndex = activeStack().getIndex();
		bool classMember = false;

		std::shared_ptr<operationalStatement> opStatement(new operationalStatement);
		opStatement->line = activeStack().getToken().lineNumber;
		opStatement->scope = scope;
		opStatement->scopeLevel = scopeLevel;
		
		if(caller)
		{
			currentStatement = caller;
		}
		else
		{
			currentStatement = opStatement;
		}
		

		// [ split the opStatement to left and right if there is an operator ;)
		plevel= 0;
		for(int32_t i = from; i <= to; i++ )
		{
			if(activeStack().getToken(i).value == "(")
			{
				plevel++;
				continue;
			}
			else if(activeStack().getToken(i).value == ")" )
			{
				plevel--;
				continue;
			}
			
			if(plevel == 0)
			{				
				if((activeStack().getToken(i).type == "operator") &&
					!(activeStack().getToken(i).value == ")" || activeStack().getToken(i).value == "(" || activeStack().getToken(i).value == "["  || activeStack().getToken(i).value == "]"))
				{
					opStatement->op = activeStack().getToken(i).value;
					if(i != from) /** not a prefix operator like ++x */
					{
						std::shared_ptr<std::vector<token>> temp = extract(from, i-1);
						opStatement->left = parseOpStatement(from, i-1, stmtType, opStatement->scopeLevel, currentStatement, parent, opStatement->op);
						opStatement->absorbDistStatements(opStatement->left);
						restore(temp);
						if(opStatement->left->type.size())
						{
							opStatement->type = opStatement->left->type;
						}
					}
			
					if(i<=to)
					{
						to -= i+1;
						i=0;
					}
					from = 0;
					
					activeStack().popToken(); /** pop the op token */
			
					std::shared_ptr<idInfo> id = nullptr;
					// should we loop inside to get the term ?
					
					std::shared_ptr<Statement> res = findTreeNode(opStatement->left, tStatement);
					
					if(res)
					{
						id  = std::static_pointer_cast<termStatement>(res)->id;
					}

					if(opStatement->op == "." ||  opStatement->op == "::" )
					{
						if(!id)
						{
							displayError(fName, activeStack().getToken(i).lineNumber, activeStack().getToken(i).columnNumber,"false class member !");
							
						}
						else
						{
							if(i == to && to > 0)
							{
								displayError(fName, activeStack().getToken(i).lineNumber, activeStack().getToken(i).columnNumber,"incomplete data member access at " +  activeStack().getToken(i).value);
							}

							opStatement->right = parseOpStatement(from, to, "-2", opStatement->scopeLevel, currentStatement, id);
							opStatement->left->type = opStatement->right->type;
							opStatement->type = opStatement->right->type;

							opStatement->absorbDistStatements(opStatement->right);
							
							
							string classID = id->name;
							
							//TODO: make sure :: is used onl with static enumns/variables ?
							if(DM14::types::isDataType(classID) && opStatement->op != "::")
							{
								std::shared_ptr<Statement> staticFunction = findTreeNode(opStatement->right, fCall);
								if(staticFunction != nullptr)
								{	
									
									auto fInfoResult = DM14::types::getClassMemberFunction(classID, *std::static_pointer_cast<functionCall>(staticFunction));
									//TODO
									if(fInfoResult.first == true)
									{
										
										if(fInfoResult.second.noAutism == true)
										{
											opStatement->op = "::";
										}
									}
									else
									{
										displayError(fName, activeStack().getToken(i).lineNumber, activeStack().getToken(i).columnNumber,"class " + classID + " has no member static function " 
 + std::static_pointer_cast<functionCall>(staticFunction)->name);
									}
								}
								else
								{
									std::shared_ptr<Statement> staticVariable = findTreeNode(opStatement->right, tStatement);
									if(staticVariable != nullptr)
									{
										auto vInfoResult = DM14::types::getClassMemberFunction(classID, *std::static_pointer_cast<functionCall>(staticVariable));
										if(vInfoResult.first == true)
										{
											
											if(vInfoResult.second.noAutism == true)
											{
												opStatement->op = "::";
											}
										}
										else
										{
											displayError(fName, activeStack().getToken(i).lineNumber, activeStack().getToken(i).columnNumber,"class " + classID + " has no member static variable " + std::static_pointer_cast<functionCall>(staticVariable)->name);
										}
									}
									else
									{
										displayError(fName, activeStack().getToken(i).lineNumber, activeStack().getToken(i).columnNumber,"parsing error, should not be here !");
									}
								}
							}
							
							
							//Statement* res = findTreeNode(opStatement->right, tStatement);
							//idInfo* rightID  =((termStatement*)res)->id;
							//rightID->arrayIndex = id->arrayIndex;
							//id->arrayIndex = NULL;
							
							/*if(stmtType != "-2" && !hasTypeValue(stmtType, findIDType(idInfo(variableName, 0, "", NULL)) ))
							{
								displayError(fName,(tokens->at(i+2)).lineNumber,(tokens->at(i+2)).columnNumber,"Wrong type variable : " + variableName + " expected : " + opStatement->type);
							}*/
						}
					}
					else if(to > -1)
					{
						opStatement->right = parseOpStatement(from, to, opStatement->type == "" ? stmtType : opStatement->type, opStatement->scopeLevel, currentStatement);
						opStatement->absorbDistStatements(opStatement->right);
						if(!opStatement->left)
						{
							opStatement->type = opStatement->right->type;
						}
					}
					
					if(opStatement->type.size() == 0)
					{
						displayError("could not determine Statement type");
					}
									
					if(DM14::types::isbinOperator(opStatement->op) && to > -1)
					{
						if(opStatement->op == "==" || opStatement->op == "||" || opStatement->op == ">" || opStatement->op == ">"
						 || opStatement->op == ">=" || opStatement->op == "<=" || opStatement->op == "&&")
						{
							opStatement->type = "bool";
						}
						
						if(!opStatement->right)
						{
							displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Missing right operand");
						}
						
						//FIXME:
						/*else if(caller && caller->StatementType == oStatement && !parent && !typeHasOperator(((operationalStatement*)caller)->op, opStatement->type) && opStatement->op != "::"  )
						{
							cerr  << opStatement->op << endl;
							cerr << tokens->at(from).value << endl;
							cerr << tokens->at(to).value << endl;
							displayError(fName,(tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"typee \"" +opStatement->type + "\" does not support operator " +((operationalStatement*)caller)->op);
						}*/
						else if(!DM14::types::typeHasOperator(opStatement->op, opStatement->type) && opStatement->op != "." && opStatement->op != "::" )
						{
							if(opStatement->left)
							cerr << "left type :" << opStatement->left->type << endl;
							displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"type \"" +opStatement->type + "\" does not support operator " + opStatement->op);
						}
						
						searchVariables(opStatement->left, distributingVariablesStatement::MODS, opStatement->op);
						searchVariables(opStatement->right, distributingVariablesStatement::DEPS);
					}
					else if(DM14::types::isSingleOperator(opStatement->op))
					{
						if(!DM14::types::typeHasOperator(opStatement->op, opStatement->type) &&(opStatement->op != "@") )
						{
							displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"type \"" +opStatement->type + "\" does not support operator " + opStatement->op);
						}
						searchVariables(opStatement->left, distributingVariablesStatement::MODS, opStatement->op);
					}
					else if(DM14::types::isCoreOperator(opStatement->op))
					{
						if(opStatement->op == "@")
						{
							//construct node address Statement
							// should we loop inside to get the term ?
							std::shared_ptr<Statement> res = findTreeNode(opStatement->right, tStatement);
					
							std::shared_ptr<idInfo> rightId = nullptr;
								
							if(res)
							{
								rightId  =std::static_pointer_cast<termStatement>(res)->id;
								int isDistributed = findIDInfo(*rightId, DISTRIBUTED);
								if(!isDistributed)
								{
									displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,activeStack().getToken().value + " : can not use @ operator with a non-distributed variable");
									//displayError("can not use @ operator with a non-distributed variable");
								}
							}
							else
							{
								displayError("Coundl't find node address");
							}
							
							// variable@node
							if(opStatement->left && opStatement->left->StatementType == tStatement) // variable@node
							{
								
								if(rightId)
								{
									id->requestAddress = rightId->name;
								}
								else
								{
									displayError("Coundl't find node address");
								}
								
								if(opStatement->right->StatementType == oStatement)
								{
									std::shared_ptr<operationalStatement> stmt = std::static_pointer_cast<operationalStatement>(opStatement->right);
									searchVariables(opStatement->left, distributingVariablesStatement::MODS, stmt->op);
									searchVariables(opStatement->left, distributingVariablesStatement::DEPS);
									opStatement->op  = stmt->op;
									stmt->op = "";
									stmt->left.reset();
									stmt->left = nullptr;
								}				
								else if(opStatement->right->StatementType == tStatement)
								{
									opStatement->right.reset();
									opStatement->right = nullptr;
									opStatement->op  = "";
								}					

								if((parentOp != "=" && parentOp != "+=" && parentOp != "-=" && parentOp != "*=" && parentOp != "/=" && parentOp != "++" && parentOp != "--" && parentOp != "@") || opStatement->op == "")
								{
									std::shared_ptr<idInfo> leftId = nullptr;
									// should we loop inside to get the term ?
									std::shared_ptr<Statement> res = findTreeNode(opStatement->left, tStatement);
						
									if(res)
									{
										leftId  =std::static_pointer_cast<termStatement>(res)->id;
										pushDependency(*leftId);
									}
									else
									{
										displayError("Coundl't find variable to add depedency");
									}							
								}
							}
							else // @variable
							{
								// remove ny depedency of this the variable as we only need its address and 
								// we don't access the variable ittself !
								for(uint32_t l =0; l < currentStatement->distStatements->size(); l++)
								{
									if(currentStatement->distStatements->at(l)->type == distributingVariablesStatement::DEPS)
									{
										 if(currentStatement->distStatements->at(l)->variable.name == rightId->name)
										 {
											 currentStatement->distStatements->remove(l);
											 break;
										 }
									}
								}
							}
							opStatement->type="string";
						}
					}
					else
					{
						displayError("Operator is not defined as single or binary  : " + opStatement->op);
					}
					
					if(  !DM14::types::typeHasOperator(opStatement->op, opStatement->type) 
					   && !(opStatement->left && opStatement->op == ".")
					   && opStatement->op != "@")
					{
						//displayError(fName,(tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"type \"" +opStatement->type + "\" does not support operator " + opStatement->op);
					}
					
					/*
					if(!hasTypeValue(opStatement->type, opStatement->right->type) && !classMember && stmtType!= "-2")
					{
						displayError(fName,(tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"Wrong operands type : " + opStatement->right->type + " with " +opStatement->type);
					}
					else if(!hasTypeValue(stmtType, opStatement->type) && stmtType != "-2")
					{
						displayError(fName,(tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"Wrong operands type : " + stmtType + " with " +opStatement->type);
					}*/
					return opStatement;
				}
			}
		}
		// ]
		


		if(activeStack().getToken(0).value.size() == 0)// consumed all tokens !
		{
			cerr << "from :" << from << " to:" << to << endl;
			displayError(fName, activeStack().getToken(0).lineNumber, activeStack().getToken(0).columnNumber,"Internal parser error@@@ !");
			return opStatement;
		}

		/** proceed to real Statement */
		/** [ find Statement type */
		string type = getOpStatementType(stmtType, classID);
		//if(type.size() && !opStatement->type.size())
		if(type == "")
		{
			displayError(fName, activeStack().getToken(0).lineNumber, activeStack().getToken(0).columnNumber,"unable to determine Statement type "+ activeStack().getToken(0).value);
		}

		if(stmtType!="-2" && stmtType != type)
		{
			displayError(fName, activeStack().getToken(0).lineNumber, activeStack().getToken(0).columnNumber,"types mismatch ! "+ activeStack().getToken(0).value);
		}

		opStatement->type = type;

		if(to == -1 || activeStack().getToken(0).value.size() == 0)// consumed all tokens !
		{
			return opStatement;
		}
		
		// ]
		
		//if(!typeHasOperator(tokens->at(currentIndex).value, findID(tokens->at(currentIndex+1).value, classID).type))
		//{
		//	displayError(fName,(tokens->at(currentIndex)).lineNumber,(tokens->at(currentIndex)).columnNumber,"single operator " +(tokens->at(currentIndex)).value + " is not supported by Statements of type " + findID(tokens->at(currentIndex+1).value, classID).type);
		//	}
		
		
		// [ find first term, function, variable or immediate ...
		
		std::shared_ptr<Statement> aIndex = nullptr;
		// if identifier , then it might be variable or user/builtin function
		if(activeStack().getToken(from).type == "identifier" ||  DM14::types::isDataType(activeStack().getToken().value) )//(tokens->at(currentIndex)).type == "datatype")
		{
			// function !
			if(isBuiltinFunction(activeStack().getToken(from).value) || isUserFunction(activeStack().getToken(from).value, true) 
				||(parent && DM14::types::classHasMemberFunction(classID, activeStack().getToken(from).value)))
			{
				std::shared_ptr<Statement> stmt = nullptr;
				if(parent)
				{
					if(opStatement->type != classID)
					{
						stmt = parseFunctionCallInternal(false, opStatement->type, classID);
					}
					else
					{
						stmt = parseFunctionCallInternal(false, "", classID);
						stmt->type = DM14::types::classMemberFunctionType(classID, activeStack().getToken(from).value);
					}
				}
				else
				{
					stmt = parseFunctionCallInternal(false, opStatement->type);
				}
				stmt->distStatements = opStatement->distStatements;
				return stmt;
			}
			// variable !
			else
			{
				activeStack().popToken();
				string variableName = activeStack().getToken().value;
				
				if(DM14::types::isDataType(variableName))
				{
					displayInfo(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber, "FIX");
				}
				else if(parent && !DM14::types::classHasMemberVariable(classID, variableName))
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"class " + classID + " has no member : " + variableName);
				}
				else if(parent && DM14::types::classHasMemberVariable(classID, variableName))
				{
					if(DM14::types::getClassMemberVariable(classID, variableName).second.classifier != DM14::types::CLASSIFIER::PUBLIC)
					{
						displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,variableName + " is a private class member variable of class type : " + classID);
					}
				}
				else if(findIDType(idInfo(variableName, 0, "", NULL)) == "NIL" && !parent)
				{
					if(peekToken(0).value == "(")
					{
						displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Un-defined function : " + variableName);
					}
					else
					{
						displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Un-defined variable : " + variableName);
					}
				}

				//classMember = isClass(opStatement->type);
				classMember = DM14::types::isClass(classID);
				std::shared_ptr<termStatement> statement(new termStatement(variableName, opStatement->type));
				
				if(findIDType(idInfo(variableName, 0, "", NULL)) != "NIL")
				{
					statement->type = findIDType(idInfo(variableName, 0, "", NULL));
				}
				statement->scope = scope;
				statement->scopeLevel = scopeLevel;
				statement->identifier = true;
				statement->size = findIDInfo(idInfo(variableName , 0, "", NULL), ARRAYSIZE);

				if(activeStack().getToken(0).value == "[")
				{
					if(!findIDInfo(idInfo(variableName , 0, "", NULL), ARRAY))
					{
						displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber, variableName + " is not an array, invalid use of indexing");
					}
					//termStatement->arrayIndex = parseConditionalExpression(currentStatement);
					//termStatement->arrayIndex = parseStatement("expression-list");
					statement->arrayIndex = parseStatement("declaration-index-list");
					aIndex = statement->arrayIndex;
				}
								
				activeStack().setIndex(origiIndex);

				//idInfo* id = new idInfo(termStatement->term, opStatement->scope, opStatement->type, aIndex);
				std::shared_ptr<idInfo> id(new idInfo(statement->term, opStatement->scope, statement->type , aIndex));
				id->distributedScope = distributedScope;
				if(parent)
				{
					id->parent = parent;
				}
				id->array =((bool)findIDInfo(*id, ARRAY) &&(aIndex != NULL));
				id->backProp =(bool)findIDInfo(*id, BACKPROB);
				id->recurrent =(bool)findIDInfo(*id, RECURRENT);
				id->channel =(bool)findIDInfo(*id, CHANNEL);
				id->noblock =(bool)findIDInfo(*id, NOBLOCK);
				id->distributed =(bool)findIDInfo(*id, DISTRIBUTED);
				id->global =(bool)findIDInfo(*id, GLOBAL);
				id->functionParent = currentFunction.name;
				if(aIndex)
				{
					id->arrayIndex = aIndex;
				}
				/*else if(parent)
				{
					id->arrayIndex = parent->arrayIndex;
					parent->arrayIndex = NULL;
				}*/
				
				
				if(DM14::types::isClass(findIDType(idInfo(variableName, 0, "", NULL))))
				{
					statement->type = findIDType(idInfo(variableName, 0, "", NULL));
					id->type = statement->type;
				}
				else
				{
					statement->type = opStatement->type;
				}
				//id.requestAddress = nodeAddress;
				
				statement->id = id;
				
				if(parentOp != "=" && parentOp != "+=" && parentOp != "-=" && parentOp != "*=" && parentOp != "/=" && parentOp != "++" && parentOp != "--" && parentOp != "@")
				{
					if(peekToken(0).value != "." && peekToken(0).value != "::") // do not pus class id, since it will be done by the member anyway !
					{
						pushDependency(*id);
					}
				}
				
				//if(currentStatement != nullptr)
				{
					
					//cerr << currentStatement << endl << flush;
					//currentStatement->absorbDistStatements(opStatement);
				}
				//else
				{
					statement->distStatements = opStatement->distStatements;
				}
				return statement;
			}
		}
		else if(DM14::types::isImmediate(activeStack().getToken(from)))
		{
			if(!DM14::types::hasTypeValue(opStatement->type, activeStack().getToken(from).type))
			{
				displayError(fName, activeStack().getToken(from).lineNumber, activeStack().getToken(from).columnNumber,"wrong immediate type : " + activeStack().getToken(from).value);
			}

			activeStack().popToken();
			
			std::shared_ptr<termStatement> statement(new termStatement(activeStack().getToken().value, opStatement->type));
			statement->line = activeStack().getToken().lineNumber;
			statement->scope = scope;
			statement->scopeLevel = scopeLevel;
			statement->identifier = false;
			statement->type = opStatement->type;
			return statement;
		}
		else
		{
			displayError(fName, activeStack().getToken(from).lineNumber, activeStack().getToken(from).columnNumber,"Un-known Variable or Function : " + activeStack().getToken(from).value);
		}
		
		// ]
		
		displayError(fName, activeStack().getToken(from).lineNumber, activeStack().getToken(from).columnNumber,"returning null Statement ?!!");
		return nullptr;
	};

	funcInfo Parser::getFunc(const string& funcID, const string& classID)
	{
		if(classID.size())
		{
			if(!DM14::types::classHasMemberFunction(classID, funcID))
			{
				return funcInfo();
			}
			
			for(uint32_t i =0; i < datatypes.size(); i++)
			{
				if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
				{
					for(uint32_t k =0; k < datatypes.at(i).memberFunctions.size(); k++)
					{
						if(datatypes.at(i).memberFunctions.at(k).name == funcID)
						{
							return datatypes.at(i).memberFunctions.at(k);
						}
					}
				}
			}
		}
		else
		{
			for(uint32_t i = 0; i < functionsInfo->size(); i++)
			{
				if(funcID ==(functionsInfo->at(i)).name)
				{
					return functionsInfo->at(i);
				}
			}
		}
		return funcInfo();
	};

	funcInfo Parser::getFunc(const string& funcID, Array<string>* mparameters, const string& returnType, const string& classID)
	{
		funcInfo finfo;
		
		if(classID.size())
		{
			if(!DM14::types::classHasMemberFunction(classID, funcID))
			{
				return funcInfo();
			}
			
			for(uint32_t i =0; i < datatypes.size(); i++)
			{
				if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
				{
					for(uint32_t k =0; k < datatypes.at(i).memberFunctions.size(); k++)
					{
						if(datatypes.at(i).memberFunctions.at(k).returnType != returnType && returnType.size())
						{
							if(!DM14::types::hasTypeValue(returnType, datatypes.at(i).memberFunctions.at(k).returnType))
							{
								continue;
							}
						}
						
						if(datatypes.at(i).memberFunctions.at(k).name == funcID)
						{
							if(!datatypes.at(i).memberFunctions.at(k).parameters->size() && !mparameters->size())
							{
								return datatypes.at(i).memberFunctions.at(k);
							}
							
							for(uint32_t l =0; l < mparameters->size(); l++)
							{
								if(mparameters->at(l) != datatypes.at(i).memberFunctions.at(k).parameters->at(l).first)
								{
									//cerr << mparameters->at(l) << ":" << datatypes.at(i).memberFunctions.at(k).parameters->at(l).first << endl;
									//cerr << datatypes.at(i).memberFunctions.at(k).name << ":" <<  funcID << endl;
									break;
								}
								
								if(l ==  mparameters->size()-1)
								{
									if(l == datatypes.at(i).memberFunctions.at(k).parameters->size()-1)
									{
										finfo = datatypes.at(i).memberFunctions.at(k);
										break;
									}
									else if(datatypes.at(i).memberFunctions.at(k).parameters->at(l+1).second)
									{
										finfo = datatypes.at(i).memberFunctions.at(k);
										break;
									}
									else
									{
										break;
									}
								}
							}
							if(finfo.name.size())
							{
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			for(uint32_t i = 0; i < functionsInfo->size(); i++)
			{
				if(functionsInfo->at(i).returnType != returnType && returnType.size())
				{
					if(!DM14::types::hasTypeValue(returnType, functionsInfo->at(i).returnType))
					{
						continue;
					}
				}
				
				if((functionsInfo->at(i)).name == funcID)
				{
					
					if(!functionsInfo->at(i).parameters->size() && !mparameters->size())
					{
						return functionsInfo->at(i);
					}

					//cerr << functionsInfo->at(i).parameters->size() << ":" << mparameters->size() << endl;
					for(uint32_t k = 0, l =0; k < functionsInfo->at(i).parameters->size() && l < mparameters->size(); k++, l++)
					{
						//cerr << mparameters->at(l)  << ":" << functionsInfo->at(i).parameters->at(k).first << endl << flush;
						if(mparameters->at(l) != functionsInfo->at(i).parameters->at(k).first)
						{
							break;
						}
						
						if(l ==  mparameters->size()-1)
						{
							if(l == functionsInfo->at(i).parameters->size()-1)
							{
								finfo = functionsInfo->at(i);
								break;
							}
							else if(functionsInfo->at(i).parameters->at(l+1).second)
							{
								finfo = functionsInfo->at(i);
								break;
							}
							else
							{
								break;
							}
						}
					}

					if(finfo.name.size())
					{
						break;
					}
				}
			}
		}
		return finfo;
	};


	mapcode::mapcode(const string& mname, std::shared_ptr<Array<ast_function>> const mfunctions, 
					 const Array<includePath>& mincludes, const bool& header,
					 const int& nodesC, const int& variablesCount)
	{
		mapcode();
		fileName = mname;

		functions = mfunctions;
		includes = mincludes;
		Header = header;
		nodesCount = nodesC;
		dVariablesCount = variablesCount;
		linkLibs = std::shared_ptr<Array<std::shared_ptr<Statement>>>();
		ExternCodes = std::shared_ptr<Array<string>>();
	};

	mapcode::mapcode()
	{
		functions =	std::shared_ptr<Array<ast_function>>();
		Header = false;
		ExternCodes = std::shared_ptr<Array<string>>();
	};

	mapcode::~mapcode()
	{
	};


	Array<includePath> Parser::getIncludes()
	{
		return includes;
	};

	std::shared_ptr<Array<ast_function>> Parser::getFunctions()
	{
		return functions;
	};

	std::shared_ptr<Array<mapcode>> Parser::getMapCodes()
	{
		return mapCodes;
	};

	std::shared_ptr<Array<string>> Parser::getExternCodes()
	{
		return ExternCodes;
	}


	string mapcode::getFileName()
	{
		return fileName;
	};

	std::shared_ptr<Array<ast_function>> mapcode::getFunctions()
	{
		return functions;
	};


	uint32_t mapcode::addInclude(includePath include)
	{
		includes.push_back(include);
		return includes.size();
	};

	Array<includePath> mapcode::getIncludes()
	{
		return includes;
	};




	bool mapcode::isHeader()
	{
		return Header;
	};

	int mapcode::Print()
	{
		displayDebug(" Code Map  ... [" + fileName + "]");
		displayDebug(" Functions : ");
				
		for(uint32_t i =0; i < functions->size(); i++)
		{
			string functionHeader;
			functionHeader += functions->at(i).returnIDType + " ";
			functionHeader += functions->at(i).name + "(";
			for(uint32_t k =0; k < functions->at(i).parameters->size(); k++)
			{
				functionHeader += functions->at(i).parameters->at(k).type;
				if(k+1 < functions->at(i).parameters->size())
				{
					functionHeader += ",";
				}
			}
			functionHeader += ")";
			displayDebug(functionHeader);
		}
		return 0;
	};

	bool mapcode::setHeader(bool set)
	{
		Header = set;
		return set;
	};

	int Parser::increaseScope(std::shared_ptr<Statement> stmt)
	{
		if(stmt)
		{
			parentStatement = stmt;
		}
		return ++scope;
	}

	int Parser::decreaseScope()
	{
		parentStatement = NULL;
		return --scope;
	}


	std::shared_ptr<Statement> Parser::parseThread()
	{
		activeStack().popToken(); //thread
		std::shared_ptr<threadStatement> thread(new threadStatement());
		currentStatement = thread;

		stringstream SS;
		SS << activeStack().getToken().columnNumber << activeStack().getToken().lineNumber;	// Number of character on the current line
		thread->Identifier = SS.str();
		
		if(parentStatement &&(parentStatement->StatementType ==  fLoop || parentStatement->StatementType == wLoop))
		{
			displayWarning(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Thread call inside a loop ! careful ");
		}
		//@TODO: fix checking the parameters to be global only
		
		/*
		for(int32_t i= 0; i < working_tokens->size(); i++)
		{
			string parent;
			
			if(getToken(1).value == ".")
			{
				parent = activeStack().getToken(0).value;
			}
			
			
			if(tokens->at(i).type == "identifier" && findID(tokens->at(i).value, parent).name.size() && !findIDInfo(idInfo((tokens->at(i)).value, 0, "", NULL), GLOBAL))
			{
				displayError(fName, activeStack().getToken(0).lineNumber, activeStack().getToken(0).columnNumber,"Only global variables are allowed to be passed to a thread call ! , " + activeStack().getToken(0).value + " is not global");
			}
		}

		Statement* stmt = parseStatement("Statement");
		
		//@TODO: FIX if parameters are not global or immediates, give error
		if(stmt->StatementType == oStatement)
		{
			//fix also consider parameters to be unique identidier
			thread->parentID =(tokens->at(from)).value;
			thread->classID = findIDType(findID((tokens->at(from)).value));
			thread->ID =(tokens->at(from+2)).value;
			thread->classMember = true;
			thread->returnType =((operationalStatement*)stmt)->type;
			thread->functioncall =((operationalStatement*)stmt)->right;
		}
		else if(stmt->StatementType == fCall)
		{
			//fix also consider parameters to be unique identidier
			thread->ID =(tokens->at(from)).value;
			thread->returnType =((functionCall*)stmt)->type;
			thread->functioncall = stmt;
		}
		
		if(stmt->distStatements.size())
		{
			thread->functioncall->distStatements = stmt->distStatements;
		}
		
		thread->type = stmt->type;
		*/
		return thread;
	}


	std::shared_ptr<Statement> Parser::parseIf()
	{
		// if [ cond ] { Statements } else {}
		activeStack().popToken(); //if	
		std::shared_ptr<IF> If(new IF);
		If->line = activeStack().getToken().lineNumber;
		If->scope = scope;
		
		increaseScope(If);

		tmpScope = true;
		activeStack().popToken(); //[
		currentStatement = If;
		
		If->condition = parseStatement("expression-list");
		
		tmpScope = false;
		activeStack().popToken(); //]
		
		addStatementDistributingVariables(If);	
		
		activeStack().popToken(); // {
		RequireValue("{", "Expected { and not : ", true);


		//while(activeStack().getToken().value != "}")
		while(!peekToken("}"))
		{
			std::shared_ptr<Statement> stmt = parseStatement("Statement");
			addStatementDistributingVariables(stmt);
			If->body->push_back(stmt);
		}
		activeStack().popToken(); // }

		while(peekToken("else"))
		{
			activeStack().popToken(); //else
			if(peekToken("if"))
			{
				activeStack().popToken(); //if
				
				std::shared_ptr<IF> elseIf(new IF);
				elseIf->line = activeStack().getToken().lineNumber;
				elseIf->scope = scope;

				tmpScope = true;
				activeStack().popToken(); //[
				currentStatement = elseIf;
				elseIf->condition = parseStatement("expression-list");
				tmpScope = false;
				activeStack().popToken(); //]
				
				activeStack().popToken(); // {
				RequireValue("{", "Expected { and not : ", true);
				
				while(!peekToken("}"))
				{
					std::shared_ptr<Statement> stmt = parseStatement("Statement");
					addStatementDistributingVariables(stmt);
					elseIf->body->push_back(stmt);
					
				}
				activeStack().popToken(); //}
				If->elseIF->push_back(elseIf);
			}
			else
			{
				activeStack().popToken(); // {
				RequireValue("{", "Expected { and not : ", true);
				
				while(!peekToken("}"))
				{
					std::shared_ptr<Statement> stmt = parseStatement("Statement");
					addStatementDistributingVariables(stmt);
					If->ELSE->push_back(stmt);
					
				}
				activeStack().popToken(); // }
			}
			//nextIndex();
		}
		
		decreaseScope();
		return If;
	};

	std::shared_ptr<Statement> Parser::parseStruct()
	{
		activeStack().popToken(); //struct
		
		DatatypeBase Struct;
		//Struct.templateNames = templateNames;
		
		activeStack().popToken(); // id
		Struct.setID(activeStack().getToken().value);
		
		activeStack().popToken();
		RequireValue("{", "Expected { and not : ", true);

		while(!peekToken("}"))
		{
			std::shared_ptr<declareStatement> stmt = std::static_pointer_cast<declareStatement>(parseStatement("Statement"));
			//declareStatement* stmt =(declareStatement*)parseDeclaration();
			//extract members ....	
			for(uint32_t i = 0; i < stmt->identifiers->size(); i++)
			{
				funcInfo finfo;
				finfo.type = DM14::types::types::DATAMEMBER;
				//finfo.returnType = stmt->identifiers->at(i).type;
				finfo.returnType = stmt->type;
				finfo.name = stmt->identifiers->at(i).name;
				Struct.memberVariables.push_back(finfo);
			}
			//activeStack().popToken();
		}
		
		activeStack().popToken();	
		RequireValue("}", "Expected } and not : ", true);

		Struct.addOperator("=");
		Struct.addOperator(".");
		Struct.addTypeValue(Struct.typeID);
		Struct.classType = true;
		
		datatypes.push_back(Struct);
		mapcodeDatatypes.push_back(Struct);
		
		//return new NOPStatement;
		//return nullptr;
		return nullptr;
	}

	std::shared_ptr<Statement> Parser::parseExtern()
	{
		activeStack().popToken();
		std::shared_ptr<EXTERN> externStatement(new EXTERN);
		externStatement->line = activeStack().getToken().lineNumber;
		externStatement->scope = scope;
		
		int from = activeStack().getIndex()+1;
		reachToken("endextern", false, true, true, true, true);
		int to = activeStack().getIndex()-1;
		
		for(int32_t i= from; i <= to; i++)
		{
			externStatement->body +=(tokens->at(i)).value;
		}
		
		if(scope == 0)
		{
			ExternCodes->push_back(externStatement->body);
		}
		else
		{
			//add externStatement; ??
		}
		
		return externStatement;
	};



	int Parser::extractSplitStatements(std::shared_ptr<Array<std::shared_ptr<Statement>>> array, std::shared_ptr<Array<std::shared_ptr<Statement>>> splitStatements)
	{
		while(splitStatements->size())
		{
			array->push_back(splitStatements->at(0));
			splitStatements->remove(0);
		}
		return array->size();
	}

	std::shared_ptr<Statement> Parser::parseWhile()
	{
		activeStack().popToken(); //while
		std::shared_ptr<whileloop> While(new whileloop);
		While->line = activeStack().getToken().lineNumber;
		While->scope = scope;
		
		increaseScope(While);
		
		tmpScope = true;
		activeStack().popToken(); //[
		currentStatement = While;
		While->condition = parseStatement("expression-list");
		tmpScope = false;
		activeStack().popToken(); //]
		
		addStatementDistributingVariables(While);
		
		
		activeStack().popToken(); // {
		RequireValue("{", "Expected { and not : ", true);

		while(!peekToken("}"))
		{
			std::shared_ptr<Statement> stmt = parseStatement("Statement");
			addStatementDistributingVariables(stmt);
			While->body->push_back(stmt);
			
		}
		activeStack().popToken(); // }
		
		decreaseScope();
		
		return While;
	};

	std::shared_ptr<Statement> Parser::parseCase()
	{
		std::shared_ptr<CASE> Case(new CASE);
		Case->line =(tokens->at(activeStack().getIndex())).lineNumber;
		Case->scope = scope;	
		
		increaseScope(Case);
		
		//Case->condition = parseConditionalExpression(Case);
		currentStatement = Case;
		Case->condition = parseStatement("expression-list");
		
		addStatementDistributingVariables(Case);
		
		RequireValue("{", "Expected { and not : ", true);
		
		while(!peekToken("}"))
		{
			//nextIndex();
			tmpScope = true;
			std::shared_ptr<Statement> CCondition = nullptr;
			//CCondition = parseConditionalExpression(CCondition);
			currentStatement = CCondition;
			CCondition = parseStatement("expression-list");
			tmpScope = false;
			
			if(tokens->at(activeStack().getIndex()).value == "}")
			{
				activeStack().setIndex(activeStack().getIndex()-1);
				break;
			}
			//check if it peek of current ???
			while(tokens->at(activeStack().getIndex()).value != "}" && tokens->at(activeStack().getIndex()).value != "[")
			{
				Case->Body[CCondition].push_back(parseStatement("Statement"));
				if(!peekToken("}") && !peekToken("["))
				{
					activeStack().nextIndex();
				}
				else
				{
					//nextIndex(); // if we add this, then we might stop at [, which needs to back a
					break;
				}
			}
		}
		
		activeStack().nextIndex();
		RequireValue("}", "Expected } and not : ", true);
		
		decreaseScope();
		
		return Case;
	};

	int32_t Parser::reachToken(	const string& Char, const bool& sameLine,  const bool& reportError, 
							const bool& actual, const bool& beforeEOF, const bool& scopeLevel) // loop till find the specified char
	{
		int32_t line = activeStack().getToken().lineNumber;
		int32_t plevel = 1;
		string firstValue = activeStack().getToken().value; // for scopeLevel
		
		if(actual)
		{
			while(activeStack().popToken().value.size())
			{
				if(sameLine)
				{
					if(activeStack().getToken().lineNumber != line )
					{
						if(reportError)
						{
							displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Missing " + Char);
						}
						else
						{
							return -1;
						}	
					}
				}
				
				if(scopeLevel)
				{
					if(activeStack().getToken().value == Char)
					{
						plevel--;
					}
					else if(activeStack().getToken().value == firstValue)
					{
						plevel++;
					}
				}
				
				if((beforeEOF && activeStack().getToken().value == ";") || activeStack().getToken().value == Char)
				{
					if(scopeLevel )
					{
						if(plevel == 0)
						{
							//return *working_tokens_index;
							return 0;
						}
					}
					else
					{
						//return *working_tokens_index;
						return 0;
					}
				}
				
				//if((unsigned) *working_tokens_index == working_tokens->size())
				if((unsigned) 0 == activeStack().Tokens()->size())
				{
					if(reportError)
					{
						displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Missing " + Char);
					}
				}
			}
		}
		else
		{
			int iterator = 0;
			while((unsigned) iterator < activeStack().Tokens()->size())
			{
				if(sameLine)
				{
					if(activeStack().getToken(iterator).lineNumber != line)
					{
						if(reportError)
						{
							displayError(fName, activeStack().getToken(iterator).lineNumber, activeStack().getToken(iterator).columnNumber,"Missing " + Char);
						}
						else
						{
							return -1;
						}
					}
				}

				
				if(scopeLevel)
				{
					if(activeStack().getToken().value == Char)
					{
						plevel--;
					}
					else if(activeStack().getToken().value == firstValue)
					{
						plevel++;
					}
				}
				
				if((beforeEOF && activeStack().getToken().value == ";") || activeStack().getToken(iterator).value == Char)
				{
					return iterator;
				}
				
				if((unsigned) iterator == activeStack().Tokens()->size()-1)
				{
					if(reportError)
					{
						displayError(fName, activeStack().getToken(iterator).lineNumber, activeStack().getToken(iterator).columnNumber,"Missing " + Char);
					}
				}
				
				iterator++;
			}
		}
		return -1; // couldnt find
	};

	token Parser::peekToken(const int& pos)
	{
		if(activeStack().Tokens()->size())
		{
			if((unsigned) pos < activeStack().Tokens()->size())
			{
				return activeStack().Tokens()->at(pos);
			}
		}
		return token();
	};


	/**
	 * @detail peek the first token in the working_tokens vector, which is not poped yet
	 */
	bool Parser::peekToken(const string& str)
	{
		if(activeStack().Tokens()->size())
		{
			if(activeStack().Tokens()->at(0).value == str)
			{
				return true;
			}
		}
		return false;
	};

	string Parser::getType(const int& Index)
	{
		if(isIdentifier(activeStack().getToken().value))
		{
			return findIDType(idInfo(activeStack().getToken().value, 0, "", NULL));
		}
		else if(DM14::types::isImmediate(activeStack().getToken()))
		{
			return activeStack().getToken().type;
		}
		else if(isFunction(activeStack().getToken().value, true))
		{
			return getFunc(activeStack().getToken().value).returnType;
		}
		else
		{
			displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"can not get type of : " + activeStack().getToken().value);
		}
		return "NIL";
	};

	bool Parser::checkToken(int type, string error, bool addtoken)
	{
		bool rError = false;
		
		if(type == DM14::types::types::Function)
		{
			if(isBuiltinFunction(activeStack().getToken().value )  || isUserFunction(activeStack().getToken().value, true) )
			{
				rError = true;
			}
		}
		else if(type == DM14::types::types::USERFUNCTION)
		{
			if(isUserFunction(activeStack().getToken().value, true) )
			{
				rError = true;
			}
		}
		else if(type == DM14::types::types::BUILTINFUNCTION)
		{
			if(isBuiltinFunction(activeStack().getToken().value ) )
			{
				rError = true;
			}
		}
		
		if(rError)
		{
			displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,error + activeStack().getToken().value);
		}
		return rError;
	};

	bool Parser::checkToken(string value, string error, bool addtoken)
	{
		bool rError = false;
		if(activeStack().getToken().value == value)
		{
			rError = true;
		}
		
		if(rError)
		{
			displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,error + activeStack().getToken().value);
		}
		return rError;
	};


	bool Parser::RequireType(string type, string error, bool addtoken) // add and int for index modification like 1 or -1 :D
	{
		bool rError = false;
		
		if(activeStack().getToken().type != type)
		{
			rError = true;
		}
		
		if(rError)
		{
			if(addtoken)
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,error + activeStack().getToken().value);
			}
			else
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,error);
			}
		}
		return rError;
	};

	bool Parser::RequireValue(string value, string error, bool addtoken) // add and int for index modification like 1 or -1 :D
	{
		bool rError = false;
		
		if(activeStack().getToken().value != value)
		{
			rError = true;
		}
		
		if(rError)
		{
			if(addtoken)
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,error + activeStack().getToken().value);
			}
			else
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,error);
			}
		}
		return rError;
	};


	// maybe w should move this to scanner ? and modify isFunction instead of reading the stupid manually made maps file !?!
	int Parser::mapFunctions(const string& package, const string& library)
	{
		// get the includes folder full path from the compiler strings

		string fullLibraryName;
		for(uint32_t i = 0; i < includePaths.size(); i++)
		{
			ifstream ifs;			
			
			if(library == package )
			{
				fullLibraryName = includePaths.at(i) + "/" + package + "/" + package + ".hpp";
			}
			else
			{
				fullLibraryName = includePaths.at(i) + "/" + package + "/" + library + ".hpp";
			}	

			ifs.open(fullLibraryName);

			if(ifs.is_open())
			{
				ifs.close();
				break;
			}
		}

		//displayInfo(" Scanning  ... [" + fullLibraryName + "]");
		std::shared_ptr<DM14::scanner> scner(new DM14::scanner(fullLibraryName));
		
		if(!scner->isReady())
		{
			displayError(scner->getFileName(), -1,-1,"Could not open Library : " + fullLibraryName);
		}
		
		scner->setShortComment("//");
		scner->setLongComment("/*", "*/");
		scner->scan();
		//scner->printTokens();
		
		displayInfo(" Parsing   ... [" + fullLibraryName + "]");
		
		std::shared_ptr<Array<token>> mapTokens = scner->getTokens();

		Array<string> templateNames;
		
		for(uint32_t i = 0; i < mapTokens->size(); i++)
		{
			
			if(mapTokens->at(i).value == "#")
			{
				int line = mapTokens->at(i).lineNumber;
				for(uint32_t k = i; k <  mapTokens->size(); k++)
				{
					if(mapTokens->at(k).lineNumber != line)
					{
						i = k-1;
						break;
					}
					if(k+1 == mapTokens->size())
					{
						i = k;
					}
				}
			}
			else if(mapTokens->at(i).value == "using") // using
			{
				if(scner->reachToken(i, ";", false, false, true) == -1)
				{
					displayError(scner->getFileName(), -1,-1,"Internal Compiler error, parsing using Statement in package : " + fullLibraryName);
				}
				i = scner->reachToken(i, ";", false, false, true);
			}
			else if(mapTokens->at(i).value == "template" || mapTokens->at(i).value == "typename")
			{
				int brace = 0;
				for(uint32_t k = i+1; k < mapTokens->size(); k++)
				{
					if(mapTokens->at(k).value == "class" || mapTokens->at(k).value == "typename")
					{
						templateNames.push_back(mapTokens->at(k+1).value);
					}
					else if(mapTokens->at(k).value == "<")
					{
						brace++;
					}
					else if(mapTokens->at(k).value == ">")
					{
						brace--;
						if(brace == 0)
						{
							i = k;
							break;
						}
					}
				}
			}
			else if(mapTokens->at(i).value == "class" ||
					 mapTokens->at(i).value == "struct" ||
					 mapTokens->at(i).value == "typedef") //class , struct, typedef
			{
				i = parseCClass(scner, i, templateNames);
				templateNames.clear();
				if(i ==0)
				{
					displayError(scner->getFileName(), -1,-1,"Internal Compiler error, parsing class in package : " + fullLibraryName);
				}
			}
			else if(DM14::types::isDataType(mapTokens->at(i).value) || mapTokens->at(i).value == "void" || mapTokens->at(i).value == "volatile")
			{
				funcInfo finfo = parseCFunction(scner, i, DatatypeBase());
				//cerr << "||" << finfo.name << ":" << finfo.returnType << endl;
				if(finfo.name.size())
				{
					functionsInfo->push_back(finfo);
				}
				i = finfo.classifier;
			}
			else
			{
				displayError(scner->getFileName(), -1,-1,"Internal Compiler error, parsing package : " + fullLibraryName + " : token -> " + mapTokens->at(i).value);
			}
		}
		
		return 0;
	};


	long Parser::parseCClass(std::shared_ptr<DM14::scanner> scner, uint32_t start, const Array<string>& templateNames)
	{
		DatatypeBase CClass;
		CClass.templateNames = templateNames;
		
		int classifier = 1;
		
		bool TYPEDEF = false;
		bool protoType  = true;
		bool forwardDecelation = false;
		std::shared_ptr<Array<token>> mapTokens = scner->getTokens();
		
		if(mapTokens->at(start).value == "typedef")
		{
			TYPEDEF = true;
		}
		
		
		if(!TYPEDEF) // class foo
		{
			int semiColon = scner->reachToken(start, ";", false, false, true);
			int rightBrace = scner->reachToken(start, "{", false, false, true);
			
			if(semiColon  == start+2)
			{
				CClass.setID(mapTokens->at(start+1).value);
				start = scner->reachToken(start, ";", false, false, true);
				forwardDecelation = true;
			}
			else if(semiColon > rightBrace && rightBrace != -1)
			{
				protoType = false;
				CClass.setID(mapTokens->at(start+1).value);
				start = scner->reachToken(start, "{", false, false, true)+1;
			}
			else
			{
				return 0;
			}
			
		}
		else // typdef foo bar
		{
			if(scner->reachToken(start, ";", false, false, true) == -1)
			{
				return 0;
			}
			start = scner->reachToken(start, ";", false, false, true);
			CClass.setID(mapTokens->at(start-1).value);
			forwardDecelation = true;
		}
		
			
		if(!forwardDecelation)
		{
			for(uint32_t i = start; i < mapTokens->size(); i++)
			{
				if(mapTokens->at(i).value == "public" || mapTokens->at(i).value == "public:")
				{
					if(mapTokens->at(i).value == "public")
					{
						if(scner->reachToken(i, ":", false, false, true) != i+1)
						{
							displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Expected : inside class declaration : " + CClass.typeID + package);
						}
					}
					classifier = DM14::types::CLASSIFIER::PUBLIC;
					i++;
				}
				else if(mapTokens->at(i).value == "private" || mapTokens->at(i).value == "private:")
				{
					if(mapTokens->at(i).value == "private")
					{
						if(scner->reachToken(i, ":", false, false, true) != i+1)
						{
							displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Expected : inside class declaration : " + CClass.typeID + package);
						}
					}
					classifier = DM14::types::CLASSIFIER::PRIVATE;
					i++;
				}
				else if(mapTokens->at(i).value == "protected" || mapTokens->at(i).value == "protected:")
				{
					if(mapTokens->at(i).value == "protected")
					{
						if(scner->reachToken(i, ":", false, false, true) != i+1)
						{
							displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Expected : inside class declaration : " + CClass.typeID + package);
						}
					}
					classifier = DM14::types::CLASSIFIER::PROTECTED;
					i++;
				}
				else if(mapTokens->at(i).value == "}")
				{
					start = i;
					if(scner->reachToken(start, ";", false, false, true) == i+1)
					{
						start++;
					}
					break;
				}
				else
				{
					funcInfo finfo = parseCFunction(scner, i, CClass);
					
					i = finfo.classifier;
					finfo.classifier = classifier;
					if(finfo.type == DM14::types::types::DATAMEMBER)
					{
						CClass.memberVariables.push_back(finfo);
					}
					else
					{
						CClass.memberFunctions.push_back(finfo);
						
						if(finfo.name == CClass.typeID)
						{
							//finfo.classConstructor = true;
							//functionsInfo->push_back(finfo);
						}
					}
				}
			}
		}

		CClass.addOperator("=");
		CClass.addTypeValue(CClass.typeID);
		CClass.classType = true;
		
		for(uint32_t i =0; i < CClass.parents.size(); i++)
		{
			CClass.addTypeValue(CClass.parents.at(i));
		}
		
		/*for(unsigned i = index+1; i < tokens->size(); i++)
		{
			if((tokens->at(i)).value == CClass.typeID)
			{
				(tokens->at(i)).type = "datatype";
			}
		}*/
		
		CClass.protoType = protoType;
		
		if(!protoType)
		{
			for(uint32_t i =0; i < datatypes.size(); i++)
			{
				if(datatypes.at(i).typeID == CClass.typeID && datatypes.at(i).classType && datatypes.at(i).protoType)
				{
					datatypes.remove(i);
					i--;
				}
			}
		}
		datatypes.push_back(CClass);

		return start;
	}



	funcInfo Parser::parseCFunction(std::shared_ptr<DM14::scanner> scner, uint32_t start, const DatatypeBase& parentClass)
	{
		funcInfo funcinfo;
		std::shared_ptr<Array<token>> mapTokens = scner->getTokens();
		
		//cout << " tempalte : " << parentClass.templateName << endl;
		bool destructor = false;
		bool Enum = false;
		for(uint32_t i = start; i < mapTokens->size(); i++) // function return, data member type
		{
			
			if(mapTokens->at(i).value  == "const" || 
				mapTokens->at(i).value == "unsigned" || 
				mapTokens->at(i).value == "&" || 
				mapTokens->at(i).value == "struct" || 
				mapTokens->at(i).value == "volatile" )
			{
				continue;
			}
			else if(mapTokens->at(i).value == "static")
			{
				funcinfo.noAutism = true;
				continue;
			}
			else if(mapTokens->at(i).value == "enum")
			{
				Enum = true;
				start = i;
				funcinfo.returnType = "int";
				break;
			}
			else if(mapTokens->at(i).value == "template" || mapTokens->at(i).value == "typename" )
			{
				for(uint32_t k = i+1; k < mapTokens->size(); k++) // skip tempalte stuff
				{
					if(mapTokens->at(k).value == ">")
					{
						i = k;
						break;
					}
				}
				continue;
			}
			else if(mapTokens->at(i).value == "*")
			{
				displayWarning(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Functions should not return a pointer  : " + funcinfo.name + package);
				continue;
			}
			else
			{
				if(scner->tokenAt(i+1).value == "::")
				{
					i++;
					continue;
				}
				
				if(mapTokens->at(i).value== "~")
				{
					destructor = true;
					continue;
				}
				else if(mapTokens->at(i).type != "identifier" && !DM14::types::isDataType(mapTokens->at(i).value))//mapTokens->at(i).type != "datatype")
				{
					displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"could not parse a c++ function retun type, expected identifier and not :  " + mapTokens->at(i).value);
				}
				
				if(parentClass.typeID.size() && mapTokens->at(i).value == parentClass.typeID)
				{
					if(!destructor)
					{
						funcinfo.returnType = parentClass.typeID;
					}
				}
				else
				{
					if(mapTokens->at(i).value == "void")
					{
						displayWarning(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Functions should not have return type of \"void\" adding as int ");
						funcinfo.returnType = "int";
					}
					else
					{
						funcinfo.returnType = DM14::types::getDataType(mapTokens->at(i).value);
					
						if(funcinfo.returnType == "NIL")
						{
							displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Uknown type : " + mapTokens->at(i).value);
						}
					}
				}
				
				if(scner->tokenAt(i+1).value == "*" ||
					scner->tokenAt(i+1).value == "&" ||
					scner->tokenAt(i+1).value == "const")
				{
					i++;
				}
			}
			start = i;
			break;
		}
		
		
		for(uint32_t i = start+1; i < mapTokens->size(); i++) //ignore template parameters
		{
			
			if(mapTokens->at(i).value == "<")
			{
				for(uint32_t k = i+1; k < mapTokens->size(); k++)
				{
					if(mapTokens->at(k).value == ">")
					{
						start = k;
						break;
					}
				}
			}
			break;
		}
		
		for(uint32_t i = start+1; i < mapTokens->size(); i++) // the function/member name
		{
			
			if(mapTokens->at(i).value == "(")
			{
				funcinfo.name = parentClass.typeID;
				start = i;
				break;
			}
			else if(mapTokens->at(i).type != "identifier" && mapTokens->at(i).type != "keyword")
			{
				displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"could not parse a c++ function/member name, expected identifier and not: " + mapTokens->at(i).value);
			}
			funcinfo.name = mapTokens->at(i).value;
			start = i;
			break;
		}
		
		if(Enum)
		{
			DatatypeBase CUSTOMENUM;
			CUSTOMENUM.setID(funcinfo.name);
			CUSTOMENUM.enumType = true;
			CUSTOMENUM.addOperator("=");
			CUSTOMENUM.addOperator("==");
			CUSTOMENUM.addOperator(">=");
			CUSTOMENUM.addOperator("<=");
			CUSTOMENUM.addOperator("<");
			CUSTOMENUM.addOperator(">");
			CUSTOMENUM.addOperator(".");
			CUSTOMENUM.addTypeValue("bool");
			CUSTOMENUM.addTypeValue("int");
			CUSTOMENUM.addTypeValue("float");
			CUSTOMENUM.parents.push_back(parentClass.typeID);
					
			for(uint32_t i = start+1; i < mapTokens->size(); i++) // the function/member name
			{
				if(mapTokens->at(i).type == "identifier")
				{
					funcInfo enumMember;
					enumMember.name = mapTokens->at(i).value;
					enumMember.classifier = DM14::types::CLASSIFIER::PUBLIC;
					enumMember.type = DM14::types::types::DATAMEMBER;
					//enumMember.returnType = "int";
					enumMember.returnType = funcinfo.name;
					CUSTOMENUM.memberVariables.push_back(enumMember);
				}
				
				if(mapTokens->at(i).value == "}")
				{
					//if(mapTokens->at(i+1).value == ";")
					if(scner->tokenAt(i+1).value == ";")
					{
						
						i++;
					}
					/*for(uint32_t l = 0; l < datatypes.size(); l++)
					{
						if(datatypes.at(i).typeID == "int")
						{
							datatypes.at(i).CEquivalent.push_back(funcinfo.name);
							break;
						}
					}*/
					
					datatypes.push_back(CUSTOMENUM);
					
					funcinfo.classifier = i;
					funcinfo.type = DM14::types::types::ENUM;
					return funcinfo;
				}
			}
		}
		
		if(mapTokens->at(start+1).value == "[") // array index ?
		{
			for(uint32_t i = start+1; i < mapTokens->size(); i++) 
			{
				if(mapTokens->at(i).value == "]")
				{
					start = i;
					break;
				}
			}
		}
		
		if(mapTokens->at(start+1).value == "[") // two diemnsional array index ?
		{
			for(uint32_t i = start+1; i < mapTokens->size(); i++) 
			{
				if(mapTokens->at(i).value == "]")
				{
					start = i;
					break;
				}
			}
		}
		
		for(uint32_t i = start+1; i < mapTokens->size(); i++) // the member variable
		{
			if(mapTokens->at(i).value == ";")
			{
				if(!funcinfo.name.size())
				{
					displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(tokens->at(i)).columnNumber,"parsing error : " + mapTokens->at(i).value);
				}
				funcinfo.classifier = i;
				funcinfo.type = DM14::types::types::DATAMEMBER;
				return funcinfo;
			}
			else if(mapTokens->at(i).value == "(")
			{
				start = i;
			}
			
			break;
		}
		
		for(uint32_t i = start+1; i < mapTokens->size(); i++) // function parameterss
		{
			if(mapTokens->at(i).value == "::") // function member dfinition,, ignore all
			{
				int depthLevel = 0;
				for(uint32_t k = i+1; k < mapTokens->size(); k++)
				{
					if(mapTokens->at(k).value == "{")
					{
						depthLevel++;
					}
					else if(mapTokens->at(k).value == "}")
					{
						depthLevel--;
						
						if(depthLevel == 0)
						{
							//if(mapTokens->at(k+1).value == ";")
							if(scner->tokenAt(k+1).value == ";")
							{
								k++;
							}
							funcinfo.classifier = k;
							funcinfo.name = "";
							break;
						}
					}
				}
				return funcinfo;
			}
			
			bool found = false;
			if(mapTokens->at(i).value == ")")
			{
				if(i+1 < mapTokens->size())
				{
					i++;
				}
			
				if(mapTokens->at(i).value != ";")
				{
					//displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(tokens->at(i)).columnNumber,"parsing error, expected ; :" + mapTokens->at(i).value);
					displayWarning(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Function definition in header, be careful !");
					
				}
				
				i--;
				start = i;
				break;
			}
			
			bool init = false;
			int depthLevel = 0;
			for(uint32_t k = i; k < mapTokens->size(); k++) // parameter type
			{
				if(mapTokens->at(k).value  == "const" || 
					mapTokens->at(k).value == "unsigned" || 
					mapTokens->at(k).value == "&" || 
					mapTokens->at(k).value == "volatile")
				{
					continue;
				}
				else if(mapTokens->at(k).value == "*")
				{
					displayWarning(scner->getFileName(),(mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"Functions should not take a pointer  : " + funcinfo.name + package);
					continue;
				}
				else if(mapTokens->at(k).value == ",")
				{
					i = k;
					break;
				}
				else if(mapTokens->at(k).value == ")")
				{
					depthLevel--;
					if(depthLevel == -1)
					{
						i = k-1;
						break;
					}
				}
				else if(mapTokens->at(k).value == "(")
				{
					depthLevel++;
					continue;
				}
				else if(mapTokens->at(k).value == "=")
				{
					//funcinfo.parameters->remove(funcinfo.parameters->size()-1);

					funcinfo.parameters->at(funcinfo.parameters->size()-1).second = true;;
					init = true;
				}
				else
				{
					if(mapTokens->at(k).value == "::")
					{
						funcinfo.parameters->remove(funcinfo.parameters->size()-1);
					}

					if(init)
					{
						continue;
					}
						
					if(mapTokens->at(k).value == "void")
					{
						displayWarning(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(k)).columnNumber,"Functions should not have return type of \"void\"  : " + funcinfo.name + package);
					}
					else if(mapTokens->at(k).type != "identifier")
					{
						//cout << k << ":" << i << ":" << start<<endl;
						//displayError(scner->getFileName(),(mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"could not parse a c++ function retun type, expected identifier: " + mapTokens->at(i).value);
					}
					
					if(parentClass.typeID.size() && mapTokens->at(k).value == parentClass.typeID)
					{
						funcinfo.parameters->push_back(pair<string, bool>(parentClass.typeID, false));
					}
					else
					{
						if(DM14::types::getDataType(mapTokens->at(k).value) != "NIL")
						{
							funcinfo.parameters->push_back(pair<string, bool>(DM14::types::getDataType(mapTokens->at(k).value), false));
							found = true;
						}
						else if(parentClass.typeID.size() && !found)
						{
							funcinfo.parameters->push_back(pair<string, bool>(parentClass.typeID, false));
							found = true;
						}
						else if(!found)
						{
							displayError(scner->getFileName(),(mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"Uknown type : " + mapTokens->at(k).value);
						}
					}
				}
			}
			start = i;
		}
		
		for(uint32_t i = start+1; i < mapTokens->size(); i++) // finish variables inizilization :X(x),Y(y),Z(z)
		{
			if(mapTokens->at(i).value == ":")
			{
				for(uint32_t k = i+1; k < mapTokens->size(); k++) 
				{
					if(mapTokens->at(k).value == ")")
					{
						//if((mapTokens->at(k+1).value == ","))
						if(scner->tokenAt(k+1).value == ",")
						{
							k++;
						}
						//else if((mapTokens->at(k+1).value == "{"))
						else if(scner->tokenAt(k+1).value == "{")
						{
							start = k;
							break;
						}
						//else if((mapTokens->at(k+1).value == ";"))
						else if(scner->tokenAt(k+1).value == ";")
						{
							//k++;
							start = k;
							break;
						}
					}
				}
			}
			break;
		}
		
		
			
		for(uint32_t i = start+1; i < mapTokens->size(); i++) // function inizliation;
		{
			int depthLevel = 1;
			if(mapTokens->at(i).value == "{")
			{
				for(uint32_t k = i+1; k < mapTokens->size(); k++) 
				{
					if(mapTokens->at(k).value == "{")
					{
						depthLevel++;
					}
					else if(mapTokens->at(k).value == "}")
					{
						depthLevel--;
					}
					
					if(depthLevel == 0)
					{
						//if(mapTokens->at(k+1).value != ";")
						if(scner->tokenAt(k+1).value != ";")
						{
							displayError(scner->getFileName(),(mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"Unterminated function definition, missing ;");
						}
						
						i = k+1;
						start = i;
						break;
					}
					
				}
			}
			else if(mapTokens->at(i).value == ";")
			{
				start = i;
			}
			else
			{
				displayError(scner->getFileName(),(mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"not expecting : " +(mapTokens->at(i)).value);
			}
			break;
		}
		
		
		funcinfo.type = DM14::types::types::BUILTINFUNCTION;
		funcinfo.classifier = start;

		return funcinfo;
	}


	idInfo Parser::findID(const string& ID, const string& parentID)
	{
		for(uint32_t i =0; i< identifiers->size(); i++ )
		{
			//if(ID ==(identifiers->at(i)).name && parentID ==(identifiers->at(i)).parent)
			if((identifiers->at(i)).parent && parentID.size())
			{
				if(ID ==(identifiers->at(i)).name && parentID ==(identifiers->at(i)).parent->name)
				{
					return identifiers->at(i);
				}
			}
			else if(!(identifiers->at(i)).parent && !parentID.size())
			{
				if(ID ==(identifiers->at(i)).name)
				{
					return identifiers->at(i);
				}
			}
		}
		return idInfo();
	}
	
	std::shared_ptr<Statement> Parser::parseDeclaration()
	{
		std::shared_ptr<Statement> result = parseDeclarationInternal();
		activeStack().popToken();
		RequireValue(";", "Expected ; and not ", true);
		return result;
	}


	std::shared_ptr<Statement> Parser::parseDeclarationInternal()
	{
		activeStack().popToken();
		std::shared_ptr<declareStatement> decStatement(new declareStatement);
		decStatement->line = activeStack().getToken().lineNumber;
		decStatement->scope = scope;
		
		if(scope == 0)
		{
			decStatement->global = true;
		}
		else
		{
			decStatement->global = false;
		}

		decStatement->tmpScope = tmpScope;

		std::shared_ptr<Array<idInfo>> tempIdentifiers(new Array<idInfo>);

		while(true)
		{
			checkToken(DM14::types::types::Function, "this is a function name ! :  ", true);
			RequireType("identifier", "Expected \"Identifier\" and not ", true);
			
			if(findIDInfo(idInfo(activeStack().getToken().value, 0, "", NULL),SCOPE) == decStatement->scope)
			{
				if(findIDInfo(idInfo(activeStack().getToken().value, 0, "", NULL),TMPSCOPE))
				{
					displayWarning(fName, activeStack().getToken().lineNumber,activeStack().getToken().columnNumber,"Pre-defined variable in different scope :  " + activeStack().getToken().value);
				}
				else if(!tmpScope)
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Pre-defineddd variable :  " + activeStack().getToken().value);
				}
			}
			else if(findIDInfo(idInfo(activeStack().getToken().value, 0, "", NULL),GLOBAL))
			{
				displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"Pre-defined Global variable :  " + activeStack().getToken().value);
			}
			
			tempIdentifiers->push_back(idInfo(activeStack().getToken().value, 0, "", NULL));	
			decStatement->identifiers->push_back(idInfo(activeStack().getToken().value, 0, "", NULL)) ;
			//cerr << "ID : " << activeStack().getToken().value << endl;
			if(!decStatement->tmpScope)
			{
				distributedVariablesCount++;
			}
			
			activeStack().popToken();
			
			if(activeStack().getToken().value != "," )
			{
				break;
			}
			
			activeStack().popToken();
		}
		
		bool distributed = true;
		
		while(true)
		{
			if(activeStack().getToken().value == "noblock" )
			{
				decStatement->noblock = true;
			}
			else if(activeStack().getToken().value == "recurrent" )
			{
				decStatement->recurrent=true;
			}
			else if(activeStack().getToken().value == "backprop" )
			{
				decStatement->backProp=true;
			}
			else if(activeStack().getToken().value == "nodist" )
			{
				if(decStatement->recurrent)
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber, "nodist variable can not be recurrent !");
				}
				
				////displayWarning(fName,(tokens->at(activeStack().getIndex())).lineNumber,(tokens->at(activeStack().getIndex())).columnNumber, "Warning, nodist variable");
				distributed = false;
				decStatement->distributed = false;
			}
			else if(activeStack().getToken().value == "channel" )
			{
				if(decStatement->recurrent || !distributed || decStatement->backProp)
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber, "channel variable is not pure !?");
				}

				decStatement->channel = true;
			}
			else if(activeStack().getToken().value == "global" )
			{
				decStatement->global = true;
			}
			else
			{
				break;
			}
			
			activeStack().popToken();
		}
		
		
		if(!DM14::types::isDataType(activeStack().getToken().value))
		{
			RequireType("datatype", "Expected \"Data type\" and not ", true);
		}
		
		decStatement->type = activeStack().getToken().value;
		decStatement->classtype = DM14::types::isClass(decStatement->type);
		decStatement->array = false;
			
		
		
		/** array index */
		if(peekToken("["))
		{
			activeStack().popToken(); // [
			decStatement->array = true;
			activeStack().popToken();
			//@TODO: only int can be array ?
			if(activeStack().getToken().type == "int")
			{
				stringstream SS;
				SS << activeStack().getToken().value;
				SS >> decStatement->size;
				distributedVariablesCount += decStatement->size;
				activeStack().popToken();
				RequireValue("]", "Expected ] and not ", true);		
			}
			else
			{
				decStatement->size = 0;
				RequireValue("]", "Expected ] and not ", true);
			}
			

			/** second array index == a matrix */
			if(peekToken("["))
			{
				activeStack().popToken();
				decStatement->array = true;
				activeStack().popToken();
				if(activeStack().getToken().type == "int")
				{
					stringstream SS;
					SS << activeStack().getToken().value;
					SS >> decStatement->size;
					distributedVariablesCount += decStatement->size;
					activeStack().popToken();
					RequireValue("]", "Expected ] and not ", true);		
				}
				else
				{
					decStatement->size = 0;
					RequireValue("]", "Expected ] and not ", true);
				}
				activeStack().popToken();
			}
		}


		
		if(peekToken("("))
		{
			activeStack().popToken();
			int to = reachToken(")", true, true, false, false, true);
			//decStatement->value = parseOpStatement(*working_tokens_index, to-1, decStatement->type, 0, decStatement);
			decStatement->value = parseOpStatement(0, to-1, decStatement->type, 0, decStatement);
			reachToken(")", true, true, true, false, true);
			//activeStack().popToken();
			//RequireValue(terminal, "Expected " + terminal + " and not ", true);
			decStatement->Initilazed = true;
		}
		else if(peekToken("="))
		{
			activeStack().popToken();
			/* TODO: FIX: fix the array list initialization or atleast review */
			if(peekToken("{"))
			{
				activeStack().popToken();
				
				if(!decStatement->array)
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"This variable is not an array");
				}
				
				int plevel = 1;
				//int from = *working_tokens_index+1;
				int from = 1;
				while(activeStack().popToken().value.size())
				{
					if(activeStack().getToken().value == "(")
					{
						plevel++;
					}
					else if(activeStack().getToken().value == ")" || activeStack().getToken().value == "}")
					{
						plevel--;
						if(plevel ==0)
						{
							//if(*working_tokens_index != from)
							if(0 != from)
							{
								//decStatement->values.push_back(parseOpStatement(from, *working_tokens_index - 1, "-2", 0, decStatement));
								decStatement->values.push_back(parseOpStatement(from, - 1, decStatement->type, 0, decStatement));
							}
							break;
						}
					}
					else if(activeStack().getToken().value == ",")
					{
						if(plevel==1)
						{		
							//if(*working_tokens_index != from)
							if(0 != from)
							{
								//decStatement->values.push_back(parseOpStatement(from, *working_tokens_index-1, "-2", 0, decStatement));
								decStatement->values.push_back(parseOpStatement(from, -1, decStatement->type, 0, decStatement));
								//from = *working_tokens_index+1;
								from = 1;
							}
						}
					}
				}
				
				if(decStatement->values.size() >(uint32_t) decStatement->size && decStatement->size !=  0)
				{
					displayError(fName, activeStack().getToken().lineNumber, activeStack().getToken().columnNumber,"too many initilizations");
				}
				
				if(decStatement->size == 0)
				{
					decStatement->size = decStatement->values.size();
				}
				
				stringstream SS;
				for(uint32_t k =0; k < decStatement->identifiers->size(); k++)
				{
					for(uint32_t i =0; i < decStatement->values.size(); i++)
					{
						std::shared_ptr<termStatement> statement(new termStatement(decStatement->identifiers->at(k).name, decStatement->type));
						statement->scope = scope;
						
						SS << i;
						std::shared_ptr<termStatement> arrayIndex(new termStatement(SS.str(), decStatement->type));
						arrayIndex->scope = scope;
						
						statement->arrayIndex = arrayIndex;
						//termStatement->identifier = true;
						
						std::shared_ptr<operationalStatement> os(new operationalStatement());
						os->left = statement;
						os->op = "=";
						os->right = decStatement->values.at(i);
						os->type = decStatement->type;
						os->scope = scope;
						currentFunction.body->push_back(os);

						/*FIX: ?? */
						if(!decStatement->global)
						{
							idInfo id(decStatement->identifiers->at(k).name, decStatement->scope, decStatement->type, arrayIndex);
							id.distributedScope = distributedScope;
							id.array = false;	
							id.distributed = distributed;				
							id.backProp = decStatement->backProp;
							id.recurrent = decStatement->recurrent;
							id.channel = decStatement->channel;
							id.noblock = decStatement->noblock;
							id.global = decStatement->global;
							id.global = decStatement->shared;
							id.type = decStatement->type;
							std::shared_ptr<Statement> origiCurrentStatement = currentStatement;
							currentStatement = os;
							pushModified("=", id);
							currentStatement = origiCurrentStatement;
						}
						
						SS.str("");
						SS.clear();
					}
				}
				if(decStatement->size == 0) /*FIX: && decStatement->distributed) */
				{
					distributedVariablesCount += decStatement->values.size();
				}
				decStatement->Initilazed = false;
			}
			else
			{
				//int to = reachToken(terminal, true, true, false, false, false);
				//decStatement->value = parseOpStatement(0, to-1, decStatement->type, 0, decStatement);
				//reachToken(terminal, true, true, true, false, false);
				//decStatement->value = parseOpStatement(0, to-1, decStatement->type, 0, decStatement);
				
				decStatement->value = parseStatement("expression-list", &Parser::parseExpressionStatement);
				for(auto _statement : *decStatement->value->distStatements)
				{
					decStatement->distStatements->push_back(_statement);
				}

				decStatement->value->distStatements->clear();

				if(decStatement->value->type != decStatement->type)
				{
					if(!DM14::types::hasTypeValue(decStatement->type, decStatement->value->type))
					{
						// error on type
						displayError(fName, -1,0,"wrong value type for declaration value", false);
					}
				}
				
				if(decStatement->global)
				{
					for(uint32_t k =0; k < decStatement->identifiers->size(); k++)
					{
						std::shared_ptr<termStatement> statement(new termStatement(decStatement->identifiers->at(k).name, decStatement->type));
						statement->scope = scope;
						
						std::shared_ptr<operationalStatement> os(new operationalStatement());
						os->left = statement;
						os->op = "=";
						os->right = decStatement->value;
						os->type = decStatement->type;
						os->scope = scope;
						//globalDefinitions.push_back(os);
						decStatement->splitStatements->push_back(os);
						////currentFunction.body->push_back(os);
					}
					
					if(decStatement->splitStatements->size())
					{
						decStatement->splitStatements->at(0)->distStatements = decStatement->distStatements;
						decStatement->distStatements->clear();
					}
					decStatement->value = NULL;
				}
				decStatement->Initilazed = true;
			}
		}
		else
		{
			//cerr << "VAL :" << activeStack().getToken().value.size() << endl;
			//RequireValue(terminal, "Expected " + terminal + " and not ", true);
			decStatement->Initilazed = false;
			decStatement->value = NULL;
		}
		
		stringstream SS;
		
		decStatement->identifiers->clear();
		for(uint32_t i = 0; i < tempIdentifiers->size(); i++)
		{
			SS << i;
			std::shared_ptr<termStatement> arrayIndex(new termStatement(SS.str(), decStatement->type));
			arrayIndex->scope = scope;
			
			idInfo idinfo(tempIdentifiers->at(i).name, decStatement->scope, decStatement->type, arrayIndex);
			idinfo.tmpScope = tmpScope;
			idinfo.size = decStatement->size;
			idinfo.distributed = distributed;
			idinfo.backProp = decStatement->backProp;
			idinfo.recurrent = decStatement->recurrent;
			idinfo.channel = decStatement->channel;
			idinfo.noblock = decStatement->noblock;
			idinfo.array = false;
			idinfo.global = decStatement->global;
			idinfo.type = decStatement->type;
			if(decStatement->array)
			{
				idinfo.array = true;
			}
			
			decStatement->identifiers->push_back(idinfo);
			identifiers->push_back(idinfo);
			
			for(uint32_t i =0; i < datatypes.size(); i++)
			{
				if(datatypes.at(i).typeID == idinfo.type && datatypes.at(i).classType)
				{
					for(uint32_t k =0; k < datatypes.at(i).memberVariables.size(); k++)
					{
						idInfo id = idInfo(datatypes.at(i).memberVariables.at(k).name, decStatement->scope, datatypes.at(i).memberVariables.at(k).returnType, NULL);
						/*FIX: bad ?! */
						std::shared_ptr<idInfo> parentID(new idInfo());
						*parentID = idinfo;
						id.parent = parentID;
						id.tmpScope = idinfo.tmpScope;
						id.size = idinfo.size;
						id.distributed = idinfo.distributed;
						id.backProp = idinfo.backProp;
						idinfo.recurrent = idinfo.recurrent;
						id.channel = idinfo.channel;
						id.noblock = idinfo.noblock;
						id.array = idinfo.array;
						id.global = decStatement->global;
						id.type = decStatement->type;
						identifiers->push_back(id);
					}
				}
			}
			SS.str("");
			SS.clear();
		}
		
		if(decStatement->global)
		{
			globalDeclarations.push_back(decStatement);
		}
		
		return decStatement;
	};
	
	
	
	EBNF::callstack_t& Parser::activeStack()
	{
		return m_activeStack;
	}

	void Parser::setActiveStack(EBNF::callstack_t& stack)
	{
		m_activeStack = stack;
	}
	
	/*Statement* parseStatement(Statement* output, const std::string starting_rule, parser_callback custom_callback)
	{
		Statement* retStmt = output;
		
		
		int32_t *temp_input_tokens_index_ptr = input_tokens_index;
		int32_t temp_input_tokens_index = 0;
		input_tokens_index = &temp_input_tokens_index;
		Array<token>* output_tokens = new Array<token>();
		int32_t working_tokens_size_before = working_tokens->size();
		
		rule_groups_depth = 0;
	
		ebnfResult result = parseEBNF(working_tokens, starting_rule, output_tokens);
		
		token errorToken = getToken(0);
		
		if(custom_callback != nullptr)
		{
			Array<token>* current_working_tokens = working_tokens;
			working_tokens = output_tokens;
			auto output_size = output_tokens->size();
			retStmt =(prser->*custom_callback)();
			working_tokens = current_working_tokens;
			for(; output_tokens->size() > 0;)
			{
				output_tokens->remove(0);
			}

			for(; output_size > 0;)
			{
				output_size--;
				popToken();
			}
		}
		else
		{
			//retStmt = result.second;
			//FIXME
			retStmt = nullptr;
			for(uint32_t i =0; i < temp_input_tokens_index; i++)
			{
				popToken();
			}
		}

		delete output_tokens;
		input_tokens_index = temp_input_tokens_index_ptr;
		
		if(output == nullptr)
		{
			//displayError(prser->fName, errorToken.lineNumber, errorToken.columnNumber,"Invalid Statement or grammar rule has no callback : " + starting_rule + " at token : " + errorToken.value);
			displayError("prser->fName", errorToken.lineNumber, errorToken.columnNumber,"Invalid Statement or grammar rule has no callback : " + starting_rule + " at token : " + errorToken.value);
		}

		return retStmt;
	};*/
	
	
	

} //namespace DM14::parser
