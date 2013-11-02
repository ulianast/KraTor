#include "stdafx.h"
#include "tcp.h"
#include "Peer.h"


class P2P_connection:public TCP_connection 
{
public:


	Peer pPeer_you;
	map<SOCKET,Peer*> mPeers;


	P2P_connection(Peer* _pPeer_you);

	int Send_HandShake(Peer *peer);									//1 - error; 0 - success
	int Send_HandShake(SOCKET sSocket);
	int Send_KeepAlive(Peer peer);									//prevent the peer from closing the connection when there's no activity
	int Send_Choke(Peer *peer);										//asking the peer to stop requesting blocks.
	int Send_Unchoke(Peer *peer);									//allowing the peer to start/resume requesting blocks
	int Send_Interested(Peer *peer);
	int Send_NotInterested(Peer *peer);
	int Send_Have(int index,Peer *peer);
	int Send_Bitfield(Peer *peer);									//bitfield representing the pieces that have been successfully downloaded
	int Send_Request(Peer* peer, int index, int* block_offset);		//is used to request a block
	int Send_Piece(Peer *peer,int index, int* offset, char * data);
	int Send_Cancel(Peer* peer, int index, int offset, int length);				//used to cancel block requests.It is typically used during "End Game"
	
	int ProcessMessage(SOCKET sSocket);
	SOCKET AcceptNewConnection();
	void DelSocket(SOCKET sock);
	SOCKET  AddClient(Peer* peer);
	
	static DWORD WINAPI ServerThread( LPVOID lpParameter )
	{
		return reinterpret_cast<P2P_connection *>(lpParameter)->ThreadProc( );
	}



private:
	enum ePeerMessages
	{
		choke,					//0
		unchoke,				//1
		interested,				//2
		not_interested,			//3
		have,					//4
		bitfield,				//5
		request,				//6
		piece,					//7
		cancel					//8
	};

	bool HandshakeIsVaryfied(char*mess);


	
	
};
