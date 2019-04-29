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

namespace DM14
{

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
		
		int	setVersion(const double&);
		uint32_t addIncludePath(const std::string& path);
		int	setgccPath(const std::string&);
		int	write(const std::string&);
		int	writeLine(const std::string&);
		int 	compile();
		int 	compileIncludes();
		int	compileDistributeNodes();
		int 	compileOuterExtern();
		int 	compileFunction();
		int	compileGlobalStructs();
		int 	compileNOPStatement(Statement*&);
		int	compileGlobalDeclarations();
		int	compileRetStatement(Statement*&);
		int 	compileForLoop(Statement*&);
		int 	compileWhileLoop(Statement*&);
		int 	compileDistribute(Statement*&);
		int 	compileDistributingVariables(Statement*&);
		int 	compileDecStatement(Statement*&);
		int 	compileOpStatement(Statement*&);
		int 	compileFunctionCall(Statement*&);
		int	compileTerm(Statement*&);
		int	compileInsider(Statement*&, iostream* = NULL, const bool = false);
		int	compileIF(Statement*&);
		int	compileCASE(Statement*&);
		int	compileResetStatement(Statement*&);
		int	compileParentAddStatement(Statement*&);
		int	compileSetNodeStatement(Statement*&);
		int	compileDistributedVariable(const idInfo&, const bool = false);
		int	compileNodeSelector(ast_function&);
		int	compileExtern(Statement*&);
		int	compileThread(Statement*&);
		int	compileAddVector(Statement*&, const idInfo&, const bool = false);
		bool	setcompileStatic(const bool&);
		bool	isCompilingMain();
		
		string	generateDistributedVariableName(idInfo* id);
		
		int		writeDepedency(idInfo&, int);
		void	printStatement(Statement* stmt, int indent);

		static bool compareIncludes(DM14::includePath include1, DM14::includePath include2);
		
		
	private:
		std::string pathSeperator = "/";
		Array<string>			includePaths;
		std::iostream*			outStream;
		bool					compileStatic;
		double					Version;
		std::string				IncludesDir;
		std::string				gccPath;
		std::string				fName;
		std::fstream			srcFile;
		std::fstream			m14FileDefs;
		Array<mapcode>*	mapCodes;
		//Array < pair<string, int> >*		nodesModifiers;
		
		int						index; // index of mapCodes ( current file )	
		unsigned int			fIndex;// function index
		
		Array<std::string>		dVariablesNames;
		int						dVariablesCount;
		int						currentNode;
		
		bool					tmpScope;
		bool					declaringVars;
		
		int 					nodesCount;
		std::string				bufferedOutput;
		bool					compilingMain;
};

} // namespace DM14::compiler

#endif /** DM14_COMPILER_HPP */
