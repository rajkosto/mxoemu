#ifndef MXOSIM_GAMESERVER_H
#define MXOSIM_GAMESERVER_H

#include "GameClient.h"
#include "Singleton.h"

#define RECV_BUFFER_SIZE 1024

class GameServer : public Singleton <GameServer>
{
	private:		
		// Client List
		typedef std::map<std::string, GameClient*> GClientList;
		GClientList Clients;
		struct sockaddr_in listen_addr, inc_addr;

		// Socket stuff
		SOCKET Socket;
		fd_set Readable;

		struct timeval timeout;
		uint32 lastCleanupTime;
		uint32 CurTime;
	public:
		GameServer() { /* TODO: Add constructor code */ };
		~GameServer() { /* TODO: Add destructor code */ };
		bool Start();
		void Stop();
		void Loop();
		int Clients_Connected(void) { return (int)Clients.size(); }
		void Handle_Incoming();
		void Broadcast(const ByteBuffer &message);
};


#define sGame GameServer::getSingleton()

#endif

