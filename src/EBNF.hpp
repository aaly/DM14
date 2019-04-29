#ifndef	DM14_EBNF_HPP
#define	DM14_EBNF_HPP

#include <map>
#include <regex>
#include "parser.hpp"
#include "ast.hpp" // Statement
#include "token.hpp" /**< class token */
#include "types.hpp" // isImmediate etc..
#include "Array.hpp" // isImmediate etc..


namespace DM14
{
	class parser;
	
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
					if(//ebnf_level <= depth.ebnf_level &&
					   //current_rule <= depth.current_rule &&
					//current_token < depth.current_token && 
					   input_tokens_index < depth.input_tokens_index)
					{
						return true;
					}
					else
					{
						return false;
					}
				};

				int32_t print()
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

		//class parser;

		
		enum class ebnfResultType
		{
			SUCCESS =0,
			FAILURE
		};
		
		typedef std::pair<ebnfResultType, Statement*> ebnfResult;
		typedef Statement* (parser::*parser_callback)();


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

		typedef std::map <std::string, std::vector<grammar_rule_t>> EBNF_map_t;
	 
		class EBNF
		{
				public:
					
					parser* setParserInstance(parser* prser)
					{
						this->prser = prser;
						return prser;
					}
					
					EBNF()
					{
						working_tokens = &tokens_stack;
						index = 0;
						input_tokens_index = &index;
					};
					
					Array<token>* working_tokens = nullptr;
					Array<token>* input_tokens = nullptr;
					int *input_tokens_index = nullptr;
					Array<token> tokens_stack;
				
					int pushToken(token tok)
					{
						if(tok.value.size())
						{
							working_tokens->push_back(tok);
						}
						return working_tokens->size();
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
							//displayError("Empty pop !!!");
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
							//displayError("Empty pop !!!");
							current_token = token();
						}
						
						return current_token;
					}

					token getToken()
					{
						return current_token;
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

					int removeToken()
					{
						if(working_tokens->size() > 0)
						{
							working_tokens->erase(working_tokens->end());
						}
						else
						{
							cerr << "error removing token from the working tokens vector" << endl;
						}
						
						return 0;
					}


				/**
				 * @details This function takes a set of input tokens, and a start map key, and a pointer to the output tokens vector
				 */
				ebnfResult parseEBNF(Array<token>* input_tokens, std::string start_map_index, Array<token>* output_tokens, const bool verify_only = false)
				{
					//cerr << "inside parseEBF : " << start_map_index << " :" <<  *input_tokens_index << endl;
					if(!grammar[start_map_index].size())
					{
						cerr << "unknown grammar map index : " << start_map_index << endl << flush;
						exit(1);
					}

					Array<token>* current_working_tokens = working_tokens;
					working_tokens = output_tokens;
					Array<token>* current_input_tokens = this->input_tokens;
					
					this->input_tokens = input_tokens;
					
					EBNF_level++;
					ebnfResult result(ebnfResultType::SUCCESS, NULL);
					bool continue_searching_rules = true;

					#if PARSER_EBNF_SHOW_TRACE == 1
					cerr << endl << std::string(EBNF_level*2, ' ') << ".LOOPING KEY : " << start_map_index  << 	 " : " <<  *input_tokens_index << endl << flush;
					#endif

					parser_depth depth;

					depth.ebnf_level = EBNF_level;
					depth.start_map_index = start_map_index;

					/** loop over std::vector<grammar_token_array_t> */
					for(uint32_t i = 0; i < grammar[start_map_index].size() && continue_searching_rules == true; i++)
					{
						result.first = ebnfResultType::SUCCESS;
						
						#if PARSER_EBNF_SHOW_TRACE == 1
						cerr << string(EBNF_level*2, ' ')  << "..LOOPING RULE : " << i << 	 " : " <<  *input_tokens_index << endl << flush;
						#endif
						depth.current_rule = i;

						/** loop over std::vector<EBNF_entity_t> */
						grammar_rule_t current_rule = grammar[start_map_index].at(i);
						int old_tokens_size = working_tokens->size(); /** used for callbacks with only current rule pushed tokens */
						int rule_old_index = *input_tokens_index; /** used for ONLY_ONE rules */
						
						for(uint32_t k = 0; k < current_rule.tokens.size() && continue_searching_rules == true; k++)
						{
							EBNF_token_t ebnf_token = current_rule.tokens.at(k);
							int token_old_index = *input_tokens_index; // used for ONLY_ONE rules
							
							/** process the invidual token first */
							ebnfResult previous_result = result;
							result.first = ebnfResultType::FAILURE; /** initial value assumes error , prove otherwise */

							depth.current_token = k;

							if(*input_tokens_index > depth.input_tokens_index)
							{
								depth.input_tokens_index = *input_tokens_index;
							}

							if(ebnf_token.tokenType == DATATYPE_TOKEN)
							{
								if(input_tokens->at(*input_tokens_index).type == ebnf_token.expansion)
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == TERMINAL_TOKEN)
							{
								if(input_tokens->at(*input_tokens_index).value == ebnf_token.expansion)
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == REGEX_TOKEN)
							{
								if(!DM14::types::isKeyword(input_tokens->at(*input_tokens_index).value) || ebnf_token.expansion == ".*")
								{
									std::regex self_regex(ebnf_token.expansion, std::regex_constants::ECMAScript);
									if(std::regex_match(input_tokens->at(*input_tokens_index).value, self_regex))
									{
										result.first = ebnfResultType::SUCCESS;
									}
								}
							}
							else if(ebnf_token.tokenType == EXPANSION_TOKEN)
							{
								if(ebnf_token.callback)
								{
									result = parseEBNF(input_tokens, ebnf_token.expansion, output_tokens, true);
								}
								else
								{
									result = parseEBNF(input_tokens, ebnf_token.expansion, output_tokens, verify_only);
								}
							}
							else if(ebnf_token.tokenType == SINGLE_OP_TOKEN)
							{
								if(DM14::types::isSingleOperator(input_tokens->at(*input_tokens_index).value))
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == CORE_OP_TOKEN)
							{
								if(DM14::types::isCoreOperator(input_tokens->at(*input_tokens_index).value))
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == IMMEDIATE_TOKEN)
							{
								if(!DM14::types::isKeyword(input_tokens->at(*input_tokens_index).value) &&
									DM14::types::isImmediate(input_tokens->at(*input_tokens_index)))
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == KEYWORD_TOKEN)
							{
								if(DM14::types::isKeyword(input_tokens->at(*input_tokens_index).value) &&
									input_tokens->at(*input_tokens_index).value == ebnf_token.expansion)
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == BINARY_OP_TOKEN)
							{
								if(DM14::types::isbinOperator(input_tokens->at(*input_tokens_index).value))
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else if(ebnf_token.tokenType == OP_TOKEN)
							{
								if(DM14::types::isOperator(input_tokens->at(*input_tokens_index).value) &&
									input_tokens->at(*input_tokens_index).value == ebnf_token.expansion)
								{
									result.first = ebnfResultType::SUCCESS;
								}
							}
							else
							{
								displayError(" ERROR : unknown grammar token type");
							}
								
							if(result.first == ebnfResultType::SUCCESS)
							{
								/*if(old_successful_depth < depth)
								{
									old_successful_depth = depth;
									std::cerr << "Update success map : " << old_depth.start_map_index << "->" << depth.start_map_index  << std::endl;
								}*/

								#if PARSER_EBNF_SHOW_TRACE == 1
								cerr << string(EBNF_level*2, ' ') << "...PROCESSING TOKEN : " << ebnf_token.expansion << " => success " << endl << flush;
								#endif

								if(ebnf_token.tokenType != EXPANSION_TOKEN) // only terminals
								{
									advance_EBNFindex();
								}
								
								if(ebnf_token.tokenType == EXPANSION_TOKEN && ebnf_token.callback && verify_only == false)
								{
									/*for(uint32_t i =0; i < working_tokens->size(); i++)
									{
										cerr << "|||" << working_tokens->at(i).value << endl;
									}*/

									Array<token> tempTokenStack = working_tokens->cut(0, old_tokens_size);
									
									//for(uint32_t i =0; i < working_tokens->size(); i++)
									{
										//cerr << "///" << working_tokens->at(i).value << endl;
									}

									displayInfo("Calling callback for rule : " +  ebnf_token.expansion);
									//cerr << EBNF_level << endl << flush;
									
									result.second =(prser->*ebnf_token.callback)();
								
									working_tokens->copy(tempTokenStack);
									/*for(uint32_t i =0; i < tempTokenStack.size(); i++)
									{
										pushToken(tempTokenStack.at(i));
									}*/
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
								int diff = *input_tokens_index - token_old_index;
								deadvance_EBNFindex(diff);
							}


							if(*input_tokens_index >= input_tokens->size()) // break the zero or more cycle
							{
								continue_searching_rules = false;
							}
							
							/** process the current grammar rule type*/
							if(current_rule.type == GRAMMAR_TOKEN_OR)
							{
								if(result.first == ebnfResultType::SUCCESS)
								{
									continue_searching_rules = false;
								}
							}
							else if(current_rule.type == GRAMMAR_TOKEN_AND)
							{
								if(result.first != ebnfResultType::SUCCESS)
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
								if(result.first == ebnfResultType::SUCCESS)
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
								if(result.first == ebnfResultType::SUCCESS)
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
								if(result.first == ebnfResultType::FAILURE)
								{
									result.first = ebnfResultType::SUCCESS;
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

						if(result.first == ebnfResultType::FAILURE)
						{
							int diff = *input_tokens_index - rule_old_index;
							deadvance_EBNFindex(diff);
						}
					}

					EBNF_level--;
					working_tokens = current_working_tokens;
					this->input_tokens = current_input_tokens;
					return result;
				};
				
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
				int printEBNF()
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
									//cerr << " ";
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

			int advance_EBNFindex(uint16_t steps = 1)
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

			int deadvance_EBNFindex(uint16_t steps = 1)
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
				
				EBNF_map_t grammar;
				parser_depth old_depth;
				parser_depth old_successful_depth;
				int								index = 0;
				int EBNF_level = -1;
				
				private:					
					
					token current_token;
					int token_index = -1;
					parser* prser = nullptr;
		};
		
	} /** namespace EBNF */
} /** namespace DM14 */
#endif // DM14_EBNF_HPP
