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

#include "Common.h"
#include "GameSocket.h"
#include "Log.h"
#include "GameClient.h"
#include "Timer.h"
#include "Database/DatabaseEnv.h"
#include "GameServer.h"
#include <Sockets/Ipv4Address.h>

GameSocket::GameSocket( ISocketHandler& theHandler ) : UdpSocket(theHandler)
{
	m_lastCleanupTime = getTime();
	// set player count to 0
	{
		sDatabase.Execute(format("UPDATE `worlds` SET `numPlayers`='0' WHERE `name`='%1%' LIMIT 1")
			% sGame.GetName() );
		m_lastPlayerCount = 0;
	}

}

GameSocket::~GameSocket()
{

}

void GameSocket::OnRawData( const char *pData,size_t len,struct sockaddr *sa_from,socklen_t sa_len )
{
	struct sockaddr_in inc_addr;
	memcpy(&inc_addr,sa_from,sa_len);
	Ipv4Address theAddr(inc_addr);

	if (theAddr.IsValid() == false)
		return;

	string IPStr = theAddr.Convert(true);
	GClientList::iterator it = m_clients.find(IPStr);
	if (it != m_clients.end())
	{
		GameClient *Client = it->second;
		if (Client->IsValid() == false)
		{
			DEBUG_LOG( format("Removing dead client [%1%]") % IPStr );
			m_clients.erase(it);
			delete Client;
		}
		else
		{
			Client->HandlePacket(pData, len);
		}
	}
	else
	{
		m_clients[IPStr] = new GameClient(inc_addr, this);
		DEBUG_LOG(format ("Client connected [%1%], now have [%2%] clients")
			% IPStr % Clients_Connected());

		m_clients[IPStr]->HandlePacket(pData, len);
	}
}

void GameSocket::PruneDeadClients()
{
	m_currTime = getTime();

	// Do client cleanup
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();)
	{
		GameClient *Client = it->second;
		if (!Client->IsValid() || (m_currTime - Client->LastActive()) >= 20)
		{
			if (!Client->IsValid())
				DEBUG_LOG( format("Removing invalidated client [%1%]") % Client->Address() );
			else
				DEBUG_LOG( format("Removing client due to time-out [%1%]") % Client->Address() );

			m_clients.erase(it++);
			delete Client;
		}
		else
		{
			++it;
		}
	}

	if ((m_currTime - m_lastCleanupTime) >= 5)
	{
		// Update player count
		if (m_lastPlayerCount != this->Clients_Connected())
		{
			sDatabase.Execute(format("UPDATE `worlds` SET `numPlayers`='%1%' WHERE `name`='%2%' LIMIT 1")
				% this->Clients_Connected()
				% sGame.GetName() );

			m_lastPlayerCount = this->Clients_Connected();
		}

		m_lastCleanupTime = m_currTime;
	}
}

void GameSocket::RemoveCharacter(string IPAddr)
{
	
	GClientList::iterator it = m_clients.find(IPAddr);
	if (it != m_clients.end())
	{
		GameClient *Client = it->second;
		DEBUG_LOG( format("Removing XXX dead client [%1%]") % IPAddr );
		m_clients.erase(it);
		delete Client;		
	}

}

GameClient * GameSocket::GetClientWithSessionId( uint32 sessionId )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second->GetSessionId() == sessionId)
		{
			return it->second;
		}
	}
	return NULL;
}

vector<GameClient*> GameSocket::GetClientsWithCharacterId( uint64 charId )
{
	vector<GameClient*> returns;
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second->GetCharacterId() == charId)
		{
			returns.push_back(it->second);
		}
	}
	return returns;
}

void GameSocket::CheckAndResend()
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		it->second->CheckAndResend();
	}
}

void GameSocket::Broadcast( const ByteBuffer &message, bool command )
{
	if (command)
		return AnnounceCommand(NULL,make_shared<StaticMsg>(message));
	else
		return AnnounceStateUpdate(NULL,make_shared<StaticMsg>(message),true);
}

void GameSocket::AnnounceStateUpdate( GameClient* clFrom, msgBaseClassPtr theMsg, bool immediateOnly, GameClient::packetAckFunc callFunc )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueState(theMsg,immediateOnly,callFunc);
		}
	}
}

void GameSocket::AnnounceCommand( GameClient* clFrom,msgBaseClassPtr theCmd, GameClient::packetAckFunc callFunc )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueCommand(theCmd,callFunc);
		}
	}
}