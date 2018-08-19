// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#include "fileWatcher.hpp"

fileWatcher::fileWatcher()
{
	 fd = inotify_init1( IN_NONBLOCK );
};

fileWatcher::~fileWatcher()
{
	close(fd);
};

int fileWatcher::Init()
{
	fd = inotify_init();
	return fd;
};

bool fileWatcher::isReady()
{
	return fd > 0 ? true : false;
};

int	fileWatcher::addWatch(const string& path, int access)
{
    return inotify_add_watch(fd, path.c_str(), access);
};

int	fileWatcher::removeWatch(const string& path)
{
    return inotify_rm_watch(fd, path.c_str());
};

bool fileWatcher::hasChanged(int desc)
{
	Update();
	for(unsigned int i =0; i < eventsQueue.size(); i++)
	{
		if (eventsQueue.at(i)->wd == desc)
		{
			//if ( eventsQueue.at(i).mask & IN_CREATE || eventsQueue.at(i).mask & IN_DELETE || eventsQueue.at(i).mask & IN_MODIFY )
               // It was a file we were monitoring!
			eventsQueue.remove(i);
			return true;   
        }
	}
	
	return false;
}

bool fileWatcher::hasChanged(const string& path)
{
	Update();
	for(unsigned int i =0; i < eventsQueue.size(); i++)
	{
		if (eventsQueue.at(i)->name == path)
		{
			eventsQueue.remove(i);
			return true;
               
        }
	}
	
	return false;
}

string fileWatcher::getNext()
{
	Update();
	
	if(eventsQueue.size())
	{
		string name = eventsQueue.at(0)->name;
		eventsQueue.remove(0);
		return name;
	}
	
	return "";
	
}


bool fileWatcher::Update(int waitUsec)
{
	bool ret = false;

    // Return from select immediately if none available
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 3;
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    // Do the select
    int selectRet = select(fd + 1, &rfds, NULL, NULL, &time);
    
    if(selectRet < 0)
    {
		return false;
	}
	
	
    if (FD_ISSET(fd, &rfds))
    {
        int length = 0;
        int i = 0;
        char buffer[BUF_LEN];

        length = read(fd, buffer, BUF_LEN);
        

        while (i < length)
        {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
			eventsQueue.push_back(&*event);
            i += EVENT_SIZE + event->len;
        }
    }
    else
    {
		return false;
	}
    return ret;
};


