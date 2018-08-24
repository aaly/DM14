/**
@file             parser.cpp
@brief            parser
@details          parser, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license


TODO: report grammar error using the longest trace and quite.

*/


#include "parser.hpp"

#define PARSER_EBNF_SHOW_TRACE 0

int parser::pushToken(token tok)
{
	if(tok.value.size())
	{
		working_tokens->push_back(tok);
	}
	return working_tokens->size();
}

token parser::popToken()
{
	if(working_tokens->size() > 0)
	{
		current_token = working_tokens->at(0);
		working_tokens->erase(working_tokens->begin());
	}
	else
	{
		displayError("Empty pop !!!");
		current_token = token();
	}
	
	return current_token;
}

token parser::getToken()
{
	return current_token;
}

token parser::getToken(const unsigned int index)
{
	if(working_tokens->size() > 0 &&
		index < working_tokens->size())
	{
		return working_tokens->at(index);
	}
	return token();
}

int parser::removeToken()
{
	if(working_tokens->size() > 0)
	{
		working_tokens->erase(working_tokens->end());
	}
	else
	{
		cerr << "error removign token from the working tokens vector" << endl;
	}
	
	return 0;
}

statement* parser::empty_file()
{
	//cerr << "unable to proceed file [" <<  fName << "] , wrong grammar at token [" << index <<"] : " << tokens->at(index).value<< endl;
	
	displayError(fName, -1,0,"unable to proceed file", false);
	displayError(fName, -1,0,"dumping stack : ", false);
	
	for(unsigned int i =0; i < working_tokens->size(); i++)
	{
		cerr << working_tokens->at(i).value << endl;
	}
	
	displayError(fName, -1,0,"Quiting");
	return NULL;
}

int parser::advance_EBNFindex()
{
	if(*input_tokens_index < input_tokens->size())
	{
		pushToken(input_tokens->at(*input_tokens_index));
		++*input_tokens_index;
	}
	else
	{
		cerr << "ERROR ADVANCING ! " << endl << flush;
	}
	return *input_tokens_index;
}

int parser::deadvance_EBNFindex()
{
	//if(index-1 > -1)
	if(input_tokens->size())
	{
		removeToken();
		--*input_tokens_index;
	}
	else
	{
		cerr << "ERROR DE-ADVANCING ! " << endl << flush;
	}
	return *input_tokens_index;
}

static int EBNF_level = -1;

int parser::printEBNF()
{
	EBNF_map_t::iterator it;

	for ( it = EBNF.begin(); it != EBNF.end(); it++ )
	{
		string start_map_index = it->first;
		cerr << "<" << start_map_index << "> ::=";
		// loop over std::vector<grammar_token_array_t>
		for (unsigned int i = 0; i < EBNF[start_map_index].size(); i++)
		{
			grammar_rule_t current_rule = EBNF[start_map_index].at(i);
			if (current_rule.tokens.size() > 1)
			{
				cerr << " {";
			}
			
			for (unsigned int k = 0; k < current_rule.tokens.size(); k++)
			{
				EBNF_token_t ebnf_token = current_rule.tokens.at(k);
				
				/** process the invidual token first */
				if(ebnf_token.tokenType == DATATTYPE_TOKEN)
				{
					cerr << "<" << ebnf_token.expansion << ">";
				}
				else if(ebnf_token.tokenType == TERMINAL_TOKEN)
				{
					cerr << ebnf_token.expansion;
				}
				else if(ebnf_token.tokenType == REGEX_TOKEN)
				{
					cerr << "[" << ebnf_token.expansion << "]";
				}
				else if(ebnf_token.tokenType == EXPANSION_TOKEN)
				{
					cerr << "<" << ebnf_token.expansion << ">";
				}
				else if(ebnf_token.tokenType == SINGLE_OP_TOKEN)
				{
					cerr << "<SINGLE_OPERATOR>";
				}
				else if(ebnf_token.tokenType == BINARY_OP_TOKEN)
				{
					cerr << "<BINARY_OPERATOR>";
				}
				else if(ebnf_token.tokenType == IMMEDIATE_TOKEN)
				{
					cerr << "<IMMEDIATE>";
				}
				else
				{
					cerr << " ERROR : unknown grammar token type" << endl;
				}
				
				if(k < current_rule.tokens.size()-1)
				{
					cerr << " ";
				}
				
				/** process the current grammar rule type*/
				if(current_rule.type == GRAMMAR_TOKEN_OR_ARRAY)
				{
					if(k < current_rule.tokens.size() -1)
					{
						cerr << "| ";
					}
				}
				else if(current_rule.type == GRAMMAR_TOKEN_AND_ARRAY)
				{
					//cerr << " ";
					;
				}
				else if(current_rule.type == GRAMMAR_TOKEN_ZERO_MORE_ARRAY)
				{
					cerr << "*";
				}
				else if(current_rule.type == GRAMMAR_TOKEN_ONE_MORE_ARRAY)
				{
					cerr << "+";
				}
				else if(current_rule.type == GRAMMAR_TOKEN_ONLY_ONE_ARRAY)
				{
					cerr << "?";
				}
				else
				{
					cerr << string(EBNF_level, ' ') << " ERROR : unknown grammar rule type" << endl;
				}
			}
			if (current_rule.tokens.size() > 1)
			{
				cerr << "}";
			}
			
		}
		cerr << endl;
	}
	return 0;
}


/**
 * @details This function takes a set of input tokens, and a start map key, and a pointer to the output tokens vector
 */
ebnfResult parser::parseEBNF(Array<token>* input_tokens, std::string start_map_index, Array<token>* output_tokens)
{
	if(!EBNF[start_map_index].size())
	{
		cerr << "unknown grammar map index : " << start_map_index << endl << flush;
		exit(1);
	}
	
	Array<token>* current_working_tokens = working_tokens;
	working_tokens = output_tokens;

	Array<token>* current_input_tokens = this->input_tokens;
	this->input_tokens = input_tokens;
	
	++EBNF_level;
	ebnfResult result(0, NULL);
	bool continue_searching_rules = true;
	
	#if PARSER_EBNF_SHOW_TRACE == 1
	cerr << endl << std::string(EBNF_level*2, ' ') << ".LOOPING KEY : " << start_map_index << endl << flush;
	#endif
	
	// loop over std::vector<grammar_token_array_t>
	for (unsigned int i = 0; i < EBNF[start_map_index].size() && continue_searching_rules == true; i++)
	{
		result.first = 0;
		
		#if PARSER_EBNF_SHOW_TRACE == 1
		cerr << string(EBNF_level*3, ' ')  << "..LOOPING RULE : " << i << endl << flush;
		#endif

		// loop over std::vector<EBNF_entity_t>
		grammar_rule_t current_rule = EBNF[start_map_index].at(i);
		
		int old_index = *input_tokens_index; // used for ONLY_ONE rules
		int old_tokens_size = working_tokens->size(); // used for callbacks with only current rule pushed tokens
		
		for (unsigned int k = 0; k < current_rule.tokens.size() && continue_searching_rules == true; k++)
		{
			EBNF_token_t ebnf_token = current_rule.tokens.at(k);
			
			//cerr << ebnf_token.expansion << ":" << ebnf_token.tokenType << ":" << current_rule.tokens.size() << endl << flush;
			/** process the invidual token first */
			if(ebnf_token.tokenType == DATATTYPE_TOKEN)
			{
				if (input_tokens->at(*input_tokens_index).type == ebnf_token.expansion)
				{
					result.first = 0;
				}
				else
				{
					result.first = 1;
				}
			}
			else if(ebnf_token.tokenType == TERMINAL_TOKEN)
			{
				////cerr << "TERMINAL" << input_input_tokens->at(*input_tokens_index).value << ":" <<  ebnf_token.expansion << endl << flush;
				if (input_tokens->at(*input_tokens_index).value == ebnf_token.expansion)
				{
					result.first = 0;
				}
				else
				{
					result.first = 1;
				}
			}
			else if(ebnf_token.tokenType == REGEX_TOKEN)
			{
				//cerr << input_tokens->at(*input_tokens_index).value << ":" <<  ebnf_token.expansion << endl << flush;
				// do regex stuff
				std::regex self_regex(ebnf_token.expansion, std::regex_constants::ECMAScript);
				if (std::regex_search(input_tokens->at(*input_tokens_index).value, self_regex))
				{
					////cerr << input_tokens->at(*input_tokens_index).value << ":" <<  ebnf_token.expansion << endl << flush;
					result.first = 0;
				}
				else
				{
					result.first = 1;
				}
			}
			else if(ebnf_token.tokenType == EXPANSION_TOKEN)
			{
				int local_index = *input_tokens_index;
				result = parseEBNF(input_tokens, ebnf_token.expansion, output_tokens);
			}
			else if(ebnf_token.tokenType == SINGLE_OP_TOKEN)
			{
				if(isSingleOperator(input_tokens->at(*input_tokens_index).value))
				{
					result.first = 0;
				}
				else
				{
					result.first = 1;
				}
			}
			else if(ebnf_token.tokenType == IMMEDIATE_TYPE_TOKEN)
			{
				if(isImmediate(input_tokens->at(*input_tokens_index).value))
				{
					result.first = 0;
				}
				else
				{
					result.first = 1;
				}
			}
			else if(ebnf_token.tokenType == BINARY_OP_TOKEN)
			{
				if(isbinOperator(input_tokens->at(*input_tokens_index).value))
				{
					result.first = 0;
				}
				else
				{
					result.first = 1;
				}
			}
			else if(ebnf_token.tokenType == IMMEDIATE_TOKEN)
			{
				//cerr << "IMMEDIATE :" << input_tokens->at(*input_tokens_index).value << ":" <<  ebnf_token.expansion << endl << flush;
				if(isImmediate(input_tokens->at(*input_tokens_index).value))
				{
					result.first = 0;
				}
				else
				{
					result.first = 1;
				}
			}
			else
			{
				cerr << " ERROR : unknown grammar token type" << endl;
			}
			
			if(result.first == 0)
			{
				#if PARSER_EBNF_SHOW_TRACE == 1
				cerr << string(EBNF_level*4, ' ') << "...PROCESSING TOKEN : " << ebnf_token.expansion << " => success " << endl << flush;
				#endif

				if(ebnf_token.tokenType != EXPANSION_TOKEN) // only terminals
				{
					advance_EBNFindex();
				}
				
				if(ebnf_token.tokenType == EXPANSION_TOKEN && ebnf_token.callback)
				{
					for(unsigned int i =0; i < working_tokens->size(); i++)
					{
						cerr << "|||" << working_tokens->at(i).value << endl;
					}
					
					Array<token> tempTokenStack;
					for(unsigned int i =0; i < old_tokens_size; i++)
					{
						cerr << "SIZE :" << i << endl;
						tempTokenStack.push_back(popToken());
					}
					
					for(unsigned int i =0; i < working_tokens->size(); i++)
					{
						cerr << "///" << working_tokens->at(i).value << endl;
					}
					
					result.second = (this->*ebnf_token.callback)();
					
					if(pushStatements && result.second)
					{
						statements_stack.push_back(result.second);
					}
					
					for(unsigned int i =0; i < tempTokenStack.size(); i++)
					{
						pushToken(tempTokenStack.at(i));
					}					
				}
			}
			else
			{
				////cerr << string(EBNF_level, ' ') << "...PROCESSING TOKEN : " << ebnf_token.expansion << " => fail " << endl << flush;
			}
			
			/** process the current grammar rule type*/
			if(current_rule.type == GRAMMAR_TOKEN_OR_ARRAY)
			{
				if(result.first != 0)
				{
					for(unsigned int i =0; i < *input_tokens_index - old_index; i++)
					{
						cerr << "DEADVANCE" << endl << flush;	
						deadvance_EBNFindex();
					}
				}
				else
				{
					continue_searching_rules = false;
				}
			}
			else if(current_rule.type == GRAMMAR_TOKEN_AND_ARRAY)
			{
				if(result.first != 0)
				{
					#if PARSER_EBNF_SHOW_TRACE == 1
					cerr << string(EBNF_level*4, ' ') << " ERROR : tokens do not (follow order/exist)" << endl;
					#endif 

					for(unsigned int i =0; i < *input_tokens_index - old_index; i++)
					{
						deadvance_EBNFindex();
					}
					//continue_searching_rules = false;
					break;
				}
				else 
				{
					if(k == current_rule.tokens.size()-1)
					{
						continue_searching_rules = false;
					}
				}
			}
			else if(current_rule.type == GRAMMAR_TOKEN_ZERO_MORE_ARRAY)
			{
				if(result.first == 0)
				{
					--k;
				}
				else
				{
					result.first= 0;
				}
			}
			else if(current_rule.type == GRAMMAR_TOKEN_ONE_MORE_ARRAY)
			{
				if(result.first == 0)
				{
					--k;
				}
				else
				{
					result.first = 0;
				}
			}
			else if(current_rule.type == GRAMMAR_TOKEN_ONLY_ONE_ARRAY)
			{
				if(result.first == 1)
				{
					for(unsigned int i =0; i < *input_tokens_index - old_index; i++)
					{
						deadvance_EBNFindex();
					}
					result.first = 0;
					break;
				}
			}
			else
			{
				#if PARSER_EBNF_SHOW_TRACE == 1
				cerr << string(EBNF_level*4, ' ') << " ERROR : unknown grammar rule type" << endl;
				#endif
			}
		}
	}
	EBNF_level--;
	
	working_tokens = current_working_tokens;
	this->input_tokens = current_input_tokens;
	return result;
}

parser::parser(Array<token>* gtokens, const string& filename, const bool insider = true)
{
	if (gtokens ==NULL || gtokens->size() == 0)
	{
		displayError(filename, 0,0,"Internal parser error !!!, no tokens , maybe empty source file ?");
	}
	
	tokens				=	gtokens;
	fName				=	filename;
	
	functions			=	new Array<ast_function>;
	identifiers			=	new Array<idInfo>;
	//builtinFunctions	=	new Array<funcInfo>;
	//userFunctions		=	new Array<funcInfo>;
	functionsInfo		=	new Array<funcInfo>;
	ExternCodes			=	new Array<string>;
	mapCodes 			=	new Array<mapcode>;
	index				=	0;
	scope				=	0;
	fileInclude			=	false;
	Header				=	false;
	diststatementTemp	=	new distStatement();
	distributedVariablesCount = 0;
	distributedNodesCount = 0;
	tmpScope = false;
	globalNoDist = false;
	
	working_tokens = &tokens_stack;
	//working_tokens_index = &token_index;
	input_tokens_index = &index;
	input_tokens = tokens;
	
	dvList				=	new Array<distributingVariablesStatement*>;
	libs				=	new Array<statement*>();

	this->insider = insider;
	
	distModifiedGlobal	= new Array<idInfo>;
	diststatementTemp	= new distStatement();
	modifiedVariablesList = Array < pair<idInfo,int> >();
	increaseScope(NULL);
	distributedScope = 0;
	functionStatementsCount = 0;
	
	EBNF["program"] = {{GRAMMAR_TOKEN_OR_ARRAY,{{"statement",EXPANSION_TOKEN},
											    {".*",REGEX_TOKEN, &parser::empty_file}}}};
	
	EBNF["statement-list"] = {{GRAMMAR_TOKEN_ZERO_MORE_ARRAY ,{{"statement",EXPANSION_TOKEN}}}};
	
	EBNF["statement"] = {{GRAMMAR_TOKEN_OR_ARRAY,{{"include-statement",EXPANSION_TOKEN,&parser::parseIncludes},
												  {"declaration-list",EXPANSION_TOKEN, &parser::parseDeclaration},
												  {"extern-statement", EXPANSION_TOKEN, &parser::parseExtern}, /** extern  { c/c++ code } endextern */
												  {"link-statement",EXPANSION_TOKEN, &parser::parseLink},
												  {"struct",EXPANSION_TOKEN, &parser::parseStruct},
												  {"if-list",EXPANSION_TOKEN}, /** if [expr] {} else if[] {} else {} */
												  {"distribute",EXPANSION_TOKEN},
												  {"reset-statement",EXPANSION_TOKEN},
												  {"setnode-statement",EXPANSION_TOKEN},
												  {"for-list",EXPANSION_TOKEN, &parser::parseForloop}, /** for (from -> to : step) { statements; } */
												  {"while-list",EXPANSION_TOKEN}, /** while [ cond ] { statements } */
												  //{"case-list",EXPANSION_TOKEN}, /** case [ID/expr] in { 1) ; 2) ; *) ;}   body is like map<condition,statments> */
												  //{"addparent-statement",EXPANSION_TOKEN},
												  {"thread-list",EXPANSION_TOKEN},
												  //{"function-call-list",EXPANSION_TOKEN, &parser::parseFunctionCall},
												  {"function-list",EXPANSION_TOKEN, &parser::parseFunction},
												  {"return-list",EXPANSION_TOKEN},
												  {"expression-statement", EXPANSION_TOKEN, &parser::parseExpressionStatement},
												  {"expression",EXPANSION_TOKEN, &parser::parseExpressionStatement},
												  {"nop-statement",EXPANSION_TOKEN, &parser::parseNOPStatement}
												  }}};
	
	/** the with statement */
	EBNF["include-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"with",TERMINAL_TOKEN},
														   {"include-statement-body",EXPANSION_TOKEN}}}};
														   
	EBNF["include-statement-body"] = {{GRAMMAR_TOKEN_OR_ARRAY ,{{"include-statement-package",EXPANSION_TOKEN},
																{"include-statement-file",EXPANSION_TOKEN}}}};
								 
	EBNF["include-statement-package"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"[a-zA-Z0-9]+",REGEX_TOKEN},
																   {"include-statement-subpackage",EXPANSION_TOKEN}}}};
	EBNF["include-statement-subpackage"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"use",TERMINAL_TOKEN},
																	    	{"[a-zA-Z0-9]+",REGEX_TOKEN}}}};
																   
	EBNF["include-statement-file"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"\"[a-zA-Z0-9]+[\.]?[[a-zA-Z0-9]+]?\"",REGEX_TOKEN}}}};
	
	/** the extern statement */
	EBNF["extern-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"extern",TERMINAL_TOKEN},
														   {"string",DATATTYPE_TOKEN},
														   {"endextern",TERMINAL_TOKEN}}}};

	/** the link statement */
	EBNF["link-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"(link)|(slink)",REGEX_TOKEN},
														   {"string",DATATTYPE_TOKEN}}}};

	/** the distribute statement */
	EBNF["distribute"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"distribute",TERMINAL_TOKEN},
												{";",TERMINAL_TOKEN}}}};
												
	/** the reset statement */
	EBNF["reset-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"reset",EXPANSION_TOKEN},
														 {";",TERMINAL_TOKEN}}}};
	EBNF["reset"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"reset",TERMINAL_TOKEN},
											   {"expression",EXPANSION_TOKEN}}},
					{GRAMMAR_TOKEN_AND_ARRAY,{{"reset",TERMINAL_TOKEN}}}};
	
	/** the setnode statement */
	EBNF["setnode-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"setnode",EXPANSION_TOKEN},
														 {";",TERMINAL_TOKEN}}}};
	EBNF["setnode"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"setnode",TERMINAL_TOKEN},
											   {"expression",EXPANSION_TOKEN}}},
					{GRAMMAR_TOKEN_AND_ARRAY,{{"setnode",TERMINAL_TOKEN}}}};
	/** the struct statement */
	EBNF["struct"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"struct",TERMINAL_TOKEN},
												{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
												{"struct-definition",EXPANSION_TOKEN}}}};
												
	EBNF["struct-definition"] = {{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{";",TERMINAL_TOKEN}}},
								 {GRAMMAR_TOKEN_AND_ARRAY ,{{"{",TERMINAL_TOKEN},
														    {"struct-declaration-list",EXPANSION_TOKEN},
														    {"}",TERMINAL_TOKEN}}}};
	EBNF["struct-declaration-list"] = {{GRAMMAR_TOKEN_ONE_MORE_ARRAY ,{{"declaration-list",EXPANSION_TOKEN}}}};
	
	/** the for loop statement */
	
	EBNF["for-list"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"for",TERMINAL_TOKEN},
												  {"[",TERMINAL_TOKEN},
												  {"loop-expression-declarator",EXPANSION_TOKEN},
												  {"loop-expression-condition",EXPANSION_TOKEN},
												  {"loop-expression-step-list",EXPANSION_TOKEN},
												  {"]",TERMINAL_TOKEN},
												  {"compound-statement",EXPANSION_TOKEN}}}};
																	
	EBNF["loop-expression-declarator"] = {{GRAMMAR_TOKEN_OR_ARRAY ,{{"declaration-list",EXPANSION_TOKEN},
																	{";",TERMINAL_TOKEN}}}};
																	
	EBNF["loop-expression-condition"] = {{GRAMMAR_TOKEN_OR_ARRAY ,{{"logical-expression-list",EXPANSION_TOKEN},
																   {";",TERMINAL_TOKEN}}}};
	
	EBNF["logical-expression-list"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"logical-expression",EXPANSION_TOKEN},
																  {";",TERMINAL_TOKEN}}}};
	EBNF["logical-expression"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"logical-expression-term",EXPANSION_TOKEN},
															 {".*",BINARY_OP_TOKEN},
																 {"logical-expression-term",EXPANSION_TOKEN}}},
								  {GRAMMAR_TOKEN_AND_ARRAY ,{{"logical-expression-term",EXPANSION_TOKEN}}}};
															 
	EBNF["logical-expression-term"] = {{GRAMMAR_TOKEN_OR_ARRAY ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
																 {".*",IMMEDIATE_TOKEN}}}};
															 

	EBNF["loop-expression-step-list"] = {{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"loop-expression-step",EXPANSION_TOKEN}}}};
	
	EBNF["loop-expression-step"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
															   {".*",SINGLE_OP_TOKEN}}},
									{GRAMMAR_TOKEN_AND_ARRAY ,{{".*",SINGLE_OP_TOKEN},
															   {"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN}}}};
	/** the if statement */
	EBNF["if-list"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"if",TERMINAL_TOKEN},
												  {"[",TERMINAL_TOKEN},
												  {"logical-expression",EXPANSION_TOKEN},
												  {"]",TERMINAL_TOKEN},
												  {"compound-statement",EXPANSION_TOKEN},
												  {"elseif-list",EXPANSION_TOKEN},
												  {"else-list",EXPANSION_TOKEN}}}};
												  
	EBNF["elseif-list"] = {{GRAMMAR_TOKEN_ZERO_MORE_ARRAY,{{"elseif-statement",EXPANSION_TOKEN}}}};
	
	EBNF["elseif-statement"] = {{GRAMMAR_TOKEN_OR_ARRAY,{{"else",TERMINAL_TOKEN},
														{"if",EXPANSION_TOKEN}}}};

	EBNF["else-list"] = {{GRAMMAR_TOKEN_ZERO_MORE_ARRAY,{{"else-statement",EXPANSION_TOKEN}}}};
	
	EBNF["else-statement"] = {{GRAMMAR_TOKEN_OR_ARRAY,{{"else",TERMINAL_TOKEN},
														{"compound-statement",EXPANSION_TOKEN}}}};
	/** the while loop statement */
	EBNF["while-list"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"while",TERMINAL_TOKEN},
												  {"[",TERMINAL_TOKEN},
												  {"logical-expression",EXPANSION_TOKEN},
												  {"]",TERMINAL_TOKEN},
												  {"compound-statement",EXPANSION_TOKEN}}}};
	/** variable */
	
	EBNF["variable"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
												   {"declaration-index-list",EXPANSION_TOKEN}}}};
												   
												   
	/** the variables declaration list */
	
	
	EBNF["declaration-list"] = {{GRAMMAR_TOKEN_OR_ARRAY ,{{"declaration-full-statement",EXPANSION_TOKEN},
														   {"declaration-statement",EXPANSION_TOKEN}}}};
	
	EBNF["declaration-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
														   {"declaration-dataflow-specifier",EXPANSION_TOKEN},
														   {"declaration-dist-specifiers-list",EXPANSION_TOKEN},
														   {"declaration-global-specifier",EXPANSION_TOKEN},
														   {"declaration-datatype-list",EXPANSION_TOKEN},
														   {"declaration-index-list",EXPANSION_TOKEN},
														   {"declaration-value-list",EXPANSION_TOKEN}}}};
														   													   
	EBNF["declaration-full-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
														   {"declaration-dataflow-specifier",EXPANSION_TOKEN},
														   {"declaration-dist-specifiers-list",EXPANSION_TOKEN},
														   {"declaration-global-specifier",EXPANSION_TOKEN},
														   {"declaration-datatype-list",EXPANSION_TOKEN},
														   {"declaration-index-list",EXPANSION_TOKEN},
														   {"declaration-value-list",EXPANSION_TOKEN},
														   {";",TERMINAL_TOKEN}}}};
														   
	EBNF["declaration-dataflow-specifier"] = {{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"backprop",TERMINAL_TOKEN}}}}; // should not be ok with nodist...
																		  
	EBNF["declaration-dist-specifiers-list"] = {{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"channel",TERMINAL_TOKEN}}},
												{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"recurrent",TERMINAL_TOKEN}}},
												{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"nodist",TERMINAL_TOKEN}}}};
																	  
	EBNF["declaration-global-specifier"] = {{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"global",TERMINAL_TOKEN}}}};
	
	EBNF["declaration-datatype-list"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"declaration-datatype",EXPANSION_TOKEN}}}};
	
	EBNF["declaration-datatype"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[a-zA-Z]+[a-zA-Z_0-9]*",REGEX_TOKEN}}}};
	
																 
	EBNF["declaration-index-list"] = {{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"declaration-index",EXPANSION_TOKEN}}}};
	
	EBNF["declaration-index"] = {{GRAMMAR_TOKEN_OR_ARRAY ,{{"matrix-index",EXPANSION_TOKEN},
														   {"array-index",EXPANSION_TOKEN}}}};

	
	EBNF["matrix-index"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"array-index",EXPANSION_TOKEN},
													   {"array-index",EXPANSION_TOKEN}}}};
																	   																   
	EBNF["array-index"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[",TERMINAL_TOKEN},
													  {"[0-9]+",REGEX_TOKEN},
													  {"]",TERMINAL_TOKEN}}},
							{GRAMMAR_TOKEN_AND_ARRAY ,{{"[",TERMINAL_TOKEN},
													  {"variable",EXPANSION_TOKEN},
													  {"]",TERMINAL_TOKEN}}}};
																	   
	
	EBNF["declaration-value-list"] = {{GRAMMAR_TOKEN_ONLY_ONE_ARRAY ,{{"declaration-value",EXPANSION_TOKEN}}}};
	EBNF["declaration-value"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"=",TERMINAL_TOKEN},
															//{".*",REGEX_TOKEN}
															{"expression",EXPANSION_TOKEN}}}};
	
	
	/** the function call statement */
	
	EBNF["function-call-list"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"function-call",EXPANSION_TOKEN},
													  {";",TERMINAL_TOKEN}}}};
													  
	EBNF["function-call"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
															   {"(",TERMINAL_TOKEN},
															   {"function-call-arguments-list",EXPANSION_TOKEN},
															   {")",TERMINAL_TOKEN}}}};
	
	EBNF["function-call-arguments-list"] = {{GRAMMAR_TOKEN_ZERO_MORE_ARRAY,{{"function-call-arguments",EXPANSION_TOKEN}}}};
	
	EBNF["function-call-arguments"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"expression",EXPANSION_TOKEN},
																		 {",",TERMINAL_TOKEN}}},
									   {GRAMMAR_TOKEN_AND_ARRAY,{{"expression",EXPANSION_TOKEN}}}};
	
	EBNF["expression-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"expression",EXPANSION_TOKEN},
													{";",TERMINAL_TOKEN}}}};
		
	EBNF["expression"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"SING_OP",SINGLE_OP_TOKEN},
													{"variable",EXPANSION_TOKEN}}},
						  {GRAMMAR_TOKEN_AND_ARRAY,{{"variable",EXPANSION_TOKEN},
													{"SING_OP",SINGLE_OP_TOKEN}}},
						  {GRAMMAR_TOKEN_AND_ARRAY,{{"function-call",EXPANSION_TOKEN}}},
						  {GRAMMAR_TOKEN_AND_ARRAY,{{"variable",EXPANSION_TOKEN},
													{"BIN_OP",BINARY_OP_TOKEN},
													{"expression",EXPANSION_TOKEN}}},
						  {GRAMMAR_TOKEN_AND_ARRAY,{{"IMMEDIATE",IMMEDIATE_TOKEN},
													{"BIN_OP",BINARY_OP_TOKEN},
													{"expression",EXPANSION_TOKEN}}},
						   {GRAMMAR_TOKEN_AND_ARRAY,{{"variable",EXPANSION_TOKEN}}}};
	/** the thread statement */
	EBNF["thread-list"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"thread",TERMINAL_TOKEN},
													 {"function-call",EXPANSION_TOKEN}}}};
	
	/** the function statement */
	EBNF["function-list"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"function-prototype",EXPANSION_TOKEN},
													   {"function-definition-list",EXPANSION_TOKEN}}}};
	
	
	EBNF["function-definition-list"] = {{GRAMMAR_TOKEN_OR_ARRAY ,{{";",TERMINAL_TOKEN},
																	{"compound-statement",EXPANSION_TOKEN}}}};
															 
	EBNF["function-prototype"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[a-zA-Z]+([_]*[0-9]*)*",REGEX_TOKEN},
													        {"(",TERMINAL_TOKEN},
													        {"function-parameter-list",EXPANSION_TOKEN},
													        {"->",TERMINAL_TOKEN},
													        {"function-return",EXPANSION_TOKEN},
													        {")",TERMINAL_TOKEN}}}};
													        
	EBNF["function-parameter-list"] = {{GRAMMAR_TOKEN_ZERO_MORE_ARRAY ,{{"function-parameter",EXPANSION_TOKEN}}},
									   {GRAMMAR_TOKEN_ZERO_MORE_ARRAY ,{{"function-extra-parameter",EXPANSION_TOKEN}}}};
																  
	EBNF["function-extra-parameter"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{",",TERMINAL_TOKEN},
																  {"function-parameter-list",EXPANSION_TOKEN}}}};															  
	
	EBNF["function-parameter"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"[a-zA-Z]+([a-zA-Z_0-9])*",REGEX_TOKEN},
														    {"declaration-datatype-list",EXPANSION_TOKEN}}}};
	
	EBNF["function-return"] = {{GRAMMAR_TOKEN_ZERO_MORE_ARRAY ,{{"function-parameter",EXPANSION_TOKEN}}}};
														        
	EBNF["compound-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY ,{{"{",TERMINAL_TOKEN},
													        {"statement-list",EXPANSION_TOKEN},
													        {"}",TERMINAL_TOKEN}}}};
	/** return statement */
	
	EBNF["return-list"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{"return",TERMINAL_TOKEN},
													{"expression-statement",EXPANSION_TOKEN}}},
						   {GRAMMAR_TOKEN_AND_ARRAY,{{"return",TERMINAL_TOKEN},
													{";",TERMINAL_TOKEN}}}};
	/* nop statement */
	EBNF["nop-statement"] = {{GRAMMAR_TOKEN_AND_ARRAY,{{";",TERMINAL_TOKEN}}}};
}



parser::~parser()
{
	delete functions;
	delete identifiers;
	//delete builtinFunctions;
	//delete userFunctions;
	delete	functionsInfo;
	//delete distIdentifiers;
	delete diststatementTemp;
	//delete ExternCodes;
};

int	parser::parse()
{
	if (!insider)
	{
		parseIncludesInsider("core/DM14GLOBAL.m14", "file", true);
	}

	cerr << ">>>>>>>>>> START " << endl << flush;
	while(index < tokens->size()-1)
	{
		if(parseEBNF(tokens, "program", &tokens_stack).first != 0)
		{
			for(unsigned int i =0; i < ebnf_verification_list.size(); i++)
			{
				cerr << *ebnf_verification_list.end() << endl;
				ebnf_verification_list.erase(ebnf_verification_list.end());
			}
		}
	}
	cerr << "<<<<<<<<<< END " << endl << flush;
	
	for (unsigned int i =0; i < working_tokens->size(); i++)
	{
		cerr << "Token:" << working_tokens->at(i).value << endl;
	}

	// check for the main function // errr no need for file include , the compiler is the one that should that
	if (!Header)
	{
		if (!isUserFunction("main", true))
		{
			displayError(fName, -1, -1, "no main function defined !");
		}
		funcInfo fun = getFunc("main");
		
		if (fun.returnType != "int")
		{
			displayError(fName, -1, -1, "Main function must return int");
		}
	}
		
		
	mapcode mapCode(fName, getFunctions(), getIncludes(), false, distributedNodesCount, distributedVariablesCount);
		
	mapCode.ExternCodes = new Array<string>();
	*mapCode.ExternCodes = *ExternCodes;
	mapCode.ExternCodes = ExternCodes;
	mapCode.libs = libs;
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
statement* parser::parseIncludes() 
{
	for(unsigned int i =0; i < working_tokens->size(); i++)
	{
		cerr << ":::" << working_tokens->at(i).value << endl;
	}
	cerr << "PARSE INCLUDES 1 " << endl << flush;
	
	popToken();
	popToken();
	string package = "";
	string library = "";
	fileInclude = false;
	if (getToken().type != "string" && getToken().type != "identifier" && !isDataType(getToken().value))//tokens->at(index).type != "datatype")
	{
		displayError(fName, getToken().lineNumber,getToken().columnNumber,"Expected Package name and not " + getToken().value );
	}

	if(getToken().type == "string")
	{
		fileInclude = true;
		
		package = getToken().value.substr(1, (getToken().value.size() - 2 ));
		library = "file";
	}
	else
	{
		package = getToken().value;
		popToken();
		if(getToken().value == "use")
		{
			RequireValue("use", "Expected \"Use\" and not ", true);
			popToken();
			
			if (getToken().value != "*" && getToken().type != "identifier" && !isDataType(getToken().value))// tokens->at(index).type != "datatype")
			{
				displayError(fName, getToken().lineNumber,getToken().columnNumber,"Expected Package name");	
			}
			
			if (getToken().value == "*" )
			{
				library = package;
			}
			else
			{
				library = getToken().value;
			}
			
			popToken();
		}
		else
		{
			library = package;
		}
	}
	
	parseIncludesInsider(package, library, fileInclude);
	
	cerr << "PARSE INCLUDES 2 " << endl << flush;
	
	return NULL;
};

int parser::addIncludePath(string path)
{
	includePaths.push_back(path);
	return includePaths.size();
}
int parser::parseIncludesInsider(const string& package, const string& library, const bool fileInclude)
{
	cerr << "PARSE INCLUDES 11 " << endl << flush;
	includes.push_back(pair<string,string>(package,library));
	if (fileInclude)
	{
		string fullPath;
		for(unsigned int i = 0; i < includePaths.size(); i++)
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
		scanner Scanner(fullPath);
		Scanner.setShortComment("~~");
		Scanner.setLongComment("~*", "*~");
		Scanner.scan();
		Scanner.printTokens();
		
		displayInfo(" Parsing   ... [" + package + "/" + library + "]");
		parser Parser(Scanner.getTokens(),package);
		for(unsigned int i = 0; i < includePaths.size(); i++)
		{
			Parser.addIncludePath(includePaths.at(i));
		}
		Parser.Header = true;
		Parser.parse();
		
		for(unsigned int i =0; i < Parser.getMapCodes()->size(); i++)
		{
			Parser.getMapCodes()->at(i).setHeader(true);
			*Parser.getMapCodes()->at(i).ExternCodes = *Parser.ExternCodes;
			
			/*for(unsigned k =0; k < Parser.getMapCodes()->at(i).globalDefinitions.size(); k++)
			{
				globalDefinitions.push_back(Parser.getMapCodes()->at(i).globalDefinitions.at(k));
			}*/
			
			for(unsigned k =0; k < Parser.identifiers->size(); k++)
			{
				if(Parser.identifiers->at(k).global)
				{
					identifiers->push_back(Parser.identifiers->at(k));
				}
			}
			
			//Parser.getMapCodes()->at(i).globalDeclarations = Array<statement*>();
			mapCodes->push_back(Parser.getMapCodes()->at(i));
			//mapCodes->at(mapCodes->size()-1).Print();
		}
		
		for (unsigned int i =0; i< Parser.libs->size(); i++)
		{
			libs->push_back(Parser.libs->at(i));
		}
	
		for (unsigned int i =0; i< Parser.getIncludes().size(); i++)
		{
			bool push = true;
			
			for (unsigned int k =0; k< includes.size(); k++)
			{
				if(Parser.getIncludes().at(i).first == includes.at(k).first &&
					Parser.getIncludes().at(i).second == includes.at(k).second)
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

		for (unsigned int i =0; i< Parser.functionsInfo->size(); i++)
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

statement* parser::parseLink()
{
	Link* stmt = new Link();
	
	popToken();
	if (getToken().value == "slink")
	{
		stmt->Static = true;
	}
	
	popToken();
		
	if (getToken().type != "string")
	{
		RequireType("string", "expected library identifier and not : ", true);
	}
	
	if(getToken().value.size() > 2)
	{
		stmt->libs = getToken().value.substr(1, getToken().value.size()-2);
	}
	
	libs->push_back(stmt);
	
	return stmt;
}


void parser::parseReturn()
{
	returnStatement* returnstatement = new returnStatement;
	returnstatement->line = (tokens->at(index)).lineNumber;
	returnstatement->scope = scope;
	nextIndex();
	if (getToken().value != ";" )
	{
		int currentIndex = index;
		returnstatement->retValue = parseOpStatement(currentIndex, reachToken(";", false, true, true, true, false)-1, currentFunction.returnIDType, 0, returnstatement);
	}
	RequireValue(";","exptected ; and not:", true);
}

statement* parser::parseStatement()
{
	statement* retStmt = NULL;
	increaseScope(retStmt);
	
	bool pushStatements = false;
	
	int *temp_input_tokens_index_ptr = input_tokens_index;
	int temp_input_tokens_index = 0;
	input_tokens_index = &temp_input_tokens_index;
	Array<token>* working_tokens2 = new Array<token>();
	int working_tokens_size_before = working_tokens->size();
	
	retStmt = parseEBNF(working_tokens, "program", working_tokens2).second;
	

	for (unsigned int i =0; i < temp_input_tokens_index; i++)
	{
		popToken();
	}

	delete working_tokens2;
	input_tokens_index = temp_input_tokens_index_ptr;
	
	if (retStmt == NULL)
	{
		displayError(fName, getToken().lineNumber, getToken().columnNumber,"Invalid Statement or grammar rule has no callback : " + getToken().value);
	}
	
	pushStatements = true;
	
	decreaseScope();
	return retStmt;
};

statement* parser::parseNOPStatement()
{
	popToken();
	
	operationalStatement* result = new operationalStatement;
	result->line = getToken().lineNumber;
	result->scope = scope;
	result->scopeLevel = scope;
	result->op = ";";
	
	return result;
}

statement* parser::parseAddParent()
{
	parentAddStatement* ps = new parentAddStatement();
	nextIndex();
	
	if((tokens->at(index)).value == "(")
	{
		int cindex = index;
		ps->ip = parseOpStatement(cindex, reachToken(")", false, true, true, true, true ), "string", 0, ps);
	}
	else
	{
		ps->ip = parseOpStatement(index, index, "string", 0, ps);
	}
	
	nextIndex();
	if((tokens->at(index)).value == "(")
	{
		int cindex = index;
		ps->socket = parseOpStatement(cindex, reachToken(")", false, true, true, true, true ), "int", 0, ps);
	}
	else
	{
		ps->socket = parseOpStatement(index, index, "int", 0, ps);
	}
	return ps;
};

statement* parser::parseSetNode()
{
	setNodeStatement* sn = new setNodeStatement();
	//sn->node = 
	return sn;
};

statement* parser::parseReset()
{
	resetStatement* rs = new resetStatement();
	if (!peekToken(";"))
	{
		nextIndex();
		int cindex = index;
		rs->count = parseOpStatement(cindex, (reachToken(";", true, true, true, false, false) - 1), getType(index-1), 0, rs);
	}
	return rs;
};

statement* parser::parseDistribute()
{
	currentFunction.distributed = true;
	
	stringstream SS;
	
	SS << getToken().lineNumber;
	currentFunction.functionNodes.push_back(currentFunction.name+SS.str());
	SS.str("");
	
	//distStatement* diststatement = new distStatement();
	diststatementTemp->line = getToken().lineNumber;
	diststatementTemp->scope = scope;
	//*diststatement->variables = *distIdentifiers;
	//fix105
	distributedNodesCount++;
	distStatement* stmt = diststatementTemp;
	diststatementTemp = new distStatement();
	distributedScope++;
	return stmt;
}

/*
statement* parser::parseTempDeclaration()
{
	statement* result = NULL;
	
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
	
	
	if (id.name == currentFunction.returnID)
	{
		return 1;
	}
	
	for(unsigned int i = 0; i < currentFunction.parameters->size(); i++)
	{
		if( currentFunction.parameters->at(i).name == id.name &&
			currentFunction.parameters->at(i).parent == id.parent)
		{
			return 1;
		}
	}	
	
	if (distributedScope == 0 && !id.backProp && !id.channel)
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
	
	
	if ( findIDInfo(id, SCOPE)-1 >  currentFunction.scope)
	{
		return 2;
	}
	
	if (!id.distributed)
	{
		return 3;
	}
	
	string parentName = "";
	if(id.parent)
	{
		parentName = id.parent->name;
	}
	
	// FIX109 a modified varaible in current scope, we dont need to receive it then.
	for ( unsigned int i =0; i<distModifiedGlobal->size(); i++)
	{
		string listParentName;
		
		if(distModifiedGlobal->at(i).parent)
		{

			listParentName = distModifiedGlobal->at(i).parent->name;
		}
	
		if (distModifiedGlobal->at(i).name == id.name &&
			listParentName == parentName &&
			distModifiedGlobal->at(i).distributedScope == id.distributedScope &&
			distModifiedGlobal->at(i).scope <= id.scope)
		{		
			if (!id.array)
			{
				if (id.arrayIndex == NULL && !id.channel)
				{
					return 1;
				}
			}
		}
	}
	
	// we modified it before in this scope!
	for ( unsigned int i =0; i<diststatementTemp->modifiedVariables->size(); i++)
	{
		string listParentName;
		
		if(diststatementTemp->modifiedVariables->at(i).parent)
		{
			listParentName = diststatementTemp->modifiedVariables->at(i).parent->name;
		}
		
		if (diststatementTemp->modifiedVariables->at(i).name == id.name &&
			listParentName == parentName &&
			diststatementTemp->modifiedVariables->at(i).scope <= id.scope)
		{
			if (!id.array)
			{
				if (id.arrayIndex == NULL && !id.channel && !id.backProp && !id.noblock)
				{
					return 1;
				}
			}
		}
	}
	
	// we requested it before !
	for ( unsigned int i =0; i<diststatementTemp->dependenciesVariables->size(); i++)
	{
		string listParentName;
		
		if(diststatementTemp->dependenciesVariables->at(i).parent)
		{
			listParentName = diststatementTemp->dependenciesVariables->at(i).parent->name;
		}
		
		if (diststatementTemp->dependenciesVariables->at(i).name == id.name &&
			listParentName == parentName &&
			diststatementTemp->dependenciesVariables->at(i).scope <= id.scope)
		{
			if (!id.array)
			{
				if (id.arrayIndex == NULL && !id.channel && !id.backProp && !id.noblock)
				{
					return 1;
				}
			}
		}
	}
	
	// we already requested it before this statement !
	for ( unsigned int i =0; i<currentStatement->distStatements.size(); i++)
	{
		string listParentName;
		
		if(currentStatement->distStatements.at(i)->variable.parent)
		{
			listParentName = currentStatement->distStatements.at(i)->variable.parent->name;
		}
		
		if (currentStatement->distStatements.at(i)->variable.name == id.name &&
			listParentName== parentName &&
			currentStatement->distStatements.at(i)->variable.scope <= id.scope)
			//&& !id.requestAddress)
		{
			if (!id.array)
			{
				return 1;
			}
		}
	}
	
	
	if (isClass(id.type))
	{
		DatatypeBase datatype = findDataType(id.type);
		for (unsigned int i =0; i < datatype.memberVariables.size(); i++)
		{
			idInfo id2(id);
			id2.name = datatype.memberVariables.at(i).name;
			id2.type = datatype.memberVariables.at(i).returnType;
			id2.arrayIndex = NULL;
			id2.array = false;
			id2.parent = &id;
			pushDependency(id2);
		}
		
		return 0;
	}
	
	distributingVariablesStatement* dvstatement = new distributingVariablesStatement();
	dvstatement->variable = id;
	dvstatement->type = distributingVariablesStatement::DEPS;
	
	if(id.channel)
	{
		dvstatement->dependencyNode = -2;
	}
	else
	{
		for ( int i = distModifiedGlobal->size()-1; i >= 0; i--)
		{
			
			string listParentName = "";
			
			if(distModifiedGlobal->at(i).parent)
			{
				listParentName = distModifiedGlobal->at(i).parent->name;
			}
			
			//cout << "Name : " << distModifiedGlobal->at(i).name << ":" << id.name << endl;
			//cout << "Parents : " << listParentName << ":" << parentName << endl;
			//cout << "Parents : " << distModifiedGlobal->at(i).distributedScope << endl;
			if (distModifiedGlobal->at(i).name == id.name &&
				listParentName == parentName)
			{
				if(id.backProp && distModifiedGlobal->at(i).distributedScope <= id.distributedScope)
				{
					dvstatement->dependencyNode = -3;
					break;
				}
				else if(distModifiedGlobal->at(i).distributedScope != id.distributedScope)
				{
					dvstatement->dependencyNode = distModifiedGlobal->at(i).distributedScope;
					break;
				}
			}
		}
		
		if(dvstatement->dependencyNode == -3)
		{
			dvstatement->dependencyNode = id.distributedScope;
		}
	}
	
	//cout << id.name << ":" << dvstatement->dependencyNode << endl;
	//cout << "\n \n \n \n "<< endl;
	currentStatement->distStatements.push_back(dvstatement);
	dvList->push_back(dvstatement);
	diststatementTemp->dependenciesVariables->push_back(id);
	return 0;
};



idInfo* parser::getTopParent(idInfo* id)
{
	if(!id->parent)
	{
		return id;
	}
	return getTopParent(id->parent);
}
int parser::pushModified(const string& op, idInfo& id)
{
	if(globalNoDist)
	{
		return 1;
	}
	
	
	if (id.name == currentFunction.returnID || getTopParent(&id)->name == currentFunction.returnID)
	{
		return 1;
	}
	
	for(unsigned int i = 0; i < currentFunction.parameters->size(); i++)
	{
		if( currentFunction.parameters->at(i).name == id.name &&
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
	
	
	
	if ( findIDInfo(id, SCOPE)-1 >  currentFunction.scope)
	{
		return 2;
	}
	
	if (!id.distributed)
	{
		return 3;
	}
	
	//cout << "MOD:::::" << id.name << ":" << op << endl;
	
	if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "++" || op == "--")
	{
		
		// also make sure it is not modified twice in the same op statement like : A = B + C(A), where C() will modify A
		// use same code like below (for array index ? )
		for ( unsigned int i =0; i< dvList->size(); i++)
		{
			if(id.channel || id.recurrent)
			{
				break;
			}
			
			if  (((dvList->at(i))->type == distributingVariablesStatement::MODS) &&
				 ((dvList->at(i))->variable.name == id.name) &&
				 ((dvList->at(i))->variable.parent == id.parent) &&
				 ((dvList->at(i))->variable.arrayIndex == NULL))
			{
				//if ( (id.backProp && ((dvList->at(i))->variable.distributedScope < distributedScope)) ||
				//	(!id.backProp && ((dvList->at(i))->variable.distributedScope > distributedScope)) )
				if (!id.backProp && ((dvList->at(i))->variable.distributedScope != distributedScope))
					
				{
					//distributingVariablesStatement* dvEntry = dvList->at(i);
					dvList->at(i)->type = -1;
					//delete dvList->at(i);
					dvList->remove(i);
					break;
				}
			}
		}
		
		distributingVariablesStatement* dvstatement = new distributingVariablesStatement();
		dvstatement->variable = id;
		dvstatement->type = distributingVariablesStatement::MODS;
		
		currentStatement->distStatements.push_back(dvstatement);
		dvList->push_back(dvstatement);
		
		
		// if array element, push final array modification
		//FIX, just loop if found bfore, copy the idInfo and push it to the list, and remove the old one
		/*if (id.arrayIndex != NULL)/
		{
			// make sure there is no previous mod statements of the same variable in the current op statement
			distributingVariablesStatement* dvstatement = new distributingVariablesStatement();
			idInfo idd = id;
			idd.arrayIndex = id.arrayIndex;
			idd.parent = id.parent;
			idd.array = true;
			dvstatement->variable = idd;
			dvstatement->type = distributingVariablesStatement::MODS;
			
			for ( unsigned int i =0; i<dvList->size(); i++)
			{
				if(id.channel)
				{
					break;
				}
				
				string parentName;
				string listParentName;
				if(id.parent)
				{
					parentName = id.parent->name;
				}
				
				if((dvList->at(i))->variable.parent)
				{
					listParentName = (dvList->at(i))->variable.parent->name;
				}
				
				if ( ((dvList->at(i))->type == distributingVariablesStatement::MODS) &&
					 ((dvList->at(i))->variable.name == id.name) &&
					 //((dvList->at(i))->variable.parent == id.parent) &&
					 (listParentName == parentName) &&
					(termStatment*)(dvList->at(i))->variable.arrayIndex == NULL &&
					((dvList->at(i))->variable.distributedScope == distributedScope) )
				{
					//delete dvList->at(i);
					dvList->at(i)->type = -1;
					dvList->remove(i);
					break;
				}
			}
			
			
			if(parentStatement)
			{
				parentStatement->distStatements->push_back(dvstatement);
			}
			else
			{
				currentStatement->distStatements->push_back(dvstatement);
			}
			
			cout << "MOD :::::: " << id.name << endl;
			dvList->push_back(dvstatement);
		}*/
		
		
		if(op == "++" || op == "--")
		{
			pushDependency(id);
		}
		
		for ( unsigned int i =0; i<diststatementTemp->dependenciesVariables->size(); i++)
		{
			
			int sameParentArrayIndex = false;
			if(id.parent && diststatementTemp->dependenciesVariables->at(i).parent)
			{
				if(id.parent->arrayIndex == diststatementTemp->dependenciesVariables->at(i).parent->arrayIndex)
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
			
			
			if(diststatementTemp->dependenciesVariables->at(i).parent)
			{
				listParentName = diststatementTemp->dependenciesVariables->at(i).parent->name;
			}
				
			if (diststatementTemp->dependenciesVariables->at(i).name == id.name
//				&&diststatementTemp->dependenciesVariables->at(i).parent == id.parent)
				&& parentName == listParentName
				&& sameParentArrayIndex
				&& diststatementTemp->dependenciesVariables->at(i).arrayIndex == id.arrayIndex)
			{
				diststatementTemp->dependenciesVariables->erase(diststatementTemp->dependenciesVariables->begin()+i);
			}
		}
		
		diststatementTemp->modifiedVariables->push_back(id);
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
}

statement* parser::parseFunctionCall(bool terminated,const string& returnType, const string& classID)
{
	exit(1);
	functionCall* funcCall = new functionCall; // for every comma , call parseOP
	funcCall->line = getToken().lineNumber;
	funcCall->scope = scope;
	funcCall->name = getToken().value;
	
	
	if (isUserFunction(getToken().value, true))
	{
		funcCall->functionType = USERFUNCTION;
	}
	else
	{
		funcCall->functionType = BUILTINFUNCTION;
	}
	
	popToken();
	RequireValue("(", "Expected \"(\" and not ", true);
	//loop through prameters
	Array<string>* parameters = new Array<string>();
	int plevel = 1;
	int counter = 0;
	while (counter++)
	{
		if (getToken(counter).value == "(")
		{
			plevel++;
		}
		else if (getToken(counter).value == ")")
		{
			plevel--;
			
			if(plevel ==0)
			{
				if(counter > 1)
				{
					if(terminated)
					{
						//funcCall->parameters->push_back(parseOpStatement(*working_tokens_index, counter-1, "-2", 0, funcCall));
						funcCall->parameters->push_back(parseOpStatement(0, counter-1, "-2", 0, funcCall));
					}
					else
					{
						//funcCall->parameters->push_back( parseOpStatement(*working_tokens_index, counter-1, "-2", 0, currentStatement));
						funcCall->parameters->push_back( parseOpStatement(0, counter-1, "-2", 0, currentStatement));
					}
					parameters->push_back((funcCall->parameters->at(funcCall->parameters->size()-1))->type);
				}
				break;
			}
		}
		else if (getToken(counter).value == ",")
		{
			if (plevel==1)
			{		
				if(counter > 1)
				{
					if(terminated)
					{
						//funcCall->parameters->push_back(parseOpStatement(*working_tokens_index, counter-1, "-2", 0, funcCall));
						funcCall->parameters->push_back(parseOpStatement(0, counter-1, "-2", 0, funcCall));
					}
					else
					{
						//funcCall->parameters->push_back( parseOpStatement(*working_tokens_index, counter-1, "-2", 0, currentStatement));
						funcCall->parameters->push_back( parseOpStatement(0, counter-1, "-2", 0, currentStatement));
					}
					parameters->push_back((funcCall->parameters->at(funcCall->parameters->size()-1))->type);
					
					counter = 0;
					popToken();
				}
				continue;
			}
		}
	}
	
	//cout <<"P:" << parameters->size() << ";;" << parameters->at(0) << endl;
	bool error = true;
	
	
	if(funcCall->name == currentFunction.name)
	{
		////cout~~ << "HEREeeeeeee : " << funcCall->name << endl;
		for (unsigned int i =0; i < currentFunction.parameters->size(); i++)
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
		if (error)
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber,"Parameters error for function call : " + funcCall->name);
		}
		funcCall->type = currentFunction.returnIDType;
	}
	else
	{
		if(!getFunc(funcCall->name, parameters, returnType, classID).name.size())
		{
			if(getFunc(funcCall->name, classID).name.size())
			{
				displayError(fName, getToken().lineNumber, getToken().columnNumber,"return type error for function call : " + funcCall->name);
			}
			else
			{
				displayError(fName, getToken().lineNumber, getToken().columnNumber,"Parameters error for function call : " + funcCall->name);
			}
		}
		funcCall->type = getFunc(funcCall->name, parameters, returnType, classID).returnType;
	}
	
	
	if (terminated)
	{
		nextIndex();
		RequireValue(";", "Expected ; and not ", true);
	}
	
	return funcCall;
};

statement* parser::parseForloop()
{
	cerr << endl << endl;
	for (unsigned int i = 0; i < working_tokens->size(); i++)
	{
		cerr << working_tokens->at(i).value << endl;
	}
	popToken();

	
	// for [decl;cond;stmt] {stmts}
	forloop* floop = new forloop;
	floop->line = getToken().lineNumber;
	floop->scope = scope;
	
	increaseScope(floop);
	
	tmpScope = true;
	
	popToken();
	RequireValue("[", "Expected [ and not ", true);
	
	if(!peekToken(";"))
	{
		//statement* stmt =  parseDeclarationInternal(";");
		statement* stmt =  parseStatement();
		stmt->line = getToken().lineNumber;
		if (stmt->statementType != dStatement && stmt->statementType != eStatement)
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber, "Expected declaration statement");
		}
		
		if (stmt->statementType == dStatement)
		{
			floop->fromCondition->push_back(stmt);
		}
	
		addStatementDistributingVariables(stmt);
	}
	
	if(!peekToken(";"))
	{
		statement* stmt= parseStatement();
		stmt->line = getToken().lineNumber;
		if (stmt->statementType != oStatement && stmt->statementType != eStatement)
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber, "Expected conditional statement");
		}
		
		if (stmt->statementType == oStatement)
		{
			floop->toCondition->push_back(stmt);
		}
		
		addStatementDistributingVariables(stmt);
	}
	
	if(!peekToken("]"))
	{
		statement* stmt= parseStatement();
		stmt->line = getToken().lineNumber;
		if (stmt->statementType != oStatement && stmt->statementType != eStatement)
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber, "Expected operational statement");
		}
		
		if (stmt->statementType == oStatement)
		{
			floop->stepCondition->push_back(stmt);
		}
		
		addStatementDistributingVariables(stmt);
	}
	
	popToken();
	RequireValue("]", "Expected ] and not ", true);
	
	tmpScope = false;
	popToken();
	RequireValue("{", "Expected { and not ", true);
	

	while(!peekToken("}"))
	{
		cerr << "HEREEEEEE" << endl;
		statement* stmt = parseStatement();
		addStatementDistributingVariables(stmt);
		floop->body->push_back(stmt);
		exit(1);
	}
	
	popToken();

	decreaseScope();
		
	return floop;
};

int parser::addStatementDistributingVariables(statement* stmt)
{
	
	for (unsigned int i=0; i < currentFunction.body->appendBeforeList.size(); i++)
	{
		//if ( ((dddd*)currentFunction.body->appendAfterList.at(i)).at(0).arrayIndex != NULL)
		if ( (termStatment*)((distributingVariablesStatement*)currentFunction.body->appendBeforeList.at(i))->variable.arrayIndex != NULL)
		{
			cerr << currentFunction.body->appendBeforeList.at(i) << endl << flush;
			stmt->distStatements.push_back((distributingVariablesStatement* )currentFunction.body->appendBeforeList.at(i));
			//currentFunction.body->append_after(dvstatement);
			currentFunction.body->appendBeforeList.erase(currentFunction.body->appendBeforeList.begin()+i);
			i--;
		}
	}
	for (unsigned int i=0; i < currentFunction.body->appendAfterList.size(); i++)
	{
		//if ( ((dddd*)currentFunction.body->appendAfterList.at(i)).at(0).arrayIndex != NULL)
		if ( (termStatment*)((distributingVariablesStatement*)currentFunction.body->appendAfterList.at(i))->variable.arrayIndex != NULL)
		{
			stmt->distStatements.push_back((distributingVariablesStatement* )currentFunction.body->appendAfterList.at(i));
			//currentFunction.body->append_after(dvstatement);
			currentFunction.body->appendAfterList.erase(currentFunction.body->appendAfterList.begin()+i);
			i--;
		}
	}
	
	return 0;
}

statement* parser::parseFunction() // add functions prototypes to userFunctions Array too :)
{
	cerr << "PARSING FUNC" << endl;
	for (unsigned int i =0; i < working_tokens->size(); i++)
	{
		cerr << working_tokens->at(i).value << endl;
	}
		
	// always keep global ids
	Array<idInfo>* tmpIdentifiers = identifiers;
	identifiers			= new Array<idInfo>;
	globalNoDist = true;
	for(unsigned int i =0; i < tmpIdentifiers->size(); i++)
	{
		if(tmpIdentifiers->at(i).global)
		{
			identifiers->push_back(tmpIdentifiers->at(i));
		}
	}
	delete tmpIdentifiers;

	
	popToken();
	
	cerr << "TOKEN :" << getToken().value << endl;
	RequireType("identifier", "Invalid function declaration", false);
	
	funcInfo Funcinfo;
	Funcinfo.protoType = false;
	ast_function Func;
	currentFunction.scope = scope;
	
	////checkToken(USERFUNCTION, "can not re-define a builtin function : ", true);
	////checkToken(BUILTINFUNCTION, "Predefined function : ", true);
	
	Funcinfo.name = getToken().value;
	Funcinfo.type = USERFUNCTION;
	
	currentFunction.name = getToken().value;
		
	popToken();
	RequireValue("(", "Expected \"(\" and not "+ getToken().value + " after function definition ", false);
		
	if (popToken().value != "->")
	{
		while (getToken().value != "->")
		{
			//FIX102
			// should make a Array of declaration satements and add the statements to it, for the compiler to pase them
			// with their inizialized values ?
			declareStatement* stmt = (declareStatement*)parseDeclarationInternal("->");
			stmt->line = getToken().lineNumber;
			for ( unsigned int i = 0; i < stmt->identifiers->size(); i++ )
			{
				idInfo Id;
				Id.name = (stmt->identifiers->at(i)).name;
				Id.parent = (stmt->identifiers->at(i)).parent;
				Id.type = (stmt->identifiers->at(i)).type;
				Id.type = stmt->type;
				Id.scope = stmt->scope;
				Id.index = index;
				currentFunction.parameters->push_back(Id);
				Funcinfo.parameters->push_back(pair<string,bool>(Id.type,stmt->value));
			}
		}
	}
	
	RequireValue("->", "Expected \"->\" and not "+getToken().value + " after function definition ", false);
	
	//if ( (tokens->at(index)).value ==  "->" )
	//{
		if (popToken().value ==  ")")
		{
			currentFunction.returnIDType = "NIL";
			currentFunction.returnID = "NIL";
		}
		else// if ( (tokens->at(index)).value ==  "|" )
		{
			//index--;
			while (getToken().value != ")")
			{
				declareStatement* stmt = (declareStatement*)parseDeclarationInternal(")");
				stmt->line = getToken().lineNumber;
				for ( unsigned int i = 0; i < stmt->identifiers->size(); i++ )
				{
					stmt->line = getToken().lineNumber;
					currentFunction.returnIDType = stmt->type;
					Funcinfo.returnType = stmt->type;
					currentFunction.returnID = stmt->identifiers->at(0).name;
					if (stmt->identifiers->size() > 1)
					{
						displayWarning(fName, getToken().lineNumber, getToken().columnNumber,"Function : " + Funcinfo.name + " : more than one variable for return, only the first will be used");
						break;
					}
				}
			}
		}
		//else
		//{
		//	displayError(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber,"Expected return identifier or ) ");
		//}
	//}
	//else
	//{
	//	displayError(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber,"Expected -> and not: " + (tokens->at(index)).value);
	//}
	
	globalNoDist = false;
	// continue to body !
	popToken();
	if (getToken().value == ";")
	{
		//check if it already exists ??
		// different functions with same name should exist
		// check if same parameters, then check for prototype
		Funcinfo.protoType = true;
		if (isFunction(Funcinfo.name, false))
		{
			for (unsigned int i =0; i < functionsInfo->size(); i++)
			{
				//FIX21
				if (functionsInfo->at(i).name == Funcinfo.name)
				{
					if ( *functionsInfo->at(i).parameters == *Funcinfo.parameters)
					{
						if (functionsInfo->at(i).protoType)
						{
							displayError(fName, getToken().lineNumber, getToken().columnNumber,"Pre-defined function prototype : " +Funcinfo.name);
						}
						else // if(function prototype already exists)
						{
							displayError(fName, getToken().lineNumber, getToken().columnNumber,"Pre-defined function, no need for prototype : "+Funcinfo.name);
						}
					}
				}
			}
		}
		functionsInfo->push_back(Funcinfo);
	}
	else if (getToken().value == "{")
	{
		globalNoDist = false;
		Funcinfo.protoType = false;

		//FIX , isFucntion again ?
		if (isFunction(Funcinfo.name, false))
		{
			for (unsigned int i =0; i < functionsInfo->size(); i++)
			{
				if (functionsInfo->at(i).name == Funcinfo.name &&
					*functionsInfo->at(i).parameters == *Funcinfo.parameters)
				{
					if (!functionsInfo->at(i).protoType)
					{
						displayError(fName, getToken().lineNumber, getToken().columnNumber,"Pre-defined function : "+Funcinfo.name);
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
				
		Array<statement*>* declarations = new Array<statement*>();
		
		while (popToken().value != "}")
		{
			statement* stmt = parseStatement();
					
			if(stmt != NULL)
			{
				if (stmt->statementType == dStatement)
				{
					
					if (((declareStatement*)stmt)->Initilazed)
					{
						declareStatement* decStatement = ((declareStatement*)stmt);
						statement* value = decStatement->value;
						decStatement->value = NULL;
						//FIX1001 if function is distributed , else put it in pushModified(os->op, id); currentFunction.body
						declarations->push_back(decStatement);
						//functionStatementsCount++;
						for (unsigned int i=0 ; i < decStatement->identifiers->size(); i++)
						{
							//functionStatementsCount++;
							//fix add type and scope ....
							operationalStatement* os = new operationalStatement();
							os->left = new termStatment(decStatement->identifiers->at(i).name);
							os->op = "=";
							os->right = value;
							os->type = stmt->type;
							os->scope = ((declareStatement*)decStatement)->scope;
							idInfo id(decStatement->identifiers->at(i).name, os->scope, os->type, NULL);
							id.distributedScope = distributedScope;
							id.backProp = (bool)findIDInfo(id, BACKPROB);
							id.recurrent = (bool)findIDInfo(id, RECURRENT);
							id.channel = (bool)findIDInfo(id, CHANNEL);
							id.array = (bool)findIDInfo(id, ARRAY);
							id.noblock = (bool)findIDInfo(id, NOBLOCK);
							id.distributed = (bool)findIDInfo(id, DISTRIBUTED);
							id.type = (bool)findIDInfo(id, TYPE);
							
							currentStatement = os;

							pushModified(os->op, id);
							currentFunction.body->push_back(os);
							functionStatementsCount++;
						}
						
						statement* stmt2 = currentFunction.body->at(currentFunction.body->size()-(decStatement->identifiers->size()));
						stmt2->distStatements = stmt->distStatements;
						stmt->distStatements.clear();
					}
					else
					{
						declarations->push_back(stmt);
						//functionStatementsCount++;
					}
				}
				else
				{
					if (stmt->statementType == rStatement)
					{
						currentFunction.body->clearQueue();
					}
					currentFunction.body->push_back(stmt);
					functionStatementsCount++;
				}
			}
		}
		cerr << "ARSING :" << getToken().value << endl;
		//let's fix all backprob variables :)
		for( unsigned int i =0; i< dvList->size(); i++ )
		{
			if( (dvList->at(i))->type == distributingVariablesStatement::DEPS &&
				(dvList->at(i))->variable.backProp  )
			{
				for( unsigned int k =i+1; k< dvList->size(); k++ )
				{
					if( (dvList->at(k))->type != distributingVariablesStatement::MODS ||
					  ( (dvList->at(k))->variable.distributedScope == (dvList->at(i))->variable.distributedScope) ||
					  ( (dvList->at(k))->variable.name != (dvList->at(i))->variable.name) )
					  {
						  continue;
					  }
					  dvList->at(i)->dependencyNode = (dvList->at(k))->variable.distributedScope;
					  break;
				}
			}
		}

		//here :D fix all functions to fill the type field , and add type to functionInfo too
		functionsInfo->push_back(Funcinfo);
		
		//before this, loop through origianl body and push the modfifyNotify statements at their right position i
		
		//fix what if , two initiated variables are pushed at the same place ?

		termStatment* termstatement = new termStatment( "DM14FUNCTIONBEGIN", "NIL" );
		termstatement->scope = scope;
		//termstatement->scopeLevel = scopeLevel;
		//termstatement->arrayIndex = aIndex;
		//termstatement->identifier = true;
		declarations->push_back(termstatement);
		for (unsigned int i=0; i < currentFunction.body->size(); i++)
		{
			declarations->push_back(currentFunction.body->at(i));
		}
		
		//delete Func.body;

		currentFunction.body = declarations;
		functions->push_back(currentFunction);
	}
	else
	{
		RequireValue("{", "Expected { or ; and not : ", true);
	}
	
	currentFunction = ast_function();
	decreaseScope();
	
	distModifiedGlobal	= new Array<idInfo>;
	//distIdentifiers	= new Array<idInfo>;
	diststatementTemp	= new distStatement();
	modifiedVariablesList = Array < pair<idInfo,int> >();
	//FIX only increment on if conditions, loops and case... ?
	increaseScope(NULL);
	distributedScope = 0;
	functionStatementsCount = 0;
	
	return NULL;
};

int parser::nextIndex()
{
	if ( (unsigned) index == ( tokens->size() - 1 ) )
	{
		displayError(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber,"Unexpected EOF");
	}
	//nextIndex();
	index++;
	return index;
};

bool parser::isIdentifier(const string& ID)
{
	for ( unsigned int i =0; i< identifiers->size(); i++ )
	{
		if ( ID == (identifiers->at(i)).name )
		{
			return true;
		}
	}
	
	return false;
};




int parser::findIDInfo(const idInfo& ID, const int& returnType)
{	
	for ( unsigned int i =0; i< identifiers->size(); i++ )
	{
		//if ( ID.name == (identifiers->at(i)).name && ID.parent && ID.parent->name == (identifiers->at(i)).parent->name)
		
		string parentName;
		string identifiersParentName;
		
		if(ID.parent)
		{
			parentName = ID.parent->name;
		}
		
		if((identifiers->at(i)).parent)
		{
			identifiersParentName = (identifiers->at(i)).parent->name;
		}
		
		if (ID.name == (identifiers->at(i)).name && parentName == identifiersParentName)
		{
			if (returnType == SCOPE)
			{
				return (identifiers->at(i)).scope;
			}
			else if (returnType == INITIALIZED)
			{
				return (identifiers->at(i)).initialized;
			}
			else if (returnType == ARRAY)
			{
				return (identifiers->at(i)).array;
			}
			else if (returnType == ARRAYSIZE)
			{
				return (identifiers->at(i)).size;
			}
			else if (returnType == TMPSCOPE)
			{
				return (identifiers->at(i)).tmpScope;
			}
			else if (returnType == DISTRIBUTED)
			{
				return (identifiers->at(i)).distributed;
			}
			else if (returnType == BACKPROB)
			{
				return (identifiers->at(i)).backProp;
			}
			else if (returnType == RECURRENT)
			{
				return (identifiers->at(i)).recurrent;
			}
			else if (returnType == CHANNEL)
			{
				return (identifiers->at(i)).channel;
			}
			else if (returnType == NOBLOCK)
			{
				return (identifiers->at(i)).noblock;
			}
			else if (returnType == GLOBAL)
			{
				return (identifiers->at(i)).global;
			}
			else
			{
				return 0;
			}
		}
	}
	
	
	if (returnType == SCOPE)
	{
		return -1;
	}
	else if (returnType == ARRAY)
	{
		return -1;
	}
	else if (returnType == ARRAYSIZE)
	{
		return -1;
	}
	
	return 0;
};

string parser::findIDType(const idInfo& ID, const string& classID)
{
	if(classID.size())
	{
		for(unsigned int i =0; i < datatypes.size(); i++)
		{
			if(datatypes.at(i).typeID == classID && (datatypes.at(i).classType || datatypes.at(i).enumType))
			{
				for (unsigned int k =0; k < datatypes.at(i).memberVariables.size(); k++)
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
		for ( unsigned int i =0; i< identifiers->size(); i++ )
		{
			//if ( ID.name == identifiers->at(i).name && !identifiers->at(i).parent.size())
			if ( ID.name == identifiers->at(i).name && !identifiers->at(i).parent)
			{
				return (identifiers->at(i)).type;
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
	for ( unsigned int i = 0; i < functionsInfo->size(); i++)
	{	
		if (func == (functionsInfo->at(i)).name)
		{
			if ((functionsInfo->at(i)).type == BUILTINFUNCTION)
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
		if (recursiveCurrent)
		{
			if (func == currentFunction.name)
			{
				return true;
			}
		}
	
		for ( unsigned int i = 0; i < functionsInfo->size(); i++)
		{	
			if (func == (functionsInfo->at(i)).name)
			{
				if ((functionsInfo->at(i)).type == USERFUNCTION)
				{
					return true;
				}
			}
		}
	}
	
	return classHasMemberFunction(classID, func);
};

bool parser::isImmediate(const string& immediate)
{
	if (!  (isOperator(immediate) || isKeyword(immediate) || 
			isDataType(immediate) || isFunction(immediate, true) ||
			isIdentifier(immediate)))
	{
		return true;
	}
	return false;
};

// should return a vector of all matching leafs ?
statement* parser::findTreeNode(statement* opstatement, int statementType) 
{
	if(!opstatement)
	{
		return NULL;
	}
	
	if(opstatement->statementType == statementType)
	{
		return opstatement;
	}
	
	statement* result = NULL;
	
	if(opstatement->statementType == oStatement)
	{
		if (((operationalStatement*)opstatement)->left)
		{
			result = findTreeNode(((operationalStatement*)opstatement)->left, statementType);
		}
			
		if (!result && ((operationalStatement*)opstatement)->right)
		{
			result = findTreeNode(((operationalStatement*)opstatement)->right, statementType);
		}
	}
	
	return result;
}



int parser::searchVariables(statement* opstatement, int depencyType, string op)
{
	
	//distributingVariablesStatement::DEPS
	if(!opstatement)
	{
		return 1;
	}
	
	if(opstatement->statementType == tStatement)
	{
		idInfo* id = ((termStatment*)opstatement)->id;
		if (id)
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
	
		
	if(opstatement->statementType == oStatement)
	{
		if (((operationalStatement*)opstatement)->left)
		{
			if ( ((operationalStatement*)opstatement)->op == "." || ((operationalStatement*)opstatement)->op == "::")
			{
				if ( ((operationalStatement*)opstatement)->left->statementType == oStatement)
				{
					statement* leftLeft = ((operationalStatement*)((operationalStatement*)opstatement)->left)->left;
					searchVariables(leftLeft, depencyType, op);
				}
			}
			else
			{
				searchVariables(((operationalStatement*)opstatement)->left, depencyType, op);
			}
		}
			
		if (((operationalStatement*)opstatement)->right)
		{
			searchVariables(((operationalStatement*)opstatement)->right, depencyType, op);
		}
	}
	
	return 0;
}


statement* parser::parseExpressionStatement()
{
	
	cerr << "ALL" << endl;
	for(unsigned int i =0; i < working_tokens->size(); i++)
	{
		cerr <<  working_tokens->at(i).value << endl;
	}
	
	cerr << "END:" << working_tokens->at(0).value << getToken(0).type << endl;
	
	
	statement* result = NULL;
	
	if(working_tokens->at(working_tokens->size()-1).value == ";")
	{
		popToken();
		result = parseOpStatement(0, working_tokens->size()-2,
								  "-2", scope, parentStatement);
	}
	else
	{
		result = parseOpStatement(0, working_tokens->size()-1,
								  "-2", scope, parentStatement);
	}
	
	return result;
}

statement* parser::parseOpStatement(const int& from, const int& to, 
									const string& stmtType, const int& scopeLevel, statement* caller, 
									idInfo* parent, const string& parentOp)
{
	int plevel =0; // percedence levels
	
	string classID = "";
	
	if(parent)
	{	
		classID = parent->type;
	}
	
	
	// [ overall check for (s and )s
	for ( int i = from; i <= to ; i++ )
	{
		if (getToken(i).value == "(" )
		{
			++plevel;
		}
		else if (getToken(i).value == ")" )
		{
			--plevel;
		}
	}
	
	if ( plevel > 0 )
	{
		displayError(fName, getToken().lineNumber, getToken().columnNumber," missing \")\" ");
	}
	else if ( plevel < 0 )
	{
		displayError(fName, getToken().lineNumber, getToken().columnNumber," missing \"(\" ");
	}
	
	// big (..) statement
	if(getToken(from).value == "("  && getToken(to).value == ")")
	{
		plevel = 0;
		bool bigStatement = true;
		for ( int i = from; i <= to ; i++ )
		{
			if (getToken(i).value == "(" )
			{
				++plevel;
			}
			else if (getToken(i).value == ")" )
			{
				--plevel;
			}
			
			if(plevel == 0 && i < to)
			{
				bigStatement = false;
				break;
			}
		}
		
		if(bigStatement)
		{
			return parseOpStatement(from+1, to-1, stmtType, scopeLevel+1, caller);
		}
	}
	// ]
	
	int origiIndex = index;
	bool classMember = false;

	operationalStatement* opstatement = new operationalStatement;
	opstatement->line = getToken().lineNumber;
	opstatement->scope = scope;
	opstatement->scopeLevel = scopeLevel;
		
	// [ split the opstatement to left and right if there is an operator ;)
	plevel= 0;
	for ( int i = from; i <= to; i++ )
	{
		cerr << "FROM:" << from << " TO:" << to << endl;
		if(caller)
		{
			currentStatement = caller;
		}
		else
		{
			currentStatement = opstatement;
		}
	
		if ((tokens->at(i)).value == "(")
		{
			plevel++;
			continue;
		}
		else if ( (tokens->at(i)).value == ")" )
		{
			plevel--;
			continue;
		}
		
		if (plevel == 0)
		{
			if ((getToken(i).type == "operator") &&
			     !(getToken(i).value == ")" || getToken(i).value == "(" || getToken(i).value == "["  || getToken(i).value == "]"))
			{		
				opstatement->op = getToken(i).value;
				
				if(i != from) // not operator like ++x
				{
					opstatement->left = parseOpStatement(from, i-1, stmtType, opstatement->scopeLevel, currentStatement, parent, opstatement->op);
				}
				
				
				idInfo* id = NULL;
				// should we loop inside to get the term ?
				statement* res = findTreeNode(opstatement->left, tStatement);
				
				if(res)
				{
					id  = ((termStatment*)res)->id;
				}
									
				if(opstatement->op == "." ||  opstatement->op == "::" )
				{
					if(!id)
					{
						displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"false class member !");
					}
					
					if(i == to)
					{
						displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"incmplemete data member access !");
					}
					
					//opstatement->op = "::";
					///opstatement->right = parseOpStatement(i+1, to, "-2", opstatement->scopeLevel, currentStatement, true, opstatement->left->type, id->name);
					opstatement->right = parseOpStatement(i+1, to, "-2", opstatement->scopeLevel, currentStatement, id);
					opstatement->left->type = opstatement->right->type;
					
					//statement* res = findTreeNode(opstatement->right, tStatement);
					//idInfo* rightID  = ((termStatment*)res)->id;
					//rightID->arrayIndex = id->arrayIndex;
					//id->arrayIndex = NULL;
					
					//if ( stmtType != "-2" && !hasTypeValue(stmtType, findIDType(idInfo(variableName, 0, "", NULL)) ))
					//{
					//	displayError(fName, (tokens->at(i+2)).lineNumber,(tokens->at(i+2)).columnNumber,"Wrong type variable : " + variableName + " expected : " + opstatement->type);
					//}
				}
				else if(i < to)
				{
					//opstatement->right = parseOpStatement(i+1, to, "-2", opstatement->scopeLevel, currentStatement, false, "", "");
					opstatement->right = parseOpStatement(i+1, to, "-2", opstatement->scopeLevel, currentStatement);
				}
				
				if(opstatement->left)
				{
					opstatement->type = opstatement->left->type;
				}
				else if(opstatement->right)
				{
					opstatement->type = opstatement->right->type;
				}
				else
				{
					displayError("could not determine statement type");
				}
								
				if (isbinOperator(opstatement->op))
				{
					if(opstatement->op == "==" || opstatement->op == "||" || opstatement->op == ">" || opstatement->op == ">"
					 || opstatement->op == ">=" || opstatement->op == "<=" || opstatement->op == "&&")
					{
						opstatement->type = "bool";
					}
					
					if(!opstatement->right)
					{
						displayError(fName, getToken(i).lineNumber, getToken(i).columnNumber,"Missing right operand");
					}
					
					//FIX
					/*else if (caller && caller->statementType == oStatement && !parent && !typeHasOperator(((operationalStatement*)caller)->op, opstatement->type) && opstatement->op != "::"  )
					{
						cerr  << opstatement->op << endl;
						cerr << tokens->at(from).value << endl;
						cerr << tokens->at(to).value << endl;
						displayError(fName, (tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"typee \"" +opstatement->type + "\" does not support operator " + ((operationalStatement*)caller)->op);
					}*/
					else if (!typeHasOperator(opstatement->op, opstatement->type) && opstatement->op != "." && opstatement->op != "::" )
					{
						displayError(fName, getToken(from).lineNumber, getToken(from+2).columnNumber,"type \"" +opstatement->type + "\" does not support operator " + opstatement->op);
					}
					
					searchVariables(opstatement->left, distributingVariablesStatement::MODS, opstatement->op);
					searchVariables(opstatement->right, distributingVariablesStatement::DEPS);
				}
				else if(isSingleOperator(opstatement->op))
				{
					if (!typeHasOperator(opstatement->op, opstatement->type) && (opstatement->op != "@") )
					{
						displayError(fName, getToken(from).lineNumber, getToken(from+2).columnNumber,"type \"" +opstatement->type + "\" does not support operator " + opstatement->op);
					}
					searchVariables(opstatement->left, distributingVariablesStatement::MODS, opstatement->op);
				}
				else if (isCoreOperator(opstatement->op))
				{
					if(opstatement->op == "@")
					{
						//construct node address statement
						// should we loop inside to get the term ?
						statement* res = findTreeNode(opstatement->right, tStatement);
				
						idInfo* rightId = NULL;
							
						if(res)
						{
							rightId  = ((termStatment*)res)->id;
						}
						else
						{
							displayError("Coundl't find node address");
						}
						
						// variable@node
						if(opstatement->left && opstatement->left->statementType == tStatement) // variable@node
						{
							
							if(rightId)
							{
								id->requestAddress = rightId->name;
							}
							else
							{
								displayError("Coundl't find node address");
							}
							
							if(opstatement->right->statementType == oStatement)
							{
								operationalStatement* stmt = (operationalStatement*) opstatement->right;
								searchVariables(opstatement->left, distributingVariablesStatement::MODS, stmt->op);
								searchVariables(opstatement->left, distributingVariablesStatement::DEPS);
								opstatement->op  = stmt->op;
								stmt->op = "";
								delete stmt->left;
								stmt->left = NULL;
							}				
							else if(opstatement->right->statementType == tStatement)
							{
								 //opstatement->right->Array<distributingVariablesStatement*>* distStatements;
								/*for (int i =0; i < opstatement->right->distStatements->size(); i++)
								{
									cout << " IIIIIIIII :|" << i << endl;
									opstatement->distStatements->push_back(opstatement->right->distStatements->at(i));
								}*/
								delete opstatement->right;
								opstatement->right = NULL;
								opstatement->op  = "";
							}					
							//stmt->left = stmt->right;
							//stmt->right = NULL;
							
							if ( (parentOp != "=" && parentOp != "+=" && parentOp != "-=" && parentOp != "*=" && parentOp != "/=" && parentOp != "++" && parentOp != "--" && parentOp != "@") || opstatement->op == "")
							{
								idInfo* leftId = NULL;
								// should we loop inside to get the term ?
								statement* res = findTreeNode(opstatement->left, tStatement);
					
								if(res)
								{
									leftId  = ((termStatment*)res)->id;
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
							for(unsigned int l =0; l < currentStatement->distStatements.size(); l++)
							{
								if(currentStatement->distStatements.at(l)->type == distributingVariablesStatement::DEPS)
								{
									 if (currentStatement->distStatements.at(l)->variable.name == rightId->name)
									 {
										 currentStatement->distStatements.remove(l);
										 break;
									 }
								}
							}
						}
						opstatement->type="string";
					}
				}
				else
				{
					displayError("Operator is not defined as single or binary  : " + getToken(i).value);
				}
				
				if(   !typeHasOperator(opstatement->op, opstatement->type) 
				   && !(opstatement->left && opstatement->op == ".")
				   && opstatement->op != "@")
				{
					//displayError(fName, (tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"type \"" +opstatement->type + "\" does not support operator " + opstatement->op);
				}
				
				/*
				if ( !hasTypeValue(opstatement->type, opstatement->right->type) && !classMember && stmtType!= "-2")
				{
					displayError(fName, (tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"Wrong operands type : " + opstatement->right->type + " with " +opstatement->type);
				}
				else if ( !hasTypeValue(stmtType, opstatement->type) && stmtType != "-2")
				{
					displayError(fName, (tokens->at(from)).lineNumber,(tokens->at(from+2)).columnNumber,"Wrong operands type : " + stmtType + " with " +opstatement->type);
				}*/
				return opstatement;
			}
		}
	}
	// ]
	
	// proceed to real statement
	// [ find statement type
				cerr << getToken(from).type << endl;

	if (stmtType == "NIL")
	{
		displayError(fName, getToken().lineNumber, getToken().columnNumber,"Wrong type variable : " + getToken().value);
	}
	else if (stmtType == "-2")
	{
		if(isFunction(getToken(from).value, true, classID))
		{
			opstatement->type  = getFunc(getToken(from).value, classID).returnType;
		}
		else if (isDataType(getToken(from).value))
		{
			opstatement->type  = getToken(from).value;
		}		
		else if (getToken(from).type == "identifier")
		{
			opstatement->type  = findIDType(idInfo(getToken(from).value, 0, "", NULL), classID);
		}
		else if (isImmediate(getToken(from).value) )
		{
			opstatement->type  = getToken(from).type;
		}
		else if ((isSingleOperator(getToken(from).value) || getToken(from).value == "@") && from < to)
		{
			if (isImmediate(getToken(from+1).value) )
			{
				opstatement->type  = getToken(from+1).type;
			}
			else if (getToken(from+1).type == "identifier" )
			{
				opstatement->type  = findIDType(idInfo(getToken(from+1).value, 0, "", NULL), classID);
			}
			else
			{
				displayError(fName, getToken(from).lineNumber,getToken(from).columnNumber,"Did not expect "+ getToken(from).value);
			}
		}
		else
		{
			displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"Did not expect "+ getToken(from).value);
		}
	}
	else
	{
		opstatement->type  = stmtType;
	}
	
	cerr << "TYPE:" << opstatement->type << endl;
	// ]
	
	//if (!typeHasOperator(tokens->at(currentIndex).value, findID(tokens->at(currentIndex+1).value, classID).type))
	//{
	//	displayError(fName, (tokens->at(currentIndex)).lineNumber,(tokens->at(currentIndex)).columnNumber,"single operator " + (tokens->at(currentIndex)).value + " is not supported by statements of type " + findID(tokens->at(currentIndex+1).value, classID).type);
	//	}
	
	
	// [ find first term, function, variable or immediate ...
	
	statement* aIndex = NULL;
	// if identifier , then it might be variable or user/builtin function
	if ( getToken(from).type == "identifier" ||  isDataType(getToken().value) )//(tokens->at(currentIndex)).type == "datatype")
	{
		// function !
		if (isBuiltinFunction(getToken(from).value) || isUserFunction(getToken(from).value, true) 
			|| (parent && classHasMemberFunction(classID, getToken(from).value)))
		{
			statement* stmt = NULL;
			if(parent)
			{
				if(opstatement->type != classID)
				{
					stmt = parseFunctionCall(false, opstatement->type, classID);
				}
				else
				{
					stmt = parseFunctionCall(false, "", classID);
					stmt->type = classMemberFunctionType(classID, getToken(from).value);
				}
			}
			else
			{
				stmt = parseFunctionCall(false, opstatement->type);
			}
			delete opstatement;
			stmt->distStatements = opstatement->distStatements;
			return stmt;
		}
		// variable !
		else
		{
			//string variableName = tokens->at(from).value;
			string variableName = getToken(from).value;
			
			if (isDataType(variableName))
			{
				;
			}
			else if(parent && !classHasMemberVariable(classID, variableName))
			{
				displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"class " + classID + " has no member : " + variableName);
			}
			else if(parent && classHasMemberVariable(classID, variableName))
			{
				if(getClassMemberVariable(classID, variableName).classifier != PUBLIC)
				{
					displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,variableName + " is a private class member variable of class type : " + classID);
				}
			}
			else if ( findIDType(idInfo(variableName, 0, "", NULL)) == "NIL" && !parent)
			{
				//fix30 fix the +1 its dangerous
				if (getToken(from+1).value == "(")
				{
					displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"Un-defined function : " + variableName);
				}
				else
				{
					displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"Un-defined variable : " + variableName);
				}
			}

			//classMember = isClass(opstatement->type);
			classMember = isClass(classID);
			termStatment* termstatement = new termStatment( variableName, opstatement->type );
			
			if( findIDType(idInfo(variableName, 0, "", NULL)) != "NIL")
			{
				termstatement->type = findIDType(idInfo(variableName, 0, "", NULL));
			}
			termstatement->scope = scope;
			termstatement->scopeLevel = scopeLevel;
			termstatement->identifier = true;
			termstatement->size = findIDInfo(idInfo(variableName , 0, "", NULL), ARRAYSIZE);
						
			if ( getToken(from).value == "[")
			{
				if (!findIDInfo(idInfo(variableName , 0, "", NULL), ARRAY))
				{
					displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber, variableName + " is not an array, invalid use of indexing");
				}
				termstatement->arrayIndex = parseConditionalExpression(currentStatement);
				aIndex = termstatement->arrayIndex;
			}
			
				
			index = origiIndex;

			//idInfo* id = new idInfo(termstatement->term, opstatement->scope, opstatement->type, aIndex);
			idInfo* id = new idInfo(termstatement->term, opstatement->scope, termstatement->type , aIndex);
			id->distributedScope = distributedScope;
			if(parent)
			{
				id->parent = parent;
			}
			id->array = ((bool)findIDInfo(*id, ARRAY) && (aIndex != NULL));
			id->backProp = (bool)findIDInfo(*id, BACKPROB);
			id->recurrent = (bool)findIDInfo(*id, RECURRENT);
			id->channel = (bool)findIDInfo(*id, CHANNEL);
			id->noblock = (bool)findIDInfo(*id, NOBLOCK);
			id->distributed = (bool)findIDInfo(*id, DISTRIBUTED);
			id->global = (bool)findIDInfo(*id, GLOBAL);
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
			
			
			if(isClass(findIDType(idInfo(variableName, 0, "", NULL))))
			{
				termstatement->type = findIDType(idInfo(variableName, 0, "", NULL));
				id->type = termstatement->type;
			}
			else
			{
				termstatement->type = opstatement->type;
			}
			//id.requestAddress = nodeAddress;
			
			termstatement->id = id;
			
			if (parentOp != "=" && parentOp != "+=" && parentOp != "-=" && parentOp != "*=" && parentOp != "/=" && parentOp != "++" && parentOp != "--" && parentOp != "@")
			{
				if(peekToken(from+1).value != "." && peekToken(from+1).value != "::") // do not pus class id, since it will be done by the member anyway !
				{
					pushDependency(*id);
				}
			}
			
			termstatement->distStatements = opstatement->distStatements;
			delete opstatement;
			return termstatement;
		}
	}
	else if ( isImmediate(getToken(from).value)  )
	{
		if ( !hasTypeValue(opstatement->type, getToken(from).type) )
		{
			displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"wrong immediate type : " + getToken(from).value);
		}
		
		termStatment* termstatement = new termStatment(getToken(from).value, opstatement->type);
		termstatement->line = getToken().lineNumber;
		termstatement->scope = scope;
		termstatement->scopeLevel = scopeLevel;
		termstatement->identifier = false;
		termstatement->type = opstatement->type;
		delete opstatement;
		return termstatement;
	}
	else
	{
		displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"Un-known Variable or Function : " + getToken(from).value);
	}
	
	// ]
	
	delete opstatement;
	displayError(fName, getToken(from).lineNumber, getToken(from).columnNumber,"returning null statement ?!!");
	return NULL;
};

funcInfo parser::getFunc(const string& funcID, const string& classID)
{
	if(classID.size())
	{
		if(!classHasMemberFunction(classID, funcID))
		{
			return funcInfo();
		}
		
		for(unsigned int i =0; i < datatypes.size(); i++)
		{
			if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
			{
				for (unsigned int k =0; k < datatypes.at(i).memberFunctions.size(); k++)
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
		for ( unsigned int i = 0; i < functionsInfo->size(); i++)
		{
			if (funcID == (functionsInfo->at(i)).name)
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
		if(!classHasMemberFunction(classID, funcID))
		{
			return funcInfo();
		}	
		
		for(unsigned int i =0; i < datatypes.size(); i++)
		{
			if(datatypes.at(i).typeID == classID && datatypes.at(i).classType)
			{
				for (unsigned int k =0; k < datatypes.at(i).memberFunctions.size(); k++)
				{
					if (datatypes.at(i).memberFunctions.at(k).returnType != returnType && returnType.size())
					{
						continue;
					}
					
					if(datatypes.at(i).memberFunctions.at(k).name == funcID)
					{
						if (!datatypes.at(i).memberFunctions.at(k).parameters->size() && !mparameters->size())
						{
							return datatypes.at(i).memberFunctions.at(k);
						}
						
						for ( unsigned int f = 0, l =0; f < datatypes.at(i).memberFunctions.at(k).parameters->size() && l < mparameters->size(); f++, l++)
						{
							if(mparameters->at(l) != datatypes.at(i).memberFunctions.at(k).parameters->at(f).first)
							{
								break;
							}
							
							if(l ==  mparameters->size()-1)
							{
								if(l == datatypes.at(i).memberFunctions.at(k).parameters->size()-1)
								{
									finfo = datatypes.at(i).memberFunctions.at(k);
									break;
								}
								else if(datatypes.at(i).memberFunctions.at(k).parameters->at(f+1).second)
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
		for ( unsigned int i = 0; i < functionsInfo->size(); i++)
		{
			if (functionsInfo->at(i).returnType != returnType && returnType.size())
			{
				continue;
			}
			
			if ((functionsInfo->at(i)).name == funcID)
			{
				if(!functionsInfo->at(i).parameters->size() && !mparameters->size())
				{
					return functionsInfo->at(i);
				}
				
				for ( unsigned int k = 0, l =0; k < functionsInfo->at(i).parameters->size() && l < mparameters->size(); k++, l++)
				{
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
				 const Array<pair<string,string> >& mincludes, const bool& header,
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
	libs = new Array<statement*>();
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
	//cout << "DELETE " << endl;
	//delete functions;
}


Array< pair<string,string> > parser::getIncludes()
{
	return includes;
};

Array<ast_function>* parser::getFunctions()
{
	return (functions);
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


Array<pair<string,string> > mapcode::getIncludes()
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
			
	for (unsigned int i =0; i < functions->size(); i++)
	{
		string functionHeader;
		functionHeader += functions->at(i).returnIDType + " ";
		functionHeader += functions->at(i).name + "(";
		for (unsigned int k =0; k < functions->at(i).parameters->size(); k++)
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

statement* parser::parseConditionalExpression(statement* caller)
{
	nextIndex();
	RequireValue("[", "Expected [ and not : ", true);
	int from = index+1;
	int to = reachToken("]", false, true, true, false, true) - 1;
	//statement* result = parseOpStatement(from, to, getType(from), 0, stmt);
	statement* result = parseOpStatement(from, to, "-2", 0, caller);
	result->line = (tokens->at(index)).lineNumber;
	nextIndex();
	
	return result;
} 

int parser::increaseScope(statement* stmt)
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


statement* parser::parseThread()
{
	threadStatement* thread = new threadStatement();
	
	currentStatement = thread;
		
	stringstream SS;
	SS << (tokens->at(index)).columnNumber << (tokens->at(index)).lineNumber;	// Number of character on the current line
	thread->Identifier = SS.str();
	
	if(parentStatement && (parentStatement->statementType ==  fLoop || parentStatement->statementType == wLoop))
	{
		displayWarning(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber,"Thread call inside a loop ! careful ");
	}
	
	nextIndex();
	int from = index;
	
	statement* stmt = parseStatement();
	
	for (int i = from; i <= index; i++)
	{
		string parent = "";
		
		if(i-2 >= tokens->size())
		{
			if(peekToken(i-1).value == ".")
			{
				parent = peekToken(i-2).value;
			}
		}
		
		if (tokens->at(i).type == "identifier" && findID(tokens->at(i).value, parent).name.size() && !findIDInfo(idInfo((tokens->at(i)).value, 0, "", NULL), GLOBAL))
		{
			displayError(fName, (tokens->at(i)).lineNumber,(tokens->at(i)).columnNumber,"Only global variables are allowed to be passed to a thread call ! , " + tokens->at(i).value + " is not global");
		}
	}
	
	//FIX if parameters are not global or immediates, give error
	if(stmt->statementType == oStatement)
	{
		//fix also consider parameters to be unique identidier
		thread->parentID = (tokens->at(from)).value;
		thread->classID = findIDType(findID((tokens->at(from)).value));
		thread->ID = (tokens->at(from+2)).value;
		thread->classMember = true;
		thread->returnType = ((operationalStatement*)stmt)->type;
		thread->functioncall = ((operationalStatement*)stmt)->right;
	}
	else if(stmt->statementType == fCall)
	{
		//fix also consider parameters to be unique identidier
		thread->ID = (tokens->at(from)).value;
		thread->returnType = ((functionCall*)stmt)->type;
		thread->functioncall = stmt;
	}
	else
	{
		displayError(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber,"Expected a function call !");
	}
	
	if(stmt->distStatements.size())
	{
		thread->functioncall->distStatements = stmt->distStatements;
	}
	
	thread->type = stmt->type;
	
	//thread->funcinfo = ??;
	
	return thread;
}
statement* parser::parseIF()
{
	// if [ cond ] { statements } else {}
	IF* If = new IF;
	If->line = (tokens->at(index)).lineNumber;
	If->scope = scope;
	
	increaseScope(If);
	
	tmpScope = true;
	If->condition = parseConditionalExpression(If);
	tmpScope = false;
	
	addStatementDistributingVariables(If);
	
	RequireValue("{", "Expected { and not : ", true);
	nextIndex();
			
	while (tokens->at(index).value != "}")
	{
		statement* stmt = parseStatement();
		addStatementDistributingVariables(stmt);
		If->body->push_back(stmt);
		nextIndex();
	}
	
	while ( peekToken("else") )
	{
		nextIndex();
		if ( peekToken("if") )
		{
			nextIndex();
			//nextIndex();
			
			IF* elseIf = new IF;
			elseIf->line = (tokens->at(index)).lineNumber;
			elseIf->scope = scope;
			
			tmpScope = true;
			elseIf->condition = parseConditionalExpression(elseIf);
			tmpScope = false;
			
			//nextIndex();
			RequireValue("{", "Expected { and not : ", true);
			nextIndex();
			
			while (tokens->at(index).value != "}")
			{
				statement* stmt = parseStatement();
				addStatementDistributingVariables(stmt);
				elseIf->body->push_back(stmt);
				nextIndex();
			}
			If->elseIF->push_back(elseIf);
		}
		else
		{
			nextIndex();
			RequireValue("{", "Expected { and not : ", true);
			nextIndex();
			
			while (tokens->at(index).value != "}")
			{
				statement* stmt = parseStatement();
				addStatementDistributingVariables(stmt);
				If->ELSE->push_back(stmt);
				nextIndex();
			}
		}
		//nextIndex();
	}
	
	//else
	//{
	decreaseScope();
	return If;
	//}
	//decreaseScope();
	//return If;
};

statement* parser::parseStruct()
{
	nextIndex();
	
	DatatypeBase Struct;
	//Struct.templateNames = templateNames;
	
	if((tokens->at(index)).type != "identifier")
	{
		displayError(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber,"expected struct identifier");
	}
	else
	{
		
		Struct.setID((tokens->at(index)).value);
	}
	
	nextIndex();
	RequireValue("{", "Expected { and not : ", true);
	nextIndex();
	
	//FIX the wile condition
	while((tokens->at(index)).value != "}")
	{
		
		if((tokens->at(index)).type != "identifier")
		{
			displayError(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber,"expected struct member identifier");
		}
		declareStatement* stmt = (declareStatement*)parseDeclaration();
		
		//extract members ....
		
		for(unsigned int i = 0; i < stmt->identifiers->size(); i++)
		{
			funcInfo finfo;
			finfo.type = DATAMEMBER;
			//finfo.returnType = stmt->identifiers->at(i).type;
			finfo.returnType = stmt->type;
			finfo.name = stmt->identifiers->at(i).name;
			Struct.memberVariables.push_back(finfo);
		}
		nextIndex();
	}
	
	//nextIndex();
	RequireValue("}", "Expected } and not : ", true);

	Struct.addOperator("=");
	Struct.addOperator(".");
	Struct.addTypeValue(Struct.typeID);
	Struct.classType = true;
	
	datatypes.push_back(Struct);
	mapcodeDatatypes.push_back(Struct);
	
	return NULL;
}

statement* parser::parseExtern()
{
	EXTERN* externstatment = new EXTERN;
	externstatment->line = (tokens->at(index)).lineNumber;
	externstatment->scope = scope;
	
	int from = index+1;
	reachToken("endextern", false, true, true, true, true);
	int to = index-1;
	
	for (int i = from; i <= to; i++)
	{
		externstatment->body += (tokens->at(i)).value;
	}
	
	if(scope == 0)
	{
		ExternCodes->push_back(externstatment->body);
	}
	else
	{
		//add externstatment; ??
	}
	
	return externstatment;
};



int parser::extractSplitStatements(Array<statement*>* array, Array<statement*>* splitStatements)
{
	while(splitStatements->size())
	{
		array->push_back(splitStatements->at(0));
		splitStatements->remove(0);
	}
	return array->size();
}
statement* parser::parseWhile()
{
	whileloop* While = new whileloop;
	While->line = (tokens->at(index)).lineNumber;
	While->scope = scope;
	
	increaseScope(While);
	
	tmpScope = true;
	While->condition = parseConditionalExpression(While);
	tmpScope = false;
	
	addStatementDistributingVariables(While);
	
	RequireValue("{", "Expected { and not : ", true);
	nextIndex();
			
	while (tokens->at(index).value != "}")
	{
		statement* stmt = parseStatement();
		addStatementDistributingVariables(stmt);
		While->body->push_back(stmt);
		nextIndex();
	}
	
	decreaseScope();
	
	return While;
};

statement* parser::parseCase()
{
	CASE* Case = new CASE;
	Case->line = (tokens->at(index)).lineNumber;
	Case->scope = scope;	
	
	increaseScope(Case);
	
	Case->condition = parseConditionalExpression(Case);
	
	addStatementDistributingVariables(Case);
	
	RequireValue("{", "Expected { and not : ", true);
	
	while(!peekToken("}"))
	{
		//nextIndex();
		tmpScope = true;
		statement* CCondition = NULL;
		CCondition = parseConditionalExpression(CCondition);
		tmpScope = false;
		
		if (tokens->at(index).value == "}")
		{
			index--;
			break;
		}
		//check if it peek of current ???
		while(tokens->at(index).value != "}" && tokens->at(index).value != "[")
		{
			Case->Body[CCondition].push_back(parseStatement());
			if (!peekToken("}") && !peekToken("["))
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
	int line = getToken().lineNumber;
	int plevel = 1;
	string firstValue = getToken().value; // for scopeLevel
	
	if (actual)
	{
		while (popToken().value.size())
		{
			if (sameLine)
			{
				if (getToken().lineNumber != line )
				{
					if (reportError)
					{
						displayError(fName, getToken().lineNumber, getToken().columnNumber,"Missing " + Char);
					}
					else
					{
						return -1;
					}	
				}
			}
			
			if (scopeLevel)
			{
				if (getToken().value == Char)
				{
					plevel--;
				}
				else if (getToken().value == firstValue)
				{
					plevel++;
				}
			}
			
			if ((beforeEOF && getToken().value == ";") || getToken().value == Char)
			{
				if ( scopeLevel )
				{
					if (plevel == 0)
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
			
			//if ((unsigned) *working_tokens_index == working_tokens->size())
			if ((unsigned) 0 == working_tokens->size())
			{
				if (reportError)
				{
					displayError(fName, getToken().lineNumber, getToken().columnNumber,"Missing " + Char);
				}
			}
		}
	}
	else
	{
		int iterator = 0;
		while ((unsigned) iterator < working_tokens->size())
		{
			if (sameLine)
			{
				if (getToken(iterator).lineNumber != line)
				{
					if (reportError)
					{
						displayError(fName, getToken(iterator).lineNumber, getToken(iterator).columnNumber,"Missing " + Char);
					}
					else
					{
						return -1;
					}
				}
			}

			
			if (scopeLevel)
			{
				if (getToken().value == Char)
				{
					plevel--;
				}
				else if (getToken().value == firstValue)
				{
					plevel++;
				}
			}
			
			if ((beforeEOF && getToken().value == ";") || getToken(iterator).value == Char)
			{
				return iterator;
			}
			
			if ((unsigned) iterator == working_tokens->size()-1)
			{
				if (reportError)
				{
					displayError(fName, getToken(iterator).lineNumber, getToken(iterator).columnNumber,"Missing " + Char);
				}
			}
			
			iterator++;
		}
	}
	return -1; // couldnt find
};

token parser::peekToken(const int& pos)
{
	if(working_tokens->size())
	{
		if ((unsigned) pos < working_tokens->size()-1)
		{
			return working_tokens->at(pos);
		}
	}
	return token();
};


/**
 * @detail peek the first token in the working_tokens vector, which is not poped yet
 */
bool parser::peekToken(const string& str)
{
	if(working_tokens->size())
	{
		if (working_tokens->at(0).value == str)
		{
			return true;
		}
	}
	return false;
};

string parser::getType(const int& Index)
{
	if (isIdentifier(getToken().value))
	{
		return findIDType(idInfo(getToken().value, 0, "", NULL));
	}
	else if (isImmediate(getToken().value))
	{
		return getToken().type;
	}
	else if (isFunction(getToken().value, true))
	{
		return getFunc(getToken().value).returnType;
	}
	else
	{
		displayError(fName, getToken().lineNumber, getToken().columnNumber,"can not get type of : " + getToken().value);
	}
	return "NIL";
};

bool parser::checkToken(int type, string error, bool addtoken)
{
	bool rError = false;
	
	if (type == Function)
	{
		if ( isBuiltinFunction(getToken().value )  || isUserFunction(getToken().value, true) )
		{
			rError = true;
		}
	}
	else if (type == USERFUNCTION)
	{
		if (isUserFunction(getToken().value, true) )
		{
			rError = true;
		}
	}
	else if (type == BUILTINFUNCTION)
	{
		if ( isBuiltinFunction(getToken().value ) )
		{
			rError = true;
		}
	}
	
	if(rError)
	{
		displayError(fName, getToken().lineNumber, getToken().columnNumber,error + getToken().value);
	}
	return rError;
};

bool parser::checkToken(string value, string error, bool addtoken)
{
	bool rError = false;
	if (getToken().value == value)
	{
		rError = true;
	}
	
	if(rError)
	{
		displayError(fName, getToken().lineNumber, getToken().columnNumber,error + getToken().value);
	}
	return rError;
};


bool parser::RequireType(string type, string error, bool addtoken) // add and int for index modification like 1 or -1 :D
{
	bool rError = false;
	
	if (getToken().type != type)
	{
		rError = true;
	}
	
	if(rError)
	{
		if (addtoken)
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber,error + getToken().value);
		}
		else
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber,error);
		}
	}
	return rError;
};

bool parser::RequireValue(string value, string error, bool addtoken) // add and int for index modification like 1 or -1 :D
{
	bool rError = false;
	
	if (getToken().value != value)
	{
		rError = true;
	}
	
	if(rError)
	{
		if (addtoken)
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber,error + getToken().value);
		}
		else
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber,error);
		}
	}
	return rError;
};


// maybe w should move this to scanner ? and modify isFunction instead of reading the stupid manually made maps file !?!
int parser::mapFunctions(const string& package, const string& library)
{
	// get the includes folder full path from the compiler strings

	string fullLibraryName;
	for(unsigned int i = 0; i < includePaths.size(); i++)
	{
		ifstream ifs;			
		
		if ( library == package )
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
	scanner* scner = new scanner(fullLibraryName);
	
	if (!scner->isReady())
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
	
	for (unsigned int i = 0; i < mapTokens->size(); i++)
	{
		
		if (mapTokens->at(i).value == "#")
		{
			int line = mapTokens->at(i).lineNumber;
			for (unsigned int k = i; k <  mapTokens->size(); k++)
			{
				if (mapTokens->at(k).lineNumber != line)
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
		else if (mapTokens->at(i).value == "using") // using
		{
			if(scner->reachToken(i, ";", false, false, true) == -1)
			{
				displayError(scner->getFileName(), -1,-1,"Internal Compiler error, parsing using statement in package : " + fullLibraryName);
			}
			i = scner->reachToken(i, ";", false, false, true);
		}
		else if (mapTokens->at(i).value == "template" || mapTokens->at(i).value == "typename")
		{
			int brace = 0;
			for (unsigned int k = i+1; k < mapTokens->size(); k++)
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
		else if (mapTokens->at(i).value == "class" ||
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
		else if (isDataType(mapTokens->at(i).value) || mapTokens->at(i).value == "void" || mapTokens->at(i).value == "volatile")
		{
			funcInfo finfo = parseCFunction(scner, i, DatatypeBase());
			
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


long parser::parseCClass(scanner* scner, unsigned int start, const Array<string>& templateNames)
{
	DatatypeBase CClass;
	CClass.templateNames = templateNames;
	
	int classifier = 1;
	
	bool TYPEDEF = false;
	bool protoType  = true;
	bool forwardDecelation = false;
	Array<token>* mapTokens = scner->getTokens();
	
	if (mapTokens->at(start).value == "typedef")
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
		else if( semiColon > rightBrace && rightBrace != -1)
		{
			protoType = false;
			CClass.setID(mapTokens->at(start+1).value);
			start = scner->reachToken(start, "{", false, false, true)+1;
		}
		else
		{
			return 0;
		}
		
		/*if(scner->reachToken(start, ";", false, false, true) == start+2)
		{
			start = scner->reachToken(start, ";", false, false, true);
			CClass.setID(mapTokens->at(start-1).value);
			forwardDecelation = true;
		}
		else if (scner->reachToken(start, "{", false, false, true) == start+2)
		{
			protoType = false;
			start = scner->reachToken(start, "{", false, false, true)+1;
			CClass.setID(mapTokens->at(start-2).value);
		}
		else
		{
			return 0;
		}*/
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
	
		
	if (!forwardDecelation)
	{
		for (unsigned int i = start; i < mapTokens->size(); i++)
		{
			if (mapTokens->at(i).value == "public" || mapTokens->at(i).value == "public:")
			{
				if (mapTokens->at(i).value == "public")
				{
					if(scner->reachToken(i, ":", false, false, true) != i+1)
					{
						displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Expected : inside class declaration : " + CClass.typeID + package);
					}
				}
				classifier = PUBLIC;
				i++;
			}
			else if (mapTokens->at(i).value == "private" || mapTokens->at(i).value == "private:")
			{
				if (mapTokens->at(i).value == "private")
				{
					if(scner->reachToken(i, ":", false, false, true) != i+1)
					{
						displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Expected : inside class declaration : " + CClass.typeID + package);
					}
				}
				classifier = PRIVATE;
				i++;
			}
			else if (mapTokens->at(i).value == "protected" || mapTokens->at(i).value == "protected:")
			{
				if (mapTokens->at(i).value == "protected")
				{
					if(scner->reachToken(i, ":", false, false, true) != i+1)
					{
						displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Expected : inside class declaration : " + CClass.typeID + package);
					}
				}
				classifier = PROTECTED;
				i++;
			}
			else if (mapTokens->at(i).value == "}")
			{
				start = i;
				if (scner->reachToken(start, ";", false, false, true) == i+1)
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
				if(finfo.type == DATAMEMBER)
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
	
	for (unsigned int i =0; i < CClass.parents.size(); i++)
	{
		CClass.addTypeValue(CClass.parents.at(i));
	}
	
	/*for (unsigned i = index+1; i < tokens->size(); i++)
	{
		if((tokens->at(i)).value == CClass.typeID)
		{
			(tokens->at(i)).type = "datatype";
		}
	}*/
	
	CClass.protoType = protoType;
	
	if(!protoType)
	{
		for(unsigned int i =0; i < datatypes.size(); i++)
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



funcInfo parser::parseCFunction(scanner* scner, unsigned int start, const DatatypeBase& parentClass)
{
	funcInfo funcinfo;
	Array<token>* mapTokens = scner->getTokens();
	
	//cout << " tempalte : " << parentClass.templateName << endl;
	bool destructor = false;
	bool Enum = false;
	for (unsigned int i = start; i < mapTokens->size(); i++) // function return, data member type
	{
		
		if (mapTokens->at(i).value  == "const" || 
		    mapTokens->at(i).value == "unsigned" || 
		    mapTokens->at(i).value == "&" || 
		    mapTokens->at(i).value == "struct" || 
		    mapTokens->at(i).value == "volatile" )
		{
			continue;
		}
		else if (mapTokens->at(i).value == "static")
		{
			funcinfo.noAutism = true;
			continue;
		}
		else if (mapTokens->at(i).value == "enum")
		{
			Enum = true;
			start = i;
			funcinfo.returnType = "int";
			break;
		}
		else if (mapTokens->at(i).value == "template" || mapTokens->at(i).value == "typename" )
		{
			for (unsigned int k = i+1; k < mapTokens->size(); k++) // skip tempalte stuff
			{
				if(mapTokens->at(k).value == ">")
				{
					i = k;
					break;
				}
			}
			continue;
		}
		else if (mapTokens->at(i).value == "*")
		{
			displayWarning(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Functions should not return a pointer  : " + funcinfo.name + package);
			continue;
		}
		else
		{
			if (scner->tokenAt(i+1).value == "::")
			{
				i++;
				continue;
			}
			
			if (mapTokens->at(i).value== "~")
			{
				destructor = true;
				continue;
			}
			else if (mapTokens->at(i).type != "identifier" && !isDataType(mapTokens->at(i).value))//mapTokens->at(i).type != "datatype")
			{
				displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"could not parse a c++ function retun type, expected identifier and not :  " + mapTokens->at(i).value);
			}
			
			if(parentClass.typeID.size() && mapTokens->at(i).value == parentClass.typeID)
			{
				if (!destructor)
				{
					funcinfo.returnType = parentClass.typeID;
				}
			}
			else
			{
				if (mapTokens->at(i).value == "void")
				{
					displayWarning(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Functions should not have return type of \"void\" adding as int ");
					funcinfo.returnType = "int";
				}
				else
				{
					funcinfo.returnType = getDataType(mapTokens->at(i).value);
				
					if( funcinfo.returnType == "NIL")
					{
						displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Uknown type : " + mapTokens->at(i).value);
					}
				}
			}
			
			if (scner->tokenAt(i+1).value == "*" ||
				scner->tokenAt(i+1).value == "&" ||
				scner->tokenAt(i+1).value == "const")
			{
				i++;
			}
		}
		start = i;
		break;
	}
	
	
	for (unsigned int i = start+1; i < mapTokens->size(); i++) //ignore template parameters
	{
		
		if (mapTokens->at(i).value == "<")
		{
			for (unsigned int k = i+1; k < mapTokens->size(); k++)
			{
				if (mapTokens->at(k).value == ">")
				{
					start = k;
					break;
				}
			}
		}
		break;
	}
	
	for (unsigned int i = start+1; i < mapTokens->size(); i++) // the function/member name
	{
		
		if (mapTokens->at(i).value == "(")
		{
			funcinfo.name = parentClass.typeID;
			start = i;
			break;
		}
		else if (mapTokens->at(i).type != "identifier" && mapTokens->at(i).type != "keyword")
		{
			displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"could not parse a c++ function/member name, expected identifier and not: " + mapTokens->at(i).value);
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
				
		for (unsigned int i = start+1; i < mapTokens->size(); i++) // the function/member name
		{
			if (mapTokens->at(i).type == "identifier")
			{
				funcInfo enumMember;
				enumMember.name = mapTokens->at(i).value;
				enumMember.classifier = PUBLIC;
				enumMember.type = DATAMEMBER;
				//enumMember.returnType = "int";
				enumMember.returnType = funcinfo.name;
				CUSTOMENUM.memberVariables.push_back(enumMember);
			}
			
			if (mapTokens->at(i).value == "}")
			{
				//if (mapTokens->at(i+1).value == ";")
				if (scner->tokenAt(i+1).value == ";")
				{
					
					i++;
				}
				/*for (unsigned int l = 0; l < datatypes.size(); l++)
				{
					if (datatypes.at(i).typeID == "int")
					{
						datatypes.at(i).CEquivalent.push_back(funcinfo.name);
						break;
					}
				}*/
				
				datatypes.push_back(CUSTOMENUM);
				
				funcinfo.classifier = i;
				funcinfo.type = ENUM;
				return funcinfo;
			}
		}
	}
	
	if (mapTokens->at(start+1).value == "[") // array index ?
	{
		for (unsigned int i = start+1; i < mapTokens->size(); i++) 
		{
			if (mapTokens->at(i).value == "]")
			{
				start = i;
				break;
			}
		}
	}
	
	if (mapTokens->at(start+1).value == "[") // two diemnsional array index ?
	{
		for (unsigned int i = start+1; i < mapTokens->size(); i++) 
		{
			if (mapTokens->at(i).value == "]")
			{
				start = i;
				break;
			}
		}
	}
	
	for (unsigned int i = start+1; i < mapTokens->size(); i++) // the member variable
	{
		if (mapTokens->at(i).value == ";")
		{
			if(!funcinfo.name.size())
			{
				displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(tokens->at(i)).columnNumber,"parsing error : " + mapTokens->at(i).value);
			}
			funcinfo.classifier = i;
			funcinfo.type = DATAMEMBER;
			return funcinfo;
		}
		else if (mapTokens->at(i).value == "(")
		{
			start = i;
		}
		
		break;
	}
	
	for (unsigned int i = start+1; i < mapTokens->size(); i++) // function parameterss
	{
		if (mapTokens->at(i).value == "::") // function member dfinition,, ignore all
		{
			int depthLevel = 0;
			for (unsigned int k = i+1; k < mapTokens->size(); k++)
			{
				if (mapTokens->at(k).value == "{")
				{
					depthLevel++;
				}
				else if (mapTokens->at(k).value == "}")
				{
					depthLevel--;
					
					if(depthLevel == 0)
					{
						//if (mapTokens->at(k+1).value == ";")
						if (scner->tokenAt(k+1).value == ";")
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
		if (mapTokens->at(i).value == ")")
		{
			if(i+1 < mapTokens->size())
			{
				i++;
			}
		
			if (mapTokens->at(i).value != ";")
			{
				//displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(tokens->at(i)).columnNumber,"parsing error, expected ; :" + mapTokens->at(i).value);
				displayWarning(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"Function definition in header, be careful !");
				
			}
			
			i--;
			start = i;
			break;
		}
		
		bool init = false;
		int depthLevel = 0;
		for (unsigned int k = i; k < mapTokens->size(); k++) // parameter type
		{
			if (mapTokens->at(k).value  == "const" || 
			    mapTokens->at(k).value == "unsigned" || 
			    mapTokens->at(k).value == "&" || 
			    mapTokens->at(k).value == "volatile")
			{
				continue;
			}
			else if (mapTokens->at(k).value == "*")
			{
				displayWarning(scner->getFileName(), (mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"Functions should not take a pointer  : " + funcinfo.name + package);
				continue;
			}
			else if (mapTokens->at(k).value == ",")
			{
				i = k;
				break;
			}
			else if (mapTokens->at(k).value == ")")
			{
				depthLevel--;
				if(depthLevel == -1)
				{
					i = k-1;
					break;
				}
			}
			else if (mapTokens->at(k).value == "(")
			{
				depthLevel++;
				continue;
			}
			else if (mapTokens->at(k).value == "=")
			{
				//funcinfo.parameters->remove(funcinfo.parameters->size()-1);
				funcinfo.parameters->at(funcinfo.parameters->size()-1).second = true;;
				init = true;
			}
			else
			{
				if(init)
				{
					continue;
				}
					
				if (mapTokens->at(k).value == "void")
				{
					displayWarning(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(k)).columnNumber,"Functions should not have return type of \"void\"  : " + funcinfo.name + package);
				}
				else if (mapTokens->at(k).type != "identifier")
				{
					//cout << k << ":" << i << ":" << start<<endl;
					//displayError(scner->getFileName(), (mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"could not parse a c++ function retun type, expected identifier: " + mapTokens->at(i).value);
				}
				
				if(parentClass.typeID.size() && mapTokens->at(k).value == parentClass.typeID)
				{
					funcinfo.parameters->push_back(pair<string, bool>(parentClass.typeID, false));
				}
				else
				{
					if( getDataType(mapTokens->at(k).value) != "NIL")
					{
						funcinfo.parameters->push_back(pair<string, bool>(getDataType(mapTokens->at(k).value), false));
						found = true;
					}
					else if(parentClass.typeID.size() && !found)
					{
						funcinfo.parameters->push_back(pair<string, bool>(parentClass.typeID, false));
						found = true;
					}
					else if(!found)
					{
						displayError(scner->getFileName(), (mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"Uknown type : " + mapTokens->at(k).value);
					}
				}
			}
		}
		start = i;
	}
	
	for (unsigned int i = start+1; i < mapTokens->size(); i++) // finish variables inizilization :X(x),Y(y),Z(z)
	{
		if (mapTokens->at(i).value == ":")
		{
			for (unsigned int k = i+1; k < mapTokens->size(); k++) 
			{
				if (mapTokens->at(k).value == ")")
				{
					//if((mapTokens->at(k+1).value == ","))
					if (scner->tokenAt(k+1).value == ",")
					{
						k++;
					}
					//else if((mapTokens->at(k+1).value == "{"))
					else if (scner->tokenAt(k+1).value == "{")
					{
						start = k;
						break;
					}
					//else if((mapTokens->at(k+1).value == ";"))
					else if (scner->tokenAt(k+1).value == ";")
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
	
	
		
	for (unsigned int i = start+1; i < mapTokens->size(); i++) // function inizliation;
	{
		int depthLevel = 1;
		if (mapTokens->at(i).value == "{")
		{
			for (unsigned int k = i+1; k < mapTokens->size(); k++) 
			{
				if (mapTokens->at(k).value == "{")
				{
					depthLevel++;
				}
				else if (mapTokens->at(k).value == "}")
				{
					depthLevel--;
				}
				
				if(depthLevel == 0)
				{
					//if (mapTokens->at(k+1).value != ";")
					if (scner->tokenAt(k+1).value != ";")
					{
						displayError(scner->getFileName(), (mapTokens->at(k)).lineNumber,(mapTokens->at(k)).columnNumber,"Unterminated function definition, missing ;");
					}
					
					i = k+1;
					start = i;
					break;
				}
				
			}
		}
		else if (mapTokens->at(i).value == ";")
		{
			start = i;
		}
		else
		{
			displayError(scner->getFileName(), (mapTokens->at(i)).lineNumber,(mapTokens->at(i)).columnNumber,"not expecting : " + (mapTokens->at(i)).value);
		}
		break;
	}
	
	
	funcinfo.type = BUILTINFUNCTION;
	funcinfo.classifier = start;

	return funcinfo;
}


idInfo parser::findID(const string& ID, const string& parentID)
{
	for ( unsigned int i =0; i< identifiers->size(); i++ )
	{
		//if ( ID == (identifiers->at(i)).name && parentID == (identifiers->at(i)).parent)
		if ((identifiers->at(i)).parent && parentID.size())
		{
			if ( ID == (identifiers->at(i)).name && parentID == (identifiers->at(i)).parent->name)
			{
				return identifiers->at(i);
			}
		}
		else if (!(identifiers->at(i)).parent && !parentID.size())
		{
			if (ID == (identifiers->at(i)).name)
			{
				return identifiers->at(i);
			}
		}
	}
	return idInfo();
}












statement* parser::parseDeclaration()
{
	cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE3" << getToken().value << endl;
	cerr << token_index << ":" << working_tokens->size() << endl;
	for(unsigned int i =0; i< working_tokens->size(); i++)
	{
		cerr << "::" << working_tokens->at(i).value << endl;
	}

	popToken();
	statement* result = parseDeclarationInternal(";");
	return result;
}

statement* parser::parseDeclarationInternal(const string& terminal)
{
	declareStatement* decStatement = new declareStatement;
	decStatement->line = getToken().lineNumber;
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

	while (true)
	{
		checkToken(Function, "this is a function name ! :  ", true);
		RequireType("identifier", "Expected \"Identifier\" and not ", true);
		
		if (findIDInfo(idInfo(getToken().value, 0, "", NULL),SCOPE) == decStatement->scope)
		{
			if (findIDInfo(idInfo(getToken().value, 0, "", NULL),TMPSCOPE))
			{
				displayWarning(fName, getToken().lineNumber,getToken().columnNumber,"Pre-defined variable in different scope :  " + getToken().value);
			}
			else if(!tmpScope)
			{
				displayError(fName, getToken().lineNumber, getToken().columnNumber,"Pre-defineddd variable :  " + getToken().value);
			}
		}
		else if (findIDInfo(idInfo(getToken().value, 0, "", NULL),GLOBAL))
		{
			displayError(fName, getToken().lineNumber, getToken().columnNumber,"Pre-defined Global variable :  " + getToken().value);
		}
		
		tempIdentifiers->push_back(idInfo(getToken().value, 0, "", NULL));	
		decStatement->identifiers->push_back(idInfo(getToken().value, 0, "", NULL)) ;
		if(!decStatement->tmpScope)
		{
			distributedVariablesCount++;
		}
		
		popToken();
		
		if(getToken().value != "," )
		{
			break;
		}
		
		popToken();
	}
	
	bool distributed = true;
	
	while(true)
	{
				cerr << "EEEEEEEEEEEEEEEEEEEEEEE 2" << getToken().value << endl;

		if (getToken().value == "noblock" )
		{
			decStatement->noblock = true;
		}
		else if (getToken().value == "recurrent" )
		{
			decStatement->recurrent=true;
		}
		
		else if (getToken().value == "backprop" )
		{
			decStatement->backProp=true;
		}
		else if (getToken().value == "nodist" )
		{
			if(decStatement->recurrent)
			{
				displayError(fName, getToken().lineNumber, getToken().columnNumber, "nodist variable can not be recurrent !");
			}
			
			////displayWarning(fName, (tokens->at(index)).lineNumber,(tokens->at(index)).columnNumber, "Warning, nodist variable");
			distributed = false;
			decStatement->distributed = false;
		}
		else if (getToken().value == "channel" )
		{
			if (decStatement->recurrent || !distributed || decStatement->backProp)
			{
				displayError(fName, getToken().lineNumber, getToken().columnNumber, "channel variable is not pure !?");
			}

			decStatement->channel=true;
		}
		else if (getToken().value == "global" )
		{
			decStatement->global=true;
		}
		else
		{
			break;
		}
		
		popToken();
	}
	
	
	if(!isDataType(getToken().value))
	{
		RequireType("datatype", "Expected \"Data type\" and not ", true);
	}
	
	decStatement->type = getToken().value;
	decStatement->classtype = isClass(decStatement->type);
	decStatement->array = false;
		
	popToken();
	
	//array
	if (getToken().value == "[" )
	{
		decStatement->array = true;
		popToken();
		if (getToken().type == "int")
		{
			stringstream SS;
			SS << getToken().value;
			SS >> decStatement->size;
			distributedVariablesCount += decStatement->size;
			popToken();
			RequireValue("]", "Expected ] and not ", true);		
		}
		else
		{
			decStatement->size = 0;
			RequireValue("]", "Expected ] and not ", true);
		}
		popToken();
	}
	
	// matrix
	if (getToken().value == "[" )
	{
		decStatement->array = true;
		popToken();
		if (getToken().type == "int")
		{
			stringstream SS;
			SS << getToken().value;
			SS >> decStatement->size;
			distributedVariablesCount += decStatement->size;
			popToken();
			RequireValue("]", "Expected ] and not ", true);		
		}
		else
		{
			decStatement->size = 0;
			RequireValue("]", "Expected ] and not ", true);
		}
		popToken();
	}


	if (getToken().value == "(")
	{
		int to = reachToken(")", true, true, false, false, true);
		//decStatement->value = parseOpStatement(*working_tokens_index, to-1, decStatement->type, 0, decStatement);
		decStatement->value = parseOpStatement(0, to-1, decStatement->type, 0, decStatement);
		reachToken(")", true, true, true, false, true);
		popToken();
		RequireValue(terminal, "Expected " + terminal + " and not ", true);
		decStatement->Initilazed = true;
	}
	else if (getToken().value == "=" )
	{
		if(peekToken("{"))
		{
			popToken();
			
			if (!decStatement->array)
			{
				displayError(fName, getToken().lineNumber, getToken().columnNumber,"This varaible is not an array");
			}
			
			int plevel = 1;
			//int from = *working_tokens_index+1;
			int from = 1;
			while (popToken().value.size())
			{
				if (getToken().value == "(")
				{
					plevel++;
				}
				else if (getToken().value == ")" || getToken().value == "}")
				{
					plevel--;
					if(plevel ==0)
					{
						//if(*working_tokens_index != from)
						if(0 != from)
						{
							//decStatement->values.push_back(parseOpStatement(from, *working_tokens_index - 1, "-2", 0, decStatement));
							decStatement->values.push_back(parseOpStatement(from, - 1, "-2", 0, decStatement));
						}
						break;
					}
				}
				else if (getToken().value == ",")
				{
					if (plevel==1)
					{		
						//if(*working_tokens_index != from)
						if(0 != from)
						{
							//decStatement->values.push_back(parseOpStatement(from, *working_tokens_index-1, "-2", 0, decStatement));
							decStatement->values.push_back(parseOpStatement(from, -1, "-2", 0, decStatement));
							//from = *working_tokens_index+1;
							from = 1;
						}
					}
				}
			}
			
			if (decStatement->values.size() > (unsigned int) decStatement->size && decStatement->size !=  0)
			{
				displayError(fName, getToken().lineNumber, getToken().columnNumber,"too many initilizations");
			}
			
			if (decStatement->size == 0)
			{
				decStatement->size = decStatement->values.size();
			}
			
			stringstream SS;
			for (unsigned int k =0; k < decStatement->identifiers->size(); k++)
			{
				for (unsigned int i =0; i < decStatement->values.size(); i++)
				{
					termStatment* termstatement = new termStatment( decStatement->identifiers->at(k).name, decStatement->type);
					termstatement->scope = scope;
					
					SS << i;
					termStatment* arrayIndex = new termStatment( SS.str(), decStatement->type);
					arrayIndex->scope = scope;
					
					termstatement->arrayIndex = arrayIndex;
					//termstatement->identifier = true;
					
					operationalStatement* os = new operationalStatement();
					os->left = termstatement;
					os->op = "=";
					os->right = decStatement->values.at(i);
					os->type = decStatement->type;
					os->scope = scope;
					currentFunction.body->push_back(os);

					//FIX ??
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
						statement* origiCurrentStatement = currentStatement;
						currentStatement = os;
						pushModified("=", id);
						currentStatement = origiCurrentStatement;
					}
					
					SS.str("");
					SS.clear();
				}
			}
			if (decStatement->size == 0) // FIX && decStatement->distributed)
			{
				distributedVariablesCount += decStatement->values.size();
			}
			decStatement->Initilazed = false;
		}
		else
		{			
			int to = reachToken(terminal, true, true, false, false, false);
			decStatement->value = parseOpStatement(0, to-1, decStatement->type, 0, decStatement);
			reachToken(terminal, true, true, true, false, false);
			
			if(decStatement->global)
			{
				for (unsigned int k =0; k < decStatement->identifiers->size(); k++)
				{
					termStatment* termstatement = new termStatment( decStatement->identifiers->at(k).name, decStatement->type);
					termstatement->scope = scope;
					
					operationalStatement* os = new operationalStatement();
					os->left = termstatement;
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
		cerr << "VAL :" << getToken().value.size() << endl;
		RequireValue(terminal, "Expected " + terminal + " and not ", true);
		decStatement->Initilazed = false;
		decStatement->value = NULL;
	}
	
	stringstream SS;
	
	decStatement->identifiers->clear();
	for ( unsigned int i = 0; i < tempIdentifiers->size(); i++ )
	{
		SS << i;
		termStatment* arrayIndex = new termStatment( SS.str(), decStatement->type);
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
		
		for(unsigned int i =0; i < datatypes.size(); i++)
		{
			if(datatypes.at(i).typeID == idinfo.type && datatypes.at(i).classType)
			{
				for (unsigned int k =0; k < datatypes.at(i).memberVariables.size(); k++)
				{
					idInfo id = idInfo(datatypes.at(i).memberVariables.at(k).name, decStatement->scope, datatypes.at(i).memberVariables.at(k).returnType, NULL);
					//FIX bad ?!
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