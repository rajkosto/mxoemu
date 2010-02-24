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
#include "GameSocket.h"
#include <Sockets/Ipv4Address.h>

initialiseSingleton( GameServer );

bool GameServer::Start()
{
	int Port = sConfig.GetIntDefault("GameServer.Port", 10000);
	INFO_LOG(format("Starting Game server on port %1%") % Port);

	m_mainSocket.reset(new GameSocket(m_udpHandler));
	port_t thePortToBind = Port;
	if (m_mainSocket->Bind(thePortToBind) != 0)
	{
		ERROR_LOG(format("Error binding Game Server to port %1%") % thePortToBind);
		return false;
	}
	m_udpHandler.Add(m_mainSocket.get());

	m_simTime=0;

	return true;
}

void GameServer::Stop()
{
	m_mainSocket.reset();
	INFO_LOG("Game Server shutdown");
}

void GameServer::Loop(void)
{
	if (m_mainSocket == NULL)
		return;

	m_mainSocket->PruneDeadClients();
	m_mainSocket->CheckAndResend();
	m_udpHandler.Select(0,4000); //4ms
}

void GameServer::Broadcast( const ByteBuffer &message )
{
	if (m_mainSocket != NULL)
	{
		m_mainSocket->Broadcast(message);
	}
}

void GameServer::AnnounceStateUpdate( GameClient* clFrom,msgBaseClassPtr theMsg, bool immediateOnly )
{
	if (m_mainSocket != NULL)
	{
		m_mainSocket->AnnounceStateUpdate(clFrom,theMsg,immediateOnly);
	}
}

void GameServer::AnnounceCommand( GameClient* clFrom,msgBaseClassPtr theCmd )
{
	if (m_mainSocket != NULL)
	{
		m_mainSocket->AnnounceCommand(clFrom,theCmd);
	}
}