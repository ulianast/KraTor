#include "stdafx.h"
#include "tcp.h"
#include "Network_helper.h"
//#include <boost/thread/locks.hpp>


//typedef boost::shared_mutex Lock;
//	typedef boost::unique_lock< Lock > WriteLock;
//	typedef boost::shared_lock< Lock > ReadLock;
//
//	Lock myLock;


int  TCP_connection::Initialize()
{

	int local_port=ServerMinPort;
	struct sockaddr_in localaddr;  
	

	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sServer == SOCKET_ERROR)
	{
		cout<<"Can't create server_socket";
		return 1;
	}

	ULONG ulBlock;
	ulBlock = 1;										// 1 - assync socket; 0 - sync socket
	if (ioctlsocket(sServer, FIONBIO, &ulBlock)			//transformation to asynchroneus
		== SOCKET_ERROR)
	{
		return 1;
	}

	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(local_port);

	while (::bind  (sServer, (struct sockaddr *)&localaddr, sizeof(localaddr)) == SOCKET_ERROR)
	{
		if(local_port<ServerMaxPort)local_port++;
		else {	cout<<"Can't bind";	return 1;}
		localaddr.sin_port=htons(local_port);
	}
 
	listen(sServer, 10);                  //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	return 0;
}

SOCKET TCP_connection::AddClient(unsigned short port, string ip_addr)  //returns 0 if success; 1 in error case
{
	SOCKET snewClient;
	struct sockaddr_in new_client_addr;
	struct hostent	*host = NULL;
	
	// Создание сокета
	snewClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (snewClient == INVALID_SOCKET)
	{
		cout<< "Can't create socket";
		return SOCKET_ERROR;
	}

	ULONG ulBlock;
	ulBlock = 1;										// 1 - assync socket; 0 - sync socket
	if (ioctlsocket(snewClient, FIONBIO, &ulBlock)			//transformation to asynchroneus
		== SOCKET_ERROR)
	{
		closesocket(snewClient);		
		return SOCKET_ERROR;
	}
	// Заполнение структуры с адресом сервера и номером порта
	new_client_addr.sin_family = AF_INET;
	new_client_addr.sin_port = htons(port);
	new_client_addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());

	if (new_client_addr.sin_addr.s_addr == INADDR_NONE)
	{
		host = gethostbyname(ip_addr.c_str());
		if (host == NULL)
		{
			closesocket(snewClient);
			return SOCKET_ERROR;
		}
		CopyMemory(&new_client_addr.sin_addr, host->h_addr_list[0],
			host->h_length);
	}

	int iError;
	int ret=connect(snewClient, (struct sockaddr *)&new_client_addr, sizeof(new_client_addr));
	if(ret=SOCKET_ERROR)
	{
        iError = WSAGetLastError();
        if(iError == WSAEWOULDBLOCK)
        {
            //cout << "Attempting to connect.\n";
            fd_set Write, Err;
            TIMEVAL Timeout;
           // int TimeoutSec = 1; // timeout after 2 seconds 

            FD_ZERO(&Write);
            FD_ZERO(&Err);

            FD_SET(snewClient, &Write);
	        FD_SET(snewClient, &Err); 

            Timeout.tv_sec = 0;
            Timeout.tv_usec =1000*1000; 

            ret = select(0, NULL, &Write,   &Err,    &Timeout);
            if(ret == 0)
            {
                //cout << "Connect Timeout (" << TimeoutSec << " Sec).\n";
				closesocket(snewClient);
                return SOCKET_ERROR;           
            }
            else
            {
                //if(FD_ISSET(snewClient, &Write))
                //{
                //   // cout << "Connected!\n";
                //}

                if(FD_ISSET(snewClient, &Err))
                {
                    //cout << "Select error.\n";   
					closesocket(snewClient);
                    return SOCKET_ERROR;
                }
            }
        }
        else
        {
            /*cout << "Failed to connect to peer.\n";
            cout << "Error: " << WSAGetLastError() << endl;*/
			closesocket(snewClient);
            return SOCKET_ERROR;
        }
    }
	
	
	mClientSockLock.lock();

	cout<<"Connected\n";
	ClientSockets.push_back(snewClient);

	mClientSockLock.unlock();

	return snewClient;
}

DWORD WINAPI TCP_connection::ThreadProc()    //returns 0 in error case
{
	int ReadySock;
	FD_SET ReadSet;

	while (1)
	{		
		FD_ZERO(&ReadSet);		


		mClientSockLock.lock();

			FD_SET(sServer, &ReadSet);
			for (list<SOCKET>::iterator itt=ClientSockets.begin();itt!=ClientSockets.end();itt++)
				if((*itt)==SOCKET_ERROR) 
					itt=ClientSockets.erase(itt);
				else
					FD_SET(*itt, &ReadSet);

		mClientSockLock.unlock();
		

		if ((ReadySock = select(0, &ReadSet, NULL, NULL, NULL))	== SOCKET_ERROR)
		{
			cout<<"Select failed ";
			cout<<WSAGetLastError();
			Sleep(10000);
			//break;
			continue;
		}

		if(ReadySock==0) continue;

		mClientSockLock.lock();

			//We have new connection 
			if (FD_ISSET(sServer, &ReadSet))    
			{
				SOCKET sNewConnection=AcceptNewConnection();
				if (sNewConnection == INVALID_SOCKET)
					cout<<"AcceptNewConnection failed\n";
				else cout <<"We have new connection\n";
			}
			//We have data from client 
		
			for (list<SOCKET>::iterator itt=ClientSockets.begin();itt!=ClientSockets.end();itt++)
			{			
				if (FD_ISSET(*itt, &ReadSet))
				{
					cout<<"We have data from client\n";				
					ProcessMessage(*itt);				
				}
			}
		
			mClientSockLock.unlock();
	}
	closesocket(sServer);
	return 1;
}

int TCP_connection::ProcessMessage(SOCKET sSocket)
{
	return 0;
}

SOCKET TCP_connection::AcceptNewConnection()
{	
	struct sockaddr_in   clientaddr;
	int iSize;

	iSize = sizeof(clientaddr);
	SOCKET sNewConnection=accept(sServer,	(struct sockaddr *)&clientaddr,&iSize);			
	if (sNewConnection != INVALID_SOCKET)
	{
		//EnterCriticalSection(&cSect);
		ClientSockets.push_back(sNewConnection); 
		//LeaveCriticalSection(&cSect);
	}
	return sNewConnection;
}

void TCP_connection::DelSocket(SOCKET sock)
{
	closesocket(sock);		
}



