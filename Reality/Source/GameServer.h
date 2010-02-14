// *************************************************************************************************
// --------------------------------------
// Copyright (C) 2006-2010 Rajko Stojadinovic
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************

#ifndef MXOSIM_GAMESERVER_H
#define MXOSIM_GAMESERVER_H

#include "Common.h"
#include "ByteBuffer.h"
#include "Singleton.h"
#include "Sockets.h"
#include "ObjectMgr.h"
#include "MessageTypes.h"

#define RECV_BUFFER_SIZE 2048

class GameServer : public Singleton <GameServer>
{
public:
	GameServer() { };
	~GameServer() { /* TODO: Add destructor code */ };
	bool Start();
	void Stop();
	void Loop();
	int Clients_Connected(void) { return (int)m_clients.size(); }
	void Handle_Incoming();
	class GameClient *GetClientWithSessionId(uint32 sessionId);
	void CheckAndResend();
	void Broadcast(const ByteBuffer &message);
	void AnnounceStateUpdate(class GameClient* clFrom,msgBaseClassPtr theMsg);
	void AnnounceCommand(class GameClient* clFrom,msgBaseClassPtr theCmd);
	ObjectMgr &getObjMgr() { return m_objMgr; }

private:	
	// Client List
	typedef std::map<std::string, class GameClient*> GClientList;
	GClientList m_clients;
	struct sockaddr_in listen_addr, inc_addr;

	// Socket stuff
	SOCKET m_socket;
	fd_set m_readable;

	struct timeval m_timeout;
	uint32 m_lastCleanupTime;
	uint32 m_currTime;

	ObjectMgr m_objMgr;
};


#define sGame GameServer::getSingleton()
#define sObjMgr GameServer::getSingleton().getObjMgr()

#endif

