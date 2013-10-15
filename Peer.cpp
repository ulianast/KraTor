#include "stdafx.h"
#include "Peer.h"
#include "MetaInfo.h"


void Peer::Initialise(struct MetaInfo meta_info)
{

	pieces.assign(meta_info.pieces);
	uploaded=0;
	downloaded=0;
	port=5050;
	for(int i=0;i<20;i++)info_hash[i]=meta_info.info_hash[i];
	//strncpy_s((char*)(info_hash),20,(const char*)(meta_info.info_hash),20);
	meta_info.pieces.assign(pieces);
	left=meta_info.total_size;
	
	peer_id="-Kr1000-";	                            //peer_id
	static const char alphanum[] =
		"0123456789"
		"!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	int stringLength = sizeof(alphanum) - 1;
	srand(time(0));
	for(int i=0;i<12;i++)
		peer_id+=alphanum[rand() % stringLength];

	key=rand();
}
