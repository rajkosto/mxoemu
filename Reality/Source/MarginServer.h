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

#ifndef MXOSIM_MARGINSERVER_H
#define MXOSIM_MARGINSERVER_H

#include "Singleton.h"
#include "MarginHandler.h"
#include "MarginSocket.h"
#include <Sockets/ListenSocket.h>

class MarginServer : public Singleton <MarginServer>
{
public:
	MarginServer();;
	~MarginServer();;
	void Start();
	void Stop();
	void Loop();

	class MarginSocket *GetSocketByCharacterUID(uint64 charUID);
	class MarginSocket *GetSocketBySessionId(uint32 sessionId);
private:
	MarginHandler marginSocketHandler;
	typedef ListenSocket<MarginSocket> MarginListenSocket;
	MarginListenSocket *listenSocketInst;
};


#define sMargin MarginServer::getSingleton()

#endif

