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

#include "GameClient.h"
#include "Singleton.h"

#define RECV_BUFFER_SIZE 1024

class GameServer : public Singleton <GameServer>
{
	private:		
		// Client List
		typedef std::map<std::string, GameClient*> GClientList;
		GClientList Clients;
		struct sockaddr_in listen_addr, inc_addr;

		// Socket stuff
		SOCKET Socket;
		fd_set Readable;

		struct timeval timeout;
		uint32 lastCleanupTime;
		uint32 CurTime;
	public:
		GameServer() { /* TODO: Add constructor code */ };
		~GameServer() { /* TODO: Add destructor code */ };
		bool Start();
		void Stop();
		void Loop();
		int Clients_Connected(void) { return (int)Clients.size(); }
		void Handle_Incoming();
		void Broadcast(const ByteBuffer &message);
};


#define sGame GameServer::getSingleton()

#endif

