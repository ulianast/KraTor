#include "stdafx.h"

class Peer
{
public:
	//Identificators
	string peer_id;	
	unsigned short port;
	string IP_address;
	SOCKET sSocket;
	int key;                   //A unique key that is randomized by the client

	// Info Payload
	char info_hash[20];	
	long long int uploaded;
	long long int downloaded;
	long long int left;	
	char * bitfield;
	string pieces;
	long long int piece_length;	
	
	// Flags
	bool bSentHandShake;
	bool bSentBitfield;
	bool bChokedByMe;
	bool bChokingMe;
	bool bInterestedInMe;
	bool bInteresting;
	

	/*enum event
	{
		none,
		completed,
		started,
		stopped
	};*/

	//Peer();
	Peer(struct MetaInfo meta_info);
	Peer();
	//Peer(struct MetaInfo meta_info);	
	
};
