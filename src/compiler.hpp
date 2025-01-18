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

#include "ast.hpp"
#include "parser.hpp"
#include "scanner.hpp"
#include <iomanip> // std::setw
#include <sstream>
#include <stdlib.h>

namespace DM14 {

enum compilerOptimizations { FORWARDFETCH = 0, ARRAYBUNCHFETCH };

class compiler {
public:
  compiler(std::shared_ptr<Array<mapcode>> const);
  ~compiler();

  int setVersion(const double &);
  uint32_t addIncludePath(const std::string &path);
  int setgccPath(const std::string &);
  int write(const std::string &);
  int writeLine(const std::string &);
  int compile();
  int compileIncludes();
  int compileDistributeNodes();
  int compileOuterExtern();
  int compileFunction();
  int compileGlobalStructs();
  int compileNOPStatement(std::shared_ptr<Statement>);
  int compileGlobalDeclarations();
  int compileRetStatement(std::shared_ptr<Statement>);
  int compileForLoop(std::shared_ptr<Statement>);
  int compileWhileLoop(std::shared_ptr<Statement>);
  int compileDistribute(std::shared_ptr<Statement>);
  int compileDistributingVariables(std::shared_ptr<Statement>);
  int compileDecStatement(std::shared_ptr<Statement>);
  int compileOpStatement(std::shared_ptr<Statement>);
  int compileFunctionCall(std::shared_ptr<Statement>);
  int compileTerm(std::shared_ptr<Statement>);
  int compileInsider(std::shared_ptr<Statement>, iostream * = nullptr,
                     const bool = false);
  int compileIF(std::shared_ptr<Statement>);
  int compileCASE(std::shared_ptr<Statement>);
  int compileResetStatement(std::shared_ptr<Statement>);
  int compileParentAddStatement(std::shared_ptr<Statement>);
  int compileSetNodeStatement(std::shared_ptr<Statement>);
  int compileDistributedVariable(const idInfo &, const bool = false);
  int compileNodeSelector(ast_function &);
  int compileExtern(std::shared_ptr<Statement>);
  int compileThread(std::shared_ptr<Statement>);
  int compileAddVector(std::shared_ptr<Statement>, const idInfo &,
                       const bool = false);
  bool setcompileStatic(const bool &);
  bool isCompilingMain();

  string generateDistributedVariableName(std::shared_ptr<idInfo> id);

  int writeDepedency(idInfo &, int);
  void printStatement(std::shared_ptr<Statement> stmt, int indent);

  static bool compareIncludes(DM14::includePath include1,
                              DM14::includePath include2);

private:
  std::string pathSeperator = "/";
  Array<string> includePaths;
  std::iostream *outStream;
  bool compileStatic;
  double Version;
  std::string IncludesDir;
  std::string gccPath;
  std::string fName;
  std::fstream srcFile;
  std::fstream m14FileDefs;
  std::shared_ptr<Array<mapcode>> mapCodes;
  // Array < pair<string, int> >*		nodesModifiers;

  int index;           // index of mapCodes ( current file )
  unsigned int fIndex; // function index

  Array<std::string> dVariablesNames;
  int dVariablesCount;
  int currentNode;

  bool tmpScope;
  bool declaringVars;

  int nodesCount;
  std::string bufferedOutput;
  bool compilingMain;
};

} // namespace DM14

#endif /** DM14_COMPILER_HPP */
