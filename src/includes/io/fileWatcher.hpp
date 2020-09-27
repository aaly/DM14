// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#ifndef FILEWATCH_HPP
#define FILEWATCH_HPP


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include "Array.hpp"
//#inclide <string>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN    (1024 * (EVENT_SIZE + 16))

typedef Array<struct inotify_event*> AA;

class fileWatcher
{
	public:
		fileWatcher();
		~fileWatcher();
		bool	isReady();
		string	getNext();
		//int		addWatch(const string&, int access = IN_CREATE | IN_DELETE | IN_MODIFY);
		int		addWatch(const string&, int access = IN_CLOSE_WRITE | IN_MOVED_TO);
		bool 	Update(int waitUsec = 0);
		int		removeWatch(const string&);
		bool	hasChanged(int);
		bool	hasChanged(const string&);
		
	private:
		int fd;
		AA eventsQueue;
		int		Init();
};

#endif // FILEWATCH_HPP

           
