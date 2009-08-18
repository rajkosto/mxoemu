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

#ifndef MXOSIM_TCPVARLENSOCKET_H
#define MXOSIM_TCPVARLENSOCKET_H

#include "Common.h"

#include <Sockets/ListenSocket.h>
#include <Sockets/TcpSocket.h>
#include <Sockets/ISocketHandler.h>
#include "TCPVariableLengthPacket.h"

class TCPVarLenSocket : public TcpSocket
{
public:
	TCPVarLenSocket(ISocketHandler& h);
	~TCPVarLenSocket();

	void OnRead();
private:
	virtual void ProcessData(const byte *buf,size_t len) = 0;
protected:
	void SendPacket(const TCPVariableLengthPacket &varLenPacket);
};

#endif
