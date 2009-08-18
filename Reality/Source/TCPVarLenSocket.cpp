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

#include "TCPVarLenSocket.h"
#include "Common.h"

TCPVarLenSocket::TCPVarLenSocket(ISocketHandler& h) : TcpSocket(h)
{
}

TCPVarLenSocket::~TCPVarLenSocket()
{
}


void TCPVarLenSocket::OnRead()
{
	// OnRead of TcpSocket actually reads the data from the socket
	// and moves it to the input buffer (ibuf)
	TcpSocket::OnRead();
	// get number of bytes in input buffer
	size_t n = ibuf.GetLength();
	if (n >= 2)
	{
		byte firstTwoBytes[2];
		//peek first 2 bytes
		ibuf.Peek((char*)&firstTwoBytes,2);

		int sizeOfPacketSize = 1;
		if (firstTwoBytes[0] > 0x7F)
		{
			sizeOfPacketSize = 2;
			firstTwoBytes[0] -= 0x80;
		}

		uint16 packetSize = 0;
		if (sizeOfPacketSize == 1)
		{
			packetSize = firstTwoBytes[0];
		}
		else if (sizeOfPacketSize == 2)
		{
			memcpy(&packetSize,firstTwoBytes,sizeof(packetSize));
			packetSize = swap16(packetSize);
		}

		n = ibuf.GetLength();
		size_t requiredLen = sizeOfPacketSize+packetSize;
		if (n >= requiredLen)
		{
			ibuf.Remove(sizeOfPacketSize);

			vector<byte> tempStorage;
			tempStorage.resize(packetSize);
			ibuf.Read((char*)&tempStorage[0],tempStorage.size());

			ProcessData(&tempStorage[0],tempStorage.size());
		}
	}
}

void TCPVarLenSocket::SendPacket( const TCPVariableLengthPacket &varLenPacket )
{
	ByteBuffer withHeader = varLenPacket.GetProcessed();
	SendBuf(withHeader.contents(),withHeader.size());
}
