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
#include "MarginServer.h"
#include "MarginSocket.h"
#include "Log.h"
#include "Config.h"

initialiseSingleton( MarginServer );

MarginServer::MarginServer()
{
	listenSocketInst = NULL;
}

MarginServer::~MarginServer()
{
	if (listenSocketInst != NULL)
	{
		delete listenSocketInst;
	}
}

void MarginServer::Start()
{
	string Interface = sConfig.GetStringDefault("MarginServer.IP","0.0.0.0");
    int Port = sConfig.GetIntDefault("MarginServer.Port",10000);
	INFO_LOG(format("Starting Margin server on port %1%") % Port);	

	if (listenSocketInst != NULL)
	{
		delete listenSocketInst;
	}
	listenSocketInst = new MarginListenSocket(marginSocketHandler);

	bool bindFailed=false;
	try
	{
		if (listenSocketInst->Bind(Port)!=0)
			bindFailed=true;
	}
	catch (Exception)
	{
		bindFailed=true;
	}
	if (bindFailed)
	{
		ERROR_LOG(format("Error binding MarginServer to port %1%") % Port);
		return;
	}
	marginSocketHandler.Add(listenSocketInst);
}

void MarginServer::Stop()
{
	INFO_LOG("Margin Server shutdown");
	if (listenSocketInst != NULL)
	{
		delete listenSocketInst;
		listenSocketInst = NULL;
	}
}

void MarginServer::Loop(void)
{
	marginSocketHandler.Select(0, 100000);// 100 ms
}

vector<MarginSocket*> MarginServer::GetSocketsForCharacterUID( uint64 charUID )
{
	return marginSocketHandler.FindByCharacterUID(charUID);
}

MarginSocket *MarginServer::GetSocketBySessionId( uint32 sessionId )
{
	return marginSocketHandler.FindBySessionId(sessionId);
}
