#include "stdafx.h"
#include "Network_helper.h"
#include "P2P_connection.h"




	static const int keepAliveInterval=120*1000;
	static const char ProtocolId[] = "BitTorrent protocol";
	static const char ProtocolIdSize = 19;
	static const int blockSize = 16384;
	static const int MinHandshakeSize=48;

	
	P2P_connection::P2P_connection(Peer *_pPeer_you)
	{	
		pPeer_you=*_pPeer_you;		
	}


	int  P2P_connection::Send_HandShake(Peer *peer)
	{
		/*sentHandShake = true;*/

		// Restart the timeout
	/*	if (timeoutTimer)
			killTimer(timeoutTimer);
		timeoutTimer = startTimer(ClientTimeout);
*/

		char message[68];
		char *ptr=message;
		write_to_byte<char>(ProtocolIdSize,&ptr);
		for (int i=0;i<ProtocolIdSize;i++){ *ptr=ProtocolId[i]; ptr++;}
		for(int i=0;i<8;i++) write_to_byte<char>(0,&ptr);					//	8 '0'byrtes. for future extensions of protocol
		for (int i=0;i<20;i++){*ptr= pPeer_you.info_hash[i];ptr++;};
		(pPeer_you.peer_id).copy(ptr,20,0);
		ptr+=20;
		int ret = send(peer->sSocket, message,68, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		cout<<" sended bytes"<<ret;
		return 0;
	}

	int P2P_connection::Send_HandShake(SOCKET sSocket)
	{
		char message[68];
		char *ptr=message;
		write_to_byte<char>(ProtocolIdSize,&ptr);
		for (int i=0;i<ProtocolIdSize;i++){ *ptr=ProtocolId[i]; ptr++;}
		for(int i=0;i<8;i++) write_to_byte<char>(0,&ptr);					//	8 '0'byrtes. for future extensions of protocol
		for (int i=0;i<20;i++){*ptr= pPeer_you.info_hash[i];ptr++;};
		(pPeer_you.peer_id).copy(ptr,20,0);
		ptr+=20;
		int ret = send(sSocket, message,68, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		//cout<<" sended bytes"<<(int)ret;
		return 0;
	}
	
	int P2P_connection::Send_KeepAlive(Peer peer)
	{
		char message[4];
		char *ptr=message;
		write_to_byte<int>(0,&ptr);								//message length
		int ret = send(peer.sSocket, message,4, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else return 0;
		
	}

	int P2P_connection::Send_Choke(Peer *peer)
	{
		char message[5];
		char*ptr=message;
		write_to_byte<int>(1,&ptr);								//message length
		*ptr=choke;
		int ret = send(peer->sSocket, message,5, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else 
			peer->bChokedByMe=true;
		return 0;

	}

	int P2P_connection::Send_Unchoke(Peer *peer)
	{
		char message[5];
		char* ptr=message;
		write_to_byte<int>(1,&ptr);								//message length
		*ptr=unchoke;
		int ret = send(peer->sSocket, message,5, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else 
			peer->bChokedByMe=false;
			return 0;
	}

	int P2P_connection::Send_Interested(Peer *peer)
	{
		char message[5];
		char*ptr=message;
		write_to_byte<int>(1,&ptr);								//message length
		*ptr=interested;
		int ret = send(peer->sSocket, message,5, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else 
			peer->bInteresting=true;
			return 0;
	}

	int P2P_connection::Send_NotInterested(Peer *peer)
	{
		char message[5];
		char* ptr=message;
		write_to_byte<int>(1,&ptr);								//message length
		*ptr=not_interested;
		int ret = send(peer->sSocket, message,5, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else 
			peer->bInteresting=false;
			return 0;

	}

	int P2P_connection::Send_Have(int index,Peer* peer)
	{
		char message[9];
		char*ptr=message;
		write_to_byte<int>(5,&ptr);								//message length
		*ptr=have;
		ptr++;
		write_to_byte<int>(index,&ptr);
		int ret = send(peer->sSocket, message,9, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else return 0;

	}

	int P2P_connection::Send_Bitfield(Peer* peer)
	{
		int bitfield_size= sizeof(peer->bitfield);
		
		// if peer hasn't got any piece, he can skip the bitfield message
		bool bNeedSendBitfield=false;
		for (int i=0;i<bitfield_size;i++)
			if (peer->bitfield[i]==1){ bNeedSendBitfield=true; break;}
		if(!bNeedSendBitfield) return 0;

		char *message= new char[5+bitfield_size];
		write_to_byte<int>(1+bitfield_size,&message);									//message length 
		
		*message=bitfield;
		message++;
		
		for (int i=0; i<bitfield_size;i++) { *message=peer->bitfield[i]; message++;}
		
		int ret = send(peer->sSocket, message,5+bitfield_size, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else return 0;

	}

	int P2P_connection::Send_Request(Peer* peer, int index, int* block_offset)
	{
		if(*block_offset+1>=peer->piece_length) return 1;
		
		char *message = new char[17];
		int block_length = blockSize;
		write_to_byte<int>(13,&message);									//message length 
		
		*message=request;
		message++;

		write_to_byte<int>(index, &message);
		write_to_byte<int>(*block_offset, &message);
		if(peer->piece_length>*block_offset+blockSize+1) 
		{
			write_to_byte<int>(peer->piece_length-*block_offset-1, &message);
			*block_offset= peer->piece_length-1;
		}
		else 
		{
			write_to_byte<int>(blockSize,&message);
			*block_offset+=blockSize;
		}

		
		int ret = send(peer->sSocket, message,17, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else return 0;
	}

	int P2P_connection::Send_Piece(Peer *peer, int index, int* offset, char * data)
	{
		int data_size=sizeof(data);
		char* message = new char[13+data_size];
		write_to_byte<int>(9+data_size,&message);				//message length 
		*message=piece;
		message++;
		write_to_byte<int>(index,&message);
		write_to_byte<int>(*offset,&message);		
		for(int i=0;i<data_size;i++){ *message=data[i]; message++;}
		
		int ret = send(peer->sSocket, message,13+data_size, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else return 0;

	}

	int P2P_connection::Send_Cancel(Peer *peer, int index, int offset, int length)
	{
			
		char *message = new char[17];
		int block_length = blockSize;
		write_to_byte<int>(13,&message);									//message length 
		
		*message=cancel;
		message++;

		write_to_byte<int>(index, &message);
		write_to_byte<int>(offset, &message);
		write_to_byte<int>(length,&message);
		
		int ret = send(peer->sSocket, message,17, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		else return 0;
	}

	//void P2P_connection::connect ( SOCKET sSocket , TCP_connection tcp) 
	//{
	//	
	//	////char c= &P2P_connection::Response_for_message;
	//	//void ( P2P_connection::* func )(SOCKET sSocket) = &P2P_connection::Response_for_message;
	//	sIncommingDataHandler.init<P2P_connection, SOCKET>
	//		( tcp.eIncommingDataEvent, Response_for_message, this ); // установим связь события с обработчиком
	//}

	SOCKET  P2P_connection::AddClient(Peer *peer)
	{
		SOCKET sSock=TCP_connection::AddClient( peer->port,peer->IP_address);
		if(sSock!=SOCKET_ERROR)
		{
			//peer->bSentBitfield=false;
			//peer->bSentHandShake=false;
			mPeers[sSock]=peer;
		}
		
		return sSock;
	}

	int P2P_connection::ProcessMessage(SOCKET sSocket)
	{
		
		int index;
		
		
		Peer *peer= mPeers[sSocket];

		if(peer==NULL) {cout<<"peer=null"; return SOCKET_ERROR;}
		if(!(*peer).bSentHandShake)
		{
			char *in_buffer=new char[68];
			int rett = recv(sSocket,in_buffer, 68, 0);					//receving handshake
			if (rett <MinHandshakeSize)
			{
				DelSocket(sSocket);
				return SOCKET_ERROR;
			}
			if(!HandshakeIsVaryfied(in_buffer))
			{
				DelSocket(sSocket);
				return SOCKET_ERROR;
			}
			cout<<"hand shake is valid";
			peer->bSentHandShake=true;
			return rett;
		}
		
		
		char *in_buffer=new char[4];
		int rett = recv(sSocket,in_buffer, 4, 0);					//receving size
		if (rett< 4)
		{
			DelSocket(sSocket);			
			return SOCKET_ERROR;
		}

		int iMessageSize=read_from_byte<int>(&in_buffer); 
		
		//Keep_alive 
		if(iMessageSize==0)
		{
			cout<<"Keep alive";
			return rett;
		}

		for(int i=0;i<4;i++) in_buffer--;
		delete[]in_buffer;	
		in_buffer=new char[iMessageSize];
		in_buffer=RecvBigData(sSocket,iMessageSize);
		
		char cMessageType=read_from_byte<char>(&in_buffer);
		switch (cMessageType)
		{
			case choke:
				cout<<"ckoke\n";
				peer->bChokingMe=true;
				break;
			case unchoke:
				cout<<"unchoke\n";
				peer->bChokingMe=false;
				break;
			case interested:
				cout<<"interested\n";
				peer->bInterestedInMe=true;
				break;
			case not_interested:
				cout<<"not interested\n";
				peer->bInterestedInMe=false;
				break;
			case have:
				cout<<"have\n";
				index=read_from_byte<int>(&in_buffer);
				if(sizeof(peer->bitfield)>=index/8+1)
					(peer->bitfield[index/8])|=(1<<(index%8));
				break;
			case bitfield:
				cout<<"bitfield\n";
				peer->bSentBitfield=true;
				peer->bitfield=in_buffer;
				break;
			case request:
				cout<<"request\n";

				break;
			case piece:
				cout<<"piece\n";
				break;
			case cancel:
				cout<<"cancel\n";
				break;
			default:
				cout<<"default\n";
				DelSocket(sSocket);
				return SOCKET_ERROR;
				break;
			
		}
		/*for(int i=0;i<iMessageSize;i++) in_buffer--;
		delete[] in_buffer;*/
		return 0;
	}


	bool P2P_connection::HandshakeIsVaryfied(char*mess)
	{
		
		int size=sizeof(mess);
		if(ProtocolIdSize!=*mess)
			return false;
		mess++;
		for(int i=0;i<ProtocolIdSize;i++)	if(mess[i]!=ProtocolId[i]) return false;
		mess+=ProtocolIdSize;
		mess+=8;							//8 reserved bytes
		for(int i=0;i<20;i++)	if(mess[i]!=pPeer_you.info_hash[i])	return false;
		//if(size==68)
			

		return true;			
	}

	SOCKET P2P_connection::AcceptNewConnection()
	{
		SOCKET sNewConnection=TCP_connection::AcceptNewConnection();
		if(sNewConnection!=INVALID_SOCKET)
		{
			Peer pNewPeer=Peer();
			pNewPeer.sSocket=sNewConnection;
			mPeers[sNewConnection]=&pNewPeer;
		}
		return sNewConnection;
	}

	void P2P_connection::DelSocket(SOCKET sock)
	{
		TCP_connection::DelSocket(sock);
		mPeers.erase(sock);
	}

