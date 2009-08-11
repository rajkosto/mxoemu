#ifndef TCPVARIABLELENGTHPACKET_H
#define TCPVARIABLELENGTHPACKET_H

#include "ByteBuffer.h"

class TCPVariableLengthPacket : public ByteBuffer
{
public:
	TCPVariableLengthPacket();
	TCPVariableLengthPacket(const byte* pData,size_t pSize);
	~TCPVariableLengthPacket();
	ByteBuffer GetContentsWithSize();
};

#endif