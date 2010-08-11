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

#ifndef MXOSIM_GAMERUNNABLE_H
#define MXOSIM_GAMERUNNABLE_H

#include "Util.h"
#include "Threading/ThreadStarter.h"
#include "GameServer.h"

class GameRunnable : public ThreadContext
{
public:
    bool run() 
	{
		SetThreadName("Game Server Thread");

        // Initiate the server. Start the loop
        new GameServer;
        if (sGame.Start())
		{
			while(m_threadRunning)
			{
				sGame.Loop();
			}
			sGame.Stop();
		}
        // Loop stopped. server shutdown. Delete object
        delete GameServer::getSingletonPtr();
		return true;
    }

};

#endif