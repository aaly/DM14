#include "includes/io/io.hpp"
#include "includes/math/math.hpp"
#include "includes/sys/sys.hpp"
#include "includes/sys/time.hpp"
#include "includes/core/Sleep.hpp"
#include "includes/io/File.hpp"
#include "includes/io/string.hpp"
#include "M14Defs.hpp"
void blockTimerInt(int variable, int mseconds);
 
void blockTimerInt(int variable, int mseconds)
{
goto blockTimerInt__nodeindex__;
beginblockTimerInt:;
;
USleep(mseconds);
goto endblockTimerInt;
blockTimerInt__nodeindex__:;
node.dataVectorReady=true;
switch (node.nodeNumber)
{
default: goto beginblockTimerInt;break;
}
endblockTimerInt:;
};

int dm14Main()
{
int xx;
node.addVectorData<int>(mainxx,-1, false, false, &xx, false, false, "int");
Time time;
node.addVectorData<Time>(maintime,-1, false, false, &time, false, false, "Time");
;
string xxx;
node.addVectorData<string>(mainxxx,-1, false, false, &xxx, false, false, "string");
;
File file;
node.addVectorData<File>(mainfile,-1, false, false, &file, false, false, "File");
;
string stringg;
node.addVectorData<string>(mainstringg,-1, false, false, &stringg, false, false, "string");
;
int x;
node.addVectorData<int>(mainx,-1, false, false, &x, false, true, "int");
;
int f;
node.addVectorData<int>(mainf,-1, false, false, &f, false, false, "int");
;
int y;
node.addVectorData<int>(mainy,-1, false, false, &y, true, false, "int");
;
float z;
node.addVectorData<float>(mainz,-1, false, false, &z, false, false, "float");
;
string str;
node.addVectorData<string>(mainstr,-1, false, false, &str, false, false, "string");
;
Array<int>* xa = new Array<int>(50000);
node.addVectorArrayData< Array<int > >(mainxa,50000, true, false, xa, false, false, "int");
;
string str2;
node.addVectorData<string>(mainstr2,-1, false, false, &str2, false, false, "string");
;
int fork;
node.addVectorData<int>(mainfork,-1, false, false, &fork, false, false, "int");
;
goto main__nodeindex__;
beginmain:;
;
nspill(strToFloat("5.5"));
time=Time::getCurrentTime();
time.useGMTTime(true);
nspill(time.toString(true, true, true, false));
spill("Time :");
spill(time.getHours());
spill(":");
spill(time.getMinutes());
spill(":");
spill(time.getSeconds());
xxx="123;456;789;10";
nspill(str_at(xxx, 1));
nspill(getToken(xxx, ';', 0));
nspill(getToken(xxx, ';', 1));
file.open("test.txt");
stringg=file.readLine();
stringg=file.readLine();
node.done<string>(mainstringg,-1,&stringg);
;
nspill(str_size(stringg));
nspill(file.readLine());
nspill(file.readLine());
nspill(file.readLine());
x=1;
z=Cos(55.5);
z++;
node.done<float>(mainz,-1,&z);
;
for (int i = 0;i<25000;i++)
{
spill("xa[");
spill(i);
spill("]=");
node.need<int>(mainxa,i,true,1,&xa->at(i), true);
;
nspill(xa->at(i));
}
;
f=1000;
node.done<int>(mainf,-1,&f);
;
spill("Y is :");
node.need<int>(mainy,-1,true,1,&y, true);
nspill(y);
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
node.need<string>(mainstr,-1,true,1,&str, true);
nspill(str);
nspill("N1");
node.need<string>(mainstr,-1,true,1,&str, true);
nspill(str);
nspill("N2");
node.need<string>(mainstr,-1,true,1,&str, true);
nspill(str);
nspill("N3");
node.need<string>(mainstr,-1,true,1,&str, true);
nspill(str);
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
spill("Y is :");
node.need<int>(mainy,-1,true,1,&y, true);
nspill(y);
goto endmain;
main95:
;
;
y=11;
node.done<int>(mainy,-1,&y);
;
for (int i = 0;i<25000;i++)
{
xa->at(i)=Random();
node.done<int>(mainxa,i,&xa->at(i));
;
}
;
xa->at(24999)=1111;
node.done<int>(mainxa,24999,&xa->at(24999));
;
node.done< Array< int > >(mainxa,-1,xa);
;
node.need<int>(mainf,-1,true,0,&f, false);
blockTimerInt(f, 6000);
spill("F is :");
node.need<int>(mainf,-1,true,0,&f, false);
nspill(f);
spill("F is :");
node.need<int>(mainf,-1,true,0,&f, false);
nspill(f);
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
x=2;
node.done<int>(mainx,-1,&x);
;
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
node.need<string>(mainstr,-1,true,2,&str, true);
nspill(str);
str="hello world from node 2 : D";
node.done<string>(mainstr,-1,&str);
;
nspill(str);
nspill(str);
goto endmain;
main133:
;
;
str2="str";
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
x=3;
node.done<int>(mainx,-1,&x);
;
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
spill("X is :");
node.need<int>(mainx,-1,true,-2,&x, true);
nspill(x);
if ( true )
{
node.need<float>(mainz,-1,true,0,&z, true);
if ( z!=0 )
{
spill("Z is :");
nspill(z);
}
;
}
;
nspill(str2);
str="hello world from now 3 !";
node.done<string>(mainstr,-1,&str);
;
fork=Fork();
if ( fork==0 )
{
nspill("forked");
nspill(fork);
x=10;
node.done<int>(mainx,-1,&x);
;
}
;
nspill("X IS : ");
node.need<int>(mainx,-1,false,-2,&x, true);
nspill(x);
x=(20+Sin(100.0));
node.done<int>(mainx,-1,&x);
;
return xx;
goto endmain;
main__nodeindex__:;
node.dataVectorReady=true;
switch (node.nodeNumber)
{
case 1:
goto main95;
break;
case 2:
goto main133;
break;
default: goto beginmain;break;
}
endmain:;
node.serve(true);
return xx;
};

int main (int argc, char* argv[])
{
string mainParameters;
setArgs(argc, argv, node, mainParameters);
node.nodesMaximum = __M14MAXIMUMNODES;
node.setCapacity(__M14DIRECTVARSCOUNT);
node.startListener(false);
return dm14Main();
}
