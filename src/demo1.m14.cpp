#include "includes/io/io.hpp"
#include "M14Defs.hpp"
int factorial(int n);
 
int factorial(int n)
{
int nFactorial;
node.addVectorData(factorialnFactorial,-1, false, false, &nFactorial, "int");
goto factorial__nodeindex__;
beginfactorial:;
;
if ( n<0 )
{
nspill("Number is a negative, converting to positive.");
n=n*-1;
}
;
node.done(factorialn,-1,&n);
;
nFactorial=n;
for (int i = 0;i<n-1;i++)
{
nFactorial*=(i+1);
}
;
node.done(factorialnFactorial,-1,&nFactorial);
;
return nFactorial;
goto endfactorial;
factorial__nodeindex__:;
switch (node.nodeNumber)
{
default: goto beginfactorial;break;
}
endfactorial:;
};

int dm14Main()
{
int ret;
node.addVectorData(mainret,-1, false, false, &ret, "int");
char answer;
node.addVectorData(mainanswer,-1, false, false, &answer, "char");
;
int result;
node.addVectorData(mainresult,-1, false, false, &result, "int");
;
goto main__nodeindex__;
beginmain:;
;
result=0;
node.done(mainresult,-1,&result);
;
spill("would you like to get the factorial of a number ?");
get(answer);
if ( answer=='y'||answer=='Y' )
{
nspill("");
spill("whats the value ?");
get(result);
nspill("");
}
;
goto endmain;
main44:
;
;
spill("Calculating factorial(");
if( ! node._DM14VARIABLESMAP.at(mainresult)->ready){
node.need<int>(mainresult,-1,false,0,&result);
}
;
spill(result);
spill(") : ");
nspill(factorial(result));
goto endmain;
main__nodeindex__:;
switch (node.nodeNumber)
{
case 1:
goto main44;
break;
default: goto beginmain;break;
}
endmain:;
node.serve(true);
return ret;
};

int main (int argc, char* argv[])
{
setArgs(argc, argv, node);
node.nodesMaximum = __M14MAXIMUMNODES;
node.setCapacity(__M14DIRECTVARSCOUNT);
node.startListener(false);
return dm14Main();
}
