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

#include "Common.h"
#include "GameServer.h"
#include "GameClient.h"
#include "MarginServer.h"
#include "Log.h"
#include "Timer.h"
#include "Config.h"
#include "Sockets.h"

initialiseSingleton( GameServer );

bool GameServer::Start()
{
	int Port;

	Port = sConfig.GetIntDefault("GameServer.Port", 10000);

	INFO_LOG(format("Starting Game server on port %1%") % Port);

#if PLATFORM == PLATFORM_WIN32
	// Winsock Startup
	WSADATA wsa;
	memset(&wsa, 0x0, sizeof(WSADATA));
	if( WSAStartup( MAKEWORD(2,0), &wsa ) != 0x0 )
	{
		CRITICAL_LOG("Unable to initialize WinSock2!");
		return false;
	}
#endif

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket == INVALID_SOCKET)
	{
		CRITICAL_LOG("Unable to create socket!");
		return false;
	}

	// 0 = Blocking sockets - 1 = Non-blocking
	unsigned long mode = 1;

#if PLATFORM == PLATFORM_WIN32
	int ret = ioctlsocket(m_socket, FIONBIO, &mode );
#else
	int ret = ioctl(m_socket, FIONBIO, &mode);
#endif

	if (ret < 0)
	{
		CRITICAL_LOG("Unable to set socket to non blocking!");
		m_socket = INVALID_SOCKET;
		return false;
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(Port);

	if (::bind(m_socket, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0)
	{
		CRITICAL_LOG("Unable to bind socket!");
		m_socket = INVALID_SOCKET;
		return false;
	}
	m_lastCleanupTime = getTime();

	return true;
}

void GameServer::Stop()
{
	INFO_LOG("Game Server shutdown");

#if PLATFORM == PLATFORM_WIN32
	closesocket(m_socket);
	WSACleanup();
#else
	close(m_socket);
#endif

	m_socket = INVALID_SOCKET;
}

void GameServer::Loop(void)
{
	FD_ZERO(&m_readable);
	FD_SET(m_socket, &m_readable);

	m_timeout.tv_sec = 0;
	m_timeout.tv_usec = 100;

	if (select(0, &m_readable, NULL, NULL, &m_timeout) == SOCKET_ERROR)
	{
		CRITICAL_LOG("Select() Failed, Shutting down World server");
		Stop();
	}

	if (FD_ISSET(m_socket, &m_readable))
	{
		Handle_Incoming();
	}

	CheckAndResend();

	m_currTime = getTime();
	if ((m_currTime - m_lastCleanupTime) >= 5)
	{
		// Do client cleanup
		GClientList::iterator i = m_clients.begin();
		for (;;)
		{
			if (i == m_clients.end())
				break;

			GameClient *Client = (*i).second;
			if (Client->IsValid() == false || (m_currTime
				- Client->LastActive()) >= 30)
			{
				DEBUG_LOG( format("Routine dead client removal [%1%]") % Client->Address() );
				m_clients.erase(i++);
				delete Client;
			}
			else
				++i;
		}
		m_lastCleanupTime = m_currTime;
	}
}

void GameServer::Handle_Incoming()
{
	char Buffer[RECV_BUFFER_SIZE];

	socklen_t addr_len = sizeof(inc_addr);

	uint16 len = 0;
	std::stringstream IP;

	len = recvfrom(m_socket, Buffer, RECV_BUFFER_SIZE, 0,
			(struct sockaddr*) &inc_addr, &addr_len);

	if ((len > 0 && len <= RECV_BUFFER_SIZE && errno != EWOULDBLOCK) ||
			(len > 0 && len <= RECV_BUFFER_SIZE))
	{
		IP << inet_ntoa(inc_addr.sin_addr) << ":" << inc_addr.sin_port;
		GClientList::iterator i = m_clients.find(IP.str());
		if (i != m_clients.end())
		{
			if (m_clients[IP.str()]->IsValid() == false)
			{
				DEBUG_LOG( format("Removing dead client [%1%]") % IP.str() );
				delete m_clients[IP.str()];
				m_clients.erase(m_clients.find(IP.str()));
			}
			else
			{
				m_clients[IP.str()]->HandlePacket(Buffer, len);
			}
		}
		else
		{
			m_clients[IP.str()] = new GameClient(inc_addr, &m_socket);
			DEBUG_LOG(format ("Client connected [%1%], now have [%2%] clients")
					% IP.str() % Clients_Connected());
			m_clients[IP.str()]->HandlePacket(Buffer, len);
		}

	}
}


void GameServer::Broadcast( const ByteBuffer &message )
{
	/*for (GClientList::iterator i = m_clients.begin();i != m_clients.end();++i)
	{
		i->second->QueueState(message);
	}*/
}

GameClient *GameServer::GetClientWithSessionId(uint32 sessionId)
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

void GameServer::CheckAndResend()
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		it->second->CheckAndResend();
	}
}

void GameServer::AnnounceStateUpdate( class GameClient* clFrom,class MsgBaseClass *theMsg )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueState(theMsg);
			it->second->FlushQueue();
		}
	}
}

void GameServer::AnnounceCommand( class GameClient* clFrom,class MsgBaseClass *theCmd )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueCommand(theCmd);
		}
	}
}