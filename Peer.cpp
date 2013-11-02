#include "stdafx.h"
#include "Peer.h"
#include "MetaInfo.h"


Peer::Peer(struct MetaInfo meta_info)
{

	Peer();
	pieces.assign(meta_info.pieces);

	//port=5050;
	
	// initializing bitfield ( all bits set to 'false' for default)
	long long int bitfield_size=(meta_info.total_size/meta_info.piece_length+7)/8;
	bitfield=new char[bitfield_size];
	for(int i=0;i<bitfield_size;i++) bitfield[i]=0;

	// info_hash
	for(int i=0;i<20;i++)info_hash[i]=meta_info.info_hash[i];
	meta_info.pieces.assign(pieces);
	left=meta_info.total_size;
	
	//setting per_id
	peer_id="-Kr1000-";	                            
	static const char alphanum[] =
		"0123456789"
		"!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	int stringLength = sizeof(alphanum) - 1;
	srand(time(0));
	for(int i=0;i<12;i++)
		peer_id+=alphanum[rand() % stringLength];

	// seting key ( random value )
	key=rand();

	piece_length=meta_info.piece_length;
}

Peer::Peer()
{
	uploaded=0;
	downloaded=0;
	bSentHandShake=false;
	bSentBitfield=false;
	bInterestedInMe=false;
	bInteresting=false;
	bChokedByMe=true;
	bChokingMe=true;
}
