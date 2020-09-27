#ifndef SLEEP_HPP
#define SLEEP_HPP


#ifdef WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif
	// sleep in milli seconds
	//int USleep( unsigned int in_mseconds );	
	int USleep( unsigned int);	
	//sleep in seconds
	//int Sleep( unsigned int in_seconds );
	int Sleep( unsigned int);
#endif


