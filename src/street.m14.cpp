#include "DM14GLOBAL.m14.hpp"
#include "includes/core/Node.hpp"
#include "includes/core/Array.hpp"
#include "includes/io/io.hpp"
#include "includes/io/File.hpp"
#include "includes/io/string.hpp"
#include "includes/io/fileWatcher.hpp"
#include "includes/math/math.hpp"
#include "includes/core/Sleep.hpp"
#include "includes/core/common.hpp"
#include "includes/sys/sys.hpp"
#include "includes/sys/time.hpp"
#include "includes/ocr/tesseract.hpp"
#include "M14Defs.hpp"
string Line;
string Command;
string regionName;
string cityName;
string streetName;
int streetPosition;
string streetSide;
string watchDirectory;
int cameraWindow;
string Detected;
Array<string>* detectedHistory = new Array<string>(1000);
Time Timer;
int detectedPlatesNumber;
string readDetected;
int readPlates(string args);
int processPlates(string args);
string searchFor(string args);
 
int dm14Main(string args)
{
int result;
Distributed.addVectorData<string>(_DM14GLOBALVARDetected,-1, false, false, &Detected, false, true, "string");
Distributed.addVectorData<int>(mainresult,-1, false, false, &result, false, false, "int");
bool init;
;
File file;
Distributed.addVectorData<File>(mainfile,-1, false, false, &file, false, false, "File");
;
int Counter;
;
string Adress;
;
int Port;
;
Array<string>* plates = new Array<string>();
Distributed.addVectorArrayData< Array<string > >(mainplates,0, true, false, plates, false, false, "string");
;
goto main__nodeindex__;
beginmain:;
;
init=false;
Distributed.checkTicks=1000;
file.open("config.txt");
Line=file.readLine();
;
Counter=1;
Port=1500;
;
while(str_size(Line))
{
if ( str_at(Line, 0)!='~' )
{
if ( !init )
{
args=Line;
init=true;
Line=file.readLine();
Sleep(2);
continue;
;
}
;
spill("launching : ");
Port+=1;
Adress="127.0.0.1:"+numberToStr(Port);
Command="urxvt -e ./street.m14.bin -N 1 -P 127.0.0.1:1234 -S "+Adress;
spill(Command);
nspill(Line);
threadMember* _DM14THREADSystem1856 = new threadMember();
_DM14THREADSystem1856->start(_DM14THREADFUNCTIONSystem1856,NULL);
;
Sleep(Counter);
Counter+=1;
}
;
Line=file.readLine();
}
;
Distributed.checkTicks=1000000;
nspill("done launching System cameras");
Distributed.nodeNumber=1;
if ( M14RESETCOUNTER != (1) )
{
M14RESETCOUNTER++;
goto main__nodeindex__;
}
;
goto endmain;
main68:
;
;
Distributed.checkTicks=1000;
if ( tokenCount(args, ';')+1!=6 )
{
displayWarning("error, expcted more arguments ");
nspill(tokenCount(args, ';'));
goto endmain;
}
else
{
spill(" Args : ");
nspill(args);
}
;
regionName=getToken(args, ';', 0);
;
cityName=getToken(args, ';', 1);
;
streetName=getToken(args, ';', 2);
;
streetPosition=strToNumber(getToken(args, ';', 3));
;
streetSide=getToken(args, ';', 4);
;
watchDirectory=getToken(args, ';', 5);
;
cameraWindow=10;
;
;
;
;
detectedPlatesNumber=0;
;
threadMember* _DM14THREADreadPlates13108 = new threadMember();
_DM14THREADreadPlates13108->start(_DM14THREADFUNCTIONreadPlates13108,NULL);
;
;
while(true)
{
Distributed.need<string>(_DM14GLOBALVARDetected,-1,false,-2,&Detected, false);
readDetected=Detected;
if ( str_size(readDetected) )
{
threadMember* _DM14THREADprocessPlates17117 = new threadMember();
_DM14THREADprocessPlates17117->start(_DM14THREADFUNCTIONprocessPlates17117,NULL);
;
}
;
}
;
Distributed.checkTicks=1000000;
goto endmain;
main__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
case 1:
goto main68;
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
int readPlates(string args)
{
int result;
Tesseract scanner;
;
fileWatcher watcher;
;
goto readPlates__nodeindex__;
beginreadPlates:;
;
displayInfo("watching :"+args);
scanner.setLanguage("eng");
watcher.addWatch(args);
Timer.useLocalTime(true);
while(true)
{
string watchedfile = watcher.getNext();
;
if ( watchedfile!="" )
{
string scanned = regionName+";";
;
scanned+=cityName+";";
scanned+=streetName+";";
scanned+=numberToStr(streetPosition)+";";
scanned+=streetSide+";";
scanned+=getToken(scanner.readImage(args+"/"+watchedfile), '\n', 1)+";";
scanned+=numberToStr(Timer.getSecondsSinceEpoch())+";";
scanned+=searchFor(scanned);
displayInfo("Captured : "+scanned);
Detected=scanned;
Distributed.done<string>(_DM14GLOBALVARDetected,-1,&Detected);
;
}
;
}
;
result=0;
goto endreadPlates;
readPlates__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginreadPlates;break;
}
endreadPlates:;
return result;
};

int processPlates(string args)
{
int result;
string remoteRegionName;
;
string remoteCityName;
;
string remoteStreetName;
;
int remoteStreetPosition;
;
string remoteStreetSide;
;
string remotePlateNumber;
;
Time detectionTime;
;
goto processPlates__nodeindex__;
beginprocessPlates:;
;
remoteRegionName=getToken(args, ';', 0);
remoteCityName=getToken(args, ';', 1);
remoteStreetName=getToken(args, ';', 2);
remoteStreetPosition=strToNumber(getToken(args, ';', 3));
remoteStreetSide=getToken(args, ';', 4);
remotePlateNumber=getToken(args, ';', 5);
detectionTime=Time(strToNumber(getToken(args, ';', 6)));
if ( remoteRegionName==regionName&&remoteCityName==cityName&&remoteStreetName==streetName&&remoteStreetSide==streetSide&&remoteStreetPosition<streetPosition )
{
detectedHistory->at(detectedPlatesNumber)=remotePlateNumber+";"+getToken(args, ';', 6)+";"+numberToStr(remoteStreetPosition)+";"+getToken(args, ';', 7)+";"+getToken(args, ';', 8);
detectedPlatesNumber++;
}
;
goto endprocessPlates;
processPlates__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginprocessPlates;break;
}
endprocessPlates:;
return result;
};

string searchFor(string args)
{
string result;
goto searchFor__nodeindex__;
beginsearchFor:;
;
for (int i = detectedPlatesNumber-1;i>=0;i--)
{
if ( getToken(detectedHistory->at(i), ';', 0)==getToken(args, ';', 5) )
{
int from = strToNumber(getToken(detectedHistory->at(i), ';', 1));
;
int to = strToNumber(getToken(args, ';', 6));
;
int timeDifference = to-from;
;
displayInfo("difference in time : "+numberToStr(timeDifference)+" seconds");
int distanceDifference = streetPosition-strToNumber(getToken(detectedHistory->at(i), ';', 2));
;
int Speed = ((distanceDifference*(60*60)))/timeDifference;
;
Speed=Speed/1000;
if ( Speed>100 )
{
Speed-=5;
}
else
{
Speed-=((5*Speed))/100;
}
;
int allSpeed = strToNumber(getToken(detectedHistory->at(i), ';', 3))+Speed;
;
int allNodes = strToNumber(getToken(detectedHistory->at(i), ';', 4))+1;
;
int averageSpeed = allSpeed/allNodes;
;
displayInfo("speed at this camera : "+numberToStr(Speed)+" Km/H");
displayInfo("average speed at all cameras : "+numberToStr(averageSpeed)+" Km/H");
result = numberToStr(allSpeed)+";"+numberToStr(allNodes)+";";
goto endsearchFor;
}
;
}
;
result = "0;1;";
goto endsearchFor;
goto endsearchFor;
searchFor__nodeindex__:;
Distributed.dataVectorReady=true;
switch (Distributed.nodeNumber)
{
default: goto beginsearchFor;break;
}
endsearchFor:;
return result;
};

void* _DM14THREADFUNCTIONSystem1856()
{
return (void*)System(Command, Line);
};
void* _DM14THREADFUNCTIONreadPlates13108()
{
return (void*)readPlates(watchDirectory);
};
void* _DM14THREADFUNCTIONprocessPlates17117()
{
return (void*)processPlates(readDetected);
};
