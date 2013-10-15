#include "stdafx.h"

class Peer
{
public:
	//Identificators
	string peer_id;	
	short port;
	int IP_address;
	int key;                   //A unique key that is randomized by the client

	// Info Payload
	char info_hash[20];	
	long long int uploaded;
	long long int downloaded;
	long long int left;			
	string pieces;
	

	enum event
	{
		none,
		completed,
		started,
		stopped
	};

	void Initialise(struct MetaInfo meta_info);
	//Peer(struct MetaInfo meta_info);	
	
};
