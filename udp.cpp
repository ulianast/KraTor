#include "stdafx.h"
#include "udp.h"
#include "Peer.h"




bool UDP_connection::socket_constructor(char servname[], int port)
{
	if(0!=WSAStartup(MAKEWORD(2,2),&wsa_data))
	{
		cout<<"winsock2 library isn't loaded";
		return false;
	}

	host = NULL;


	sSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sSocket == INVALID_SOCKET)
	{
		cout<< "Can't create socket";
		return false;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(servname);
	
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(5050);


	if (servaddr.sin_addr.s_addr == INADDR_NONE)
	{
		host = gethostbyname(servname);
		if (host == NULL)
		{
			cout<< "Unable to resolve server";
			return false;
		}
		CopyMemory(&servaddr.sin_addr, host->h_addr_list[0],
			host->h_length);
	}
	if (bind  (sSocket, (struct sockaddr *)&localaddr, sizeof(localaddr)) == SOCKET_ERROR)
	{
		cout<< "Can't bind";
		return false;
	}
	
	DWORD time_out = 2000;
	setsockopt (sSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time_out, sizeof (DWORD));
	return true;
}

UDP_connection::connect_request      UDP_connection::send_connect()
{
	char message[16];
	char* ptr = message;
	UDP_connection:: connect_request request_struct;
	request_struct.action=action_connect;
	request_struct.connection_id=0x41727101980;
	request_struct.transaction_id=rand() ^ (rand() << 16);

	write_to_byte<long long int>(request_struct.connection_id, &ptr);// connection_id
	write_to_byte<int>(action_connect, &ptr); // action (connect)
	write_to_byte<int>(request_struct.transaction_id, &ptr); // transaction_id

	sendto(sSocket, message, 16, 0, (struct sockaddr *)&servaddr,	sizeof(servaddr));
	return request_struct;
		
}

UDP_connection::connect_response       UDP_connection::response_connect()
{
	
	UDP_connection:: connect_response response_struct;
	response_struct.connection_id=-1;
	response_struct.transaction_id=-1;
	char buffer[16];
	char *ptr=buffer;
	int size_localaddr=sizeof(localaddr);

	if( SOCKET_ERROR==recvfrom(sSocket,buffer,16,0,(struct sockaddr *)&localaddr,&size_localaddr))return response_struct;
	
	response_struct.action=read_from_byte<int>(&ptr);
	response_struct.transaction_id=read_from_byte<int>(&ptr);
	response_struct.connection_id=read_from_byte<long long int>(&ptr);

	return response_struct;	
}

UDP_connection::announce_request     UDP_connection::send_announce(Peer peer_you,long long int connect_id)
{
	char message[98];
	char*ptr=message;
	UDP_connection::announce_request request_struct;
	
	request_struct.connection_id=connect_id;
	request_struct.action=action_announce;	
	request_struct.transaction_id=rand() ^ (rand() << 16);
	
	for(int i=0;i<20;i++)	request_struct.info_hash[i]=peer_you.info_hash[i];
	request_struct.peer_id.assign(peer_you.peer_id);
	request_struct.downloaded=peer_you.downloaded;
	request_struct.left=peer_you.left;
	request_struct.uploaded=peer_you.uploaded;
	request_struct.event=peer_you.none;
	request_struct.IP_address=0;			//default
	request_struct.key=peer_you.key;					
	request_struct.num_want=-1;				//default
	request_struct.port=peer_you.port;

	write_to_byte<long long int>(request_struct.connection_id,&ptr);
	write_to_byte<int>(action_announce,&ptr);
	write_to_byte<int>(request_struct.transaction_id,&ptr);
	
	for(int i=0;i<20;i++){	*ptr=request_struct.info_hash[i];	ptr++;}
	(request_struct.peer_id).copy(ptr,20,0);
	ptr+=20;
	write_to_byte<long long int>(peer_you.downloaded,&ptr);
	write_to_byte<long long int> (peer_you.left,&ptr);
	write_to_byte<long long int>(peer_you.uploaded,&ptr);
	write_to_byte<int> (request_struct.event,&ptr);
	write_to_byte<int>(request_struct.IP_address,&ptr);
	write_to_byte<int>(request_struct.key,&ptr);
	write_to_byte<int>(request_struct.num_want,&ptr);
	write_to_byte<short>(request_struct.port,&ptr);

	int sended_bytes=sendto(sSocket, message, 98, 0, (struct sockaddr *)&servaddr,	sizeof(servaddr));
	return request_struct;
}

UDP_connection::announce_response    UDP_connection::response_announce()
{
	UDP_connection::announce_response response_struct;
	char buffer[10000];
	char *ptr=buffer;
	response_struct.transaction_id=-1;         
	int size_localaddr=sizeof(localaddr);	
	int packet_size;


	
	packet_size=recvfrom(sSocket,buffer,10000,0,(struct sockaddr *)&localaddr,&size_localaddr);
	int peer_quantity=(packet_size-20)/6;
	
	if( (packet_size<20)  ||  ((packet_size-20)%6!=0))return response_struct;	
	
	response_struct.action=read_from_byte<int>(&ptr);
	response_struct.transaction_id=read_from_byte<int>(&ptr);
	response_struct.interval=read_from_byte<int>(&ptr);
	response_struct.leachers=read_from_byte<int>(&ptr);
	
	for(int i=0;i<peer_quantity;i++)
	{
		Peer peer;
		peer.IP_address=read_from_byte<int>(&ptr);
		peer.port=read_from_byte<short>(&ptr);
		response_struct.peers.push_back(peer);
	}
	return response_struct;
}

long long int UDP_connection::connect()
{
	UDP_connection::connect_request request=send_connect();
		
	int n=0;	

	UDP_connection:: connect_response response=response_connect();
	if((response.transaction_id!=request.transaction_id)||(response.action!=action_connect))	
		while(true)
		{		
			Sleep(1000*15*pow(2,n));
			Beep(880,100);
				
			request=send_connect();
			response=response_connect();
			if((response.transaction_id!=request.transaction_id)||(response.action!=action_connect))	 n++;
			else break;
			if(n>8) {response.connection_id=-1; break;}			
		}
	
	return response.connection_id;

}

UDP_connection::announce_response       UDP_connection::announce(Peer peer_you,long long int connect_id)
{
	UDP_connection::announce_request request=send_announce(peer_you,connect_id);

	int n=0;
	UDP_connection:: announce_response response=response_announce();
	if((response.transaction_id!=request.transaction_id)||(response.action!=action_announce))	
		while(true)
		{		
			//Sleep(1000*15*pow(2,n));
			Beep(880,100);
				
			request=send_announce(peer_you,connect_id);
			response=response_announce();
			if((response.transaction_id!=request.transaction_id)||(response.action!=action_announce))	 n++;
			else break;
			if(n>8) {response.transaction_id=-1; break;}			
		}
	
	return response;

}




