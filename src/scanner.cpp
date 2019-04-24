// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license

//if there is no "enter" or new line at the end of the file, it will keep loopin :s
#include "scanner.hpp"

bool scanner::isReady()
{
	return iFile.is_open();
};



string scanner::getFileName()
{
	return fName;
};


scanner::scanner(const string& fileName)
{
	fName = fileName;
	iFile.open(fileName.c_str(),ios::in);
	if ( !iFile.is_open() )
	{
		displayError(fName, 0,0,"can't open source file for scanning : " + fileName);
	}
	
	if ( !iFile.good() )
	{
		displayError(fName, 0,0,"source file : " + fileName + "is corrupted?");
	}
	
	tokens = new Array<token>;
	line = 0;
	column = 0;
	tmptoken.lineNumber = line = 1;
	tmptoken.columnNumber = column = 0;
	
	Extern = false;
};

scanner::~scanner()
{
	if (iFile.is_open())
	{
		iFile.close();
	}
	
	if (tokens && tokens->size() != 0)
	{
		delete tokens;
	}
};

Array<token>* scanner::getTokens()
{
	if ((tokens->size() == 0) ||
		(tokens == NULL))
	{
		return (NULL);
	}
	return tokens;
};

char scanner::getNextChar()
{
	if (!iFile.is_open() || iFile.eof())
	{
		displayError(fName, line,column," can't get next char , end of file ?"); // column : " + tmptoken.columnNumber);
	}
	
	column++;
	return iFile.get();
};

int scanner::unGetChar()
{
	if (!iFile.is_open() || iFile.eof())
	{
		displayError(fName, line,column," can't go back :( , end of file ?"); // column : " + tmptoken.columnNumber);
	}
	
	iFile.unget();
	return 0;
};


int scanner::printTokens()
{
	if (tokens->size() != 0)
	{
		for ( int unsigned i =0; i< tokens->size(); i++ )
		{
			cout << "tokens[" << i << "] " << "[" << (tokens->at(i)).type << "] " << "[" << (tokens->at(i)).lineNumber << "] " << "= "<< (tokens->at(i)).value  << endl;
		}
	}
	else
	{
		return (1);
	}
	return (0);
};

int scanner::pushToken()
{
	if (!tmptoken.value.empty())
	{
		tmptoken.columnNumber = column;
		tmptoken.lineNumber = line;
		tokens->push_back(tmptoken);
	}
	tmptoken.type = "NIL";
	tmptoken.value.clear();
	return (0);
};

int scanner::addNumber()
{
	if(ch == '-')
	{
		tmptoken.value+= "-";
		ch = getNextChar();
	}

	if(ch == '.')
	{
		displayWarning(fName, line,column,"Immediate float does not begin with 0 !");
		tmptoken.value+= "0.";
		tmptoken.type = "float";
		ch = getNextChar();
	}
	bool dot = false;
	while (DM14::types::isNumber(ch))
	{	
		tmptoken.value+= ch;
			
		if (DM14::types::isNumber(iFile.peek()))
		{
			ch = getNextChar();
		}
		else if (iFile.peek() == '.')
		{
			if (dot)
			{
				printTokens();
				displayError(fName, line,column," Expected only one decimal point !");
			}
			else
			{
				dot = true;
				tmptoken.type = "float";
				tmptoken.value+= getNextChar();
				if (iFile.peek() == '.' || DM14::types::isNumber(iFile.peek()) )
				{
					ch = iFile.get();
				}
			}
		}
		else
		{
			break;
		}
		
		if ( !iFile.good() )
		{
			break;
		}	
	}
	
	if ( tmptoken.type != "float" )
	{
		tmptoken.type = "int";
	}
	
	pushToken();
	return (0);
};

int scanner::addString()
{
	tmptoken.value+=ch;
	while ( ( !iFile.eof() ) )
	{
		ch = getNextChar();
		if ( ch != '"' )
		{
			tmptoken.value+= ch;
		}
		else
		{
			tmptoken.value+= ch;
			tmptoken.type = "string";

			pushToken();
			break;
		}
	}
	if ( iFile.eof() && ch != '\"' )
	{
		displayError(fName, line,column,"unterminated string");
	}
	return (0);
};


int scanner::addChar()
{
	ch = getNextChar();

	tmptoken.value += "'";
	tmptoken.value += ch;
	
	//if ( !DM14::types::isLetter(ch) )
	//{
	//	displayError(fName, line,column,"Expected character");
	//}
	//else
	{
		if ( ch == '\\')
		{
			tmptoken.value += getNextChar();
		}
		
		if ( getNextChar() != '\'')
		{
			displayError(fName, line,column,"unterminated character");
		}
		
		tmptoken.value += "'";
		tmptoken.type = "char";
		pushToken();
	}
	return (0);
};

int scanner::addKeyword()
{
	
	return (0);
};

int scanner::addOperator()
{
	tmptoken.value+= ch;
	if (DM14::types::isOperator(tmptoken.value))
	{
		if ( ( ch = iFile.peek() ) )
		{
			if (DM14::types::isOperator((tmptoken.value + ch)))
			{
				tmptoken.value+= ch;
				getNextChar();
			}
		}
	}
	else
	{
		return 1;
	}
	tmptoken.type = "operator";
	pushToken();
	return 0;
};

int scanner::addLetter()
{
	tmptoken.value+= ch;
	if (!(DM14::types::isLetter(iFile.peek()) || DM14::types::isNumber(iFile.peek())))
	{
		if (DM14::types::isKeyword(tmptoken.value))
		{
			tmptoken.type = "keyword";
			
			if( tmptoken.value == "extern")
			{
				Extern = true;
			}
			else if( tmptoken.value == "endextern")
			{
				Extern = false;
			}
			
			pushToken();
		}
		else if (DM14::types::isDataType(tmptoken.value))
		{
			tmptoken.type = "datatype";
			pushToken();
		}
		else if (tmptoken.value == "true" || tmptoken.value == "false")
		{
			tmptoken.type = "bool";
			pushToken();
		}
		else
		{
			tmptoken.type = "identifier";
			pushToken();
		}
	}
	
	return (0);
};

int	scanner::scan()
{
	if (!iFile.is_open())
	{
		displayError(fName, 0,0,"Could not open file", false);
		return (1);
	}
	while( !iFile.eof()  && (ch = getNextChar()))
	{
		//(ch = getNextChar()) ;
		if (DM14::types::isWhiteSpace(ch)) // white space , doesn't matter
		{
			column++;
			space = true;
			
			if(Extern)
			{
				tmptoken.value = ch;
				pushToken();
			}
			continue;
		}
		else if (DM14::types::isNewLine(ch))
		{
			line++;
			column = 0;
			if(Extern)
			{
				tmptoken.value = ch;
				pushToken();
			}
			continue;
		}
		else if ( ch == '"' ) // a double qutation , which means a string
		{
			addString();
		}
		else if ( ch == '\'' )
		{
			addChar();
		}
		else if ( DM14::types::isLetter(ch) ) // a letter , which means an identifier or a keyword !
		{
			//if (DM14::types::isWhiteSpace(iFile.peek()))
			//{
//				tmptoken.value+= ch;
				//tmptoken.type = Char;	
				//pushToken();
			//}
			addLetter();
		}
		//else if ( ( ch == '.' && DM14::types::isNumber((char)iFile.peek()) ) || ( ch == '-' && ( DM14::types::isNumber(iFile.peek()) || iFile.peek() == '.' ) ) || DM14::types::isNumber(ch) )
		else if ( (ch == '.' && DM14::types::isNumber((char)iFile.peek())) || 
				  (ch == '-' && DM14::types::isNumber((char)iFile.peek())) ||
				  DM14::types::isNumber(ch))
		{
			if(tmptoken.value.size() > 0)
			{
				if (DM14::types::isLetter(tmptoken.value.at(0)))
				{
					addLetter();
					continue;
				}
			}
			addNumber();
		}
		else // we got a comment \o/ 
		{
			bool read = true;
			if (shortComment.size())
			{
				for (unsigned int k =0; k < shortComment.size(); k++)
				{
					if (ch == shortComment.at(k))
					{
						ch = iFile.get();
					}
					else
					{
						for (unsigned int l =0; l < k; l++)
						{
							unGetChar();
						}
						
						if( k > 0)
						{
							unGetChar();
							ch = iFile.get();
						}
						
						read = false;
						break;
					}
				}
				
				while (read)
				{	
					ch=iFile.get();
					if (DM14::types::isNewLine(ch) || iFile.eof())
					{	
						line++;
						column = 0;
						if (DM14::types::isNewLine((char)iFile.peek()))
						{
							iFile.get();
						}
						break;
						
					}
				}
			}
			else
			{
				read = false;
			}
			
			if (longCommentStart.size() )
			{
				read = true;
				for (unsigned int k =0; k < longCommentStart.size(); k++)
				{
					if (ch == longCommentStart.at(k))
					{
						ch = iFile.get();
					}
					else
					{
						for (unsigned int l =0; l < k; l++)
						{
							unGetChar();
						}
						
						if( k > 0)
						{
							unGetChar();
							ch = iFile.get();
						}
						read = false;
						break;
					}
				}

				while (read)
				{
					ch = iFile.get();
					if (DM14::types::isNewLine(ch))
					{
						if (DM14::types::isNewLine((char)iFile.peek()))
						{
							iFile.get();
						}
						line++;
						column = 0;
						continue;
					}
					
					if (ch == longCommentEnd.at(0))
					{
						for (unsigned int k =1; k < longCommentStart.size(); k++)
						{
							if (ch == longCommentStart.at(k))
							{
								ch = getNextChar();
							}
							else
							{
								for (unsigned int l =0; l < k; l++)
								{
									unGetChar();
								}
								read = false;
								break;
							}
						}
						if (DM14::types::isNewLine((char)iFile.peek()))
						{
							iFile.get();
						}
						line++;
						column = 0;
						break;
					}
				}
			}
			else
			{
				read = false;
			}
			
			tmptoken.value.clear();
			
			if(!read)
			{
				if(addOperator())
				{
					//displayError(fName, line,0," couldn't recognize token :" + ch, false);
				}
				tmptoken.value.clear();
			}
			else
			{
				continue;
			}
		}
		column++;
	}
	
	return 0;
};




int	scanner::setShortComment(const string& deli)
{
	shortComment = deli;
	return deli.size();
}

int	scanner::setLongComment(const string& deliStart, const string& deliEnd)
{
	
	if(!deliStart.size() || !deliEnd.size())
	{
		displayError(fName, line,column,"Long comment start delimeter and end delimeter must have a size", false);
	}
	
	longCommentStart = deliStart;
	longCommentEnd = deliEnd;
	
	return deliStart.size() + deliEnd.size();
}





int scanner::reachToken(const int& from, const string& Char, const bool& sameLine,  const bool& reportError, const bool& beforeEOF)
{
	int line = tokens->at(from).lineNumber;
	unsigned int iterator = from;
	
	while (iterator++)
	{
		if (sameLine)
		{
			if ( tokens->at(iterator).lineNumber != line )
			{
				if (reportError)
				{
					displayError(fName, (tokens->at(iterator)).lineNumber,(tokens->at(iterator)).columnNumber,"Missing " + Char);
				}
				else
				{
					return -1;
				}
			}
		}
		
		if ((beforeEOF && tokens->at(iterator).value == ";") || tokens->at(iterator).value == Char)
		{
			return iterator;
		}
		
		if ( iterator == tokens->size() )
		{
			if (reportError)
			{
				displayError(fName, (tokens->at(iterator)).lineNumber,(tokens->at(iterator)).columnNumber,"Missing " + Char);
			}
		}
	}
	return -1; /** couldn't find */
}

const token scanner::tokenAt(const unsigned int& at)
{
	if(at > tokens->size()-1)
	{
		return token(); /** couldn't find */
	}
	
	return tokens->at(at);
}
