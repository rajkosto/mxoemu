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
#include "Log.h"
#include "Timer.h"
#include "Config.h"
#include "Sockets.h"

initialiseSingleton( GameServer );

bool GameServer::Start()
{
	int Port;

	Port = sConfig.GetIntDefault("GameServer.Port", 10000);

	INFO_LOG("Starting Game server on port %d", Port);

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

	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (Socket == INVALID_SOCKET)
	{
		CRITICAL_LOG("Unable to create socket!");
		return false;
	}

	// 0 = Blocking sockets - 1 = Non-blocking
	unsigned long mode = 1;

#if PLATFORM == PLATFORM_WIN32
	int ret = ioctlsocket(Socket, FIONBIO, &mode );
#else
	int ret = ioctl(Socket, FIONBIO, &mode);
#endif

	if (ret < 0)
	{
		CRITICAL_LOG("Unable to set socket to non blocking!");
		Socket = INVALID_SOCKET;
		return false;
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(Port);

	if (::bind(Socket, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0)
	{
		CRITICAL_LOG("Unable to bind socket!");
		Socket = INVALID_SOCKET;
		return false;
	}
	lastCleanupTime = getTime();

	return true;
}

void GameServer::Stop()
{
	INFO_LOG("Game Server shutdown");

#if PLATFORM == PLATFORM_WIN32
	closesocket(Socket);
	WSACleanup();
#else
	close(Socket);
#endif

	Socket = INVALID_SOCKET;
}

void GameServer::Loop(void)
{
	FD_ZERO(&Readable);
	FD_SET(Socket, &Readable);

	timeout.tv_sec = 0;
	timeout.tv_usec = 100;

	if (select(0, &Readable, NULL, NULL, &timeout) == SOCKET_ERROR)
	{
		CRITICAL_LOG("Select() Failed, Shutting down World server");
		Stop();
	}

	if (FD_ISSET(Socket, &Readable))
	{
		Handle_Incoming();
	}
	CurTime = getTime();
	if ((CurTime - lastCleanupTime) >= 5)
	{
		// Do client cleanup
		GClientList::iterator i = Clients.begin();
		for (;;)
		{
			if (i == Clients.end())
				break;

			GameClient *Client = (*i).second;
			if (Client->IsValid() == false || (CurTime
				- Client->LastActive()) >= 30)
			{
				DEBUG_LOG("Routine dead client removal [%s]",Client->Address().c_str());
				Clients.erase(i++);
				delete Client;
			}
			else
				++i;
		}
		lastCleanupTime = CurTime;
	}
}

void GameServer::Handle_Incoming()
{
	char Buffer[RECV_BUFFER_SIZE];

	socklen_t addr_len = sizeof(inc_addr);

	uint16 len = 0;
	std::stringstream IP;

	len = recvfrom(Socket, Buffer, RECV_BUFFER_SIZE, 0,
			(struct sockaddr*) &inc_addr, &addr_len);

	if ((len > 0 && len <= RECV_BUFFER_SIZE && errno != EWOULDBLOCK) ||
			(len > 0 && len <= RECV_BUFFER_SIZE))
	{
		IP << inet_ntoa(inc_addr.sin_addr) << ":" << inc_addr.sin_port;
		GClientList::iterator i = Clients.find(IP.str());
		if (i != Clients.end())
		{
			if (Clients[IP.str()]->IsValid() == false)
			{
				DEBUG_LOG("Removing dead client [%s]", IP.str().c_str());
				delete Clients[IP.str()];
				Clients.erase(Clients.find(IP.str()));
			}
			else
			{
				Clients[IP.str()]->HandlePacket(Buffer, len);
			}
		}
		else
		{
			Clients[IP.str()] = new GameClient(inc_addr, &Socket);
			DEBUG_LOG("Client connected [%s], now have [%d] clients",
					IP.str().c_str(), Clients_Connected());
			Clients[IP.str()]->HandlePacket(Buffer, len);
		}

	}
}

void GameServer::Broadcast( const ByteBuffer &message )
{
	for (GClientList::iterator i = Clients.begin();i != Clients.end();++i)
	{
		i->second->Send(message);
	}
}