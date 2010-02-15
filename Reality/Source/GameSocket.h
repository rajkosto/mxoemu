#ifndef MXOSIM_GAMESOCKET_H
#define MXOSIM_GAMESOCKET_H

#include "Common.h"
#include "ByteBuffer.h"
#include "MessageTypes.h"
#include <Sockets/UdpSocket.h>
#include <Sockets/ISocketHandler.h>
#include <Sockets/SocketAddress.h>

class GameSocket : public UdpSocket
{
public:
	GameSocket(ISocketHandler& theHandler);
	~GameSocket();
	void OnRawData( const char *pData,size_t len,struct sockaddr *sa_from,socklen_t sa_len );
	void PruneDeadClients();
	void CheckAndResend();
	size_t Clients_Connected(void) { return m_clients.size(); }
	class GameClient *GetClientWithSessionId(uint32 sessionId);
	void Broadcast(const ByteBuffer &message);
	void AnnounceStateUpdate(class GameClient* clFrom,msgBaseClassPtr theMsg, bool immediateOnly=false);
	void AnnounceCommand(class GameClient* clFrom,msgBaseClassPtr theCmd);
private:
	// Client List
	typedef map<string, class GameClient*> GClientList;
	GClientList m_clients;

	uint32 m_lastCleanupTime;
	uint32 m_currTime;
};


#endif