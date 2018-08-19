// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license


#include "scanner.hpp"
#include "parser.hpp"
#include <stdlib.h>
#include "ast.hpp"
#include <iomanip> // std::setw

//#include "common.hpp"
#include <sstream>
using namespace std;

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
		int		setIncludesDir(const string&);
		int		setgccPath(const string&);
		int		write(const string&);
		int		writeLine(const string&);
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
	
		iostream*				outStream;
		bool					compileStatic;
		double					Version;
		string					IncludesDir;
		string					gccPath;
		string					fName;
		fstream					srcFile;
		fstream					m14FileDefs;
		Array<mapcode>*						mapCodes;
		//Array < pair<string, int> >*		nodesModifiers;
		
		int						index; // index of mapCodes ( current file )	
		unsigned int			fIndex;// function index
		
		Array<string>			dVariablesNames;
		int						dVariablesCount;
		int						currentNode;
		
		bool					tmpScope;
		bool					declaringVars;
		
		int 					nodesCount;
		string					bufferedOutput;
		bool					compilingMain;
};
