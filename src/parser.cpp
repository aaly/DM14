/**
@file             parser.cpp
@brief            parser
@details          parser, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
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
	Statement* parser::bad_program()
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
		/*for(uint32_t i =0; i < working_tokens->size(); i++)
		{
			cerr << working_tokens->at(i).value << endl;
		}*/
		exit(1);
		return nullptr;
	}

	//bool EBNF_is_index_frozen = false; /** boolean used by freeze_EBNFindex() and unfreeze_EBNFindex() */
	//int EBNF_frozen_index = -1; /** last index before freezing the EBNF index using freeze_EBNFindex(), to be used for restoration using unfreeze_EBNFindex()*/

	parser::parser(Array<token>* gtokens, const string& filename, const bool insider = true)
	{
		if(gtokens ==NULL || gtokens->size() == 0)
		{
			displayError(filename, 0,0,"Internal parser error !!!, no tokens , maybe empty source file ?");
		}
		
		tokens				=	gtokens;
		fName				=	filename;
		
		functions			=	new Array<ast_function>;
		identifiers			=	new Array<idInfo>;
		functionsInfo		=	new Array<funcInfo>;
		ExternCodes			=	new Array<string>;
		mapCodes 			=	new Array<mapcode>;
		scope				=	0;
		Header				=	false;
		distStatementTemp	=	new distStatement();
		distributedVariablesCount = 0;
		distributedNodesCount = 0;
		tmpScope = false;
		globalNoDist = false;
		
		ebnf.input_tokens = tokens;
		
		dvList				=	new Array<distributingVariablesStatement*>;
		linkLibs			=	new Array<Statement*>();

		this->insider = insider;
		
		distModifiedGlobal	= new Array<idInfo>;
		distStatementTemp	= new distStatement();
		modifiedVariablesList = Array < pair<idInfo,int> >();
		increaseScope(nullptr);
		distributedScope = 0;
		functionStatementsCount = 0;
		
		ebnf.setParserInstance(this);
		
		ebnf.grammar["program"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR,
														{{"function-list", DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseFunction},
														//TODO {"global-Statement",EXPANSION_TOKEN}, add all allowed Statements in the global scope !?
														{"Statement", DM14::EBNF::EXPANSION_TOKEN},
														{"unknown-list", DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::bad_program}}}};
		
		ebnf.grammar["unknown-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{".*",DM14::EBNF::REGEX_TOKEN}}}};
		ebnf.grammar["Statement-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ZERO_MORE ,{{"Statement",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR,{
			{"include-Statement",DM14::EBNF::EXPANSION_TOKEN,&DM14::parser::parseIncludes},
								{"declaration-full-Statement",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseDeclaration},
								{"for-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseForloop}, /** for(from -> to : step) { Statements; } */
								//{"extern-Statement", DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseExtern}, /** extern  { c/c++ code } endextern */
								{"link-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseLink},
								{"struct-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseStruct},
								{"if-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseIf}, /** if [expr] {} else if[] {} else {} */
								{"distribute",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseDistribute},
								{"reset-Statement",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseReset},
								{"setnode-Statement",DM14::EBNF::EXPANSION_TOKEN},
								{"while-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseWhile}, /** while [ cond ] { Statements } */
								//{"case-list",DM14::EBNF::EXPANSION_TOKEN}, /** case [ID/expr] in { 1) ; 2) ; *) ;}   body is like map<condition,Statements> */
								//{"addparent-Statement",DM14::EBNF::EXPANSION_TOKEN},
								{"thread-Statement",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseThread},
								{"function-call-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseFunctionCall},
								{"return-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseReturn},
								{"break-Statement",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseBreak},
								{"continue-Statement",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseContinue},
								{"nop-Statement",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseNOPStatement},
								{"expression-Statement", DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseExpressionStatement},
																	  }}};

		/** the with Statement */
		ebnf.grammar["include-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"with",DM14::EBNF::KEYWORD_TOKEN}, {"include-Statement-body",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["include-Statement-body"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"include-Statement-package",DM14::EBNF::EXPANSION_TOKEN}, {"include-Statement-file",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["include-Statement-package"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"[a-zA-Z0-9]+",DM14::EBNF::REGEX_TOKEN}, {"include-Statement-subpackage",DM14::EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["include-Statement-subpackage"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"use",DM14::EBNF::KEYWORD_TOKEN}, {"[a-zA-Z0-9]+",DM14::EBNF::REGEX_TOKEN}}}};

		ebnf.grammar["include-Statement-file"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"\"[a-zA-Z0-9]+[\.]?[[a-zA-Z0-9]+]?\"",DM14::EBNF::REGEX_TOKEN}}}};
		
		/** the extern Statement */
		ebnf.grammar["extern-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"extern",DM14::EBNF::KEYWORD_TOKEN},
									{"string",DM14::EBNF::DATATYPE_TOKEN},
									{"endextern",DM14::EBNF::KEYWORD_TOKEN}}}};

		/** the link Statement */
		ebnf.grammar["link-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"link-Statement",DM14::EBNF::EXPANSION_TOKEN}, {"slink-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["link-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"link",DM14::EBNF::KEYWORD_TOKEN}, {"string",DM14::EBNF::DATATYPE_TOKEN}}}};
		ebnf.grammar["slink-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"slink",DM14::EBNF::KEYWORD_TOKEN}, {"string",DM14::EBNF::DATATYPE_TOKEN}}}};

		/** the distribute Statement */
		ebnf.grammar["distribute"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"distribute",DM14::EBNF::KEYWORD_TOKEN}, {";",DM14::EBNF::TERMINAL_TOKEN}}}};

		/** the break Statement */
		ebnf.grammar["break-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"break",DM14::EBNF::KEYWORD_TOKEN}, {";",DM14::EBNF::TERMINAL_TOKEN}}}};

		/** the continue Statement */
		ebnf.grammar["continue-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"continue",DM14::EBNF::KEYWORD_TOKEN}, {";",DM14::EBNF::TERMINAL_TOKEN}}}};
													
		/** the reset Statement */
		ebnf.grammar["reset-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"reset",DM14::EBNF::EXPANSION_TOKEN}, {";",DM14::EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["reset"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"reset",DM14::EBNF::KEYWORD_TOKEN}, {"full-expression-list",DM14::EBNF::EXPANSION_TOKEN}}},
						{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"reset",DM14::EBNF::KEYWORD_TOKEN}}}};
		
		/** the setnode Statement */
		ebnf.grammar["setnode-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"setnode-expr",DM14::EBNF::EXPANSION_TOKEN},
															 {";",DM14::EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["setnode-expr"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"setnode",DM14::EBNF::KEYWORD_TOKEN},
												   {"full-expression-list",DM14::EBNF::EXPANSION_TOKEN}}},
						{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"setnode",DM14::EBNF::KEYWORD_TOKEN}}}};
		/** the struct Statement */
		ebnf.grammar["struct-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"struct",DM14::EBNF::KEYWORD_TOKEN},
													{"[a-zA-Z]+([a-zA-Z_0-9])*",DM14::EBNF::REGEX_TOKEN},
													{"struct-body",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["struct-body"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{";",DM14::EBNF::TERMINAL_TOKEN},
															   {"struct-definition",DM14::EBNF::EXPANSION_TOKEN}}}};											
		ebnf.grammar["struct-definition"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"{",DM14::EBNF::TERMINAL_TOKEN},
																{"struct-declaration-list",DM14::EBNF::EXPANSION_TOKEN},
																{"}",DM14::EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["struct-declaration-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ONE_MORE ,{{"declaration-full-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		/** the for loop Statement */
		
		ebnf.grammar["for-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"for",DM14::EBNF::KEYWORD_TOKEN},
													  {"[",DM14::EBNF::TERMINAL_TOKEN},
													  {"loop-expression-declarator",DM14::EBNF::EXPANSION_TOKEN},
													  {"loop-expression-condition",DM14::EBNF::EXPANSION_TOKEN},
													  {"loop-expression-step-list",DM14::EBNF::EXPANSION_TOKEN},
													  {"]",DM14::EBNF::TERMINAL_TOKEN},
													  {"compound-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
																		
		ebnf.grammar["loop-expression-declarator"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"declaration-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseDeclaration},
																		{";",DM14::EBNF::TERMINAL_TOKEN, &DM14::parser::parseNOPStatement}}}};
																		
		ebnf.grammar["loop-expression-condition"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"expression-Statement",DM14::EBNF::EXPANSION_TOKEN,  &DM14::parser::parseExpressionStatement},
																	   {";",DM14::EBNF::TERMINAL_TOKEN, &DM14::parser::parseNOPStatement}}}};
		
		ebnf.grammar["logical-expression-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"logical-expression",DM14::EBNF::EXPANSION_TOKEN},
																	  {";",DM14::EBNF::TERMINAL_TOKEN}}}};
		ebnf.grammar["logical-expression"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"logical-expression-term",DM14::EBNF::EXPANSION_TOKEN},
																 {".*",DM14::EBNF::BINARY_OP_TOKEN},
																 {"logical-expression-term",DM14::EBNF::EXPANSION_TOKEN}}},
									  {DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"logical-expression-term",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["logical-expression-term"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",DM14::EBNF::REGEX_TOKEN},
																	 {".*",DM14::EBNF::IMMEDIATE_TOKEN}}}};

		ebnf.grammar["loop-expression-step-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"expression-Statement",DM14::EBNF::EXPANSION_TOKEN,  &DM14::parser::parseExpressionStatement},
																	   {";",DM14::EBNF::TERMINAL_TOKEN, &DM14::parser::parseNOPStatement}}}};

		/** the if Statement */
		ebnf.grammar["if-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"if",DM14::EBNF::KEYWORD_TOKEN},
													  {"[",DM14::EBNF::TERMINAL_TOKEN},
													  {"full-expression-list",DM14::EBNF::EXPANSION_TOKEN},
													  {"]",DM14::EBNF::TERMINAL_TOKEN},
													  {"compound-Statement",DM14::EBNF::EXPANSION_TOKEN},
													  {"elseif-list",DM14::EBNF::EXPANSION_TOKEN},
													  {"else-list",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["elseif-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ZERO_MORE,{{"elseif-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["elseif-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"else",DM14::EBNF::KEYWORD_TOKEN},
															{"if-list",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["else-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ZERO_MORE,{{"else-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["else-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"else",DM14::EBNF::KEYWORD_TOKEN},
															{"compound-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
		/** the while loop Statement */
		ebnf.grammar["while-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"while",DM14::EBNF::KEYWORD_TOKEN},
													  {"[",DM14::EBNF::TERMINAL_TOKEN},
													  //{"logical-expression",DM14::EBNF::EXPANSION_TOKEN},
													  {"full-expression-list",DM14::EBNF::EXPANSION_TOKEN},
													  {"]",DM14::EBNF::TERMINAL_TOKEN},
													  {"compound-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		/** variable */
		ebnf.grammar["variable"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",DM14::EBNF::REGEX_TOKEN},
													   {"declaration-index-list",DM14::EBNF::EXPANSION_TOKEN}}}};

		/** the variables declaration list */
		ebnf.grammar["declaration-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"declaration-full-Statement",DM14::EBNF::EXPANSION_TOKEN},
															{"declaration-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["declaration-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",DM14::EBNF::REGEX_TOKEN},
															   //{"declaration-dataflow-specifier",DM14::EBNF::EXPANSION_TOKEN},
															   //{"declaration-dist-specifiers-list",DM14::EBNF::EXPANSION_TOKEN},
															   //{"declaration-global-specifier",DM14::EBNF::EXPANSION_TOKEN},
															   {"declaration-specifiers-list", DM14::EBNF::EXPANSION_TOKEN},
															   {"declaration-datatype-list", DM14::EBNF::EXPANSION_TOKEN},
															   {"declaration-index-list", DM14::EBNF::EXPANSION_TOKEN},
															   {"declaration-value-list", DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["declaration-specifiers-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ZERO_MORE, {{"declaration-specifiers", DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["declaration-specifiers"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR, {{"global",DM14::EBNF::KEYWORD_TOKEN},
						{"nodist", DM14::EBNF::KEYWORD_TOKEN},
						{"channel", DM14::EBNF::KEYWORD_TOKEN},
						{"backprop", DM14::EBNF::KEYWORD_TOKEN},
						{"recurrent", DM14::EBNF::KEYWORD_TOKEN},
						{"noblock", DM14::EBNF::KEYWORD_TOKEN}}}};

		ebnf.grammar["declaration-full-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"declaration-Statement",DM14::EBNF::EXPANSION_TOKEN},
									{";",DM14::EBNF::TERMINAL_TOKEN}}}};

		/*ebnf.grammar["declaration-dataflow-specifier"] = {{DM14::EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"backprop",DM14::EBNF::KEYWORD_TOKEN}}}}; // should not be ok with nodist...

		ebnf.grammar["declaration-dist-specifiers-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"channel",DM14::EBNF::KEYWORD_TOKEN}}},
													{DM14::EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"recurrent",DM14::EBNF::KEYWORD_TOKEN}}},
													{DM14::EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"nodist",DM14::EBNF::KEYWORD_TOKEN}}}};

		ebnf.grammar["declaration-global-specifier"] = {{DM14::EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"global",DM14::EBNF::KEYWORD_TOKEN}}}};*/
		
		ebnf.grammar["declaration-datatype-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"declaration-datatype",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["declaration-datatype"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+[a-zA-Z_0-9]*",DM14::EBNF::REGEX_TOKEN}}}};
		

		ebnf.grammar["declaration-index-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"declaration-index",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["declaration-index"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{"matrix-index",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseMatrixIndex},
								 {"array-index",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseArrayIndex}}}};

		
		ebnf.grammar["matrix-index"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"array-index",DM14::EBNF::EXPANSION_TOKEN},
							   {"array-index",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["array-index"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"[",DM14::EBNF::TERMINAL_TOKEN},
							  {"[0-9]+",DM14::EBNF::REGEX_TOKEN},
							  {"]",DM14::EBNF::TERMINAL_TOKEN}}},
				{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"[",DM14::EBNF::TERMINAL_TOKEN},
						  {"variable",DM14::EBNF::EXPANSION_TOKEN},
						  {"]",DM14::EBNF::TERMINAL_TOKEN}}}};

		
		ebnf.grammar["declaration-value-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ONLY_ONE ,{{"declaration-value",DM14::EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["declaration-value"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND , {{"=",DM14::EBNF::TERMINAL_TOKEN}, 
								   {"expression-list",DM14::EBNF::EXPANSION_TOKEN}}}};

		/** the function call Statement */
		
		ebnf.grammar["function-call-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"function-call",DM14::EBNF::EXPANSION_TOKEN}, {";",DM14::EBNF::TERMINAL_TOKEN}}}};
														  
		ebnf.grammar["function-call"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"[a-zA-Z]+([a-zA-Z_0-9])*",DM14::EBNF::REGEX_TOKEN},
																   {"(",DM14::EBNF::TERMINAL_TOKEN},
																   {"function-call-arguments-list",DM14::EBNF::EXPANSION_TOKEN},
																   {")",DM14::EBNF::TERMINAL_TOKEN}}}};
		
		ebnf.grammar["function-call-arguments-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ZERO_MORE,{{"function-call-arguments",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["function-call-arguments"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"full-expression-list",DM14::EBNF::EXPANSION_TOKEN},
																	 {",",DM14::EBNF::TERMINAL_TOKEN}}},
										   {DM14::EBNF::GRAMMAR_TOKEN_AND,{{"full-expression-list",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["expression-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"expression-list",DM14::EBNF::EXPANSION_TOKEN},
														{";",DM14::EBNF::TERMINAL_TOKEN, &DM14::parser::parseExpressionStatement}}}};

		ebnf.grammar["expression-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ONE_MORE ,{{"full-expression-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseExpressionStatement}}}};

		ebnf.grammar["full-expression-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR,{{"big-expression-list",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseExpressionStatement},
														{"expression",DM14::EBNF::EXPANSION_TOKEN, &DM14::parser::parseExpressionStatement}}}};
														
		ebnf.grammar["big-expression-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"expression-types",DM14::EBNF::EXPANSION_TOKEN},
														{"BIN_OP",DM14::EBNF::BINARY_OP_TOKEN},
														{"expression-types",DM14::EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["expression-types"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR,{{"big-expression",DM14::EBNF::EXPANSION_TOKEN},
														{"expression",DM14::EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["big-expression"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"(",DM14::EBNF::OP_TOKEN},
														{"big-expression-list",DM14::EBNF::EXPANSION_TOKEN},
														{")",DM14::EBNF::OP_TOKEN}}}};
		ebnf.grammar["expression"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"(",DM14::EBNF::OP_TOKEN},
														{"expression",DM14::EBNF::EXPANSION_TOKEN},
														{")",DM14::EBNF::OP_TOKEN}}},
								{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"expression-extend",DM14::EBNF::EXPANSION_TOKEN},
														{"BIN_OP",DM14::EBNF::BINARY_OP_TOKEN},
														{"expression-types",DM14::EBNF::EXPANSION_TOKEN}}},
								{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"SING_OP",DM14::EBNF::SINGLE_OP_TOKEN},
														{"expression-extend",DM14::EBNF::EXPANSION_TOKEN}}},
								{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"CORE_OP",DM14::EBNF::CORE_OP_TOKEN},
														{"expression-extend",DM14::EBNF::EXPANSION_TOKEN}}},
								{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"expression-extend",DM14::EBNF::EXPANSION_TOKEN},
														{"SING_OP",DM14::EBNF::SINGLE_OP_TOKEN}}},
								{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"expression-extend",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["expression-extend"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR,{{"function-call",DM14::EBNF::EXPANSION_TOKEN},
															 {"variable",DM14::EBNF::EXPANSION_TOKEN},
															  {"IMMEDIATE",DM14::EBNF::IMMEDIATE_TOKEN}}}};
		/** the thread Statement */

		ebnf.grammar["thread-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"thread-list",DM14::EBNF::EXPANSION_TOKEN},
														 {";",DM14::EBNF::TERMINAL_TOKEN}}}};

		ebnf.grammar["thread-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"thread",DM14::EBNF::KEYWORD_TOKEN},
														 {"function-call",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		/** the function Statement */
		ebnf.grammar["function-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"function-prototype",DM14::EBNF::EXPANSION_TOKEN},
														   {"function-definition-list",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		
		ebnf.grammar["function-definition-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_OR ,{{";",DM14::EBNF::TERMINAL_TOKEN},
																		{"compound-Statement",DM14::EBNF::EXPANSION_TOKEN}}}};
																 
		ebnf.grammar["function-prototype"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+([_]*[0-9]*)*",DM14::EBNF::REGEX_TOKEN},
																{"(",DM14::EBNF::TERMINAL_TOKEN},
																{"function-parameter-list",DM14::EBNF::EXPANSION_TOKEN},
																{"->",DM14::EBNF::TERMINAL_TOKEN},
																{"function-return",DM14::EBNF::EXPANSION_TOKEN},
																{")",DM14::EBNF::TERMINAL_TOKEN}}}};

		ebnf.grammar["function-parameter-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_ZERO_MORE ,{{"function-parameters",DM14::EBNF::EXPANSION_TOKEN}}}};
		ebnf.grammar["function-parameters"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"function-parameter",DM14::EBNF::EXPANSION_TOKEN}}},
										{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"function-extra-parameter",DM14::EBNF::EXPANSION_TOKEN}}}};

		ebnf.grammar["function-extra-parameter"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{",",DM14::EBNF::TERMINAL_TOKEN},
																	  {"function-parameter",DM14::EBNF::EXPANSION_TOKEN}}}};															  
		
		ebnf.grammar["function-parameter"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",DM14::EBNF::REGEX_TOKEN},
																{"declaration-datatype-list",DM14::EBNF::EXPANSION_TOKEN}}}};
		
		ebnf.grammar["function-return"] = {{DM14::EBNF::GRAMMAR_TOKEN_ZERO_MORE ,{{"function-parameter",DM14::EBNF::EXPANSION_TOKEN}}}};
																	
		ebnf.grammar["compound-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND ,{{"{",DM14::EBNF::TERMINAL_TOKEN},
																{"Statement-list",DM14::EBNF::EXPANSION_TOKEN},
																{"}",DM14::EBNF::TERMINAL_TOKEN}}}};
		/** return Statement */
		
		ebnf.grammar["return-list"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{"return",DM14::EBNF::KEYWORD_TOKEN},
														{"expression-Statement",DM14::EBNF::EXPANSION_TOKEN}}},
							   {DM14::EBNF::GRAMMAR_TOKEN_AND,{{"return",DM14::EBNF::KEYWORD_TOKEN},
														{";",DM14::EBNF::TERMINAL_TOKEN}}}};
		/* nop Statement */
		ebnf.grammar["nop-Statement"] = {{DM14::EBNF::GRAMMAR_TOKEN_AND,{{";",DM14::EBNF::TERMINAL_TOKEN}}}};
	};



	parser::~parser()
	{
		delete functions;
		delete identifiers;
		delete	functionsInfo;
		//delete distIdentifiers;
		delete distStatementTemp;
		//delete ExternCodes;
	};

	Statement* parser::parseContinue()
	{
		return new continueStatement();
	}

	Statement* parser::parseBreak()
	{
		return new breakStatement();
	}

	int parser::parse()
	{
		if(!insider)
		{
			parseIncludesInsider("core/DM14GLOBAL.m14", "", includePath::sourceFileType::FILE_DM14);
		}

		while(ebnf.index < tokens->size()-1)
		{	
			if(ebnf.parseEBNF(tokens, "program", &ebnf.tokens_stack).first != DM14::EBNF::ebnfResultType::SUCCESS)
			{
				bad_program();
			}

		}

		// check for the main function // errr no need for file include , the compiler is the one that should that
		if(!Header)
		{
			if(!isUserFunction("main", true))
			{
				displayError(fName, -1, -1, "no main function defined !");
			}
			funcInfo fun = getFunc("main");
			
			if(fun.returnType != "int")
			{
				displayError(fName, -1, -1, "Main function must return int");
			}
		}
			
			
		mapcode mapCode(fName, getFunctions(), getIncludes(), false, distributedNodesCount, distributedVariablesCount);
			
		mapCode.ExternCodes = new Array<string>();
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
	Statement* parser::parseIncludes() 
	{	
		ebnf.popToken();
		ebnf.popToken();
		string package = "";
		string library = "";
		includePath::sourceFileType includeType = includePath::sourceFileType::LIBRARY;
		if(ebnf.getToken().type != "string" && ebnf.getToken().type != "identifier" && !DM14::types::isDataType(ebnf.getToken().value))//tokens->at(ebnf.index).type != "datatype")
		{
			displayError(fName, ebnf.getToken().lineNumber,ebnf.getToken().columnNumber,"Expected Package name and not " + ebnf.getToken().value );
		}

		if(ebnf.getToken().type == "string")
		{
			
			package = ebnf.getToken().value.substr(1,(ebnf.getToken().value.size() - 2 ));
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
			package = ebnf.getToken().value;
			includeType = includePath::sourceFileType::LIBRARY;
			ebnf.popToken();
			if(ebnf.getToken().value == "use")
			{
				RequireValue("use", "Expected \"Use\" and not ", true);
				ebnf.popToken();
				
				if(ebnf.getToken().value != "*" && ebnf.getToken().type != "identifier" && !DM14::types::isDataType(ebnf.getToken().value))// tokens->at(ebnf.index).type != "datatype")
				{
					displayError(fName, ebnf.getToken().lineNumber,ebnf.getToken().columnNumber,"Expected Package name");	
				}
				
				if(ebnf.getToken().value == "*" )
				{
					library = package;
				}
				else
				{
					library = ebnf.getToken().value;
				}
				
				ebnf.popToken();
			}
			else
			{
				library = package;
			}
		}
		
		parseIncludesInsider(package, library, includeType);
			
		return NULL;
	};

	int parser::addIncludePath(string path)
	{
		includePaths.push_back(path);
		return includePaths.size();
	}

	int parser::parseIncludesInsider(const string& package, const string& library, const includePath::sourceFileType includeType)
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
			parser Parser(Scanner.getTokens(),package);
			for(uint32_t i = 0; i < includePaths.size(); i++)
			{
				Parser.addIncludePath(includePaths.at(i));
			}
			Parser.Header = true;
			Parser.parse();
			
			for(uint32_t i =0; i < Parser.getMapCodes()->size(); i++)
			{
				Parser.getMapCodes()->at(i).setHeader(true);
				Parser.getMapCodes()->at(i).ExternCodes = Parser.ExternCodes;
				
				/*for(unsigned k =0; k < Parser.getMapCodes()->at(i).globalDefinitions.size(); k++)
				{
					globalDefinitions.push_back(Parser.getMapCodes()->at(i).globalDefinitions.at(k));
				}*/
				
				
				for(const auto& identifier : *(Parser.identifiers))
				{
					if(identifier.global)
					{
						identifiers->push_back(identifier);
					}
				}

				/*for(const auto& globalDeclaration : Parser.globalDeclarations)
				{
					//globalDeclarations.push_back(globalDeclaration);
					for(const auto& identifier : globalDeclaration)
					{
						indetifiers->push_back(identifier)
					}
				}*/

				//Parser.getMapCodes()->at(i).globalDeclarations = Array<Statement*>();
				mapCodes->push_back(Parser.getMapCodes()->at(i));
			//mapCodes->at(mapCodes->size()-1).Print();
			}
			
			for(uint32_t i =0; i< Parser.linkLibs->size(); i++)
			{
				linkLibs->push_back(Parser.linkLibs->at(i));
			}
		
			for(uint32_t i =0; i< Parser.getIncludes().size(); i++)
			{
				bool push = true;
				
				for(uint32_t k =0; k< includes.size(); k++)
				{
					if(Parser.getIncludes().at(i).package == includes.at(k).package &&
						Parser.getIncludes().at(i).library == includes.at(k).library)
					{
						push = false;
					}
				}
				
				if(push)
				{
					includes.push_back(Parser.getIncludes().at(i));
				}
			}
			
			// push new stuff

			for(uint32_t i =0; i< Parser.functionsInfo->size(); i++)
			{
				functionsInfo->push_back(Parser.functionsInfo->at(i));
			}
			
			
			scope += Parser.scope;
		}
		else
		{
			mapFunctions(package, library);
		}
		
		return 0;
	}

	Statement* parser::parseLink()
	{
		Link* stmt = new Link();
		
		ebnf.popToken();
		if(ebnf.getToken().value == "slink")
		{
			stmt->Static = true;
		}
		
		ebnf.popToken();
			
		if(ebnf.getToken().type != "string")
		{
			RequireType("string", "expected library identifier and not : ", true);
		}
		
		if(ebnf.getToken().value.size() > 2)
		{
			stmt->libs = ebnf.getToken().value.substr(1, ebnf.getToken().value.size()-2);
		}
		
		linkLibs->push_back(stmt);
		
		return stmt;
	}


	Statement* parser::parseReturn()
	{
		ebnf.popToken();
		returnStatement* Statement = new returnStatement();
		Statement->line = ebnf.getToken().lineNumber;
		Statement->scope = scope;
		
		if(!peekToken(";"))
		{
			//returnStatement->retValue = parseOpStatement(0, reachToken(";", false, true, true, true, false)-1, currentFunction.returnIDType, 0, returnStatement);
			Statement->retValue = parseStatement("Statement");
		}
		else
		{
			ebnf.popToken();
			RequireValue(";","expected ; and not:", true);
		}

		return Statement;
	}

	Statement* parser::parseStatement(const std::string starting_rule, EBNF::parser_callback custom_callback)
	{
		Statement* retStmt = NULL;
		increaseScope(retStmt);
			
		int *temp_input_tokens_index_ptr = ebnf.input_tokens_index;
		int temp_input_tokens_index = 0;
		ebnf.input_tokens_index = &temp_input_tokens_index;
		Array<token>* output_tokens = new Array<token>();
		int working_tokens_size_before = ebnf.working_tokens->size();
		
		DM14::EBNF::ebnfResult result = ebnf.parseEBNF(ebnf.working_tokens, starting_rule, output_tokens, custom_callback != nullptr ? true : false);
		
		token errorToken = ebnf.getToken(0);
		
		if(custom_callback != nullptr)
		{
			Array<token>* current_working_tokens = ebnf.working_tokens;
			ebnf.working_tokens = output_tokens;
			auto output_size = output_tokens->size();
			retStmt =(this->*custom_callback)();
			ebnf.working_tokens = current_working_tokens;
			for(; output_tokens->size() > 0;)
			{
				output_tokens->remove(0);
			}

			for(; output_size > 0;)
			{
				output_size--;
				ebnf.popToken();
			}
		}
		else
		{
			retStmt = result.second;
			for(uint32_t i =0; i < temp_input_tokens_index; i++)
			{
				ebnf.popToken();
			}
		}

		delete output_tokens;
		ebnf.input_tokens_index = temp_input_tokens_index_ptr;

		if(retStmt == nullptr)
		{
			displayError(fName, errorToken.lineNumber, errorToken.columnNumber,"Invalid Statement or grammar rule has no callback : " + starting_rule + " at token : " + errorToken.value);
		}
				
		decreaseScope();
		return retStmt;
	};

	Statement* parser::parseNOPStatement()
	{
		ebnf.popToken();
		NOPStatement* result = new NOPStatement;
		result->line = ebnf.getToken().lineNumber;
		result->scope = scope;
		result->scopeLevel = scope;
		
		return result;
	};

	Statement* parser::parseMatrixIndex()
	{
		//@TODO: need to handle 2nd or multiple diemnsions , have to implement in the AST first then parseDeclraration and ParseOpStatement...
		ebnf.popToken();
		RequireValue("[", "Expected [ and not : ", true);
		int from = 0;
		int to = reachToken("]", false, true, false, true, true)-1;
		Statement* result = parseOpStatement(from, to, "-2", 0, currentStatement);
		result->line = ebnf.getToken().lineNumber;
		ebnf.popToken();
		return result;
	}

	Statement* parser::parseArrayIndex()
	{
		ebnf.popToken();
		RequireValue("[", "Expected [ and not : ", true);
		int from = 0;
		int to = reachToken("]", false, true, false, true, true)-1;
		Statement* result = parseOpStatement(from, to, "-2", 0, currentStatement);
		result->line = ebnf.getToken().lineNumber;
		ebnf.popToken();
		return result;
	};


	Statement* parser::parseAddParent()
	{
		parentAddStatement* ps = new parentAddStatement();
		nextIndex();
		
		if((tokens->at(ebnf.index)).value == "(")
		{
			int cindex = ebnf.index;
			ps->ip = parseOpStatement(cindex, reachToken(")", false, true, true, true, true ), "string", 0, ps);
		}
		else
		{
			ps->ip = parseOpStatement(ebnf.index, ebnf.index, "string", 0, ps);
		}
		
		nextIndex();
		if((tokens->at(ebnf.index)).value == "(")
		{
			int cindex = ebnf.index;
			ps->socket = parseOpStatement(cindex, reachToken(")", false, true, true, true, true ), "int", 0, ps);
		}
		else
		{
			ps->socket = parseOpStatement(ebnf.index, ebnf.index, "int", 0, ps);
		}
		return ps;
	};

	Statement* parser::parseSetNode()
	{
		setNodeStatement* sn = new setNodeStatement();
		//sn->node = 
		return sn;
	};

	Statement* parser::parseReset()
	{
		resetStatement* rs = new resetStatement();
		ebnf.popToken(); // reset
		if(!peekToken(";"))
		{
			ebnf.popToken();
			int cindex = ebnf.index;
			rs->count = parseOpStatement(cindex,(reachToken(";", true, true, true, false, false) - 1), getType(ebnf.index-1), 0, rs);
		}
		return rs;
	};

	Statement* parser::parseDistribute()
	{
		currentFunction.distributed = true;
		
		stringstream SS;
		
		SS << ebnf.getToken().lineNumber;
		currentFunction.functionNodes.push_back(currentFunction.name+SS.str());
		SS.str("");
		
		//distStatement* distStatement = new distStatement();
		distStatementTemp->line = ebnf.getToken().lineNumber;
		distStatementTemp->scope = scope;
		//*distStatement->variables = *distIdentifiers;
		//fix105
		distributedNodesCount++;
		distStatement* stmt = distStatementTemp;
		distStatementTemp = new distStatement();
		distributedScope++;
		return stmt;
	};

	/*
	Statement* parser::parseTempDeclaration()
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

	int parser::pushDependency(idInfo& id)
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
		for(uint32_t i =0; i<currentStatement->distStatements.size(); i++)
		{
			string listParentName;
			
			if(currentStatement->distStatements.at(i)->variable.parent)
			{
				listParentName = currentStatement->distStatements.at(i)->variable.parent->name;
			}
			
			if(currentStatement->distStatements.at(i)->variable.name == id.name &&
				listParentName== parentName &&
				currentStatement->distStatements.at(i)->variable.scope <= id.scope)
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
				id2.parent = &id;
				pushDependency(id2);
			}
			
			return 0;
		}
		
		distributingVariablesStatement* dvStatement = new distributingVariablesStatement();
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
		currentStatement->distStatements.push_back(dvStatement);
		dvList->push_back(dvStatement);
		distStatementTemp->dependenciesVariables->push_back(id);
		return 0;
	};



	idInfo* parser::getTopParent(idInfo* id)
	{
		if(!id->parent)
		{
			return id;
		}
		return getTopParent(id->parent);
	};

	int parser::pushModified(const string& op, idInfo& id)
	{
		if(globalNoDist)
		{
			return 1;
		}
		
		
		if(id.name == currentFunction.returnID || getTopParent(&id)->name == currentFunction.returnID)
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
			
			distributingVariablesStatement* dvStatement = new distributingVariablesStatement();
			dvStatement->variable = id;
			dvStatement->type = distributingVariablesStatement::MODS;
			
			currentStatement->distStatements.push_back(dvStatement);
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


	int parser::pushModifiedGlobal(idInfo& id)
	{
		//if exists, remove it and out the newest/ just to save memory ?
		distModifiedGlobal->push_back(id);
		return 0;
	};

	Statement* parser::parseFunctionCall()
	{
		Statement* result = parseFunctionCallInternal(true, "", "");
		return result;
	}

	Statement* parser::parseFunctionCallInternal(bool terminated,const string& returnType, const string& classID)
	{
		ebnf.popToken();
		functionCall* funcCall = new functionCall; // for every comma , call parseOP
		funcCall->line = ebnf.getToken().lineNumber;
		funcCall->scope = scope;
		funcCall->name = ebnf.getToken().value;
		
		
		if(isUserFunction(ebnf.getToken().value, true))
		{
			funcCall->functionType = DM14::types::types::USERFUNCTION;
		}
		else
		{
			funcCall->functionType = DM14::types::types::BUILTINFUNCTION;
		}
		
		ebnf.popToken();
		RequireValue("(", "Expected \"(\" and not ", true);
		
		/*loop through parameters*/
		Array<string>* parameters = new Array<string>();
		if(peekToken(")"))
		{
			ebnf.popToken();
		}
		else /** we have parameters ! */
		{
			currentStatement = funcCall;
			while(!peekToken(")"))
			{
				Statement* parameter = parseStatement("full-expression-list");
				funcCall->absorbDistStatements(parameter);
				funcCall->parameters->push_back(parameter);
				parameters->push_back(parameter->type);
				if(peekToken(","))
				{
					ebnf.popToken();
				}
			}

			ebnf.popToken();
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
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"1Parameters error for function call : " + funcCall->name);
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
					
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"2Parameters error for function call : " + funcCall->name);
				}
				else
				{
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"return type error [" + returnType + "] for function call : [" + funcCall->name + "] expected : " + getFunc(funcCall->name, classID).returnType);
				}
			}
			funcCall->type = getFunc(funcCall->name, parameters, returnType, classID).returnType;
		}
		
		if(terminated)
		{
			ebnf.popToken();
			RequireValue(";", "Expected ; and not ", true);
		}
		
		return funcCall;
	};

	Statement* parser::parseForloop()
	{
		ebnf.popToken();
		
		// for [decl;cond;stmt] {stmts}
		forloop* floop = new forloop;
		floop->line = ebnf.getToken().lineNumber;
		floop->scope = scope;
		
		increaseScope(floop);
		
		tmpScope = true; 
		
		ebnf.popToken();
		RequireValue("[", "Expected [ and not ", true);

		if(!peekToken(";"))
		{
			//Statement* stmt =  parseDeclarationInternal(";");
			Statement* stmt =  parseStatement("loop-expression-declarator");
			cerr << "declarator :" << stmt << endl;
			stmt->line = ebnf.getToken().lineNumber;
			if(stmt->StatementType != dStatement && stmt->StatementType != eStatement)
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber, "Expected declaration Statement");
			}
			
			if(stmt->StatementType == dStatement)
			{
				floop->fromCondition->push_back(stmt);
			}
		
			addStatementDistributingVariables(stmt);
		}
		
		if(!peekToken(";"))
		{
			Statement* stmt= parseStatement("loop-expression-condition");
			stmt->line = ebnf.getToken().lineNumber;
			if(stmt->StatementType != oStatement && stmt->StatementType != eStatement)
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber, "Expected conditional Statement");
			}
			
			if(stmt->StatementType == oStatement)
			{
				floop->toCondition->push_back(stmt);
			}
			
			addStatementDistributingVariables(stmt);
		}

		
		
		if(!peekToken("]"))
		{
			Statement* stmt= parseStatement("loop-expression-step-list");
			stmt->line = ebnf.getToken().lineNumber;
			if(stmt->StatementType != oStatement && stmt->StatementType != eStatement)
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber, "Expected operational Statement");
			}
			
			if(stmt->StatementType == oStatement)
			{
				floop->stepCondition->push_back(stmt);
			}
			
			addStatementDistributingVariables(stmt);
		}
		
		
		ebnf.popToken();
		RequireValue("]", "Expected ] and not ", true);
		
		tmpScope = false;
		ebnf.popToken();
		RequireValue("{", "Expected { and not ", true);
		

		while(!peekToken("}"))
		{
			Statement* stmt = parseStatement("Statement");
			addStatementDistributingVariables(stmt);
			floop->body->push_back(stmt);
		}
		
		ebnf.popToken();
		decreaseScope();
			
		return floop;
	};

	int parser::addStatementDistributingVariables(Statement* stmt)
	{
		
		for(uint32_t i=0; i < currentFunction.body->appendBeforeList.size(); i++)
		{
			//if(((dddd*)currentFunction.body->appendAfterList.at(i)).at(0).arrayIndex != NULL)
			if((termStatement*)((distributingVariablesStatement*)currentFunction.body->appendBeforeList.at(i))->variable.arrayIndex != NULL)
			{
				cerr << currentFunction.body->appendBeforeList.at(i) << endl << flush;
				stmt->distStatements.push_back((distributingVariablesStatement* )currentFunction.body->appendBeforeList.at(i));
				//currentFunction.body->append_after(dvStatement);
				currentFunction.body->appendBeforeList.erase(currentFunction.body->appendBeforeList.begin()+i);
				i--;
			}
		}
		for(uint32_t i=0; i < currentFunction.body->appendAfterList.size(); i++)
		{
			//if(((dddd*)currentFunction.body->appendAfterList.at(i)).at(0).arrayIndex != NULL)
			if((termStatement*)((distributingVariablesStatement*)currentFunction.body->appendAfterList.at(i))->variable.arrayIndex != NULL)
			{
				stmt->distStatements.push_back((distributingVariablesStatement* )currentFunction.body->appendAfterList.at(i));
				//currentFunction.body->append_after(dvStatement);
				currentFunction.body->appendAfterList.erase(currentFunction.body->appendAfterList.begin()+i);
				i--;
			}
		}
		
		return 0;
	}

	Statement* parser::parseFunction() // add functions prototypes to userFunctions Array too :)
	{
		// always keep global ids
		Array<idInfo>* tmpIdentifiers = identifiers;
		identifiers			= new Array<idInfo>;
		globalNoDist = true;
		for(uint32_t i =0; i < tmpIdentifiers->size(); i++)
		{
			if(tmpIdentifiers->at(i).global)
			{
				identifiers->push_back(tmpIdentifiers->at(i));
			}
		}
		delete tmpIdentifiers;

		
		ebnf.popToken();
		RequireType("identifier", "Invalid function declaration", false);
		
		funcInfo Funcinfo;
		Funcinfo.protoType = false;
		ast_function Func;
		currentFunction.scope = scope;
		
		////checkToken(USERFUNCTION, "can not re-define a builtin function : ", true);
		////checkToken(BUILTINFUNCTION, "Predefined function : ", true);
		
		Funcinfo.name = ebnf.getToken().value;
		Funcinfo.type = DM14::types::types::USERFUNCTION;
		
		currentFunction.name = ebnf.getToken().value;
			
		ebnf.popToken();
		RequireValue("(", "Expected \"(\" and not "+ ebnf.getToken().value + " after function definition ", false);

		if(!peekToken("->"))
		{
			while(ebnf.getToken().value != "->")
			{
				//TODO: FIX 102
				// should make a Array of declaration Statements and add the Statements to it, for the compiler to parse them
				// with their initialized values ?
				//declareStatement* stmt =(declareStatement*)parseDeclarationInternal("->");
				//declareStatement* stmt =(declareStatement*)parseStatement("Statement");
				
				declareStatement* stmt =(declareStatement*)parseStatement("declaration-Statement", &parser::parseDeclarationInternal);
				
				stmt->line = ebnf.getToken().lineNumber;
				for(uint32_t i = 0; i < stmt->identifiers->size(); i++ )
				{
					idInfo Id;
					Id.name =(stmt->identifiers->at(i)).name;
					Id.parent =(stmt->identifiers->at(i)).parent;
					Id.type =(stmt->identifiers->at(i)).type;
					Id.type = stmt->type;
					Id.scope = stmt->scope;
					Id.index = ebnf.index;
					currentFunction.parameters->push_back(Id);
					Funcinfo.parameters->push_back(pair<string,bool>(Id.type,stmt->value));
				}
				
				if(peekToken(","))
				{
					ebnf.popToken();
				}
				else if(peekToken("->"))
				{
					ebnf.popToken();
				}
			}
		}
		else
		{
			ebnf.popToken(); // ->
		}
		
		RequireValue("->", "Expected \"->\" and not "+ebnf.getToken().value + " after function definition ", false);
		
		if(peekToken(")"))
		{
			ebnf.popToken();
			currentFunction.returnIDType = "NIL";
			currentFunction.returnID = "NIL";
		}
		else
		{	
			while(ebnf.getToken().value != ")")
			{
				//declareStatement* stmt =(declareStatement*)parseDeclarationInternal();
				declareStatement* stmt =(declareStatement*)parseStatement("declaration-Statement", &parser::parseDeclarationInternal);
				ebnf.popToken();
				RequireValue(")", "Expected ) and not "+ebnf.getToken().value + " after function definition ", false);
				stmt->line = ebnf.getToken().lineNumber;
				for(uint32_t i = 0; i < stmt->identifiers->size(); i++ )
				{
					stmt->line = ebnf.getToken().lineNumber;
					currentFunction.returnIDType = stmt->type;
					Funcinfo.returnType = stmt->type;
					currentFunction.returnID = stmt->identifiers->at(0).name;
					if(stmt->identifiers->size() > 1)
					{
						displayWarning(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Function : " + Funcinfo.name + " : more than one variable for return, only the first will be used");
						break;
					}
				}
			}
		}
		
		globalNoDist = false;
		// continue to body !
		ebnf.popToken();
		if(ebnf.getToken().value == ";")
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
								displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Pre-defined function prototype : " +Funcinfo.name);
							}
							else // if(function prototype already exists)
							{
								displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Pre-defined function, no need for prototype : "+Funcinfo.name);
							}
						}
					}
				}
			}
			functionsInfo->push_back(Funcinfo);
		}
		else if(ebnf.getToken().value == "{")
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
							displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Pre-defined function : "+Funcinfo.name);
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
					
			Array<Statement*>* declarations = new Array<Statement*>();
			
			//while(ebnf.popToken().value != "}")
			while(!peekToken("}"))
			{
				Statement* stmt = parseStatement("Statement");

				if(stmt == NULL)
				{
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"error parsing function : "+Funcinfo.name);
				}

				if(stmt->StatementType == dStatement)
				{
					if(((declareStatement*)stmt)->Initilazed && !((declareStatement*)stmt)->global)
					{
						declareStatement* decStatement =((declareStatement*)stmt);
						Statement* value = decStatement->value;
						decStatement->value = NULL;
						//FIX1001 if function is distributed , else put it in pushModified(os->op, id); currentFunction.body
						declarations->push_back(decStatement);
						//functionStatementsCount++;
						for(uint32_t i=0 ; i < decStatement->identifiers->size(); i++)
						{
							//fix add type and scope ....
							operationalStatement* os = new operationalStatement();
							os->left = new termStatement(decStatement->identifiers->at(i).name);
							os->op = "=";
							os->right = value;
							os->type = stmt->type;
							os->scope =((declareStatement*)decStatement)->scope;

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
						
						Statement* stmt2 = currentFunction.body->at(currentFunction.body->size()-(decStatement->identifiers->size()));
						stmt2->distStatements = stmt->distStatements;
						stmt->distStatements.clear();
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

			termStatement* statement = new termStatement("DM14FUNCTIONBEGIN", "NIL" );
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

			ebnf.popToken();
			RequireValue("}", "Expected } and not : ", true);
		}
		else
		{
			RequireValue("{", "Expected { or ; and not : ", true);
		}
		

		currentFunction = ast_function();
		decreaseScope();
		
		distModifiedGlobal	= new Array<idInfo>;
		//distIdentifiers	= new Array<idInfo>;
		distStatementTemp	= new distStatement();
		modifiedVariablesList = Array < pair<idInfo,int> >();
		//FIX only increment on if conditions, loops and case... ?
		increaseScope(NULL);
		distributedScope = 0;
		functionStatementsCount = 0;
		return NULL;
	};

	int parser::nextIndex()
	{
		if((unsigned) ebnf.index ==(tokens->size() - 1))
		{
			displayError(fName,(tokens->at(ebnf.index)).lineNumber,(tokens->at(ebnf.index)).columnNumber,"Unexpected EOF");
		}
		ebnf.index++;
		return ebnf.index;
	};

	bool parser::isIdentifier(const string& ID)
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




	int parser::findIDInfo(const idInfo& ID, const int& returnType)
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

	string parser::findIDType(const idInfo& ID, const string& classID)
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


	bool parser::isFunction(const string& func, bool recursiveCurrent,const string& classID )
	{
		if(isUserFunction(func, recursiveCurrent, classID) || isBuiltinFunction(func))
		{
			return true;
		}
		
		return false;
	};

	bool parser::isBuiltinFunction(const string& func)
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


	bool parser::isUserFunction(const string& func, bool recursiveCurrent,const string& classID)
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
	Statement* parser::findTreeNode(Statement* opStatement, int StatementType) 
	{
		if(!opStatement)
		{
			return NULL;
		}
		
		if(opStatement->StatementType == StatementType)
		{
			return opStatement;
		}
		
		Statement* result = NULL;
		
		if(opStatement->StatementType == oStatement)
		{
			if(((operationalStatement*)opStatement)->left)
			{
				result = findTreeNode(((operationalStatement*)opStatement)->left, StatementType);
			}
				
			if(!result &&((operationalStatement*)opStatement)->right)
			{
				result = findTreeNode(((operationalStatement*)opStatement)->right, StatementType);
			}
		}
		
		return result;
	}



	int parser::searchVariables(Statement* opStatement, int depencyType, string op)
	{
		
		//distributingVariablesStatement::DEPS
		if(!opStatement)
		{
			return 1;
		}
		
		if(opStatement->StatementType == tStatement)
		{
			idInfo* id =((termStatement*)opStatement)->id;
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
			if(((operationalStatement*)opStatement)->left)
			{
				if(((operationalStatement*)opStatement)->op == "." ||((operationalStatement*)opStatement)->op == "::")
				{
					if(((operationalStatement*)opStatement)->left->StatementType == oStatement)
					{
						Statement* leftLeft =((operationalStatement*)((operationalStatement*)opStatement)->left)->left;
						searchVariables(leftLeft, depencyType, op);
					}
				}
				else
				{
					searchVariables(((operationalStatement*)opStatement)->left, depencyType, op);
				}
			}
				
			if(((operationalStatement*)opStatement)->right)
			{
				searchVariables(((operationalStatement*)opStatement)->right, depencyType, op);
			}
		}
		
		return 0;
	}


	Statement* parser::parseExpressionStatement()
	{
		Statement* result = nullptr;
		
		if(ebnf.working_tokens->at(ebnf.working_tokens->size()-1).value == ";")
		{
			result = parseOpStatement(0, ebnf.working_tokens->size()-2, "-2", scope, parentStatement);
		}
		else
		{
			result = parseOpStatement(0, ebnf.working_tokens->size()-1, "-2", scope, parentStatement);
		}
		return result;
	}


	std::string parser::getOpStatementType(std::string stmtType, const std::string& classID)
	{
		std::string type;

		if(stmtType == "NIL")
		{
			displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Wrong type variable : " + ebnf.getToken().value);
		}
		else if(stmtType == "-2")
		{
			if(isFunction(ebnf.getToken(0).value, true, classID)) /// function, set to it's return type
			{
				type  = getFunc(ebnf.getToken(0).value, classID).returnType;
			}
			else if(DM14::types::isDataType(ebnf.getToken(0).value)) /// if it is  datatype, use it as the Statement type
			{
				type  = ebnf.getToken(0).value;
			}		
			else if(ebnf.getToken(0).type == "identifier") // if identifier, then use it's type
			{
				type  = findIDType(idInfo(ebnf.getToken(0).value, 0, "", NULL), classID);
			}
			else if(DM14::types::isImmediate(ebnf.getToken(0)) ) /// use immediate type
			{
				type  = ebnf.getToken(0).type;
			}
			/*else if((DM14::types::isSingleOperator(ebnf.getToken(0).value) || ebnf.getToken(0).value == "@") && 0 < to)
			{
				if(isImmediate(ebnf.getToken(0+1)))
				{
					type  = ebnf.getToken(0+1).type;
				}
				else if(ebnf.getToken(0+1).type == "identifier" )
				{
					type  = findIDType(idInfo(ebnf.getToken(0+1).value, 0, "", NULL), classID);
				}
				else
				{
					displayError(fName, ebnf.getToken(0).lineNumber,ebnf.getToken(0).columnNumber,"Did not expect "+ ebnf.getToken(0).value);
				}
			}
			else
			{
				displayError(fName, ebnf.getToken(0).lineNumber, ebnf.getToken(0).columnNumber,"Did not expect "+ ebnf.getToken(0).value);
			}*/
		}
		else
		{
			type  = stmtType;
		}

		return type;
	}

	bool parser::restore(std::vector<token>* extract_temp_vector)
	{
		bool result = true;

		for(uint32_t i =0; i < extract_temp_vector->size(); i++)
		{
			ebnf.pushToken(extract_temp_vector->at(i));
		}
		delete extract_temp_vector;
		return result;
	}

	std::vector<token>* parser::extract(int32_t from, int32_t to)
	{
		std::vector<token>*  extract_temp_vector = new std::vector<token>();
		for(uint32_t i = 0; i < from; i++)
		{
			extract_temp_vector->push_back(ebnf.popToken());
		}

		for(uint32_t i = to+1, current_size = ebnf.working_tokens->size(); i < current_size; i++)
		{
			extract_temp_vector->push_back(ebnf.popToken(to+1));
		}
		return extract_temp_vector;
	}

	Statement* parser::parseOpStatement(int32_t from, int32_t to, 
										const string& stmtType, const int& scopeLevel, Statement* caller, 
										idInfo* parent, const string& parentOp)
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
			if(ebnf.getToken(i).value == "(" )
			{
				++plevel;
			}
			else if(ebnf.getToken(i).value == ")" )
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
			displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber," missing \")\" ");
		}
		else if(plevel < 0 )
		{
			displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber," missing \"(\" ");
		}
		
		// big(..) Statement
		if(ebnf.getToken(from).value == "("  && ebnf.getToken(to).value == ")" && removeBigParenthes)
		{
			ebnf.popToken();
			ebnf.working_tokens->remove(ebnf.working_tokens->size()-1);
			return parseOpStatement(0, to-2, stmtType, scopeLevel+1, caller);
		}
		// ]
		
		int origiIndex = ebnf.index;
		bool classMember = false;

		operationalStatement* opStatement = new operationalStatement;
		opStatement->line = ebnf.getToken().lineNumber;
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
			if(ebnf.getToken(i).value == "(")
			{
				plevel++;
				continue;
			}
			else if(ebnf.getToken(i).value == ")" )
			{
				plevel--;
				continue;
			}
			
			if(plevel == 0)
			{				
				if((ebnf.getToken(i).type == "operator") &&
					!(ebnf.getToken(i).value == ")" || ebnf.getToken(i).value == "(" || ebnf.getToken(i).value == "["  || ebnf.getToken(i).value == "]"))
				{
					opStatement->op = ebnf.getToken(i).value;
					if(i != from) /** not a prefix operator like ++x */
					{
						auto* temp = extract(from, i-1);
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
					
					ebnf.popToken(); /** pop the op token */
			
					idInfo* id = NULL;
					// should we loop inside to get the term ?
					
					Statement* res = findTreeNode(opStatement->left, tStatement);
					
					if(res)
					{
						id  =((termStatement*)res)->id;
					}

					if(opStatement->op == "." ||  opStatement->op == "::" )
					{
						if(!id)
						{
							displayError(fName, ebnf.getToken(i).lineNumber, ebnf.getToken(i).columnNumber,"false class member !");
							
						}
						else
						{
							if(i == to && to > 0)
							{
								displayError(fName, ebnf.getToken(i).lineNumber, ebnf.getToken(i).columnNumber,"incomplete data member access at " +  ebnf.getToken(i).value);
							}

							opStatement->right = parseOpStatement(from, to, "-2", opStatement->scopeLevel, currentStatement, id);
							opStatement->left->type = opStatement->right->type;
							opStatement->type = opStatement->right->type;

							opStatement->absorbDistStatements(opStatement->right);
							
							
							string classID = id->name;
							
							//TODO: make sure :: is used onl with static enumns/variables ?
							if(DM14::types::isDataType(classID) && opStatement->op != "::")
							{
								Statement* staticFunction = findTreeNode(opStatement->right, fCall);
								if(staticFunction != nullptr)
								{	
									
									auto fInfoResult = DM14::types::getClassMemberFunction(classID, (*(functionCall*)staticFunction));
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
										displayError(fName, ebnf.getToken(i).lineNumber, ebnf.getToken(i).columnNumber,"class " + classID + " has no member static function " 
 + (*(functionCall*)staticFunction).name);
									}
								}
								else
								{
									Statement* staticVariable = findTreeNode(opStatement->right, tStatement);
									if(staticVariable != nullptr)
									{
										auto vInfoResult = DM14::types::getClassMemberFunction(classID, (*(functionCall*)staticVariable));
										if(vInfoResult.first == true)
										{
											
											if(vInfoResult.second.noAutism == true)
											{
												opStatement->op = "::";
											}
										}
										else
										{
											displayError(fName, ebnf.getToken(i).lineNumber, ebnf.getToken(i).columnNumber,"class " + classID + " has no member static variable " + (*(functionCall*)staticVariable).name);
										}
									}
									else
									{
										displayError(fName, ebnf.getToken(i).lineNumber, ebnf.getToken(i).columnNumber,"parsing error, should not be here !");
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
							displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Missing right operand");
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
							displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"type \"" +opStatement->type + "\" does not support operator " + opStatement->op);
						}
						
						searchVariables(opStatement->left, distributingVariablesStatement::MODS, opStatement->op);
						searchVariables(opStatement->right, distributingVariablesStatement::DEPS);
					}
					else if(DM14::types::isSingleOperator(opStatement->op))
					{
						if(!DM14::types::typeHasOperator(opStatement->op, opStatement->type) &&(opStatement->op != "@") )
						{
							displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"type \"" +opStatement->type + "\" does not support operator " + opStatement->op);
						}
						searchVariables(opStatement->left, distributingVariablesStatement::MODS, opStatement->op);
					}
					else if(DM14::types::isCoreOperator(opStatement->op))
					{
						if(opStatement->op == "@")
						{
							//construct node address Statement
							// should we loop inside to get the term ?
							Statement* res = findTreeNode(opStatement->right, tStatement);
					
							idInfo* rightId = NULL;
								
							if(res)
							{
								rightId  =((termStatement*)res)->id;
								int isDistributed = findIDInfo(*rightId, DISTRIBUTED);
								if(!isDistributed)
								{
									displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,ebnf.getToken().value + " : can not use @ operator with a non-distributed variable");
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
									operationalStatement* stmt =(operationalStatement*) opStatement->right;
									searchVariables(opStatement->left, distributingVariablesStatement::MODS, stmt->op);
									searchVariables(opStatement->left, distributingVariablesStatement::DEPS);
									opStatement->op  = stmt->op;
									stmt->op = "";
									delete stmt->left;
									stmt->left = NULL;
								}				
								else if(opStatement->right->StatementType == tStatement)
								{
									delete opStatement->right;
									opStatement->right = NULL;
									opStatement->op  = "";
								}					

								if((parentOp != "=" && parentOp != "+=" && parentOp != "-=" && parentOp != "*=" && parentOp != "/=" && parentOp != "++" && parentOp != "--" && parentOp != "@") || opStatement->op == "")
								{
									idInfo* leftId = NULL;
									// should we loop inside to get the term ?
									Statement* res = findTreeNode(opStatement->left, tStatement);
						
									if(res)
									{
										leftId  =((termStatement*)res)->id;
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
								for(uint32_t l =0; l < currentStatement->distStatements.size(); l++)
								{
									if(currentStatement->distStatements.at(l)->type == distributingVariablesStatement::DEPS)
									{
										 if(currentStatement->distStatements.at(l)->variable.name == rightId->name)
										 {
											 currentStatement->distStatements.remove(l);
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
		


		if(ebnf.getToken(0).value.size() == 0)// consumed all tokens !
		{
			cerr << "from :" << from << " to:" << to << endl;
			displayError(fName, ebnf.getToken(0).lineNumber, ebnf.getToken(0).columnNumber,"Internal parser error@@@ !");
			return opStatement;
		}

		/** proceed to real Statement */
		/** [ find Statement type */
		string type = getOpStatementType(stmtType, classID);
		//if(type.size() && !opStatement->type.size())
		if(type == "")
		{
			displayError(fName, ebnf.getToken(0).lineNumber, ebnf.getToken(0).columnNumber,"unable to determine Statement type "+ ebnf.getToken(0).value);
		}

		if(stmtType!="-2" && stmtType != type)
		{
			displayError(fName, ebnf.getToken(0).lineNumber, ebnf.getToken(0).columnNumber,"types mismatch ! "+ ebnf.getToken(0).value);
		}

		opStatement->type = type;

		if(to == -1 || ebnf.getToken(0).value.size() == 0)// consumed all tokens !
		{
			return opStatement;
		}
		
		// ]
		
		//if(!typeHasOperator(tokens->at(currentIndex).value, findID(tokens->at(currentIndex+1).value, classID).type))
		//{
		//	displayError(fName,(tokens->at(currentIndex)).lineNumber,(tokens->at(currentIndex)).columnNumber,"single operator " +(tokens->at(currentIndex)).value + " is not supported by Statements of type " + findID(tokens->at(currentIndex+1).value, classID).type);
		//	}
		
		
		// [ find first term, function, variable or immediate ...
		
		Statement* aIndex = NULL;
		// if identifier , then it might be variable or user/builtin function
		if(ebnf.getToken(from).type == "identifier" ||  DM14::types::isDataType(ebnf.getToken().value) )//(tokens->at(currentIndex)).type == "datatype")
		{
			// function !
			if(isBuiltinFunction(ebnf.getToken(from).value) || isUserFunction(ebnf.getToken(from).value, true) 
				||(parent && DM14::types::classHasMemberFunction(classID, ebnf.getToken(from).value)))
			{
				Statement* stmt = NULL;
				if(parent)
				{
					if(opStatement->type != classID)
					{
						stmt = parseFunctionCallInternal(false, opStatement->type, classID);
					}
					else
					{
						stmt = parseFunctionCallInternal(false, "", classID);
						stmt->type = DM14::types::classMemberFunctionType(classID, ebnf.getToken(from).value);
					}
				}
				else
				{
					stmt = parseFunctionCallInternal(false, opStatement->type);
				}
				stmt->distStatements = opStatement->distStatements;
				delete opStatement;
				return stmt;
			}
			// variable !
			else
			{
				ebnf.popToken();
				string variableName = ebnf.getToken().value;
				
				if(DM14::types::isDataType(variableName))
				{
					displayInfo(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber, "FIX");
				}
				else if(parent && !DM14::types::classHasMemberVariable(classID, variableName))
				{
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"class " + classID + " has no member : " + variableName);
				}
				else if(parent && DM14::types::classHasMemberVariable(classID, variableName))
				{
					if(DM14::types::getClassMemberVariable(classID, variableName).second.classifier != DM14::types::CLASSIFIER::PUBLIC)
					{
						displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,variableName + " is a private class member variable of class type : " + classID);
					}
				}
				else if(findIDType(idInfo(variableName, 0, "", NULL)) == "NIL" && !parent)
				{
					if(peekToken(0).value == "(")
					{
						displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Un-defined function : " + variableName);
					}
					else
					{
						displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Un-defined variable : " + variableName);
					}
				}

				//classMember = isClass(opStatement->type);
				classMember = DM14::types::isClass(classID);
				termStatement* statement = new termStatement(variableName, opStatement->type );
				
				if(findIDType(idInfo(variableName, 0, "", NULL)) != "NIL")
				{
					statement->type = findIDType(idInfo(variableName, 0, "", NULL));
				}
				statement->scope = scope;
				statement->scopeLevel = scopeLevel;
				statement->identifier = true;
				statement->size = findIDInfo(idInfo(variableName , 0, "", NULL), ARRAYSIZE);

				if(ebnf.getToken(0).value == "[")
				{
					if(!findIDInfo(idInfo(variableName , 0, "", NULL), ARRAY))
					{
						displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber, variableName + " is not an array, invalid use of indexing");
					}
					//termStatement->arrayIndex = parseConditionalExpression(currentStatement);
					//termStatement->arrayIndex = parseStatement("expression-list");
					statement->arrayIndex = parseStatement("declaration-index-list");
					aIndex = statement->arrayIndex;
				}
								
				ebnf.index = origiIndex;

				//idInfo* id = new idInfo(termStatement->term, opStatement->scope, opStatement->type, aIndex);
				idInfo* id = new idInfo(statement->term, opStatement->scope, statement->type , aIndex);
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
				delete opStatement;
				return statement;
			}
		}
		else if(DM14::types::isImmediate(ebnf.getToken(from)))
		{
			if(!DM14::types::hasTypeValue(opStatement->type, ebnf.getToken(from).type))
			{
				displayError(fName, ebnf.getToken(from).lineNumber, ebnf.getToken(from).columnNumber,"wrong immediate type : " + ebnf.getToken(from).value);
			}

			ebnf.popToken();
			
			termStatement* statement = new termStatement(ebnf.getToken().value, opStatement->type);
			statement->line = ebnf.getToken().lineNumber;
			statement->scope = scope;
			statement->scopeLevel = scopeLevel;
			statement->identifier = false;
			statement->type = opStatement->type;
			delete opStatement;
			return statement;
		}
		else
		{
			displayError(fName, ebnf.getToken(from).lineNumber, ebnf.getToken(from).columnNumber,"Un-known Variable or Function : " + ebnf.getToken(from).value);
		}
		
		// ]
		
		delete opStatement;
		displayError(fName, ebnf.getToken(from).lineNumber, ebnf.getToken(from).columnNumber,"returning null Statement ?!!");
		return NULL;
	};

	funcInfo parser::getFunc(const string& funcID, const string& classID)
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

	funcInfo parser::getFunc(const string& funcID, Array<string>* mparameters, const string& returnType, const string& classID)
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


	mapcode::mapcode(const string& mname, Array<ast_function>* const mfunctions, 
					 const Array<includePath>& mincludes, const bool& header,
					 const int& nodesC, const int& variablesCount)
	{
		mapcode();
		fileName = mname;
		functions = new Array<ast_function>();
		*functions = *mfunctions;
		includes = mincludes;
		Header = header;
		nodesCount = nodesC;
		dVariablesCount = variablesCount;
		linkLibs = new Array<Statement*>();
		ExternCodes = new Array<string>();
	};

	mapcode::mapcode()
	{
		functions =	new	Array<ast_function>;
		Header = false;
		ExternCodes = new Array<string>();
	};

	mapcode::~mapcode()
	{
	};


	Array<includePath> parser::getIncludes()
	{
		return includes;
	};

	Array<ast_function>* parser::getFunctions()
	{
		return functions;
	};

	Array<mapcode>* parser::getMapCodes()
	{
		return mapCodes;
	};

	Array<string>* parser::getExternCodes()
	{
		return ExternCodes;
	}


	string mapcode::getFileName()
	{
		return fileName;
	};

	Array<ast_function>* mapcode::getFunctions()
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

	int parser::increaseScope(Statement* stmt)
	{
		if(stmt)
		{
			parentStatement = stmt;
		}
		return ++scope;
	}

	int parser::decreaseScope()
	{
		parentStatement = NULL;
		return --scope;
	}


	Statement* parser::parseThread()
	{
		ebnf.popToken(); //thread
		threadStatement* thread = new threadStatement();
		currentStatement = thread;

		stringstream SS;
		SS << ebnf.getToken().columnNumber << ebnf.getToken().lineNumber;	// Number of character on the current line
		thread->Identifier = SS.str();
		
		if(parentStatement &&(parentStatement->StatementType ==  fLoop || parentStatement->StatementType == wLoop))
		{
			displayWarning(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Thread call inside a loop ! careful ");
		}
		//@TODO: fix checking the parameters to be global only
		
		/*
		for(int32_t i= 0; i < working_tokens->size(); i++)
		{
			string parent;
			
			if(ebnf.getToken(1).value == ".")
			{
				parent = ebnf.getToken(0).value;
			}
			
			
			if(tokens->at(i).type == "identifier" && findID(tokens->at(i).value, parent).name.size() && !findIDInfo(idInfo((tokens->at(i)).value, 0, "", NULL), GLOBAL))
			{
				displayError(fName, ebnf.getToken(0).lineNumber, ebnf.getToken(0).columnNumber,"Only global variables are allowed to be passed to a thread call ! , " + ebnf.getToken(0).value + " is not global");
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


	Statement* parser::parseIf()
	{
		// if [ cond ] { Statements } else {}
		ebnf.popToken(); //if	
		IF* If = new IF;
		If->line = ebnf.getToken().lineNumber;
		If->scope = scope;
		
		increaseScope(If);

		tmpScope = true;
		ebnf.popToken(); //[
		currentStatement = If;
		
		If->condition = parseStatement("expression-list");
		
		tmpScope = false;
		ebnf.popToken(); //]
		
		addStatementDistributingVariables(If);	
		
		ebnf.popToken(); // {
		RequireValue("{", "Expected { and not : ", true);


		//while(ebnf.getToken().value != "}")
		while(!peekToken("}"))
		{
			Statement* stmt = parseStatement("Statement");
			addStatementDistributingVariables(stmt);
			If->body->push_back(stmt);
		}
		ebnf.popToken(); // }

		while(peekToken("else"))
		{
			ebnf.popToken(); //else
			if(peekToken("if"))
			{
				ebnf.popToken(); //if
				
				IF* elseIf = new IF;
				elseIf->line = ebnf.getToken().lineNumber;
				elseIf->scope = scope;

				tmpScope = true;
				ebnf.popToken(); //[
				currentStatement = elseIf;
				elseIf->condition = parseStatement("expression-list");
				tmpScope = false;
				ebnf.popToken(); //]
				
				ebnf.popToken(); // {
				RequireValue("{", "Expected { and not : ", true);
				
				while(!peekToken("}"))
				{
					Statement* stmt = parseStatement("Statement");
					addStatementDistributingVariables(stmt);
					elseIf->body->push_back(stmt);
					
				}
				ebnf.popToken(); //}
				If->elseIF->push_back(elseIf);
			}
			else
			{
				ebnf.popToken(); // {
				RequireValue("{", "Expected { and not : ", true);
				
				while(!peekToken("}"))
				{
					Statement* stmt = parseStatement("Statement");
					addStatementDistributingVariables(stmt);
					If->ELSE->push_back(stmt);
					
				}
				ebnf.popToken(); // }
			}
			//nextIndex();
		}
		
		decreaseScope();
		return If;
	};

	Statement* parser::parseStruct()
	{
		ebnf.popToken(); //struct
		
		DatatypeBase Struct;
		//Struct.templateNames = templateNames;
		
		ebnf.popToken(); // id
		Struct.setID(ebnf.getToken().value);
		
		ebnf.popToken();
		RequireValue("{", "Expected { and not : ", true);

		while(!peekToken("}"))
		{
			declareStatement* stmt =(declareStatement*)parseStatement("Statement");
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
			//ebnf.popToken();
		}
		
		ebnf.popToken();	
		RequireValue("}", "Expected } and not : ", true);

		Struct.addOperator("=");
		Struct.addOperator(".");
		Struct.addTypeValue(Struct.typeID);
		Struct.classType = true;
		
		datatypes.push_back(Struct);
		mapcodeDatatypes.push_back(Struct);
		
		//return new NOPStatement;
		//return nullptr;
		return(Statement*)this;
	}

	Statement* parser::parseExtern()
	{
		ebnf.popToken();
		EXTERN* externStatement = new EXTERN;
		externStatement->line = ebnf.getToken().lineNumber;
		externStatement->scope = scope;
		
		int from = ebnf.index+1;
		reachToken("endextern", false, true, true, true, true);
		int to = ebnf.index-1;
		
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



	int parser::extractSplitStatements(Array<Statement*>* array, Array<Statement*>* splitStatements)
	{
		while(splitStatements->size())
		{
			array->push_back(splitStatements->at(0));
			splitStatements->remove(0);
		}
		return array->size();
	}

	Statement* parser::parseWhile()
	{
		ebnf.popToken(); //while
		whileloop* While = new whileloop;
		While->line = ebnf.getToken().lineNumber;
		While->scope = scope;
		
		increaseScope(While);
		
		tmpScope = true;
		ebnf.popToken(); //[
		currentStatement = While;
		While->condition = parseStatement("expression-list");
		tmpScope = false;
		ebnf.popToken(); //]
		
		addStatementDistributingVariables(While);
		
		
		ebnf.popToken(); // {
		RequireValue("{", "Expected { and not : ", true);

		while(!peekToken("}"))
		{
			Statement* stmt = parseStatement("Statement");
			addStatementDistributingVariables(stmt);
			While->body->push_back(stmt);
			
		}
		ebnf.popToken(); // }
		
		decreaseScope();
		
		return While;
	};

	Statement* parser::parseCase()
	{
		CASE* Case = new CASE;
		Case->line =(tokens->at(ebnf.index)).lineNumber;
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
			Statement* CCondition = NULL;
			//CCondition = parseConditionalExpression(CCondition);
			currentStatement = CCondition;
			CCondition = parseStatement("expression-list");
			tmpScope = false;
			
			if(tokens->at(ebnf.index).value == "}")
			{
				ebnf.index--;
				break;
			}
			//check if it peek of current ???
			while(tokens->at(ebnf.index).value != "}" && tokens->at(ebnf.index).value != "[")
			{
				Case->Body[CCondition].push_back(parseStatement("Statement"));
				if(!peekToken("}") && !peekToken("["))
				{
					nextIndex();
				}
				else
				{
					//nextIndex(); // if we add this, then we might stop at [, which needs to back a
					break;
				}
			}
		}
		
		nextIndex();
		RequireValue("}", "Expected } and not : ", true);
		
		decreaseScope();
		
		return Case;
	};

	int parser::reachToken(	const string& Char, const bool& sameLine,  const bool& reportError, 
							const bool& actual, const bool& beforeEOF, const bool& scopeLevel) // loop till find the specified char
	{
		int line = ebnf.getToken().lineNumber;
		int plevel = 1;
		string firstValue = ebnf.getToken().value; // for scopeLevel
		
		if(actual)
		{
			while(ebnf.popToken().value.size())
			{
				if(sameLine)
				{
					if(ebnf.getToken().lineNumber != line )
					{
						if(reportError)
						{
							displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Missing " + Char);
						}
						else
						{
							return -1;
						}	
					}
				}
				
				if(scopeLevel)
				{
					if(ebnf.getToken().value == Char)
					{
						plevel--;
					}
					else if(ebnf.getToken().value == firstValue)
					{
						plevel++;
					}
				}
				
				if((beforeEOF && ebnf.getToken().value == ";") || ebnf.getToken().value == Char)
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
				if((unsigned) 0 == ebnf.working_tokens->size())
				{
					if(reportError)
					{
						displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Missing " + Char);
					}
				}
			}
		}
		else
		{
			int iterator = 0;
			while((unsigned) iterator < ebnf.working_tokens->size())
			{
				if(sameLine)
				{
					if(ebnf.getToken(iterator).lineNumber != line)
					{
						if(reportError)
						{
							displayError(fName, ebnf.getToken(iterator).lineNumber, ebnf.getToken(iterator).columnNumber,"Missing " + Char);
						}
						else
						{
							return -1;
						}
					}
				}

				
				if(scopeLevel)
				{
					if(ebnf.getToken().value == Char)
					{
						plevel--;
					}
					else if(ebnf.getToken().value == firstValue)
					{
						plevel++;
					}
				}
				
				if((beforeEOF && ebnf.getToken().value == ";") || ebnf.getToken(iterator).value == Char)
				{
					return iterator;
				}
				
				if((unsigned) iterator == ebnf.working_tokens->size()-1)
				{
					if(reportError)
					{
						displayError(fName, ebnf.getToken(iterator).lineNumber, ebnf.getToken(iterator).columnNumber,"Missing " + Char);
					}
				}
				
				iterator++;
			}
		}
		return -1; // couldnt find
	};

	token parser::peekToken(const int& pos)
	{
		if(ebnf.working_tokens->size())
		{
			if((unsigned) pos < ebnf.working_tokens->size())
			{
				return ebnf.working_tokens->at(pos);
			}
		}
		return token();
	};


	/**
	 * @detail peek the first token in the working_tokens vector, which is not poped yet
	 */
	bool parser::peekToken(const string& str)
	{
		if(ebnf.working_tokens->size())
		{
			if(ebnf.working_tokens->at(0).value == str)
			{
				return true;
			}
		}
		return false;
	};

	string parser::getType(const int& Index)
	{
		if(isIdentifier(ebnf.getToken().value))
		{
			return findIDType(idInfo(ebnf.getToken().value, 0, "", NULL));
		}
		else if(DM14::types::isImmediate(ebnf.getToken()))
		{
			return ebnf.getToken().type;
		}
		else if(isFunction(ebnf.getToken().value, true))
		{
			return getFunc(ebnf.getToken().value).returnType;
		}
		else
		{
			displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"can not get type of : " + ebnf.getToken().value);
		}
		return "NIL";
	};

	bool parser::checkToken(int type, string error, bool addtoken)
	{
		bool rError = false;
		
		if(type == DM14::types::types::Function)
		{
			if(isBuiltinFunction(ebnf.getToken().value )  || isUserFunction(ebnf.getToken().value, true) )
			{
				rError = true;
			}
		}
		else if(type == DM14::types::types::USERFUNCTION)
		{
			if(isUserFunction(ebnf.getToken().value, true) )
			{
				rError = true;
			}
		}
		else if(type == DM14::types::types::BUILTINFUNCTION)
		{
			if(isBuiltinFunction(ebnf.getToken().value ) )
			{
				rError = true;
			}
		}
		
		if(rError)
		{
			displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,error + ebnf.getToken().value);
		}
		return rError;
	};

	bool parser::checkToken(string value, string error, bool addtoken)
	{
		bool rError = false;
		if(ebnf.getToken().value == value)
		{
			rError = true;
		}
		
		if(rError)
		{
			displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,error + ebnf.getToken().value);
		}
		return rError;
	};


	bool parser::RequireType(string type, string error, bool addtoken) // add and int for index modification like 1 or -1 :D
	{
		bool rError = false;
		
		if(ebnf.getToken().type != type)
		{
			rError = true;
		}
		
		if(rError)
		{
			if(addtoken)
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,error + ebnf.getToken().value);
			}
			else
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,error);
			}
		}
		return rError;
	};

	bool parser::RequireValue(string value, string error, bool addtoken) // add and int for index modification like 1 or -1 :D
	{
		bool rError = false;
		
		if(ebnf.getToken().value != value)
		{
			rError = true;
		}
		
		if(rError)
		{
			if(addtoken)
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,error + ebnf.getToken().value);
			}
			else
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,error);
			}
		}
		return rError;
	};


	// maybe w should move this to scanner ? and modify isFunction instead of reading the stupid manually made maps file !?!
	int parser::mapFunctions(const string& package, const string& library)
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
		DM14::scanner* scner = new DM14::scanner(fullLibraryName);
		
		if(!scner->isReady())
		{
			displayError(scner->getFileName(), -1,-1,"Could not open Library : " + fullLibraryName);
		}
		
		scner->setShortComment("//");
		scner->setLongComment("/*", "*/");
		scner->scan();
		//scner->printTokens();
		
		displayInfo(" Parsing   ... [" + fullLibraryName + "]");
		
		Array<token>* mapTokens = scner->getTokens();

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
		
		delete scner;
		return 0;
	};


	long parser::parseCClass(DM14::scanner* scner, uint32_t start, const Array<string>& templateNames)
	{
		DatatypeBase CClass;
		CClass.templateNames = templateNames;
		
		int classifier = 1;
		
		bool TYPEDEF = false;
		bool protoType  = true;
		bool forwardDecelation = false;
		Array<token>* mapTokens = scner->getTokens();
		
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



	funcInfo parser::parseCFunction(DM14::scanner* scner, uint32_t start, const DatatypeBase& parentClass)
	{
		funcInfo funcinfo;
		Array<token>* mapTokens = scner->getTokens();
		
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


	idInfo parser::findID(const string& ID, const string& parentID)
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

	Statement* parser::parseDeclaration()
	{
		Statement* result = parseDeclarationInternal();
		ebnf.popToken();
		RequireValue(";", "Expected ; and not ", true);
		return result;
	}


	Statement* parser::parseDeclarationInternal()
	{
		ebnf.popToken();
		declareStatement* decStatement = new declareStatement;
		decStatement->line = ebnf.getToken().lineNumber;
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

		Array<idInfo>* tempIdentifiers = new Array<idInfo>;

		while(true)
		{
			checkToken(DM14::types::types::Function, "this is a function name ! :  ", true);
			RequireType("identifier", "Expected \"Identifier\" and not ", true);
			
			if(findIDInfo(idInfo(ebnf.getToken().value, 0, "", NULL),SCOPE) == decStatement->scope)
			{
				if(findIDInfo(idInfo(ebnf.getToken().value, 0, "", NULL),TMPSCOPE))
				{
					displayWarning(fName, ebnf.getToken().lineNumber,ebnf.getToken().columnNumber,"Pre-defined variable in different scope :  " + ebnf.getToken().value);
				}
				else if(!tmpScope)
				{
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Pre-defineddd variable :  " + ebnf.getToken().value);
				}
			}
			else if(findIDInfo(idInfo(ebnf.getToken().value, 0, "", NULL),GLOBAL))
			{
				displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"Pre-defined Global variable :  " + ebnf.getToken().value);
			}
			
			tempIdentifiers->push_back(idInfo(ebnf.getToken().value, 0, "", NULL));	
			decStatement->identifiers->push_back(idInfo(ebnf.getToken().value, 0, "", NULL)) ;
			//cerr << "ID : " << ebnf.getToken().value << endl;
			if(!decStatement->tmpScope)
			{
				distributedVariablesCount++;
			}
			
			ebnf.popToken();
			
			if(ebnf.getToken().value != "," )
			{
				break;
			}
			
			ebnf.popToken();
		}
		
		bool distributed = true;
		
		while(true)
		{
			if(ebnf.getToken().value == "noblock" )
			{
				decStatement->noblock = true;
			}
			else if(ebnf.getToken().value == "recurrent" )
			{
				decStatement->recurrent=true;
			}
			else if(ebnf.getToken().value == "backprop" )
			{
				decStatement->backProp=true;
			}
			else if(ebnf.getToken().value == "nodist" )
			{
				if(decStatement->recurrent)
				{
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber, "nodist variable can not be recurrent !");
				}
				
				////displayWarning(fName,(tokens->at(ebnf.index)).lineNumber,(tokens->at(ebnf.index)).columnNumber, "Warning, nodist variable");
				distributed = false;
				decStatement->distributed = false;
			}
			else if(ebnf.getToken().value == "channel" )
			{
				if(decStatement->recurrent || !distributed || decStatement->backProp)
				{
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber, "channel variable is not pure !?");
				}

				decStatement->channel = true;
			}
			else if(ebnf.getToken().value == "global" )
			{
				decStatement->global = true;
			}
			else
			{
				break;
			}
			
			ebnf.popToken();
		}
		
		
		if(!DM14::types::isDataType(ebnf.getToken().value))
		{
			RequireType("datatype", "Expected \"Data type\" and not ", true);
		}
		
		decStatement->type = ebnf.getToken().value;
		decStatement->classtype = DM14::types::isClass(decStatement->type);
		decStatement->array = false;
			
		
		
		/** array index */
		if(peekToken("["))
		{
			ebnf.popToken(); // [
			decStatement->array = true;
			ebnf.popToken();
			//@TODO: only int can be array ?
			if(ebnf.getToken().type == "int")
			{
				stringstream SS;
				SS << ebnf.getToken().value;
				SS >> decStatement->size;
				distributedVariablesCount += decStatement->size;
				ebnf.popToken();
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
				ebnf.popToken();
				decStatement->array = true;
				ebnf.popToken();
				if(ebnf.getToken().type == "int")
				{
					stringstream SS;
					SS << ebnf.getToken().value;
					SS >> decStatement->size;
					distributedVariablesCount += decStatement->size;
					ebnf.popToken();
					RequireValue("]", "Expected ] and not ", true);		
				}
				else
				{
					decStatement->size = 0;
					RequireValue("]", "Expected ] and not ", true);
				}
				ebnf.popToken();
			}
		}


		
		if(peekToken("("))
		{
			ebnf.popToken();
			int to = reachToken(")", true, true, false, false, true);
			//decStatement->value = parseOpStatement(*working_tokens_index, to-1, decStatement->type, 0, decStatement);
			decStatement->value = parseOpStatement(0, to-1, decStatement->type, 0, decStatement);
			reachToken(")", true, true, true, false, true);
			//ebnf.popToken();
			//RequireValue(terminal, "Expected " + terminal + " and not ", true);
			decStatement->Initilazed = true;
		}
		else if(peekToken("="))
		{
			ebnf.popToken();
			/* TODO: FIX: fix the array list initialization or atleast review */
			if(peekToken("{"))
			{
				ebnf.popToken();
				
				if(!decStatement->array)
				{
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"This variable is not an array");
				}
				
				int plevel = 1;
				//int from = *working_tokens_index+1;
				int from = 1;
				while(ebnf.popToken().value.size())
				{
					if(ebnf.getToken().value == "(")
					{
						plevel++;
					}
					else if(ebnf.getToken().value == ")" || ebnf.getToken().value == "}")
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
					else if(ebnf.getToken().value == ",")
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
					displayError(fName, ebnf.getToken().lineNumber, ebnf.getToken().columnNumber,"too many initilizations");
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
						termStatement* statement = new termStatement(decStatement->identifiers->at(k).name, decStatement->type);
						statement->scope = scope;
						
						SS << i;
						termStatement* arrayIndex = new termStatement(SS.str(), decStatement->type);
						arrayIndex->scope = scope;
						
						statement->arrayIndex = arrayIndex;
						//termStatement->identifier = true;
						
						operationalStatement* os = new operationalStatement();
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
							Statement* origiCurrentStatement = currentStatement;
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
				
				decStatement->value = parseStatement("expression-list", &parser::parseExpressionStatement);
				for(auto* Statement : decStatement->value->distStatements)
				{
					decStatement->distStatements.push_back(Statement);
				}

				decStatement->value->distStatements.clear();

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
						termStatement* statement = new termStatement(decStatement->identifiers->at(k).name, decStatement->type);
						statement->scope = scope;
						
						operationalStatement* os = new operationalStatement();
						os->left = statement;
						os->op = "=";
						os->right = decStatement->value;
						os->type = decStatement->type;
						os->scope = scope;
						//globalDefinitions.push_back(os);
						decStatement->splitStatements.push_back(os);
						////currentFunction.body->push_back(os);
					}
					
					if(decStatement->splitStatements.size())
					{
						decStatement->splitStatements.at(0)->distStatements = decStatement->distStatements;
						decStatement->distStatements.clear();
					}
					decStatement->value = NULL;
				}
				decStatement->Initilazed = true;
			}
		}
		else
		{
			//cerr << "VAL :" << ebnf.getToken().value.size() << endl;
			//RequireValue(terminal, "Expected " + terminal + " and not ", true);
			decStatement->Initilazed = false;
			decStatement->value = NULL;
		}
		
		stringstream SS;
		
		decStatement->identifiers->clear();
		for(uint32_t i = 0; i < tempIdentifiers->size(); i++)
		{
			SS << i;
			termStatement* arrayIndex = new termStatement(SS.str(), decStatement->type);
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
						idInfo* parentID = new idInfo();
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

} //namespace DM14::parser
