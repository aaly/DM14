#include "includes/io/io.hpp"
#include "includes/math/math.hpp"
#include "M14Defs.hpp"
int fib(int n);
 
int dm14Main()
{
int xx;
node.addVectorData(mainxx,-1, false, false, &xx, "int");
goto main__nodeindex__;
beginmain:;
;
nspill(fib(3));
goto endmain;
main__nodeindex__:;
switch (node.nodeNumber)
{
default: goto beginmain;break;
}
endmain:;
node.serve(true);
};

int main (int argc, char* argv[])
{
setArgs(argc, argv, node);
node.nodesMaximum = __M14MAXIMUMNODES;
node.setCapacity(__M14DIRECTVARSCOUNT);
node.startListener(false);
return dm14Main();
}
int fib(int n)
{
int ret;
node.addVectorData(fibret,-1, false, false, &ret, "int");
goto fib__nodeindex__;
beginfib:;
;
if ( n==0 )
{
return 0;
}
;
if ( n==1 )
{
return 1;
}
;
ret=fib(n-1)+fib(n-2);
node.done(fibret,-1,&ret);
;
goto endfib;
fib__nodeindex__:;
switch (node.nodeNumber)
{
default: goto beginfib;break;
}
endfib:;
};

