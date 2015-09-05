/*
  as-util.h 2011/06/12
  by 7k
  email: blxode [at] gmail.com 

  modified by Fuyin 
  2014/5/2
  email: 811197881 [at] qq.com
*/


#ifndef _AS_UTIL_H_
#define _AS_UTIL_H_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fcntl.h>
//#include <direct.h>
#include <stdlib.h>
//#include <io.h>
#include <cctype>
#include <algorithm>


using namespace std;

namespace as
{
	int open_or_die(string in_filename, const int type)
	{
		const char *fn = in_filename.c_str();
		int fd = open(fn, type);
		if(fd == NULL)
		{
			cerr<<"Could not open "<<fn<<" (No such file or directory)"<<endl;
			exit(1);
		}
		return fd;
	}

	void write_file(string filename, unsigned char *buff, unsigned long len)
	{
		const char *fn = filename.c_str();
		fstream file;
		file.open(fn,ios::binary | ios::out);
		if(!file.is_open())
		{
			cerr<<"Could not open "<<fn<<" (Permission denied)"<<endl;
			exit(1);
		}
		file.write((const char *)buff,len);
		file.close();
	}
};

#endif
