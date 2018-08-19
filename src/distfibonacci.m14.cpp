#include "DM14GLOBAL.m14.hpp"
#include "includes/core/Node.hpp"
#include "includes/core/Array.hpp"
#include "includes/io/io.hpp"
#include "includes/math/math.hpp"
#include "M14Defs.hpp"
int fib(int n);
 
int dm14Main(string args)
{
int xx;
Distributed.addVectorData<int>(mainxx,-1, false, false, &xx, false, false, "int");
int x;
Distributed.addVectorData<int>(mainx,-1, false, false, &x, false, false, "int");
;
goto main__nodeindex__;
beginmain:;
;
spill("please enter X :");
get(x);
goto endmain;
main12:
;
;
spill("X = ");
nspill(x);
spill("fib(X) = ");
nspill(fib(x));
goto endmain;
main__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
case 1:
goto main12;
break;
default: goto beginmain;break;
}
endmain:;
Distributed.serve(true);
return xx;
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
int fib(int n)
{
int ret;
goto fib__nodeindex__;
beginfib:;
;
if ( n==0 )
{
ret = 0;
goto endfib;
}
;
if ( n==1 )
{
ret = 1;
goto endfib;
}
;
if ( n<0 )
{
ret = 0;
goto endfib;
}
;
ret=fib(n-1)+fib(n-2);
goto endfib;
fib__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginfib;break;
}
endfib:;
return ret;
};

