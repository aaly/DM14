// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#ifndef FILEWATCH_HPP
#define FILEWATCH_HPP

#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>

class FileTools
{
	public:
		int		addWatch(char*);
		int		removeWatch(int);
		bool	hasChanged(int);
	private:
		int fd;
};

#endif // FILEWATCH_HPP
