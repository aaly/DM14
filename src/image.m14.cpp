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
#include "includes/imgproc/objectDistance.hpp"
#include "M14Defs.hpp"
struct Point_st
{
float x;
float y;
};
struct nodeInfo
{
string nodeID;
float distance;
Point_st location;
Point_st firstPinpoint;
Point_st secondPinpoint;
};
string Line;
int nodesInfoSize;
int nodesInfoCurrentSize;
Array<nodeInfo>* nodesInfo = new Array<nodeInfo>(10000);
float localLastUpdate;
float lastUpdate;
Point_st localLocation;
Point_st Location;
objectDistance objectMonitor;
bool Init;
string currentToken;
int hsv1;
int hsv2;
int hsv3;
int localDistanceUpdate;
Time Timer;
Time Timer2;
int timerDiff;
int size;
Point_st firstPinointAverage;
Point_st secondPinointAverage;
void processNodeInformation(int distanceUpdateValue, string nodeID);
void monitorBoat();
int findNodeIndex(string nodeID);
int updateAverageDistance();
void calculateCoordinates(int nodeIndex);
 
int dm14Main(string args)
{
int result;
Distributed.addVectorData<float>(_DM14GLOBALVARlastUpdate,-1, false, false, &lastUpdate, false, true, "float");
Distributed.addVectorData<Point_st>(_DM14GLOBALVARlocalLocation,-1, false, false, &localLocation, false, false, "Point_st");
Distributed.addVectorClassData<float>(_DM14GLOBALVARlocalLocationx,-1, false, false, &localLocation.x, false, false, "float");
Distributed.addVectorClassData<float>(_DM14GLOBALVARlocalLocationy,-1, false, false, &localLocation.y, false, false, "float");
Distributed.addVectorData<Point_st>(_DM14GLOBALVARLocation,-1, false, false, &Location, false, true, "Point_st");
Distributed.addVectorClassData<float>(_DM14GLOBALVARLocationx,-1, false, false, &Location.x, false, true, "float");
Distributed.addVectorClassData<float>(_DM14GLOBALVARLocationy,-1, false, false, &Location.y, false, true, "float");
Distributed.addVectorData<int>(mainresult,-1, false, false, &result, false, false, "int");
File file;
Distributed.addVectorData<File>(mainfile,-1, false, false, &file, false, false, "File");
;
goto main__nodeindex__;
beginmain:;
;
file.open("config.txt");
Line=file.readLine();
nodesInfoSize=10000;
nodesInfoCurrentSize=0;
;
;
;
;
;
;
Init=false;
while(str_size(Line))
{
if ( str_at(Line, 0)!='~' )
{
for (int i = 0;i<=tokenCount(Line, ';');i++)
{
if ( i==0 )
{
objectMonitor.realWidth=strToNumber(getToken(Line, ';', i));
}
else if ( i==1 )
{
objectMonitor.realHeight=strToNumber(getToken(Line, ';', i));
}
else if ( i==2 )
{
objectMonitor.realDistance=strToNumber(getToken(Line, ';', i));
}
else if ( i==3 )
{
objectMonitor.pixelsPerCM=strToFloat(getToken(Line, ';', i));
}
else if ( i==4 )
{
objectMonitor.focalLength=strToFloat(getToken(Line, ';', i));
}
else if ( i==5 )
{
currentToken=getToken(Line, ';', i);
hsv1=strToNumber(getToken(currentToken, ',', 0));
hsv2=strToNumber(getToken(currentToken, ',', 1));
hsv3=strToNumber(getToken(currentToken, ',', 2));
objectMonitor.setLowHSV(hsv1, hsv2, hsv3);
}
else if ( i==6 )
{
currentToken=getToken(Line, ';', i);
hsv1=strToNumber(getToken(currentToken, ',', 0));
hsv2=strToNumber(getToken(currentToken, ',', 1));
hsv3=strToNumber(getToken(currentToken, ',', 2));
objectMonitor.setHighHSV(hsv1, hsv2, hsv3);
}
else if ( i==7 )
{
objectMonitor.minimumArea=strToNumber(getToken(Line, ';', i));
}
else if ( i==8 )
{
localLocation.x=strToNumber(getToken(Line, ';', i));
Distributed.done<float>(_DM14GLOBALVARlocalLocationx,-1,&localLocation.x);
;
Location.x=localLocation.x;
Distributed.done<float>(_DM14GLOBALVARLocationx,-1,&Location.x);
;
}
else if ( i==9 )
{
localLocation.y=strToNumber(getToken(Line, ';', i));
Distributed.done<float>(_DM14GLOBALVARlocalLocationy,-1,&localLocation.y);
;
Location.y=localLocation.y;
Distributed.done<float>(_DM14GLOBALVARLocationy,-1,&Location.y);
;
}
;
}
;
Init=true;
break;
;
}
;
Line=file.readLine();
}
;
if ( !Init )
{
nspill("Error reading configuration file");
Distributed.Exit(-1);
result = -1;
goto endmain;
}
;
for (int i = 0;i<=tokenCount(args, ' ');i++)
{
if ( getToken(args, ' ', i)=="-C" )
{
objectMonitor.setSrcType(objectDistance::sourceType::CAMSRC);
}
else if ( getToken(args, ' ', i)=="-I" )
{
objectMonitor.setSrcType(objectDistance::sourceType::IMAGESRC);
}
else if ( getToken(args, ' ', i)=="-V" )
{
objectMonitor.setSrcType(objectDistance::sourceType::VIDEOSRC);
}
else
{
objectMonitor.setSrcPath(getToken(args, ' ', i));
}
;
}
;
objectMonitor.Init();
threadMember* _DM14THREADmonitorBoat13159 = new threadMember();
_DM14THREADmonitorBoat13159->start(_DM14THREADFUNCTIONmonitorBoat13159,NULL);
;
while(true)
{
Distributed.need<float>(_DM14GLOBALVARlastUpdate,-1,false,-2,&lastUpdate, true);
localDistanceUpdate=lastUpdate;
threadMember* _DM14THREADprocessNodeInformation15166 = new threadMember();
_DM14THREADprocessNodeInformation15166->start(_DM14THREADFUNCTIONprocessNodeInformation15166,NULL);
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
displayInfo("remote update from  :"+nodeID);
nodeindex=findNodeIndex(nodeID);
if ( nodeindex!=-1 )
{
displayInfo("remote update2 :"+numberToStr(distanceUpdateValue)+"cm");
nodesInfo->at(nodeindex).distance=distanceUpdateValue;
}
else
{
displayInfo("remote update1 :"+numberToStr(distanceUpdateValue)+"cm");
nodesInfo->at(nodesInfoCurrentSize).nodeID=nodeID;
nodesInfo->at(nodesInfoCurrentSize).distance=distanceUpdateValue;
Distributed.need<float>(_DM14GLOBALVARLocationx,-1,false,-2,&Location.x, true,nodeID);
Distributed.need<float>(_DM14GLOBALVARLocationy,-1,false,-2,&Location.y, true,nodeID);
nodesInfo->at(nodesInfoCurrentSize).location=Location;
displayInfo("remote locaton :"+floatToStr(nodesInfo->at(nodesInfoCurrentSize).location.x)+","+floatToStr(nodesInfo->at(nodesInfoCurrentSize).location.y));
nodesInfoCurrentSize++;
}
;
updateAverageDistance();
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
;
Timer.updateTime();
;
;
while(objectMonitor.newDataAvailable())
{
Timer2.updateTime();
timerDiff=Timer2.Substract(Timer);
if ( timerDiff>=1 )
{
localLastUpdate=objectMonitor.getCurrentDistance();
if ( localLastUpdate>-1 )
{
lastUpdate=localLastUpdate;
Distributed.done<float>(_DM14GLOBALVARlastUpdate,-1,&lastUpdate);
;
Timer.updateTime();
displayInfo("Local Distance : "+floatToStr(localLastUpdate)+"cm");
updateAverageDistance();
}
;
}
;
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
if ( nodeID=="" )
{
nodeIndex = -1;
goto endfindNodeIndex;
}
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

int updateAverageDistance()
{
int result;
goto updateAverageDistance__nodeindex__;
beginupdateAverageDistance:;
;
size=nodesInfoCurrentSize;
if ( localLastUpdate<=0||size<1 )
{
result = 1;
goto endupdateAverageDistance;
}
;
;
;
for (int i = 0;i<size;i++)
{
calculateCoordinates(i);
firstPinointAverage.x=firstPinointAverage.x+nodesInfo->at(i).firstPinpoint.x;
firstPinointAverage.y=firstPinointAverage.y+nodesInfo->at(i).firstPinpoint.y;
secondPinointAverage.x=secondPinointAverage.x+nodesInfo->at(i).secondPinpoint.x;
secondPinointAverage.y=secondPinointAverage.y+nodesInfo->at(i).secondPinpoint.y;
}
;
firstPinointAverage.x=firstPinointAverage.x/size;
firstPinointAverage.y=firstPinointAverage.y/size;
secondPinointAverage.x=secondPinointAverage.x/size;
secondPinointAverage.y=secondPinointAverage.y/size;
displayInfo("Average First Location : "+floatToStr(firstPinointAverage.x)+","+floatToStr(firstPinointAverage.y));
displayInfo("Average Second Location : "+floatToStr(secondPinointAverage.x)+","+floatToStr(secondPinointAverage.y));
goto endupdateAverageDistance;
updateAverageDistance__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginupdateAverageDistance;break;
}
endupdateAverageDistance:;
return result;
};

void calculateCoordinates(int nodeIndex)
{
float Distance;
;
float a;
;
float b;
;
Point_st midPoint;
;
float T;
;
goto calculateCoordinates__nodeindex__;
begincalculateCoordinates:;
;
Distance=FAbsoulte(localLocation.x-nodesInfo->at(nodeIndex).location.x)+FAbsoulte(localLocation.y-nodesInfo->at(nodeIndex).location.y);
if ( Distance>(localLastUpdate+nodesInfo->at(nodeIndex).distance) )
{
displayInfo("no intersection points exist and the circles are separate.");
}
else if ( Distance<FAbsoulte(localLastUpdate-nodesInfo->at(nodeIndex).distance) )
{
displayInfo("no intersection points exist, as one circle contains the other.");
}
else if ( ((Distance==0))&&(localLastUpdate==nodesInfo->at(nodeIndex).distance) )
{
displayInfo(" circles are coincident and there is an infinite number of solutions");
}
;
a=((((((nodesInfo->at(nodeIndex).distance*nodesInfo->at(nodeIndex).distance))-(localLastUpdate*localLastUpdate)))+(Distance*Distance)))/(2*Distance);
b=Distance-a;
midPoint.x=((((a*localLocation.x))+(b*nodesInfo->at(nodeIndex).location.x)))/Distance;
midPoint.y=((((a*localLocation.y))+(b*nodesInfo->at(nodeIndex).location.y)))/Distance;
T=Sqrtf(localLastUpdate*localLastUpdate-(b*b));
nodesInfo->at(nodeIndex).firstPinpoint.x=midPoint.x+(T*(FAbsoulte(localLocation.y-nodesInfo->at(nodeIndex).location.y)/(Distance)));
nodesInfo->at(nodeIndex).firstPinpoint.y=midPoint.y-(T*(FAbsoulte(localLocation.x-nodesInfo->at(nodeIndex).location.x)/(Distance)));
nodesInfo->at(nodeIndex).secondPinpoint.x=midPoint.x-(T*(FAbsoulte(localLocation.y-nodesInfo->at(nodeIndex).location.y)/(Distance)));
nodesInfo->at(nodeIndex).secondPinpoint.y=midPoint.y+(T*(FAbsoulte(localLocation.x-nodesInfo->at(nodeIndex).location.x)/(Distance)));
goto endcalculateCoordinates;
calculateCoordinates__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto begincalculateCoordinates;break;
}
endcalculateCoordinates:;
};

void* _DM14THREADFUNCTIONmonitorBoat13159()
{
monitorBoat();
};
void* _DM14THREADFUNCTIONprocessNodeInformation15166()
{
processNodeInformation(localDistanceUpdate, Distributed.getLastActiveNode(_DM14GLOBALVARlastUpdate, -1));
};
