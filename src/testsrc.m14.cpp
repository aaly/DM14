#include "includes/io/io.hpp"
#include "includes/math/math.hpp"
#include "M14Defs.hpp"
 
int dm14Main()
{
int xx;
node.addVectorData(mainxx,-1, false, false, &xx, "int");
Array<int>* xa = new Array<int>();
node.addVectorData(mainxa,1000000, true, false, xa, "int");
for ( unsigned int i =0; i <1000000; i++)
{
xa->push_back(int());
node.addVectorData(mainxa, i, false, false, &xa->at(i), "");
}
;
goto main__nodeindex__;
beginmain:;
;
for (int i = 0;i<25000;i++)
{
xa->at(i)=Random();
node.done(mainxa,i,&xa->at(i));
;
;
Random();
}
;
for (int i = 0;i<25000;i++)
{
for (int l = 0;l<25000;l++)
{
if ( xa->at(l)>xa->at(i) )
{
int tmp = xa->at(l);
xa->at(l)=xa->at(i);
node.done(mainxa,l,&xa->at(l));
;
;
xa->at(i)=tmp;
node.done(mainxa,i,&xa->at(i));
;
;
tmp;
}
;
}
;
}
;
node.done(mainxa,-1,xa);
;
for (int i = 0;i<25000;i++)
{
spill("xa[");
spill(i);
spill("]=");
nspill(xa->at(i));
}
;
goto endmain;
main43:
;
;
for (int i = 25000;i<50000;i++)
{
xa->at(i)=Random();
node.done(mainxa,i,&xa->at(i));
;
;
Random();
}
;
for (int i = 25000;i<50000;i++)
{
for (int k = 25000;k<50000;k++)
{
if( ! node._DM14VARIABLESMAP.at(mainxa).elements->at(k)->ready){
node.need<int>(mainxa,k,false,0,&xa->at(k));
}
;
;
if( ! node._DM14VARIABLESMAP.at(mainxa).elements->at(i)->ready){
node.need<int>(mainxa,i,false,0,&xa->at(i));
}
;
;
if ( xa->at(k)>xa->at(i) )
{
if( ! node._DM14VARIABLESMAP.at(mainxa).elements->at(k)->ready){
node.need<int>(mainxa,k,false,0,&xa->at(k));
}
;
;
int tmp = xa->at(k);
if( ! node._DM14VARIABLESMAP.at(mainxa).elements->at(i)->ready){
node.need<int>(mainxa,i,false,0,&xa->at(i));
}
;
;
xa->at(k)=xa->at(i);
node.done(mainxa,k,&xa->at(k));
;
;
xa->at(i)=tmp;
node.done(mainxa,i,&xa->at(i));
;
;
tmp;
}
;
}
;
}
;
node.done(mainxa,-1,xa);
;
for (int i = 25000;i<50000;i++)
{
spill("xa[");
spill(i);
spill("]=");
if( ! node._DM14VARIABLESMAP.at(mainxa).elements->at(i)->ready){
node.need<int>(mainxa,i,false,0,&xa->at(i));
}
;
;
nspill(xa->at(i));
}
;
goto endmain;
main77:
;
;
for (int i = 26000;i<50000;i++)
{
spill("xa[");
spill(i);
spill("]=");
if( ! node._DM14VARIABLESMAP.at(mainxa).elements->at(i)->ready){
node.need<int>(mainxa,i,false,1,&xa->at(i));
}
;
if( ! node._DM14VARIABLESMAP.at(mainxa).elements->at(i)->ready){
node.need<int>(mainxa,i,false,0,&xa->at(i));
}
;
;
nspill(xa->at(i));
}
;
goto endmain;
main__nodeindex__:;
switch (node.nodeNumber)
{
case 1:
goto main43;
break;
case 2:
goto main77;
break;
default: goto beginmain;break;
}
endmain:;
node.serve(true);
return xx;
};

int main (int argc, char* argv[])
{
setArgs(argc, argv, node);
node.nodesMaximum = __M14MAXIMUMNODES;
node.setCapacity(__M14DIRECTVARSCOUNT);
node.startListener(false);
return dm14Main();
}
