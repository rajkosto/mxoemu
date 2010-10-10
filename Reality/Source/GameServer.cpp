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
#include "GameServer.h"
#include "GameClient.h"
#include "MarginServer.h"
#include "Log.h"
#include "Timer.h"
#include "Config.h"
#include "GameSocket.h"
#include "Database/DatabaseEnv.h"
#include <Sockets/Ipv4Address.h>

initialiseSingleton( GameServer );

bool GameServer::Start()
{
	m_simtimeStart = getFloatTime();
	m_simtimeOffset = 0;

	string Interface = sConfig.GetStringDefault("GameServer.IP", "0.0.0.0");
	int Port = sConfig.GetIntDefault("GameServer.Port", 10000);
	INFO_LOG(format("Starting Game server on port %1%") % Port);

	m_mainSocket.reset(new GameSocket(m_udpHandler));
	port_t thePortToBind = Port;
	if (m_mainSocket->Bind(Interface,thePortToBind) != 0)
	{
		ERROR_LOG(format("Error binding Game Server to port %1%") % thePortToBind);
		return false;
	}
	m_udpHandler.Add(m_mainSocket.get());
	m_serverStartMS = getMSTime();

	// Mark server as "up"
	{
		sDatabase.WaitExecute(format("UPDATE `worlds` SET `status`='1' WHERE `name`='%1%' LIMIT 1")
			% this->GetName() );
		m_serverUp=true;
	}

	return true;
}

void GameServer::Stop()
{
	m_mainSocket.reset();
	// Mark server as "down"
	{
		sDatabase.WaitExecute(format("UPDATE `worlds` SET `status`='0' WHERE `name`='%1%' LIMIT 1")
			% this->GetName() );
		m_serverUp = false;
	}
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

GameClient* GameServer::GetClientWithSessionId( uint32 sessionId )
{
	return m_mainSocket->GetClientWithSessionId(sessionId);
}

vector<GameClient*> GameServer::GetClientsWithCharacterId( uint64 charId )
{
	return m_mainSocket->GetClientsWithCharacterId(charId);
}

void GameServer::Broadcast( const ByteBuffer &message, bool command )
{
	if (m_mainSocket != NULL)
	{
		m_mainSocket->Broadcast(message, command);
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

string GameServer::GetName() const
{
	return sConfig.GetStringDefault("GameServer.WorldName", "Reality");
}

string GameServer::GetChatPrefix() const
{
	return sConfig.GetStringDefault("GameServer.ChatPrefix", "SOE+MXO") + string("+") + GetName();
}