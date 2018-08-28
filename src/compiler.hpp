#ifndef DM14_COMPILER_HPP
#define DM14_COMPILER_HPP
/**
@file             compiler.hpp
@brief            compiler implementation
@details          compiler implementation
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license
*/


#include "scanner.hpp"
#include "parser.hpp"
#include <stdlib.h>
#include "ast.hpp"
#include <iomanip> // std::setw
#include <sstream>


extern Array<DatatypeBase>	datatypes;

enum compilerOptimizations
{
	FORWARDFETCH = 0,
	ARRAYBUNCHFETCH
};

class compiler
{
	public:
		compiler(Array<mapcode>* const);
		~compiler();
		
		int		setVersion(const double&);
		uint32_t addIncludePath(const std::string& path);
		int		setgccPath(const std::string&);
		int		write(const std::string&);
		int		writeLine(const std::string&);
		int 	compile();
		int 	compileIncludes();
		int		compileDistributeNodes();
		int 	compileOuterExtern();
		int 	compileFunction();
		int		compileGlobalStructs();
		int		compileGlobalDeclarations();
		int		compileRetStatement(statement*&);
		int 	compileForLoop(statement*&);
		int 	compileWhileLoop(statement*&);
		int 	compileDistribute(statement*&);
		int 	compileDitributingVariables(statement*&);
		int 	compileDecStatement(statement*&, const bool = false);
		int 	compileOpStatement(statement*&);
		int 	compileFunctionCall(statement*&);
		int		compileTerm(statement*&);
		int		compileInsider(statement*&, iostream* = NULL, const bool = false);
		int		compileIF(statement*&);
		int		compileCASE(statement*&);
		int		compileResetStatement(statement*&);
		int		compileParentAddStatement(statement*&);
		int		compileSetNodeStatement(statement*&);
		int		compileDistributedVariable(const idInfo&, const bool = false);
		int		compileNodeSelector(ast_function&);
		int		compileExtern(statement*&);
		int		compileThread(statement*&);
		int		compileAddVector(statement*&, const idInfo&, const bool = false);
		bool	setcompileStatic(const bool&);
		bool	isCompilingMain();
		
		string	generateDistributedVariableName(idInfo* id);
		
		int		writeDepedency(idInfo&, int);
		void	printStatement(statement* stmt, int indent);
		
		
	private:
		std::string pathSeperator = "/";
		Array<string>			includePaths;
		std::iostream*			outStream;
		bool					compileStatic;
		double					Version;
		std::string					IncludesDir;
		std::string					gccPath;
		std::string					fName;
		std::fstream					srcFile;
		std::fstream					m14FileDefs;
		Array<mapcode>*						mapCodes;
		//Array < pair<string, int> >*		nodesModifiers;
		
		int						index; // index of mapCodes ( current file )	
		unsigned int			fIndex;// function index
		
		Array<std::string>			dVariablesNames;
		int						dVariablesCount;
		int						currentNode;
		
		bool					tmpScope;
		bool					declaringVars;
		
		int 					nodesCount;
		std::string					bufferedOutput;
		bool					compilingMain;
};

#endif /** DM14_COMPILER_HPP */