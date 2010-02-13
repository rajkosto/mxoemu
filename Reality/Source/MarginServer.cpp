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
    int Port = sConfig.GetIntDefault("MarginServer.Port",10000);
	INFO_LOG(format("Starting Margin server on port %1%") % Port);	

	if (listenSocketInst != NULL)
	{
		delete listenSocketInst;
	}
	listenSocketInst = new MarginListenSocket(marginSocketHandler);
	listenSocketInst->Bind(Port);
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

MarginSocket *MarginServer::GetSocketByCharacterUID( uint64 charUID )
{
	return marginSocketHandler.FindByCharacterUID(charUID);
}

MarginSocket *MarginServer::GetSocketBySessionId( uint32 sessionId )
{
	return marginSocketHandler.FindBySessionId(sessionId);
}