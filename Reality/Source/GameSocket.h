// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
// ***************************************************************************

#ifndef MXOSIM_GAMESOCKET_H
#define MXOSIM_GAMESOCKET_H

#include "Common.h"
#include "ByteBuffer.h"
#include "MessageTypes.h"
#include "GameClient.h"
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
	GameClient *GetClientWithSessionId(uint32 sessionId);
	vector<GameClient*> GetClientsWithCharacterId( uint64 charId );
	void Broadcast(const ByteBuffer &message, bool command);
	void AnnounceStateUpdate(GameClient* clFrom,msgBaseClassPtr theMsg, bool immediateOnly=false, GameClient::packetAckFunc callFunc=0);
	void AnnounceCommand(GameClient* clFrom,msgBaseClassPtr theCmd, GameClient::packetAckFunc callFunc=0);
	void RemoveCharacter(string IPAddr);
private:
	// Client List
	typedef map<string, GameClient*> GClientList;
	GClientList m_clients;

	uint32 m_lastCleanupTime;
	uint32 m_currTime;
	size_t m_lastPlayerCount;
};


#endif