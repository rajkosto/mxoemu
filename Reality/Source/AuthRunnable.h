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

#ifndef MXOSIM_AUTHRUNNABLE_H
#define MXOSIM_AUTHRUNNABLE_H

#include "Util.h"
#include "Threading/ThreadStarter.h"
#include "AuthServer.h"

class AuthRunnable : public ThreadContext
{
public:
	bool run() 
	{
		SetThreadName("Auth Server Thread");

		// Initiate the server. Start the loop
		new AuthServer();
		sAuth.Start();
		while(m_threadRunning)
		{
			sAuth.Loop();
		}
		sAuth.Stop();
		// Loop stopped. server shutdown. Delete object
		delete AuthServer::getSingletonPtr();

		return true;
	}
};

#endif
