#include "DM14GLOBAL.m14.hpp"
#include "includes/core/Node.hpp"
#include "includes/core/Array.hpp"
#include "includes/io/io.hpp"
#include "includes/io/File.hpp"
#include "includes/io/string.hpp"
#include "includes/math/math.hpp"
#include "includes/core/Sleep.hpp"
#include "includes/core/common.hpp"
#include "includes/sys/sys.hpp"
#include "includes/sys/time.hpp"
#include "M14Defs.hpp"
#include <vector>

string Response;
string Line;
string Command;
bool isOperator(string command);
bool isNumber(string command);
bool validateCommand(string command);
string processEquation(string command, string nodeID);
 
bool isOperator(string command)
{
	if ( command=="+" || command=="-" || command=="*" || command=="/" )
	{
		return true;
	}

	return false;
};

bool isNumber(string command)
{
	int size;
	int i;
	size=str_size(command);
	i=0;
	while(i<size)
	{
		char c = str_at(command, i);
		if ( c<'0'||c>'9' )
		{
			return false;
		}
		i++;
	}
	
	return true;
};

bool validateCommand(string command)
{
	Distributed.dataVectorReady=true;
	int i;

	displayInfo("Validating : "+command);
	i=tokenCount(command, ' ');
	while(i>0)
	{
		if ( (!isNumber(getToken(command, ' ', i))) &&(!isOperator(getToken(command, ' ', i))) )
		{
			displayError("Error in token : |"+getToken(command, ' ', i)+"|");
			return false;
		}
		i--;
	}

	if (!isOperator(getToken(command, ' ', tokenCount(command, ' '))) )
	{
		displayError("Command does not terminate with an operator : "+getToken(command, ' ', tokenCount(command, ' ')));
		return false;
	}

	displayInfo("Perfect !");
	return true;
};


string processEquation(string command, string nodeID)
{
	Distributed.dataVectorReady=true;
	displayInfo("Calculating : "+ command + " from " + nodeID);

	string localResult;
	
	std::vector<string> numbers;
	std::vector<string> operators;
	
	
	int count =tokenCount(command, ' ');
	int i = 0;
	
	while(i<= count)
	{
		string token = getToken(command, ' ', i);
		cout << "Token : " << token  << endl;
		if (isNumber(token))
		{
			numbers.push_back(token);
			
			cout << "STACK : ";
			for(int i =0; i < numbers.size(); i++)
			{
				cout << numbers.at(i) << ",";
			}
			cout << endl << flush;
		}
		else if (isOperator(token))
		{
			if(numbers.size() > 1)
			{
				int two = strToNumber(numbers.at(numbers.size()-1));
				numbers.pop_back();
				int one = strToNumber(numbers.at(numbers.size()-1));
				numbers.pop_back();
				
				if (token == "+")
				{
					numbers.push_back(numberToStr(one + two));
				}
				else if (token == "-")
				{
					numbers.push_back(numberToStr(one - two));
				}
				else if (token == "*")
				{
					numbers.push_back(numberToStr(one * two));
				}
				else if (token == "/")
				{
					numbers.push_back(numberToStr(one / two));
				}
			}
			else
			{
				operators.push_back(token);
				cout << "Operators : ";
				for(int i =0; i < operators.size(); i++)
				{
					cout << operators.at(i) << ",";
				}
				cout << endl << flush;
			}
		}
		i++;
	}
	
	cout << "Result : " << numbers.at(numbers.size()-1) << endl << flush;
	localResult = numbers.at(numbers.size()-1);
	
	Distributed.done<string>(_DM14GLOBALVARResponse,-1,&localResult, nodeID);
	return localResult;
};



int main (int argc, char* argv[])
{
	string args;
	setArgs(argc, argv, Distributed, args);
	Distributed.nodesMaximum = __M14MAXIMUMNODES;
	Distributed.setCapacity(__M14DIRECTVARSCOUNT);
	Distributed.startListener(false);

	int result;
	Distributed.addVectorData<string>(_DM14GLOBALVARResponse,-1, false, false, &Response, false, true, "string");
	Distributed.addVectorData<int>(mainresult,-1, false, false, &result, false, false, "int");
	string Equation;
	Distributed.addVectorData<string>(mainEquation,-1, false, false, &Equation, false, true, "string");
	File file;
	Distributed.addVectorData<File>(mainfile,-1, false, false, &file, false, false, "File");
	int Counter;
	string Adress;
	int Port;
	
	goto main__nodeindex__;
	beginmain:
	Distributed.checkTicks=1000000;
	file.open("rpn.txt");
	Line=file.readLine();
	Counter=1;
	Port=1235;
	Distributed.checkTicks=1000000;
	nspill("done launching RPN tests");
	while(true)
	{
		Distributed.need<string>(mainEquation,-1,false,-2,&Equation, true);
		processEquation(Equation, Distributed.getLastActiveNode(mainEquation, -1));
	}

	goto endmain;
	main155:
	Distributed.checkTicks=1000000;
	if ( args=="" )
	{
		displayWarning("error, expcted more arguments ");
		nspill(tokenCount(args, ';'));
		goto endmain;
	}
	nspill(" Args : "+args);
	if ( validateCommand(args)==false )
	{
		nspill("error parsing input !");
		Distributed.Exit();
		Exit(1);
	}

	Sleep(5);
	Equation=args;
	Distributed.done<string>(mainEquation,-1,&Equation);
	nspill("Wrote Variable");
	Distributed.need<string>(_DM14GLOBALVARResponse,-1,false,-2,&Response, true);
	displayInfo("Solution is : "+Response);
	Distributed.Exit();
	Exit(0);
	
	goto endmain;
	main__nodeindex__:;

	Distributed.dataVectorReady=true;
	switch (Distributed.nodeNumber)
	{
		case 1:  goto main155;
				 break;
		default: goto beginmain;break;
	}
	endmain:;
	Distributed.serve(true);
	return result;
}
