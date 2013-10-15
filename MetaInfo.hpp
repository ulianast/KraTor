#include "stdafx.h"

struct file			//determined if multi file
{
	long long int file_length;
	list<string> path;  //list of subdirectory's name,the last of which is the file name
}; 

struct MetaInfo
{
	list<string>announce_list;  //tracker's url
	string announce;
	char info_hash[20];
	string encoding;
	string created_by;
	long long int total_size;
	long long int creation_date;
	string comment;	
	string name;
	long long int piece_length;		// should be a power of 2
	string pieces;				//length is multiple of 20; 
								//subdivided into strings, which are the sha1 hash of each piece
	

	list<file> files;
	long long int length;				//determined if single file 
	bool multi_file;
	bool correct;	
};

//void MetaInfoInit(*MetaInfo meta_info);
