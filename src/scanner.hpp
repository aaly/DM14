/**
@file             scanner.hpp
@brief            scanner
@details          Ad-hoc Scanner, Part of DM14 programming language
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2020
@version          1.1a
@copyright        See file "license" for bsd license
*/

#ifndef	SCANNER_H
#define	SCANNER_H

#include "types.hpp"
#include "common.hpp"
#include "token.hpp" /**< class token */

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <memory>
// we need escape sequence :|

// fix Lines counting

using namespace std;

namespace DM14
{
	

	class scanner
	{
		public:
		
			scanner(const string&);
			~scanner();
			string				getFileName();
			int					scan();			// start scanning process
			std::shared_ptr<Array<token>>		getTokens();	// return a pointer to the tokens Array
			char				getNextChar();	// return File.get() and increase the column by 1
			int					unGetChar();	// go back one character
			int					printTokens();	// Loop through the vctor of tokens and print each
			int					pushToken();	// Push temp token to the Array of tokens
			int					addString();	// start adding string process to tmp token
			int					addOperator();	// start adding operator process to tmp token
			int					addKeyword();	// Useless at the moment
			int					addNumber();	// start adding Number process to tmp token
			int					addLetter();	// start adding (letters) process to tmp token
			int					addChar();		// Adds a character
			
			std::shared_ptr<Array<token>>		tokens;				// scanned tokens
			int					line;			// Current line number
			int					column;			// Current character number on line
			bool				space;			// we have space , then we need to push the token :)
			bool				isReady();
			
			
			//int					displayWarning(const string&);
			//int					displayError(const string&);
			
			int					setShortComment(const string& deli);
			int					setLongComment(const string& deliStart, const string& deliEnd);
			
			
			int					reachToken( const int& from, const string& Char, const bool& sameLine,
											const bool& reportError, const bool& beforeEOF);
			const token		tokenAt(const unsigned int& at);
		private:
		
			string	fName;
			fstream	iFile;		// file to operate on
			char	ch;			// byte character
			token	tmptoken;		// Temp token for the scanned data
			bool	Extern;
			string	shortComment;
			string	longCommentStart;
			string	longCommentEnd;
	};

} //namespace DM14::scanner

#endif // SCANNER_H
