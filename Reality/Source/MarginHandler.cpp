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

#include "MarginHandler.h"
#include "Common.h"
#include "MarginSocket.h"

MarginHandler::MarginHandler()
:SocketHandler()
{
}


MarginHandler::~MarginHandler()
{
}

vector<MarginSocket*> MarginHandler::FindByCharacterUID( uint64 charUID )
{
	vector<MarginSocket*> tempVect;
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		Socket *p = it->second;
		if (p == NULL)
			continue;
		TcpSocket *tcpSock = dynamic_cast<TcpSocket *>(p);
		if (tcpSock == NULL)
			continue;
		MarginSocket *margSock = dynamic_cast<MarginSocket *>(tcpSock);
		if (margSock == NULL)
			continue;

		if (margSock->GetCharUID() == charUID)
		{
			tempVect.push_back(margSock);
		}
	}
	return tempVect;
}

class MarginSocket *MarginHandler::FindBySessionId( uint32 sessionId )
{
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		Socket *p = it->second;
		if (p == NULL)
			continue;
		TcpSocket *tcpSock = dynamic_cast<TcpSocket *>(p);
		if (tcpSock == NULL)
			continue;
		MarginSocket *margSock = dynamic_cast<MarginSocket *>(tcpSock);
		if (margSock == NULL)
			continue;

		if (margSock->GetSessionId() == sessionId)
			return margSock;
	}
	return NULL;
}