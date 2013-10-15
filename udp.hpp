#include "stdafx.h"


#define CONNECT_ID_TIME_FOR_ 60000
class Peer;

class UDP_connection
{
private:
	struct connect_request
	{
		long long int connection_id;
		int action;
		int transaction_id;
	};
	struct connect_response
	{
		int action;
		int transaction_id;
		long long int connection_id;
	};
	struct announce_request
	{
		long long int connection_id;	//The connection id acquired from establishing the connection.
		int action;
		int transaction_id;
		unsigned char info_hash [20];	//The info-hash of the torrent you want announce yourself in
		string peer_id ;
		long long int downloaded;
		long long int left;
		long long int uploaded;
		int event;
		int IP_address;					//Your ip address. Set to 0 if you want the tracker to use the sender of this udp packet
		int key;						//A unique key that is randomized by the client
		int num_want;					//The maximum number of peers you want in the reply. Use -1 for default.
		short port;

};	
	struct announce_response
	{
		int action;
		int transaction_id;
		int interval;
		int leachers;
		int seeders;
		list< Peer> peers;
	};
	enum action_t
	{
		action_connect,			//0
		action_announce,        //1
		action_scrape,			//2
		action_error			//3
	};	
		
	connect_request send_connect();             
	connect_response response_connect();
	announce_request send_announce(Peer peer_you,long long int connect_id);
	announce_response response_announce();

	WSADATA wsa_data;
	SOCKET	sSocket;	
	struct	sockaddr_in servaddr;
	struct sockaddr_in localaddr;
	struct hostent * host;

public:
	//UDP_connection();
	


	bool socket_constructor(char servname[], int port);
	long long int connect();												       //return connection_id or -1 in error case
	announce_response  announce(Peer peer_you,long long int connect_id);
	bool scrape();
	bool error();

	
	//~UDP_connection();

};

template <class T>
	inline	 void write_to_byte(T val, char *start[])     //to bytestream(bigendian)
		{
			for (int i = (int)sizeof(T)-1; i >= 0; --i)
			{
				**start = static_cast< unsigned char>((val >> (i * 8)) & 0xff);
				++(*start);
			}
		}



template <class T>
	inline T read_from_byte(char *start[])          //  from bytestream(littleendian)
		{
			T ret = 0;
			for (int i = 0; i < (int)sizeof(T); ++i)
			{
				ret <<= 8;
				ret |= static_cast<unsigned char>(**start);
				++(*start);
			}
			return ret;
		}









