#include "main.hpp"
#include <vector>
#include <map>
#include <string>
//std::map <std::string, std::vector<string> > EBNF;




// we need boolean and arrays
int main(int argc, char** argv)
{
	// when  variavle declared , if no value initialized , then just make a pointer to it
	// wait when the operational statement that assigns it , then initialize it with NEW , and use it as *P :)
	// make a vector with initialzed variables to check if its initialized yet or not :)

	intSymbols();
	string fname;
	
	if (argc > 1)
	{
		fname = argv[1];
	}
	else
	{
		 fname = "testsrc.m14";
	 }
	
	//cout << " Scanning ... [" << fname << "]" << endl;
	displayInfo(" Scanning  ... [" + fname + "]");
	scanner* scner = new scanner(fname);
	scner->setShortComment("~~");
	scner->setLongComment("~*", "*~");
	scner->scan();
	scner->printTokens();
	//exit(1);
	
	displayInfo(" Parsing   ... [" + fname + "]");
	parser* prser = new parser(scner->getTokens(), fname, false);
	//prser->printEBNF();
	prser->parse();
	
	
	//cout << " Compiling ... " << endl;
	displayInfo(" Compiling  ... [" + fname + "]");
	compiler* Compiler = new compiler(prser->getMapCodes());
	Compiler->setVersion(0.01);
	Compiler->setIncludesDir("includes");
	Compiler->setgccPath("");
	Compiler->setcompileStatic(false);
	Compiler->compile();
	
	delete scner;
	delete prser;
	delete Compiler;
	
	return 0;
}
