/**
@file             compiler.cpp
@brief            compiler implementation
@details          compiler implementation
@author           AbdAllah Aly Saad <aaly90@gmail.com>
@date			  2010-2018
@version          1.1a
@copyright        See file "license" for bsd license


// hmm , the cin function must be awesome such that if a user enters char in a int it wont give him C++ error but a M14 error
// add argv and argc to main

// we should add the count of the last mapcode as the new base to add to ???

//if a new node added, all parents should send this child to all other childs
//or
//all childs should send parents to this nodes

*/

#include "compiler.hpp"

namespace DM14::compiler
{
int scopeLevel = 0;

compiler::compiler(Array<parser::mapcode>* const codes)
{
	//mapCodes =	new	Array<mapcode>;
	mapCodes = codes;
	//nodesModifiers = new Array < pair<string, int> >();
	dVariablesCount=0;
	tmpScope = false;
	nodesCount = 0;
	compileStatic = false;
	fIndex = 0;
	compilingMain = false;
	
};

compiler::~compiler()
{
	delete mapCodes;
	//delete nodesModifiers;
	
};


/*void compiler::printStatement(operationalStatement* stmt, int spaces)
{
  int i;
 
  if( stmt != NULL )
  {
	if(stmt->statementType == oStatement)
	{
		printStatement( (operationalStatement*)stmt->right, spaces + 3 );
	}
    
    for( i = 0; i < spaces; i++ )
    {
		cout <<' ';
	}
    cout << stmt->op << endl;
    
    if(stmt->statementType == oStatement)
    {
		printStatement((operationalStatement*)stmt->left, spaces + 3 );
	}
  }
}*/


void compiler::printStatement(statement* stmt, int indent)
{
    if(stmt != NULL && stmt->statementType == oStatement)
    {
        if(((operationalStatement*)stmt)->left)
        {
            printStatement(((operationalStatement*)stmt)->left, indent+4);
        }
        
        if (indent)
        {
            std::cout << std::setw(indent) << ' ';
        }
        
        if (((operationalStatement*)stmt)->left)
        {
			std::cout<<" /\n" << std::setw(indent) << ' ';
		}
		
		//std::cout<< ((operationalStatement*)stmt)->op << "\n ";
		std::cout<< ((operationalStatement*)stmt)->op << "\n ";
        
        if(((operationalStatement*)stmt)->right)
        {
            std::cout << std::setw(indent) << ' ' <<" \\\n";
            printStatement(((operationalStatement*)stmt)->right, indent+4);
        }
    }
    else if(stmt != NULL && stmt->statementType == tStatement)
    {
		std::cout<< ((termStatment*)stmt)->term << "\n ";
	}
}



bool compiler::setcompileStatic(const bool& Static)
{
	compileStatic = Static;
	return compileStatic;
};


int compiler::setVersion(const double& version)
{
	Version = version;
	return 0;
};



uint32_t compiler::addIncludePath(const std::string& path)
{
	includePaths.push_back(path);
	return includePaths.size();
}

int compiler::setgccPath(const std::string& gccpath)
{
	gccPath = gccpath;
	return 0;
};

int compiler::write(const std::string& output)
{
	if(outStream)
	{
		*outStream << output<< flush;
	}
	else
	{
		std::cout << "ERROR STREAM" << std::endl;
		exit(1);
	}
	return 0;
};

int compiler::writeLine(const std::string& output)
{
	if(outStream)
	{
		*outStream << output<< std::endl << std::flush;
	}
	else
	{
		std::cout << "ERROR STREAM" << std::endl;
		exit(1);
	}
	
	return 0;
};

bool compiler::compareIncludes(DM14::parser::includePath include1, DM14::parser::includePath include2)
{
	if(include1.package == include2.package &&
		include1.library == include2.library)
		{
			cerr << include1.package << ":" << include2.package  << endl;
			cerr << include1.library << ":" << include2.library  << endl;
			cerr << "true" << endl;
			return true;
		}
		cerr << include1.package << ":" << include2.package  << endl;
		cerr << include1.library << ":" << include2.library  << endl;
		cerr << "false" << endl;
		return false;
}

void remove(std::vector<DM14::parser::includePath> &v)
{
	auto end = v.end();
	for (auto it = v.begin(); it != end; ++it) {
		end = std::remove(it + 1, end, *it);
	}

	v.erase(end, v.end());
}



int compiler::compile()
{
	// loop through mapCodes ( files ) and call other funcs to write src files
	m14FileDefs.open("M14Defs.hpp", std::ios::out);
	m14FileDefs << "#ifndef __DM14HEADER_HPP" << std::endl;
	m14FileDefs << "#define __DM14HEADER_HPP" << std::endl;
	m14FileDefs << "#include <core" + pathSeperator + "M14Helper.hpp>" << std::endl;
	m14FileDefs << "unsigned int M14RESETCOUNTER = 0;" << std::endl;
	//m14FileDefs << "map<string, unsigned int> _DM14VARIABLESMAP;" << endl;
	//m14FileDefs << "string SERVINGIP;" << endl;
	//m14FileDefs << "int SERVINGPORT;" << endl;
	//m14FileDefs << "string PARENTIP;" << endl;
	//m14FileDefs << "int PARENTORT;" << endl;
	
	std::string com;
	
	cerr << "SIZE : " << mapCodes->size() << endl;
	for (uint32_t i =0; i < mapCodes->size(); i++)
	{
		bufferedOutput = "";
		//mapCodes->at(i).Print();
		index = i;
		
		nodesCount += (mapCodes->at(index)).nodesCount;
		dVariablesCount += (mapCodes->at(index)).dVariablesCount;
		
		std::string fileName = (mapCodes->at(index)).getFileName() ;
		std::size_t splitPoint = fileName.find_last_of("/\\");
		fileName = fileName.substr(splitPoint+1);

		// get source file name
		if ((mapCodes->at(index)).isHeader())
		{
			fName = fileName + ".hpp";
		}
		else
		{
			fName = fileName + ".cpp";
		}
		// open source file
		srcFile.open(fName.c_str(),ios::out);
		outStream = &srcFile;
		
		std::string headrmacroname = "";
		
		for(unsigned int i =0; i<fName.size(); i++)
		{
			if(fName.at(i) != '.')
			{
				headrmacroname += fName.at(i);
			}
		}
		// loop through includes nad output them
		
		if ((mapCodes->at(index)).isHeader())
		{
			writeLine("#ifndef "  + headrmacroname);
			writeLine("#define " + headrmacroname);
		}
		compileIncludes();
		compileOuterExtern();
		compileGlobalStructs();
		compileGlobalDeclarations();
		compileFunction();
		
		if ((mapCodes->at(index)).isHeader())
		{
			write("#endif ");
		}
		
		// close source file
		write(bufferedOutput);
		outStream = NULL;
		srcFile.close();
		
		if (!(mapCodes->at(index)).isHeader())
		{
			com += "g++ " + fName;	
			Array<DM14::parser::includePath> incs = (mapCodes->at(index)).getIncludes();
			
			incs.push_back(DM14::parser::includePath("core", "common", DM14::parser::includePath::sourceFileType::LIBRARY));
			incs.push_back(DM14::parser::includePath("core", "M14Helper", DM14::parser::includePath::sourceFileType::LIBRARY));
			incs.push_back(DM14::parser::includePath("core", "Socket", DM14::parser::includePath::sourceFileType::LIBRARY));
			//incs.push_back(DM14::parser::includePath("core", "Node", DM14::parser::includePath::sourceFileType::LIBRARY));
			incs.push_back(DM14::parser::includePath("core", "message", DM14::parser::includePath::sourceFileType::LIBRARY));
			incs.push_back(DM14::parser::includePath("io", "string", DM14::parser::includePath::sourceFileType::LIBRARY));

			remove(incs);
			
			for (unsigned int i =0; i < incs.size(); i++)
			{
				std::string headerName;

				if (incs.at(i).includeType == DM14::parser::includePath::sourceFileType::FILE_DM14)
				{
					//headerName = incs.at(i).library + ".hpp";
					//headerName = headerName.substr(headerName.find_last_of(pathSeperator));
				}
				else if (incs.at(i).includeType == DM14::parser::includePath::sourceFileType::FILE_C)
				{
					//headerName = incs.at(i).library + ".hpp";
				}
				else
				{
					string fullPath;
					for(uint32_t k = 0; k < includePaths.size(); k++)
					{
						ifstream ifs;			

						ifs.open(includePaths.at(k) +pathSeperator + incs.at(i).package + pathSeperator + incs.at(i).library + ".cpp");
						if(ifs.is_open())
						{
							fullPath = includePaths.at(k) +pathSeperator + incs.at(i).package + pathSeperator + incs.at(i).library + ".cpp";
							com += " -I" +  includePaths.at(k) +pathSeperator + incs.at(i).package +  " ";
							ifs.close();
							break;
						}
					}

					if (!fullPath.size())
					{
						displayError("Error not able to find unit : " +  incs.at(i).package + pathSeperator + incs.at(i).library  + ".cpp");
						exit(1);
					}
					
					std::fstream file;
					file.open(fullPath.c_str(),ios::in);
					if(file.is_open())
					{
						com += " " + fullPath + " ";
					}
					file.close();
				}
			}
			
			// FIXME 1022, only add m14main if this map has main function ?
			
			for (unsigned int k =0; k <  (mapCodes->at(index)).linkLibs->size(); k++ )
			{
				bool cont = false;
				for (unsigned int i =k+1; i < (mapCodes->at(index)).linkLibs->size(); i++)
				{
					if ( ((Link*)mapCodes->at(index).linkLibs->at(i))->libs == ((Link*)mapCodes->at(index).linkLibs->at(k))->libs)
					{
						 cont = true;
					}
				}
				
				if (cont)
				{
					continue;
				}
				
				if(((Link*)mapCodes->at(index).linkLibs->at(k))->Static)
				{
					com += " ";
					com += ((Link*)mapCodes->at(index).linkLibs->at(k))->libs;
				}
				else
				{
					com += " -l";
					com += ((Link*)mapCodes->at(index).linkLibs->at(k))->libs;
					/*for (unsigned int l =0; l < ((Link*)mapCodes->at(index).libs->at(k))->libs.size(); l++)
					{
						if ( ((Link*)mapCodes->at(index).libs->at(k))->libs.at(l) == ',')
						{
							com += " -l";
						}
						else
						{
							com += ((Link*)mapCodes->at(index).libs->at(k))->libs.at(l);
						}
					}*/
				}
			}

			for(uint32_t k = 0; k < includePaths.size(); k++)
			{
				com += " -I" + includePaths.at(k) + " ";
			}
			
			if(compileStatic)
			{
				com += "-static -static-libgcc -static-libstdc++";
			}

			com += " -I . -fpermissive  -lpthread -Wall -o " + fileName + ".bin -g;";
		}
	}
	
	compileDistributeNodes();
	m14FileDefs << "#endif" << std::endl;
	m14FileDefs.close();
	
	displayDebug("Compiling : " + com);
	int compilationResult = system(com.c_str());
	if (compilationResult == 0)
	{
		displayInfo("Compiled Scucessfully");
	}
	else
	{
		displayError("Compilation failed !!");
	}
	
	
	return compilationResult;
};

int compiler::compileIncludes()
{
	// copy includes
	
	std::vector<DM14::parser::includePath> incs = (mapCodes->at(index)).getIncludes();
	std::string headerName;
	
	for (unsigned int i =0; i < incs.size(); i++)
	{
		if (incs.at(i).includeType == DM14::parser::includePath::sourceFileType::FILE_DM14)
		{
			headerName = incs.at(i).package + ".hpp";
			if(headerName.find_last_of(pathSeperator) != std::string::npos)
			{
				headerName = headerName.substr(headerName.find_last_of(pathSeperator)+1);
			}
		}
		else if (incs.at(i).includeType == DM14::parser::includePath::sourceFileType::FILE_C)
		{
			headerName = incs.at(i).package + ".hpp";
		}
		else
		{
			string fullPath;
			for(uint32_t k = 0; k < includePaths.size(); k++)
			{
				ifstream ifs;			

				ifs.open(includePaths.at(k) +pathSeperator + incs.at(i).package + pathSeperator + incs.at(i).library + ".hpp");
				if(ifs.is_open())
				{
					fullPath = includePaths.at(k) +pathSeperator + incs.at(i).package + pathSeperator + incs.at(i).library + ".hpp";
					ifs.close();
					break;
				}
			}
			if(fullPath.size())
			{
				headerName = fullPath;
			}
			else
			{
				displayError("unable to find library header : " + incs.at(i).package + pathSeperator + incs.at(i).library + ".hpp");
			}
			//headerName = IncludesDir + "/" + incs.at(i).first + "/" + incs.at(i).second + ".hpp";
		}
		
		write("#include <");
		write(headerName);
		writeLine(">");
	}
	
	write("#include <");
	write("M14Defs.hpp");
	writeLine(">");
		
	return 0;
};



int compiler::compileDistributeNodes()
{
	m14FileDefs << "#define __M14DIRECTVARSCOUNT " << dVariablesNames.size() << std::endl;
	std::stringstream ss;
	ss << nodesCount;
	m14FileDefs << "#define __M14MAXIMUMNODES " << ss.str() << std::endl;
	ss.str("");
	ss.clear();
	ss << dVariablesCount;
	m14FileDefs << "#define __M14MAXIMUMVARSCOUNT " << ss.str() << std::endl;
	//writeLine("#define __M14VPOINTERS[M14VPOINTERSCOUNT] void* __M14VPOINTERS[M14VPOINTERSCOUNT]");
	//m14FileDefs << "void* __M14VPOINTERS[__M14MAXIMUMVARSCOUNT];" << endl;
	//m14FileDefs << "bool __M14VSTATUSES[__M14MAXIMUMVARSCOUNT];" << endl;
	
	
	return 0;
};

int compiler::compileGlobalStructs()
{
	for (unsigned int i =0; i < (mapCodes->at(index)).dataTypes.size(); i++)
	{
		writeLine("struct " + (mapCodes->at(index)).dataTypes.at(i).typeID);
		writeLine("{");
		for (unsigned int k =0; k < (mapCodes->at(index)).dataTypes.at(i).memberVariables.size(); k++)
		{
			writeLine((mapCodes->at(index)).dataTypes.at(i).memberVariables.at(k).returnType + " " +
					  (mapCodes->at(index)).dataTypes.at(i).memberVariables.at(k).name + ";");
			
		}
		writeLine("};");
	}
	return 0;
};

int compiler::compileGlobalDeclarations()
{
	for (unsigned int i =0; i < (mapCodes->at(index)).globalDeclarations.size(); i++)
	{
		compileDecStatement((mapCodes->at(index)).globalDeclarations.at(i), true);
	}
	return 0;
};

int compiler::compileFunction()
{
	//delete nodesModifiers;
	////dVariablesNames.clear();
	currentNode = 0;
	//nodesModifiers = new Array < pair<string, int> >();
	Array<ast_function>* funcs;
	funcs = (mapCodes->at(index)).getFunctions();
	// push main to end
	for (fIndex =0; fIndex < funcs->size(); fIndex++)
	{
		if ( (funcs->at(fIndex)).name == "main" )
		{
			//FIX100
			//funcs->swap(funcs->begin()+fIndex, funcs->end());
		}
	}
	
	// print all functions prototypes;
	for (fIndex =0; fIndex < funcs->size(); fIndex++)
	{
		if ( (funcs->at(fIndex)).name == "main")
		{
			continue;
		}
		
		//function type
		if ( (funcs->at(fIndex)).returnIDType != "NIL" )
		{
			write((funcs->at(fIndex)).returnIDType);
			write(" ");
		}
		else
		{
			write("void ");
		}
		
		// function name
		write((funcs->at(fIndex)).name+"(");
		// function paramenters
		
		for ( unsigned int k=0; k < (funcs->at(fIndex)).parameters->size(); k++ )
		{
			write( ((funcs->at(fIndex)).parameters->at(k)).type +" "+((funcs->at(fIndex)).parameters->at(k)).name);
			if ( k !=  (funcs->at(fIndex)).parameters->size()-1)
			{
				write(", ");
			}
		}
		// close function parameters
		writeLine(");");
	}
	writeLine(" ");
	
	for (fIndex =0; fIndex < funcs->size(); fIndex++)
	{
		//cout << "COMPILING FUNC : " << (funcs->at(fIndex)).name << endl;
		//function type
		if ( (funcs->at(fIndex)).returnIDType != "NIL" )
		{
			write((funcs->at(fIndex)).returnIDType);
			write(" ");
		}
		else
		{
			write("void ");
		}
		// function name
		if ( (funcs->at(fIndex)).name == "main")
		{
			write("dm14Main(");
			compilingMain = true;
		}
		else
		{
			write((funcs->at(fIndex)).name+"(");
			compilingMain = false;
		}
		
		// function paramenters
		if ((funcs->at(fIndex)).name == "main")
		{
			if ( (funcs->at(fIndex)).parameters->size() > 1)
			{
				displayError((mapCodes->at(index)).getFileName(), -1, 0,"Main should have only one parameter of type string");
			}
			
			if ( (funcs->at(fIndex)).parameters->size())
			{
				if ( ((funcs->at(fIndex)).parameters->at(0)).type != "string" )
				{
					displayError((mapCodes->at(index)).getFileName(), -1, 0,"Main should have only one parameter of type string");
				}
			}
			
		}
		
		for ( unsigned int k=0; k < (funcs->at(fIndex)).parameters->size(); k++ )
		{
			//if ((funcs->at(fIndex)).name == "main")
			//{
			//	displayWarning((mapCodes->at(index)).getFileName(), 0, 0,"Main should have no parameters !!!!");
			//	break;
			//}
			
			//compileDistributedVariable((funcs->at(fIndex)).parameters->at(k).name);
			write( ((funcs->at(fIndex)).parameters->at(k)).type +" "+((funcs->at(fIndex)).parameters->at(k)).name);
			if ( ((funcs->at(fIndex)).parameters->at(k)).value != "NIL" )
			{
				//write("="+((funcs->at(fIndex)).parameters->at(k)).value);
			}
			
			if ( k !=  (funcs->at(fIndex)).parameters->size()-1)
			{
				write(", ");
			}
		}
		// close function parameters
		writeLine(")");
		// open function body 
		writeLine("{");
		//add the return variable
		if( (funcs->at(fIndex)).returnID != "NIL" )
		{
			write((funcs->at(fIndex)).returnIDType);
			write(" ");
			write((funcs->at(fIndex)).returnID);
			writeLine(";");
			if (!(mapCodes->at(index)).isHeader())
			{
				if ( (funcs->at(fIndex)).name == "main")
				{
					for ( unsigned int k =0; k < mapCodes->size(); k++)
					{
						for ( unsigned int l =0; l < mapCodes->at(k).globalDeclarations.size(); l++)
						{
							declareStatement* decStatement = (declareStatement*) mapCodes->at(k).globalDeclarations.at(l);
							
							for ( unsigned int f =0; f < decStatement->identifiers->size(); f++)
							{
								compileAddVector(mapCodes->at(k).globalDeclarations.at(l), decStatement->identifiers->at(f), true);
							}
						}
						
						for ( unsigned int l =0; l < mapCodes->at(k).globalDefinitions.size(); l++)
						{
							compileInsider(mapCodes->at(k).globalDefinitions.at(l));
							writeLine(";");
						}
					}
					
					writeLine("Distributed.addVectorData<int>(" + (funcs->at(fIndex)).name  + (funcs->at(fIndex)).returnID + ",-1, false, false, &" + (funcs->at(fIndex)).returnID + ", false, false, \"int\");");
					compileDistributedVariable(idInfo((funcs->at(fIndex)).returnID, 0, "", NULL));
				}
			}
		}
		
		
		// loop through function statements
		//writeLine("goto "+(funcs->at(fIndex)).name+"__nodeindex__"+";");
		//writeLine("begin"+(funcs->at(fIndex)).name+":;");
		for ( unsigned int k=0; k < (funcs->at(fIndex)).body->size(); k++ )
		{
			//cout~ <<"Stmt:" << k << ";" << (funcs->at(fIndex)).body->size() << endl;
			compileInsider((funcs->at(fIndex)).body->at(k));
			writeLine(";");
			//fix1 not all statements need ; at the end, like the for loop
			
		}
		// close function body
		writeLine("goto end"+(funcs->at(fIndex)).name+";");
		writeLine((funcs->at(fIndex)).name+"__nodeindex__"+":;");
		writeLine("Distributed.dataVectorReady=true;");
		compileNodeSelector((funcs->at(fIndex)));
		writeLine("end"+(funcs->at(fIndex)).name+":;");
		
		if ( (funcs->at(fIndex)).name == "main" )
		{
			writeLine("Distributed.serve(true);");
			if ( (mapCodes->at(index)).getFunctions()->at(fIndex).distributed )
			{
				//write("return ");
				//write((mapCodes->at(index)).getFunctions()->at(fIndex).returnID);
				//writeLine(";");
			}
			//writeLine("while(true){}");
		}
		
		if((mapCodes->at(index)).getFunctions()->at(fIndex).returnID != "NIL")
		{
			write("return ");
			write((mapCodes->at(index)).getFunctions()->at(fIndex).returnID);
			writeLine(";");
		}
				
		writeLine("};");
		writeLine("");
		
		if ( (funcs->at(fIndex)).name == "main" )
		{
			writeLine("int main (int argc, char* argv[])");
			writeLine("{");
			//writeLine ("Node node;");
			writeLine ("string mainParameters;");
			writeLine("setArgs(argc, argv, Distributed, mainParameters);");
			writeLine("Distributed.nodesMaximum = __M14MAXIMUMNODES;");
			//writeLine("node.setCapacity(__M14MAXIMUMVARSCOUNT);");
			writeLine("Distributed.setCapacity(__M14DIRECTVARSCOUNT);");
			//writeLine("_DM14VARIABLESMAP = new map<string, unsigned int>();");
			//writeLine("initLocalVectorMap();");
			//writeLine("node.setServer(SERVINGIP, SERVINGPORT, 60);");
			//writeLine("node.addParent(PARENTIP,PARENTORT);");
			writeLine("Distributed.startListener(false);");
			write("return dm14Main(");
			if ( (funcs->at(fIndex)).parameters->size())
			{
				write("mainParameters");
			}
			writeLine(");");
			writeLine("}");
		}
	}
	return 0;
};

int	compiler::compileNodeSelector(ast_function& fun)
{
	writeLine("switch (Distributed.nodeNumber)");
	writeLine("{");
	std::stringstream SS;
	for (unsigned int k =0; k < fun.functionNodes.size(); k++)
	{
		SS << k+1;
		write("case "+SS.str());
		SS.str("");
		SS.clear();
		writeLine(":");
		writeLine("goto "+fun.functionNodes.at(k)+";");
		writeLine("break;");
	}
	write("default: goto begin" + fun.name + ";" );
	writeLine("break;");
	writeLine("}");
	
	return 0;
};

int compiler::compileRetStatement(statement*& stmt)
{
	if ( (mapCodes->at(index)).getFunctions()->at(fIndex).distributed )
	{
		displayWarning("return statement in a distributed function !");
		//writeLine("end"+(mapCodes->at(index)).getFunctions()->at(fIndex).name+":;");
		//return 1;
	}
	
	if( ((returnStatement*)stmt)->retValue != NULL)
	{
		write( (mapCodes->at(index)).getFunctions()->at(fIndex).returnID+ " = ");
		compileInsider(((returnStatement*)stmt)->retValue);
		writeLine(";");
	}
	//else
	{
		//write((mapCodes->at(index)).getFunctions()->at(fIndex).returnID);
	}
	
	write("goto end");
	write((mapCodes->at(index)).getFunctions()->at(fIndex).name);
	
	return 0;
}

int compiler::compileForLoop(statement*& stmt)
{
	forloop* ForLoop = (forloop*)stmt;

	write("for (");
	for (unsigned int i =0; i < ForLoop->fromCondition->size(); i++)
	{
		compileInsider(ForLoop->fromCondition->at(i));
		if (i < ForLoop->fromCondition->size()-1)
		{
			write(",");
		}
	}
	
	write(";");
	for (unsigned int i =0; i < ForLoop->toCondition->size(); i++)
	{
		compileInsider(ForLoop->toCondition->at(i));
		if (i < ForLoop->toCondition->size()-1)
		{
			write(",");
		}
	}

	
	write(";");
	for (unsigned int i =0; i < ForLoop->stepCondition->size(); i++)
	{
		compileInsider(ForLoop->stepCondition->at(i));
		if (i < ForLoop->stepCondition->size()-1)
		{
			write(",");
		}
	}
	writeLine(")");
	////srcFile << "( int i = " << ForLoop->from << "; i < " << ForLoop->to << "; i++ )";
	// open for loop body
	writeLine("{");
	// loop through the body
	for ( unsigned int i =0; i < ForLoop->body->size(); i++ )
	{
		compileInsider(ForLoop->body->at(i));
		writeLine(";");
	}
	// close for loop body
	writeLine("}");
	return 0;
	
};


int compiler::compileWhileLoop(statement*& stmt)
{
	whileloop* WhileLoop = (whileloop*)stmt;
	write("while(");
	compileInsider(WhileLoop->condition);
	writeLine(")");
	// open for loop body
	writeLine("{");
	// loop through the body
	for ( unsigned int i =0; i < WhileLoop->body->size(); i++ )
	{
		compileInsider(WhileLoop->body->at(i));
		writeLine(";");
	}
	// close for loop body
	writeLine("}");
	return 0;
};

bool compiler::isCompilingMain()
{
	return compilingMain;
}
int compiler::compileDecStatement(statement*& stmt, const bool global)
{
	bool tscope = tmpScope;
	bool dvars = declaringVars;
	declaringVars = true;

	//inline void Node::addVectorData(int var, int index, bool parray, bool pready, void* pvar, const string& ptype)
	
	declareStatement* decStatement = (declareStatement*)stmt;
	
	tmpScope = decStatement->tmpScope;
	
	if(!tmpScope && decStatement->distributed)
	{
		for ( unsigned int i = 0; i < decStatement->identifiers->size() ;i++ )
		{
			//write("_DM14VARIABLESMAP[\"");
			//writeLine(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + decStatement->identifiers->at(i) +"\"]=_DM14VARIABLESMAP.size();");
			if (  isCompilingMain() || global)
			{
				compileDistributedVariable(decStatement->identifiers->at(i), decStatement->global);
				
				if(DM14::types::isClass(decStatement->type)) // && isExternalClass(decStatement->type) declared outside DM14
				{
					std::vector<funcInfo> classVars = DM14::types::getClassMemberVariables(decStatement->type);
					
					for ( unsigned int k = 0; k < classVars.size() ; k++ )
					{
						if(classVars.at(k).classifier == DM14::types::CLASSIFIER::PUBLIC)
						{
							idInfo id = decStatement->identifiers->at(i);
							id.name = id.name + classVars.at(k).name;
							compileDistributedVariable(id, decStatement->global);
						}
					}
				}
			}
		}
	}
	

	for ( unsigned int i = 0; i < decStatement->identifiers->size() ;i++ )
	{
		//declareStatement* decStatement = (declareStatement*)stmt;
		
		
		if (decStatement->array)
		{
			write("Array<");
		}
		write(decStatement->type);
		if (decStatement->array)
		{
			write(">*");
		}
		write(" ");
		
		if(decStatement->identifiers->at(i).parent)
		{
			write(decStatement->identifiers->at(i).parent->name + decStatement->identifiers->at(i).name);
		}
		else
		{
			write(decStatement->identifiers->at(i).name);
		}
				
		
		if (decStatement->value != NULL)
		{
			write(" = ");
			compileInsider(decStatement->value);
		}
			
		if (decStatement->array)
		{
			write(" = new Array<");
			write(decStatement->type);
			
			if(decStatement->size != 0)
			{
				std::stringstream SS;
				SS <<  decStatement->size;
				write(">("+SS.str()+")");
			}
			else
			{
				write(">()");
			}
		}
		
		if(!decStatement->tmpScope)
		{
			writeLine(";");
			
			if(!global)
			{
				compileAddVector(stmt, decStatement->identifiers->at(i), global);
			}
		}
	}
	tmpScope = tscope;
	declaringVars = dvars;
	return 0;
};


int compiler::compileAddVector(statement*& stmt, const idInfo& id, const bool global)
{
	declareStatement* decStatement = (declareStatement*)stmt;

	if(!decStatement->distributed)
	{
		return 1;
	}
	
	if (!(mapCodes->at(index)).isHeader() && isCompilingMain())
	{
		//write("node.addVectorData<");
		if (decStatement->array)
		{
			write("Distributed.addVectorArrayData< Array<");
		}
		else
		{
			write("Distributed.addVectorData<");
		}
		
		write (decStatement->type);
		if (decStatement->array)
		{
			write(" > ");
		}
	
		write(">(");
	
		
		if(global)
		{
			if(id.parent)
			{
				write( "_DM14GLOBALVAR" + id.parent->name + id.name);
			}
			else
			{
				write( "_DM14GLOBALVAR" + id.name);
			}
		}
		else
		{
			if(id.parent)
			{
				write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.parent->name + id.name);
			}
			else
			{
				write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.name);
			}
		}
		
		write(",");
		
		string arraySpec;
		if(decStatement->array)
		{
			std::stringstream SS;
			SS <<  decStatement->size;
			arraySpec=SS.str()+", true";
		}
		else
		{
			arraySpec="-1, false";
		}
		
		write(arraySpec+", false, ");
		if(!decStatement->array)
		{
			write("&");
		}
		
		
		if(id.parent)
		{
			write(id.parent->name + id.name + ", ");
		}
		else
		{
			write(id.name + ", ");
		}
		
		if(decStatement->recurrent)
		{
			write("true");
		}
		else
		{
			write("false");
		}
		
		write(", ");
		
		if(decStatement->channel)
		{
			write("true");
		}
		else
		{
			write("false");
		}
		
		writeLine(", \"" +  decStatement->type + "\");");
		
		
		if(DM14::types::isClass(decStatement->type)) // && isExternalClass(decStatement->type) declared outside DM14
		{
			if(decStatement->array)
			{
				std::stringstream SS;
				SS <<  decStatement->size;
				writeLine("for (unsigned int i =0; i < " + SS.str() + "; i++)");
				writeLine("{");
			}
			
			std::vector<funcInfo> classVars = DM14::types::getClassMemberVariables(decStatement->type);	
			for ( unsigned int k = 0; k < classVars.size() ; k++ )
			{
				if(classVars.at(k).classifier != DM14::types::CLASSIFIER::PUBLIC)
				{
					continue;
				}
				
				write("Distributed.addVectorClassData<");
				
				write (classVars.at(k).returnType);
				write(">(");
				if(decStatement->global)
				{
					if(id.parent)
					{
						write( "_DM14GLOBALVAR" + id.parent->name + id.name + classVars.at(k).name);// + classVars.at(k).name);
					}
					else
					{
						write( "_DM14GLOBALVAR"  + id.name + classVars.at(k).name);// + classVars.at(k).name);
					}
				}
				else
				{
					if(id.parent)
					{
						write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.parent->name + id.name);// + classVars.at(k).name);
					}
					else
					{
						write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.name);
					}
					write(classVars.at(k).name);// + classVars.at(k).name);
				}
				
				write(",");
				string arraySpec;
				if(decStatement->array)
				{
					//stringstream SS;
					//SS <<  decStatement->size;
					//arraySpec=SS.str()+", true";
					arraySpec= "i, false";
				}
				else
				{
					arraySpec="-1, false";
				}
				write(arraySpec+", false, ");
				write("&");
				write(id.name);
				if(decStatement->array)
				{
					write("->at(i)");
				}
				write("."+ classVars.at(k).name + ", ");
		
				if(decStatement->recurrent)
				{
					write("true");
				}
				else
				{
					write("false");
				}
		
				write(", ");
		
				if(decStatement->channel)
				{
					write("true");
				}
				else
				{
					write("false");
				}
				
				writeLine(", \"" +  classVars.at(k).returnType + "\");");
			}
			if(decStatement->array)
			{
				writeLine("}");
			}
		}
	}
	
	return 0;
};
int compiler::compileOpStatement(statement*& stmt)
{
	if(!stmt)
	{
		return 0;
	}
	
	operationalStatement* opStatement = (operationalStatement*)stmt;

	if (opStatement->op == "@")
	{
		
		printStatement(stmt, 0);
		if(!(operationalStatement*)opStatement->left)
		{
			termStatment* innerOp = (termStatment*)opStatement->right;
			//if (innerOp->right) // node address
			write("Distributed.getLastActiveNode(");
			write(generateDistributedVariableName(innerOp->id));
			write(", -1)");
			//Distributed.done<string>(_DM14GLOBALVARResponse,-1,&localResult, nodeID);
			return 0;
		}
	}
	
	
	//write identifiers
	bool bigStatement = false;
	
	if (opStatement->scopeLevel > scopeLevel)
	{
		bigStatement = true;
		scopeLevel = opStatement->scopeLevel;
	}
	
	if(bigStatement)
	{
		//write("(");
	}
	
	if (opStatement->left != NULL)
	{
		if (opStatement->left->scopeLevel > scopeLevel)
		{
			write("(");
		}
	
		compileInsider(opStatement->left);
		
		if (opStatement->left && opStatement->left->scopeLevel > scopeLevel)
		{
			write(")");
		}
	}
	//write operator
	write(opStatement->op);
	
	
	//write right side
	if (opStatement->right != NULL)
	{
		bool tt = false;
		if (opStatement->right->scopeLevel > scopeLevel)
		{
			tt = true;
			scopeLevel = opStatement->right->scopeLevel;
		}
		
		if (tt)
		{
			write("(");
		}
		
		compileInsider(opStatement->right);
		
		if (tt)
		{
			write(")");
		}
	}
	
	if(bigStatement)
	{
		//write(")");
	}
	
	/*
	if (opStatement->op == "@")
	{
		if((operationalStatement*)opStatement->left)
		{
			//operationalStatement* innerOp = (operationalStatement*)((operationalStatement*)opStatement->right)->right;
			//if (innerOp->right) // node address
			write(";");
			writeLine("Distributed.getLastActiveNode(");
			compileInsider(opStatement->left);
			compileInsider(opStatement->right);
			write(", -1)");
			return 0;
		}
	}*/
	
	
	return 0;
};

int compiler::compileFunctionCall(statement*& stmt)
{
	functionCall* funCall = (functionCall*)stmt;
	// function name
	write(funCall->name);
	write("(");
	bool implemented = false;
	
	if (funCall->functionType == DM14::types::types::USERFUNCTION)
	{
		for (unsigned int i =0; i < mapCodes->size() && funCall->functionType != DM14::types::types::BUILTINFUNCTION; i++)
		{
			for ( unsigned int x =0; x < (mapCodes->at(i)).getFunctions()->size(); x++)
			{
				if ( (mapCodes->at(i)).getFunctions()->at(x).name == funCall->name  )
				{
					implemented = true;
				}
			}
		}
	}
	else
	{
		implemented = true;
	}
	
	if (!implemented)
	{
		std::stringstream ss;
		ss <<  funCall->line ;
		displayError(funCall->name + " is not implemnted, called at: " + ss.str());
	}
	
	// function parameters
	for ( unsigned int i=0; i < funCall->parameters->size(); i++)
	{
		//srcFile << funCall->parameters->at(i);
		compileInsider(funCall->parameters->at(i));
		if (i != (funCall->parameters->size() - 1 ) )
		{
			write(", ");
		}
	}
	write(")");
	return 0;
};

int compiler::compileIF(statement*& stmt)
{
	IF* IFstatement = (IF*)stmt;	
	write("if ( ");
	//compileOpStatement(IFstatement->condition);
	compileInsider(IFstatement->condition);
	writeLine(" )");
	writeLine("{");
	
	for ( unsigned int i =0; i < IFstatement->body->size(); i++ )
	{
		compileInsider(IFstatement->body->at(i));
		writeLine(";");
	}
	
	writeLine("}");
	
	for ( unsigned int i =0; i < IFstatement->elseIF->size(); i++ )
	{
		write("else ");
		compileInsider(IFstatement->elseIF->at(i));
		//writeLine(";");
	}
	
	if ( IFstatement->ELSE->size() != 0 )
	{
		writeLine("else");
		writeLine("{");
		for ( unsigned int i =0; i < IFstatement->ELSE->size(); i++ )
		{
			compileInsider(IFstatement->ELSE->at(i));
			writeLine(";");
		}	
		writeLine("}");
	}
	return 0;
};

int compiler::compileCASE(statement*& stmt)
{
	CASE* CASEstatement = (CASE*)stmt;
	map<statement*, Array<statement*> >::iterator IT;
	for ( IT = CASEstatement->Body.begin(); IT != CASEstatement->Body.end(); ++IT )
	{
		if ( IT == CASEstatement->Body.begin() )
		{
			write("if (");
		}
		else
		{
			write("else if (");
		}		
		compileInsider(CASEstatement->condition);	
		write(" == ");
		compileInsider((statement*&)IT->first);
		writeLine(")");
		writeLine("{");
		for ( unsigned int i =0; i < IT->second.size(); i++ )
		{
			compileInsider(IT->second.at(i));
			writeLine(";");
		}
		writeLine("}");
	}
	return 0;
};

int compiler::compileInsider(statement*& stmt, iostream* outstream, bool overridedStream)
{
	if (stmt == NULL)
	{
		return 0;
	}
	
	iostream* originalOutStream = outStream;
	
	if(!outstream && overridedStream)
	{
		outStream = &srcFile;
	}
	else
	{
		if(overridedStream)
		{
			outStream = outstream;
		}
	}
	
	for(unsigned int i =0; i < stmt->distStatements.size(); i++)
	{
		if ( (stmt->distStatements.at(i))->type == distributingVariablesStatement::DEPS)
		{
			compileDitributingVariables((statement*&)stmt->distStatements.at(i));
			stmt->distStatements.erase(stmt->distStatements.begin()+i);
			i--;
			
		}
	}
	
	switch (stmt->statementType)
	{
		case rStatement	:		compileRetStatement(stmt);
								break;
		case dStatement :		compileDecStatement(stmt);
								break;
		case fCall :			compileFunctionCall(stmt);
								break;
		case fLoop :			compileForLoop(stmt);
								break;
		case IFStatement :		compileIF(stmt);
								break;
		case CASEStatement :	compileCASE(stmt);
								break;
		case oStatement :		compileOpStatement(stmt);
								scopeLevel=0;
								break;
		case tStatement :  		compileTerm(stmt);
								break;
		case wLoop		:  		compileWhileLoop(stmt);
								break;
		case DISTStatement:		compileDistribute(stmt);
								break;
		case dvStatement:		compileDitributingVariables(stmt);
								break;
		case RESETStatement:	compileResetStatement(stmt);
								break;
		case PAStatement:		compileParentAddStatement(stmt);
								break;
		case EXTERNStatement:	compileExtern(stmt);
								break;
		case THREADStatement:	compileThread(stmt);
								break;
		case NOPSTATEMENT:		compileNOPStatement(stmt);
								break;
		default : return(-1);
	}
	
	
	while(stmt->splitStatements.size())
	{
		compileInsider(stmt->splitStatements.at(0));
		stmt->splitStatements.remove(0);
	}
	
	for(unsigned int i =0; i < stmt->distStatements.size(); i++)
	{
		if ( (stmt->distStatements.at(i))->type == distributingVariablesStatement::MODS)
		{
			writeLine(";");
			compileDitributingVariables((statement*&)stmt->distStatements.at(i));
			stmt->distStatements.erase(stmt->distStatements.begin()+i);
			i--;
		}
	}
	
	if(overridedStream)
	{
		outStream = originalOutStream;
	}
	return 0;
};

string compiler::generateDistributedVariableName(idInfo* id)
{
	if(!id)
	{
		return "";
	}
	
	string name;
	
	string parentName;
	
	if(id->parent)
	{
		parentName = id->parent->name;
	}
	
	if(id->global)
	{
		name = "_DM14GLOBALVAR" + parentName + id->name;
	}
	else
	{
		name = id->functionParent + parentName + id->name;
	}
	
	return name;
};


int compiler::compileDistributedVariable(const idInfo& id, const bool global)
{
	 if ((mapCodes->at(index)).isHeader() && !global)
	 {
		 return 1;
	 }
	 
	//string name = (mapCodes->at(index)).getFileName() + ( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id;
	string name;
	
	if(global)
	{
		//name = (mapCodes->at(index)).getFileName()+ id;
		if(id.parent)
		{
			name = "_DM14GLOBALVAR" + id.parent->name + id.name;
		}
		else
		{
			name = "_DM14GLOBALVAR" + id.name;
		}
	}
	else
	{
		if(id.parent)
		{
			name = ( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.parent->name + id.name;
		}
		else
		{
			name = ( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.name;
		}
	}
	
	for (unsigned int i =0; i < dVariablesNames.size(); i++)
	{
		if (dVariablesNames.at(i) == name)
		{
			return 0;
		}
	}
	m14FileDefs << "#define " << name << " " << dVariablesNames.size() << std::endl;
	dVariablesNames.push_back(name);
	
	DatatypeBase dType =  DM14::types::findDataType(id.type);
	if(dType.classType)
	{
		for(unsigned int i =0; i < dType.memberVariables.size(); i++)
		{
			idInfo id2;
			id2.name = dType.memberVariables.at(i).name;
			
			id2.parent = new idInfo();
			*id2.parent = id;
			compileDistributedVariable(id2, global);
		}
	}
	return 0;
};


int compiler::compileTerm(statement*& stmt)
{
	termStatment* termstatement = (termStatment*)stmt;
	if (termstatement->term == "DM14FUNCTIONBEGIN")
	{
		writeLine("goto "+( (mapCodes->at(index)).getFunctions()->at(fIndex)).name+"__nodeindex__"+";");
		writeLine("begin"+( (mapCodes->at(index)).getFunctions()->at(fIndex)).name+":;");
		return 0;
	}

	if(DM14::types::isEnum(termstatement->term))
	{
		DatatypeBase dt = DM14::types::findDataType(termstatement->term);
		if (dt.parents.size())
		{
			write(dt.parents.at(0)+"::");
		}
		//write(termstatement->id->parent->name);
	}
	write(termstatement->term);
	
	
	/*if (termstatement->arrayIndex != -1)
	{
		//cout~ << termstatement->term << ":" << termstatement->arrayIndex << endl;
		srcFile << "->at(";
		stringstream ss;
		ss << termstatement->arrayIndex;
		srcFile << ss.str();
		srcFile << ")";
	}*/
	
	if (termstatement->arrayIndex != NULL)
	{
		//cout~ << termstatement->term << ":" << termstatement->arrayIndex << endl;
		write("->at(");
		compileInsider(termstatement->arrayIndex);
		write(")");
	}
	
	if (termstatement->identifier)
	{
		string id = termstatement->term;
		
		/*if ( termstatement->arrayIndex != -1)
		{
			stringstream SS;
			SS << termstatement->arrayIndex;
			id += SS.str();
		}*/

		if ( ((mapCodes->at(index)).getFunctions()->at(fIndex)).scope <= termstatement->scope)
		{
			/*if( ! tmpScope && declaringVars)
			{
				compileDistributedVariable(id);
			}*/
			
			/*if(termstatement->size > 0)
			{
				stringstream SS;
				//cout~ << "SIZE" << termstatement->size << endl;
				for (unsigned int i =0; i < termstatement->size ; i++)
				{
					SS << i;
					compileDistributedVariable(id + SS.str());
					SS.str("");
					SS.clear();
				}
			}*/
		}
	}

	return 0;
};



int compiler::compileOuterExtern()
{
	if (!(mapCodes->at(index)).ExternCodes)
	{
		return 1;
	}
	
	for (unsigned int i =0; i < (mapCodes->at(index)).ExternCodes->size(); i++)
	{
		writeLine((mapCodes->at(index)).ExternCodes->at(i));
	}

	return 0;
};

int compiler::compileThread(statement*& stmt)
{
	threadStatement* threadStmt = (threadStatement*)stmt;
	
	string ID = "_DM14THREAD";
	ID += threadStmt->classID + threadStmt->parentID + threadStmt->ID + threadStmt->Identifier;
	
	string functionID = "_DM14THREADFUNCTION";
	functionID += (threadStmt->classID + threadStmt->parentID + threadStmt->ID);
	functionID +=  threadStmt->Identifier;
	
	writeLine( "threadMember* " +  ID + " = new threadMember();");
	write(ID + "->start(" +  functionID + ",");
	
	if(threadStmt->classMember)
	{
		writeLine( "&" + threadStmt->parentID + ");");
		m14FileDefs <<  threadStmt->returnType << " " << functionID << "(void*);" << std::endl;
		
		bufferedOutput +=  threadStmt->returnType + " " + functionID + "(void* caller)\n";
		bufferedOutput += "{\n";
		bufferedOutput += "return ((" +  threadStmt->classID + "*)caller)->";
		std::stringstream SS;
		compileInsider(threadStmt->functioncall, &SS, true);
		bufferedOutput += SS.str();
		bufferedOutput += ";\n};\n";
	}
	else
	{
		writeLine("NULL);");
		//m14FileDefs <<  threadStmt->returnType << " " << functionID << "();" << endl;
		m14FileDefs <<  "void* " << functionID << "();" << std::endl;
		
		//bufferedOutput +=  threadStmt->returnType + " " + functionID + "()\n";
		bufferedOutput +=  "void* " + functionID + "()\n";
		bufferedOutput += "{\n";
		//bufferedOutput += "return (void*)";
		std::stringstream SS;
		compileInsider(threadStmt->functioncall, &SS, true);
		bufferedOutput += SS.str();
		bufferedOutput += ";\n};\n";
	}
	
	return 0;
};

int compiler::compileExtern(statement*& stmt)
{
	EXTERN* Extern = (EXTERN*)stmt;
	write("{" + Extern->body + "}");

	return 0;
};


int compiler::compileParentAddStatement(statement*& stmt)
{
	//cout~ << "ARENTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" <<endl;
	parentAddStatement* ap = (parentAddStatement*) stmt;
	write("Distributed.addParent(");
	compileInsider(ap->ip);
	write(", ");
	compileInsider(ap->socket);
	writeLine(");");
	return 0;
};


int compiler::compileNOPStatement(statement*& stmt)
{
	writeLine(";");
	return 0;
};

int compiler::compileSetNodeStatement(statement*& stmt)
{
	setNodeStatement* ns = (setNodeStatement*) stmt;
	write("Distributed.changeNodeNumber(");
	compileInsider(ns->node);
	writeLine(");");
	return 0;
};


int compiler::compileDitributingVariables(statement*& stmt)
{
	if ((mapCodes->at(index)).isHeader())
	{
		return 1;
	}
	 
	//stringstream SS;

	distributingVariablesStatement* dvstatement = (distributingVariablesStatement*)stmt;
	//SS << ( (dvstatement->variable.type & dvstatement->variable.scope) + (int)dvstatement->variable.name.at(0));
	if(dvstatement->type == distributingVariablesStatement::MODS)
	{
		//cout~ << "NEED1111:" << dvstatement->variables.at(i).name << ":" << dvstatement->variables.at(i).arrayIndex << endl;
		write("Distributed.done<");
		if (dvstatement->variable.array && (dvstatement->variable.arrayIndex == NULL))
		{
			write(" Array< ");
		}
		write (dvstatement->variable.type);
		if (dvstatement->variable.array && (dvstatement->variable.arrayIndex == NULL) )
		{
			write(" > ");
		}
		write(">(");
		
		
		if(dvstatement->variable.global)
		{
			if(dvstatement->variable.parent)
			{
				write( "_DM14GLOBALVAR" +dvstatement->variable.parent->name +dvstatement->variable.name +",");
			}
			else
			{
				write( "_DM14GLOBALVAR" +dvstatement->variable.name +",");
			}
		}
		else
		{
			if(dvstatement->variable.parent)
			{
				write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name +dvstatement->variable.parent->name +dvstatement->variable.name +",");
			}
			else
			{
				write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + dvstatement->variable.name +",");
			}
		}
	
		if (dvstatement->variable.arrayIndex != NULL)
		{
			compileInsider(dvstatement->variable.arrayIndex);
		}
		else
		{
			
			if(dvstatement->variable.parent &&  dvstatement->variable.parent->arrayIndex != NULL)
			{
				compileInsider(dvstatement->variable.parent->arrayIndex);
			}
			else
			{
				write("-1");
			}
		}
		write(",");
		
		if (!dvstatement->variable.array || (dvstatement->variable.arrayIndex != NULL))
		{
			write("&");
		}
		
		if(dvstatement->variable.parent && dvstatement->variable.parent->name.size())
		{
			
			write(dvstatement->variable.parent->name);
			if(dvstatement->variable.parent->arrayIndex)
			{
				write("->at(");
				compileInsider(dvstatement->variable.parent->arrayIndex);
				write(")");
			}
			write(".");
		}
		write(dvstatement->variable.name);
		if (dvstatement->variable.arrayIndex != NULL)
		{
			write("->at(");
			compileInsider(dvstatement->variable.arrayIndex);
			write(")");
		}
		if(dvstatement->variable.requestAddress.size())
		{
			write(",");
			write(dvstatement->variable.requestAddress);
		}
		writeLine(");");
	}
	else if(dvstatement->type == distributingVariablesStatement::DEPS)
	{
		if(dvstatement->variable.backProp)
		{
			if(currentNode == (mapCodes->at(index)).nodesCount || dvstatement->dependencyNode == -1)
			{
				return 1;
			}
		}
		writeDepedency(dvstatement->variable, dvstatement->dependencyNode);
	}
	
	return 0;
};




int compiler::writeDepedency(idInfo& id, int node)
{
	if(node == -1)
	{
		return 1;
	}
	
	std::stringstream SS;
				
	write( "Distributed.need<");
	if (id.array && (id.arrayIndex == NULL))
	{
		write(" Array< ");
	}
	write (id.type);
	if (id.array && (id.arrayIndex == NULL))
	{
		write(" > ");
	}
	write(">(");
	
	if(id.global)
	{
		if(id.parent)
		{
			write( "_DM14GLOBALVAR" + id.parent->name +id.name +",");
		}
		else
		{
			write( "_DM14GLOBALVAR" + id.name +",");
		}
	}
	else
	{
		if(id.parent)
		{
			write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.parent->name +id.name +",");
		}
		else
		{
			write(( (mapCodes->at(index)).getFunctions()->at(fIndex)).name + id.name +",");
		}
	}
	
	if (id.arrayIndex != NULL)
	{
		compileInsider(id.arrayIndex);
	}
	else
	{
		if(id.parent && id.parent->arrayIndex)
		{
			compileInsider(id.parent->arrayIndex);
		}
		else
		{
			write("-1");
		}
		
	}
	write(",");
	if (!id.array && ! id.pointer)
	{
		write("false");	
	}
	else
	{
		write("true");	
	}
			
	SS.str("");
	SS.clear();
	
	if(id.arrayIndex != NULL)
	{
		//SS << -2;
		SS << node;
	}
	else
	{
		SS << node;
	}
				
	write("," + SS.str() + ",");
				
	//if (!id.array && ! id.pointer)
	{
		write("&");	
	}
	
	if(id.parent && id.parent->name.size())
	{
		write(id.parent->name);
		
		if(id.parent->arrayIndex)
		{
			write("->at(");
			compileInsider(id.parent->arrayIndex);
			write(")");
		}
		write(".");
	}
						
	write(id.name);
			
	if (id.arrayIndex != NULL)
	{
		write ("->at(");
		compileInsider(id.arrayIndex);
		write(")");
	}
	
	if(!id.noblock)
	{
		write(", true");
	}
	else
	{
		write(", false");
	}
	
	if(id.requestAddress.size())
	{
		write(",");
		write(id.requestAddress);
	}
						
	writeLine(");");
					
	//writeLine("}");
						
	SS.str("");
	SS.clear();	 
	if (id.arrayIndex == NULL)
	{
		return 1;
	}
	writeLine(";");
	return 0;
}

int compiler::compileDistribute(statement*& stmt)
{	
	//distStatement* diststatement = (distStatement*)stmt;
	std::string funcName = ((mapCodes->at(index)).getFunctions()->at(fIndex)).name;
	if (funcName != "main")
	{
		displayError((mapCodes->at(index)).getFileName(), stmt->line,0,"Distribute statement outside main function, ignored.");
		//return 0;
	}

	writeLine("goto end"+funcName+";");
	std::stringstream ss;
	ss << stmt->line;
	funcName += ss.str();
	writeLine(funcName+":");
	currentNode++;
	return 0;
};

int compiler::compileResetStatement(statement*& stmt)
{
	resetStatement* rs = (resetStatement*) stmt;
	write("if ( M14RESETCOUNTER != (");
	compileInsider(rs->count);
	writeLine(") )");
	writeLine("{");
	writeLine("M14RESETCOUNTER++;");
	writeLine("goto " +  ((mapCodes->at(index)).getFunctions()->at(fIndex)).name +"__nodeindex__"+";");
	writeLine("}");
	return 0;
};

}