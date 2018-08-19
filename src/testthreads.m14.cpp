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
struct nodeInfo
{
string nodeID;
float distance;
float readValue;
};
int nodesInfoSize;
int nodesInfoCurrentSize;
Array<nodeInfo>* nodesInfo = new Array<nodeInfo>(10000);
int localLastUpdate;
int lastUpdate;
int nodeUpdateIndex;
int distanceUpdate;
int localDistanceUpdate;
string distanceUpdateAddress;
void processNodeInformation(int distanceUpdateValue, string nodeID);
void monitorBoat();
int findNodeIndex(string nodeID);
 
int dm14Main(string args)
{
int result;
Distributed.addVectorArrayData< Array<nodeInfo > >(_DM14GLOBALVARnodesInfo,10000, true, false, nodesInfo, false, false, "nodeInfo");
for (unsigned int i =0; i < 10000; i++)
{
Distributed.addVectorClassData<string>(_DM14GLOBALVARnodesInfonodeID,i, false, false, &nodesInfo->at(i).nodeID, false, false, "string");
Distributed.addVectorClassData<float>(_DM14GLOBALVARnodesInfodistance,i, false, false, &nodesInfo->at(i).distance, false, false, "float");
Distributed.addVectorClassData<float>(_DM14GLOBALVARnodesInforeadValue,i, false, false, &nodesInfo->at(i).readValue, false, false, "float");
}
Distributed.addVectorData<int>(_DM14GLOBALVARlastUpdate,-1, false, false, &lastUpdate, false, true, "int");
Distributed.addVectorData<int>(_DM14GLOBALVARnodeUpdateIndex,-1, false, false, &nodeUpdateIndex, false, true, "int");
Distributed.addVectorData<int>(_DM14GLOBALVARdistanceUpdate,-1, false, false, &distanceUpdate, false, true, "int");
Distributed.addVectorData<int>(mainresult,-1, false, false, &result, false, false, "int");
goto main__nodeindex__;
beginmain:;
;
nodesInfoSize=10000;
nodesInfoCurrentSize=0;
;
;
;
;
;
threadMember* _DM14THREADmonitorBoat1343 = new threadMember();
_DM14THREADmonitorBoat1343->start(_DM14THREADFUNCTIONmonitorBoat1343,NULL);
;
while(true)
{
Distributed.need<int>(_DM14GLOBALVARdistanceUpdate,-1,false,-2,&distanceUpdate, true);
localDistanceUpdate=distanceUpdate;
distanceUpdateAddress=Distributed.getLastActiveNode(_DM14GLOBALVARdistanceUpdate, -1);
threadMember* _DM14THREADprocessNodeInformation1551 = new threadMember();
_DM14THREADprocessNodeInformation1551->start(_DM14THREADFUNCTIONprocessNodeInformation1551,NULL);
;
}
;
goto endmain;
main__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
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
void processNodeInformation(int distanceUpdateValue, string nodeID)
{
int nodeindex;
;
goto processNodeInformation__nodeindex__;
beginprocessNodeInformation:;
;
nodeindex=findNodeIndex(nodeID);
if ( nodeindex!=-1 )
{
Distributed.need<int>(_DM14GLOBALVARdistanceUpdate,-1,false,-2,&distanceUpdate, true);
nodesInfo->at(nodeindex).readValue=distanceUpdate;
Distributed.done<float>(_DM14GLOBALVARnodesInforeadValue,nodeindex,&nodesInfo->at(nodeindex).readValue);
;
}
else
{
nodesInfo->at(nodesInfoCurrentSize).nodeID=nodeID;
Distributed.done<string>(_DM14GLOBALVARnodesInfonodeID,nodesInfoCurrentSize,&nodesInfo->at(nodesInfoCurrentSize).nodeID);
;
Distributed.need<int>(_DM14GLOBALVARdistanceUpdate,-1,false,-2,&distanceUpdate, true);
nodesInfo->at(nodesInfoCurrentSize).readValue=distanceUpdate;
Distributed.done<float>(_DM14GLOBALVARnodesInforeadValue,nodesInfoCurrentSize,&nodesInfo->at(nodesInfoCurrentSize).readValue);
;
nodesInfoCurrentSize++;
}
;
Distributed.need<int>(_DM14GLOBALVARdistanceUpdate,-1,false,-2,&distanceUpdate, true);
displayInfo("average distance :"+numberToStr(((distanceUpdate+localLastUpdate))/2));
goto endprocessNodeInformation;
processNodeInformation__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginprocessNodeInformation;break;
}
endprocessNodeInformation:;
};

void monitorBoat()
{
goto monitorBoat__nodeindex__;
beginmonitorBoat:;
;
while(true)
{
Sleep(10);
}
;
goto endmonitorBoat;
monitorBoat__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginmonitorBoat;break;
}
endmonitorBoat:;
};

int findNodeIndex(string nodeID)
{
int nodeIndex;
goto findNodeIndex__nodeindex__;
beginfindNodeIndex:;
;
for (int i = 0;i<nodesInfoSize;i++)
{
if ( nodesInfo->at(i).nodeID==nodeID )
{
nodeIndex = i;
goto endfindNodeIndex;
}
;
}
;
nodeIndex = -1;
goto endfindNodeIndex;
goto endfindNodeIndex;
findNodeIndex__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginfindNodeIndex;break;
}
endfindNodeIndex:;
return nodeIndex;
};

void* _DM14THREADFUNCTIONmonitorBoat1343()
{
monitorBoat();
};
void* _DM14THREADFUNCTIONprocessNodeInformation1551()
{
processNodeInformation(localDistanceUpdate, distanceUpdateAddress);
};
