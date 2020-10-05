#ifndef	DM14_EBNF_HPP
#define	DM14_EBNF_HPP

#include <map>
#include <regex>
#include "parser.hpp"
#include "ast.hpp" 	 /// Statement
#include "token.hpp" /// class token
#include "types.hpp" /// isImmediate etc..
#include "Array.hpp" /// Array<T>


namespace DM14
{
	class Parser;
	
	namespace EBNF
	{	
		#define PARSER_EBNF_SHOW_TRACE 0
		
		using namespace std;
		
		class parser_depth
		{
			public:
				int32_t ebnf_level = 0;
				std::string start_map_index = "";
				uint32_t current_rule = 0;
				uint32_t current_token = 0;
				int64_t input_tokens_index = 0;
				bool operator <(const parser_depth& depth)
				{
					bool result = false;
					if(input_tokens_index < depth.input_tokens_index)
					{
						result = true;
					}
					else
					{
						result = false;
					}
					
					return result;
				};

				uint32_t print()
				{
					std::cerr << "Map index  : " << start_map_index << std::endl;
					std::cerr << "ebnf level : " << ebnf_level << std::endl;
					std::cerr << "current_rule : " << current_rule << std::endl;
					std::cerr << "current_token : " << current_token << std::endl;

					return 0;
				}
		};
		
		typedef enum
		{
			DATATYPE_TOKEN=0,
			TERMINAL_TOKEN,
			REGEX_TOKEN,
			EXPANSION_TOKEN,
			SINGLE_OP_TOKEN,
			CORE_OP_TOKEN,
			OP_TOKEN,
			BINARY_OP_TOKEN,
			IMMEDIATE_TOKEN,
			KEYWORD_TOKEN,
		}
		grammar_token_type;
		
												

		
		//typedef std::pair<ebnfResultType, Statement*> ebnfResult;
		typedef std::shared_ptr<Statement> (Parser::*parser_callback)();


		class EBNF_token_t
		{
			public:
			
			EBNF_token_t() = default;
			EBNF_token_t(const EBNF_token_t&) = default;
			std::string expansion;
			grammar_token_type tokenType; // type or terminal or regex
			parser_callback callback;
		};

		typedef enum
		{
			GRAMMAR_TOKEN_OR=0,
			GRAMMAR_TOKEN_AND,
			GRAMMAR_TOKEN_ZERO_MORE,
			GRAMMAR_TOKEN_ONE_MORE,
			GRAMMAR_TOKEN_ONLY_ONE,
		}
		grammar_rule_type_t;


		typedef struct
		{
			grammar_rule_type_t type;
			std::vector<EBNF_token_t> tokens;
		}
		grammar_rule_t;

		typedef std::map<std::string, std::vector<grammar_rule_t>> EBNF_map_t;
		
		//typedef Array<std::pair<std::string, std::pair<parser_callback, Array<token>>>> callstack_t;
		
		class callstack_t
		{
			public:
			
				callstack_t()
				{
					nested_stacks = make_shared<Array<callstack_t>>();
				}
				
				
				std::shared_ptr<Array<callstack_t>> nestedStacks()
				{
					return nested_stacks;
				}
				
				std::string rule;
				std::string expansion;
				parser_callback callback;
				
				
				void Print(uint32_t indent = 0)
				{
					for (uint32_t i =0; i < indent; i++)
					{
						cerr << " ";
					}
					
					cerr << "Rule : " << rule << " nested_stacks size:" << nested_stacks->size() << " tokens:[";
					for(auto token : *tokens)
					{
						cerr << token.value << ",";
					}
					cerr << "]" << endl;
					for(auto nested_stack : *nested_stacks)
					{
						nested_stack.Print(indent+1);
					}
				}
				
				bool selectRule(const std::string& rule)
				{
					bool result = false;
					for(auto nested_stack : *nested_stacks)
					{
						if(nested_stack.rule == rule)
						{
							result = true;
						}
					}
					return result;
				}
				
				int32_t nextIndex()
				{
					if(tokens->size() > 0 &&
						index < tokens->size())
					{
						index++;
					}
					else
					{
						displayError("prser>fName", (tokens->at(getIndex())).lineNumber,(tokens->at(getIndex())).columnNumber,"Unexpected EOF");
					}
					return index;
				};
			
				
				const int32_t getIndex()
				{
					return index;
				}
				
				
								
				int32_t setIndex(int32_t index)
				{
					this->index = index;
					return this->index;
				}
				
							
				token getToken(const uint32_t index)
				{
					if(tokens->size() > 0 &&
						index < tokens->size())
					{
						return tokens->at(index);
					}
					return token();
				}


				token getToken()
				{
					return current_token;
				}



				token popToken(const uint32_t index)
				{
					if(tokens->size() > 0)
					{
						current_token = tokens->at(index);
						tokens->remove(index);
					}
					else
					{
						displayError("Empty pop !!!");
						current_token = token();
					}
					
					return current_token;
				}

				token popToken()
				{
					if(tokens->size() > 0)
					{
						current_token = tokens->at(0);
						tokens->erase(tokens->begin());
					}
					else
					{
						displayError("Empty pop !!!");
						current_token = token();
					}
					
					return current_token;
				}
				
				std::shared_ptr<Array<token>> Tokens()
				{
					return tokens;
				}
				
				void setTokens(Array<token> tokens)
				{
					this->tokens = std::make_shared<Array<token>>(tokens);
				}
				
			private:
			
				int32_t index = 0;	/** current tokens index */
				token current_token; /** current poped token */
				std::shared_ptr<Array<token>> tokens = nullptr;
				std::shared_ptr<Array<callstack_t>> nested_stacks = nullptr;
				
		};
		
		enum class ebnfResultType
		{
			SUCCESS =0,
			FAILURE
		};
		
		class ebnfResult
		{
			public:
				ebnfResultType status;
				callstack_t stack;
		};
		
		
		
		class EBNF
		{
			public:
				
				Statement* popStatement(std::string rule)
				{
					Statement* result = nullptr;

					return result;
				}

				Parser* setParserInstance(Parser* prser)
				{
					this->prser = prser;
					return prser;
				}
				
				EBNF()
				{
					index = 0;
					working_tokens = tokens_stack;
					input_tokens_index = &index;
				};
				
				
				
				int32_t nextIndex()
				{
					if(working_tokens->size() > 0 &&
						index < working_tokens->size())
					{
						index++;
					}
					else
					{
						displayError("prser>fName", (tokens->at(getIndex())).lineNumber,(tokens->at(getIndex())).columnNumber,"Unexpected EOF");
					}
					return index;
				};
			
				int32_t pushToken(token tok)
				{
					if(tok.value.size())
					{
						working_tokens->push_back(tok);
					}
					return working_tokens->size();
				}

		
				int32_t removeToken()
				{
					if(working_tokens->size() > 0)
					{
						working_tokens->erase(working_tokens->end());
					}
					else
					{
						cerr << "error removing token from the working tokens vector" << endl;
					}
					
					return working_tokens->size();
				}

				
				/**
				 * @details Exposed wrapper function to call internal parseEBNF function
				 */
				ebnfResult parse(std::string start_map_index)
				{
					rule_groups_depth = 0;
					ebnfResult result = parseEBNF(tokens, start_map_index, tokens_stack);
					if(result.status == ebnfResultType::SUCCESS)
					{
						/*while(callStack.size())
						{
							popStatement();
						}*/
					}
					return result;
				}
				
				/**
				 * @details get the depth of a specific rule starting from a specific start rule
				 */
				uint32_t getLevelOfEBNFRule(const std::string rule, const std::string start)
				{
					int32_t result = 0;
					for(uint32_t i = 0; i < grammar[start].size(); i++)
					{
						grammar_rule_t current_rule = grammar[start].at(i);
						
						for(uint32_t k = 0; k < current_rule.tokens.size(); k++)
						{
							EBNF_token_t ebnf_token = current_rule.tokens.at(k);

							if(ebnf_token.expansion == rule)
							{
								return 1;
							}
						}

						for(uint32_t k = 0; k < current_rule.tokens.size(); k++)
						{
							EBNF_token_t ebnf_token = current_rule.tokens.at(k);
							uint32_t tmp = getLevelOfEBNFRule(rule, ebnf_token.expansion);
							if(tmp > 0)
							{
								return result + tmp;
							}
						}
					}
					return result;
				};

				/** @details print the Extended BNF grammar */
				int32_t printEBNF()
				{
					EBNF_map_t::iterator it;

					for(it = grammar.begin(); it != grammar.end(); it++ )
					{
						string start_map_index = it->first;
						cerr << "<" << start_map_index << "> ::=";
						// loop over std::vector<grammar_token_array_t>
						for(uint32_t i = 0; i < grammar[start_map_index].size(); i++)
						{
							grammar_rule_t current_rule = grammar[start_map_index].at(i);
							if(current_rule.tokens.size() > 1)
							{
								cerr << " {";
							}
							
							for(uint32_t k = 0; k < current_rule.tokens.size(); k++)
							{
								EBNF_token_t ebnf_token = current_rule.tokens.at(k);
								
								/** process the individual token first */
								if(ebnf_token.tokenType == DATATYPE_TOKEN)
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
								else if(ebnf_token.tokenType == CORE_OP_TOKEN)
								{
									cerr << "<CORE_OPERATOR>";
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
								else if(ebnf_token.tokenType == OP_TOKEN)
								{
									cerr << "<OP_TOKEN>";
								}
								else if(ebnf_token.tokenType == KEYWORD_TOKEN)
								{
									cerr << "<KEYWORD_TOKEN>";
								}
								else
								{
									cerr << " ERROR : unknown grammar token type : " << typeid(ebnf_token.tokenType).name()  << endl;
								}
								
								if(k < current_rule.tokens.size()-1)
								{
									cerr << " ";
								}
								
								/** process the current grammar rule type*/
								if(current_rule.type == GRAMMAR_TOKEN_OR)
								{
									if(k < current_rule.tokens.size() -1)
									{
										cerr << "| ";
									}
								}
								else if(current_rule.type == GRAMMAR_TOKEN_AND)
								{
									cerr << " ";
									;
								}
								else if(current_rule.type == GRAMMAR_TOKEN_ZERO_MORE)
								{
									cerr << "*";
								}
								else if(current_rule.type == GRAMMAR_TOKEN_ONE_MORE)
								{
									cerr << "+";
								}
								else if(current_rule.type == GRAMMAR_TOKEN_ONLY_ONE)
								{
									cerr << "?";
								}
								else
								{
									//	cerr << string(EBNF_level, ' ') << " ERROR : unknown grammar rule type" << endl;
								}
							}
							if(current_rule.tokens.size() > 1)
							{
								cerr << "}";
							}
							
						}
						cerr << endl;
					}
					return 0;
				};

				int32_t advance_EBNFindex(uint16_t steps = 1)
				{
					for(uint16_t i =0; i < steps; i++)
					{
						if(*input_tokens_index < input_tokens->size())
						{
							//cerr << "Pushing :  " << input_tokens->at(*input_tokens_index).value << endl;
							pushToken(input_tokens->at(*input_tokens_index));
							++*input_tokens_index;
						}
						else
						{
							cerr << "ERROR ADVANCING ! " << endl << flush;
						}
					}
					return *input_tokens_index;
				};

				int32_t deadvance_EBNFindex(uint16_t steps = 1)
				{
					for(uint16_t i =0; i < steps; i++)
					{
						if(input_tokens->size())
						{
							//cerr << "Removing :  " << working_tokens->at(working_tokens->size()-1).value << endl;
							removeToken();
							--*input_tokens_index;
						}
						else
						{
							cerr << "ERROR DE-ADVANCING ! " << endl << flush;
						}
					}
					return *input_tokens_index;
				};
				
				void groupBy(std::vector<std::string> rules)
				{
					for(const auto& rule : rules)
					{
						this->groupByRules[rule] = true;
					}
				};
				
				const int32_t getIndex()
				{
					return index;
				};
				
				
				
				std::shared_ptr<Statement> parseStatement(std::shared_ptr<Statement> output, const std::string starting_rule, parser_callback custom_callback)
				{
					std::shared_ptr<Statement> retStmt = output;
					
					
					int32_t *temp_input_tokens_index_ptr = input_tokens_index;
					int32_t temp_input_tokens_index = 0;
					input_tokens_index = &temp_input_tokens_index;
					std::shared_ptr<Array<token>> output_tokens(new Array<token>());
					int32_t working_tokens_size_before = working_tokens->size();
					
					rule_groups_depth = 0;
				
					ebnfResult result = parseEBNF(working_tokens, starting_rule, output_tokens);
					
					token errorToken = getToken(0);
					
					if(custom_callback != nullptr)
					{
						std::shared_ptr<Array<token>> current_working_tokens = working_tokens;
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

					output_tokens.reset();
					input_tokens_index = temp_input_tokens_index_ptr;
					
					if(output == nullptr)
					{
						//displayError(prser->fName, errorToken.lineNumber, errorToken.columnNumber,"Invalid Statement or grammar rule has no callback : " + starting_rule + " at token : " + errorToken.value);
						displayError("prser->fName", errorToken.lineNumber, errorToken.columnNumber,"Invalid Statement or grammar rule has no callback : " + starting_rule + " at token : " + errorToken.value);
					}

					return retStmt;
				};
				
				int32_t setIndex(int32_t index)
				{
					this->index = index;
					return this->index;
				}
							
				token getToken(const uint32_t index)
				{
					if(working_tokens->size() > 0 &&
						index < working_tokens->size())
					{
						return working_tokens->at(index);
					}
					return token();
				}


				token getToken()
				{
					return current_token;
				}



				token popToken(const uint32_t index)
				{
					if(working_tokens->size() > 0)
					{
						current_token = working_tokens->at(index);
						working_tokens->remove(index);
					}
					else
					{
						displayError("Empty pop !!!");
						current_token = token();
					}
					
					return current_token;
				}

				token popToken()
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
				
				
				EBNF_map_t 		grammar;
				parser_depth 	old_depth;
				std::shared_ptr<Array<token>>	tokens;
				std::shared_ptr<Array<token>> working_tokens = nullptr;
				std::shared_ptr<Array<token>> input_tokens = nullptr;
				
			private:
			
				/**
				 * @details This function takes a set of input tokens, and a start map key, and a pointer to the output tokens vector
				 */
				ebnfResult parseEBNF(std::shared_ptr<Array<token>> input_tokens, std::string start_map_index, std::shared_ptr<Array<token>> output_tokens, parser_callback callback = nullptr)
				{
					//cerr << "inside parseEBF : " << start_map_index << " :" <<  *input_tokens_index << endl;
					if(!grammar[start_map_index].size())
					{
						cerr << "unknown grammar map index : " << start_map_index << endl << flush;
						exit(1);
					}

					std::shared_ptr<Array<token>> current_working_tokens = working_tokens;
					working_tokens = output_tokens;
					std::shared_ptr<Array<token>> current_input_tokens = this->input_tokens;
					
					this->input_tokens = input_tokens;
					
					EBNF_level++;
					
					ebnfResult result;
					result.status  = ebnfResultType::SUCCESS;
					result.stack.rule  = start_map_index;
					
					bool continue_searching_rules = true;

					#if PARSER_EBNF_SHOW_TRACE == 1
					cerr << endl << std::string(EBNF_level*2, ' ') << ".LOOPING KEY : " << start_map_index  << 	 " : " <<  *input_tokens_index << endl << flush;
					#endif

					parser_depth depth;

					depth.ebnf_level = EBNF_level;
					depth.start_map_index = start_map_index;
					
					bool localInsideGroup = false;
					
					if(groupByRules[start_map_index] && rule_groups_depth < max_rule_groups_depth)
					{
						//cerr << ">>> entering local group :" << start_map_index << " [" << rule_groups_depth+1 << "]" << endl;
						rule_groups_depth++;
						localInsideGroup = true;
					}

					uint32_t current_working_tokens_size = output_tokens->size();


					/// loop over std::vector<grammar_token_array_t>
					for(uint32_t i = 0; i < grammar[start_map_index].size() && continue_searching_rules == true; i++)
					{
						result.status = ebnfResultType::SUCCESS;
						
						#if PARSER_EBNF_SHOW_TRACE == 1
						cerr << string(EBNF_level*2, ' ')  << "..LOOPING RULE : " << i << 	 " : " <<  *input_tokens_index << endl << flush;
						#endif
						depth.current_rule = i;

						/** loop over std::vector<EBNF_entity_t> */
						grammar_rule_t current_rule = grammar[start_map_index].at(i);
						//DELETE uint32_t old_stack_size = tokens_stack.size();
						int32_t rule_old_index = *input_tokens_index; /// used for ONLY_ONE rules

						for(uint32_t k = 0; k < current_rule.tokens.size() && continue_searching_rules == true; k++)
						{
							EBNF_token_t ebnf_token = current_rule.tokens.at(k);
							int32_t token_old_index = *input_tokens_index; /// used for ONLY_ONE rules
							/** process the invidual token first */
							ebnfResult previous_result = result;
							result.status = ebnfResultType::FAILURE; /// initial value assumes error , prove otherwise

							depth.current_token = k;

							uint32_t current_tokens_size = working_tokens->size();
							
							if(*input_tokens_index > depth.input_tokens_index)
							{
								depth.input_tokens_index = *input_tokens_index;
							}

							if(ebnf_token.tokenType == DATATYPE_TOKEN)
							{
								if(input_tokens->at(*input_tokens_index).type == ebnf_token.expansion)
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == TERMINAL_TOKEN)
							{
								if(input_tokens->at(*input_tokens_index).value == ebnf_token.expansion)
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == REGEX_TOKEN)
							{
								if(!DM14::types::isKeyword(input_tokens->at(*input_tokens_index).value) || ebnf_token.expansion == ".*")
								{
									std::regex self_regex(ebnf_token.expansion, std::regex_constants::ECMAScript);
									if(std::regex_match(input_tokens->at(*input_tokens_index).value, self_regex))
									{
										result.status = ebnfResultType::SUCCESS;
									}
								}
							}
							else if(ebnf_token.tokenType == EXPANSION_TOKEN)
							{
								auto expansion_result = parseEBNF(input_tokens, ebnf_token.expansion, output_tokens, ebnf_token.callback);
								result.status = expansion_result.status;
								if(result.status == ebnfResultType::SUCCESS)
								{
									if(groupByRules[ebnf_token.expansion])
									{
										result.stack.nestedStacks()->push_back(expansion_result.stack);
									}
								}
								
								/*auto expansion_result = parseEBNF(input_tokens, ebnf_token.expansion, output_tokens, ebnf_token.callback);
								
								if(expansion_result.status == ebnfResultType::SUCCESS)
								{
									if(groupByRules[ebnf_token.expansion])
									{
										if(ebnf_token.callback != nullptr)
										{
											result = expansion_result;
										}
										else
										{
											result.status = ebnfResultType::SUCCESS;
											result.stack.nested_stacks.push_back(expansion_result.stack);
										}
									}
								}*/
							}
							else if(ebnf_token.tokenType == SINGLE_OP_TOKEN)
							{
								if(DM14::types::isSingleOperator(input_tokens->at(*input_tokens_index).value))
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == CORE_OP_TOKEN)
							{
								if(DM14::types::isCoreOperator(input_tokens->at(*input_tokens_index).value))
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == IMMEDIATE_TOKEN)
							{
								if(!DM14::types::isKeyword(input_tokens->at(*input_tokens_index).value) &&
									DM14::types::isImmediate(input_tokens->at(*input_tokens_index)))
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == KEYWORD_TOKEN)
							{
								if(DM14::types::isKeyword(input_tokens->at(*input_tokens_index).value) &&
									input_tokens->at(*input_tokens_index).value == ebnf_token.expansion)
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == BINARY_OP_TOKEN)
							{
								if(DM14::types::isbinOperator(input_tokens->at(*input_tokens_index).value))
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == OP_TOKEN)
							{
								if(DM14::types::isOperator(input_tokens->at(*input_tokens_index).value) &&
									input_tokens->at(*input_tokens_index).value == ebnf_token.expansion)
								{
									result.status = ebnfResultType::SUCCESS;
								}
							}
							else
							{
								displayError(" ERROR : unknown grammar token type");
							}
								
							if(result.status == ebnfResultType::SUCCESS)
							{
								if(old_successful_depth < depth)
								{
									old_successful_depth = depth;
									//std::cerr << "Update success map : " << old_depth.start_map_index << "->" << depth.start_map_index  << std::endl;
								}

								#if PARSER_EBNF_SHOW_TRACE == 1
								cerr << string(EBNF_level*2, ' ') << "...PROCESSING TOKEN : " << ebnf_token.expansion << " => success " << endl << flush;
								#endif

								if(ebnf_token.tokenType != EXPANSION_TOKEN) /// only terminals
								{
									
									advance_EBNFindex();
								}
							}
							else
							{
								if(old_depth < depth)
								{
									old_depth = depth;
									//std::cerr << "Update fail map : " << old_depth.start_map_index << "->" << depth.start_map_index  << std::endl;
								}	
								#if PARSER_EBNF_SHOW_TRACE == 1
								cerr << string(EBNF_level, ' ') << "...PROCESSING TOKEN : " << ebnf_token.expansion << " => fail " << endl << flush;
								#endif
								int32_t diff = *input_tokens_index - token_old_index;
								deadvance_EBNFindex(diff);
							}


							if(*input_tokens_index >= input_tokens->size()) // break the zero or more cycle
							{
								continue_searching_rules = false;
							}
							
							/// process the current grammar rule type
							if(current_rule.type == GRAMMAR_TOKEN_OR)
							{
								if(result.status == ebnfResultType::SUCCESS)
								{
									continue_searching_rules = false;
								}
							}
							else if(current_rule.type == GRAMMAR_TOKEN_AND)
							{
								if(result.status != ebnfResultType::SUCCESS)
								{
									#if PARSER_EBNF_SHOW_TRACE == 1
									cerr << string(EBNF_level*4, ' ') << "ERROR : tokens do not(follow order/exist) in map : " << start_map_index << " rule : " << i << endl;
									#endif 
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
							else if(current_rule.type == GRAMMAR_TOKEN_ZERO_MORE)
							{
								if(result.status == ebnfResultType::SUCCESS)
								{
									previous_result = result;
									--k;
								}
								else
								{
									result = previous_result;
									continue_searching_rules = false;
								}
							}
							else if(current_rule.type == GRAMMAR_TOKEN_ONE_MORE)
							{
								if(result.status == ebnfResultType::SUCCESS)
								{
									previous_result = result;
									--k;
								}
								else
								{
									result = previous_result;
									continue_searching_rules = false;
								}
							}
							else if(current_rule.type == GRAMMAR_TOKEN_ONLY_ONE)
							{
								if(result.status == ebnfResultType::FAILURE)
								{
									result.status = ebnfResultType::SUCCESS;
								}
								else
								{
									continue_searching_rules = false;
								}
								break;
							}
							else
							{
								#if PARSER_EBNF_SHOW_TRACE == 1
								cerr << string(EBNF_level*4, ' ') << "ERROR : unknown grammar rule type" << endl;
								#endif
							}
						}

						if(result.status == ebnfResultType::FAILURE)
						{
							int32_t diff = *input_tokens_index - rule_old_index;
							deadvance_EBNFindex(diff);
						}
					}
					

					if(result.status == ebnfResultType::SUCCESS && groupByRules[start_map_index])
					{
						
						
						/// replace with nested statement if this has no tokens...
						//if(result.stack.Tokens().size() == 0 
						if(working_tokens->size()-current_working_tokens_size == 0
						   &&result.stack.nestedStacks()->size() == 1)
						{
							result.stack = result.stack.nestedStacks()->at(0);
						}
						else
						{
							result.stack.callback = callback;
							result.stack.rule =  start_map_index;
							result.stack.setTokens(working_tokens->cut(current_working_tokens_size, working_tokens->size()));
						}
					}
					
					if(localInsideGroup)
					{
						//cerr << "<<< entering local group :" << start_map_index << endl; 
						rule_groups_depth--;
					}
					
					
					EBNF_level--;
					working_tokens = current_working_tokens;
					this->input_tokens = current_input_tokens;
					
					return result;
				};
				
				
				
				
				std::map<std::string, bool> groupByRules; 	/** list of the groups to be used in the for the parseEBNF fuction */
				int32_t EBNF_level = -1; 				/** current EBNF level */	
				
				int32_t token_index = -1; 				/** internal token index */
				Parser* prser = nullptr; 			/** internal parser pointer, used for the callbacks */
				parser_depth old_successful_depth; 		/** lat successfil EBND parsed statement  */
				callstack_t callStack; 				/** */
				uint32_t rule_groups_depth = 0; 	/** counter of the current ebnf group level inside parseEBNF */
				uint32_t max_rule_groups_depth = 2; 	/** maximum level of the ebnf groups depth */
				int32_t *input_tokens_index = nullptr;
				std::shared_ptr<Array<token>> tokens_stack = std::shared_ptr<Array<token>>(new Array<token>());
				
				int32_t 			index = 0;
				token current_token; 				/** current poped token */

		};
	} /** namespace EBNF */
} /** namespace DM14 */
#endif // DM14_EBNF_HPP
