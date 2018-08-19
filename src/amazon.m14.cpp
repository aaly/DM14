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
string Equation;
string Response;
string Line;
string Command;
bool isOperator(string command);
bool isNumber(string command);
bool validateCommand(string command);
void processEquation(string command, string nodeID);
 
int dm14Main(string args)
{
int result;
Distributed.addVectorData<string>(_DM14GLOBALVAREquation,-1, false, false, &Equation, false, false, "string");
Distributed.addVectorData<string>(_DM14GLOBALVARResponse,-1, false, false, &Response, false, true, "string");
Distributed.addVectorData<int>(mainresult,-1, false, false, &result, false, false, "int");
File file;
Distributed.addVectorData<File>(mainfile,-1, false, false, &file, false, false, "File");
;
int Counter;
;
string Adress;
;
int Port;
;
goto main__nodeindex__;
beginmain:;
;
;
;
Distributed.checkTicks=1000;
file.open("rpn.txt");
Line=file.readLine();
;
Counter=1;
Port=1235;
;
Distributed.checkTicks=1000;
nspill("done launching RPN tests");
while(true)
{
Distributed.need<string>(_DM14GLOBALVAREquation,-1,false,1,&Equation, true);
threadMember* _DM14THREADprocessEquation1562 = new threadMember();
_DM14THREADprocessEquation1562->start(_DM14THREADFUNCTIONprocessEquation1562,NULL);
;
}
;
goto endmain;
main67:
;
;
Distributed.checkTicks=1000;
if ( args=="" )
{
displayWarning("error, expcted more arguments ");
nspill(tokenCount(args, ';'));
goto endmain;
}
;
nspill(" Args : "+args);
if ( validateCommand(args)==false )
{
nspill("error parsing input !");
Distributed.Exit();
Exit(1);
}
;
Sleep(5);
Equation=args;
Distributed.done<string>(_DM14GLOBALVAREquation,-1,&Equation);
;
nspill("Wrote Variable");
Distributed.need<string>(_DM14GLOBALVARResponse,-1,false,-2,&Response, true);
nspill("Solution is : "+Response);
Distributed.Exit();
Exit(0);
goto endmain;
main__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
case 1:
goto main67;
break;
default: goto beginmain;break;
}
endmain:;
Distributed.serve(true);
return result;
};

int main (int argc, char* argv[])
{
string mainParameters;
setArgs(argc, argv, Distributed, mainParameters);
Distributed.nodesMaximum = __M14MAXIMUMNODES;
Distributed.setCapacity(__M14DIRECTVARSCOUNT);
Distributed.startListener(false);
return dm14Main(mainParameters);
}
bool isOperator(string command)
{
bool result;
goto isOperator__nodeindex__;
beginisOperator:;
;
if ( command=="+"||command=="-"||command=="*"||command=="/" )
{
result=true;
}
else
{
result=false;
}
;
goto endisOperator;
isOperator__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginisOperator;break;
}
endisOperator:;
return result;
};

bool isNumber(string command)
{
bool result;
int size;
;
int i;
;
goto isNumber__nodeindex__;
beginisNumber:;
;
result=true;
size=str_size(command);
i=0;
while(i<size)
{
char c = str_at(command, i);
;
if ( c<'0'||c>'9' )
{
result=false;
break;
;
}
;
i++;
}
;
goto endisNumber;
isNumber__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginisNumber;break;
}
endisNumber:;
return result;
};

bool validateCommand(string command)
{
bool result;
int i;
;
goto validateCommand__nodeindex__;
beginvalidateCommand:;
;
displayInfo("Validating : "+command);
i=tokenCount(command, ' ');
while(i>0)
{
if ( (isNumber(getToken(command, ' ', i))==false==false) )
{
displayError("Error in token : "+getToken(command, ' ', i));
result=false;
goto endvalidateCommand;
}
;
i--;
}
;
if ( isOperator(getToken(command, ' ', tokenCount(command, ' ')))==false )
{
displayError("Command does not terminate with an operator : "+getToken(command, ' ', i));
result=false;
goto endvalidateCommand;
}
;
displayInfo("Perfect !");
result=true;
goto endvalidateCommand;
validateCommand__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginvalidateCommand;break;
}
endvalidateCommand:;
return result;
};

void processEquation(string command, string nodeID)
{
int i;
;
goto processEquation__nodeindex__;
beginprocessEquation:;
;
displayInfo("Calculating : "+command);
i=tokenCount(command, ' ');
while(i>0)
{
if ( isNumber(getToken(command, ' ', i))==false )
{
displayError("NUMBER");
}
;
if ( isOperator(getToken(command, ' ', i))==false )
{
displayError("OPERATOR");
}
;
i--;
}
;
Response="result";
Distributed.done<string>(_DM14GLOBALVARResponse,-1,&Response,nodeID);
;
goto endprocessEquation;
processEquation__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginprocessEquation;break;
}
endprocessEquation:;
};

void* _DM14THREADFUNCTIONprocessEquation1562()
{
Distributed.need<string>(_DM14GLOBALVAREquation,-1,false,1,&Equation, true);
processEquation(Equation, Distributed.getLastActiveNode(_DM14GLOBALVAREquation, -1));
};
