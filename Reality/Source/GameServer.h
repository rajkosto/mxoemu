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
#include "ObjectMgr.h"
#include <Sockets/SocketHandler.h>
#include "MessageTypes.h"

#include <boost/timer.hpp>

class GameServer : public Singleton <GameServer>
{
public:
	GameServer() { };
	~GameServer() { /* TODO: Add destructor code */ };
	bool Start();
	void Stop();
	void Loop();
	ObjectMgr &getObjMgr() { return m_objMgr; }
	class GameClient *GetClientWithSessionId(uint32 sessionId);
	void Broadcast(const ByteBuffer &message);
	void AnnounceStateUpdate(class GameClient* clFrom,msgBaseClassPtr theMsg, bool immediateOnly=false);
	void AnnounceCommand(class GameClient* clFrom,msgBaseClassPtr theCmd);
	uint32 GetSimTime() 
	{
		if (m_timer.elapsed() >= double(1/15))
		{
			m_simTime+=uint32(double(m_timer.elapsed())/double(1/30));
			m_timer.restart();
		}
		return m_simTime;
	}
private:	
	ObjectMgr m_objMgr;
	SocketHandler m_udpHandler;
	shared_ptr<class GameSocket> m_mainSocket;
	uint32 m_simTime;
	boost::timer m_timer;
};


#define sGame GameServer::getSingleton()
#define sObjMgr GameServer::getSingleton().getObjMgr()

#endif

