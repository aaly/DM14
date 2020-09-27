#include "Sleep.hpp"

// sleep in milli seconds
int USleep( unsigned int in_mseconds )
{
#ifdef WINDOWS
	Sleep( in_mseconds );
#else
	::usleep( in_mseconds * 1000 );
#endif
return in_mseconds;
}
	
//sleep in seconds
int Sleep( unsigned int in_seconds )
{
#ifdef WINDOWS
	Sleep(in_seconds * 1000);
#else
	::sleep(in_seconds);
#endif
return in_seconds;
}
