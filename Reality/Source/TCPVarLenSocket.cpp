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
