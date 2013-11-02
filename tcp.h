#include "stdafx.h"
#define MAX_CONNECTIONS_FOR_START 4
#include <mutex>



class TCP_connection
{
public:	
	
	int Initialize();													//returns 1 in error case ; 0 - in case of success
	SOCKET  AddClient(unsigned short port,string ip_addr);				//returns 0 if success; 1 in error case
	static DWORD WINAPI ServerThread( LPVOID lpParameter )
	{
		cout<<"ServerThread tcp";
		return reinterpret_cast<TCP_connection *>(lpParameter)->ThreadProc( );
	}	
	DWORD WINAPI ThreadProc();
	virtual int ProcessMessage(SOCKET sSocket);
	virtual SOCKET AcceptNewConnection();
	virtual void DelSocket(SOCKET sock);
	
	static const int ServerMinPort = 6881;
	static const int ServerMaxPort =  7000;
	static const int BlockSize = 16384;
	static const int MaxBlocksInProgress = 5;
	static const int MaxBlocksInMultiMode = 2;
	static const int MaxConnectionPerPeer = 1;
	static const int RateControlWindowLength = 10;
	static const int RateControlTimerDelay = 1000;
	static const int MinimumTimeBeforeRevisit = 30;
	static const int MaxUploads = 4;
	static const int UploadScheduleInterval = 10000;	
	static const int EndGamePieces = 5;

private:

	SOCKET sServer;
	list<SOCKET> ClientSockets;
	mutex mClientSockLock;
	
};
