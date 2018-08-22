#include "main.hpp"
#include <vector>
#include <map>
#include <string>
//std::map <std::string, std::vector<string> > EBNF;

class InputParser
{
	//https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
    public:
        InputParser (int &argc, char **argv)
		{
            for (int i=1; i < argc; ++i)
			{
                this->tokens.push_back(std::string(argv[i]));
			}
        }

        const std::string& getCmdOption(const std::string &option) const
		{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end())
			{
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }

        bool cmdOptionExists(const std::string &option) const
		{
            return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};


// we need boolean and arrays
int main(int argc, char** argv)
{
	InputParser input(argc, argv);
 
    

	// when  variavle declared , if no value initialized , then just make a pointer to it
	// wait when the operational statement that assigns it , then initialize it with NEW , and use it as *P :)
	// make a vector with initialzed variables to check if its initialized yet or not :)

	intSymbols();
	string fname;
	

	if(input.cmdOptionExists("--sources"))
	{
        fname = input.getCmdOption("--sources");
    	if (fname.empty())
		{
        	fname = "testsrc.m14";
    	}
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
	
	if(input.cmdOptionExists("-I"))
	{
        const std::string &includePath = input.getCmdOption("-I");
    	if (!includePath.empty())
		{
        	//Compiler->addIncludePath(includePath);
    	}
    }

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
