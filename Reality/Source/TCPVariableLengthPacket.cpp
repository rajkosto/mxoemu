#include "Common.h"
#include "TCPVariableLengthPacket.h"


TCPVariableLengthPacket::TCPVariableLengthPacket() : ByteBuffer()
{
}

TCPVariableLengthPacket::TCPVariableLengthPacket(const ByteBuffer &srcBuf) : ByteBuffer(srcBuf)
{
}

TCPVariableLengthPacket::~TCPVariableLengthPacket()
{
}

ByteBuffer TCPVariableLengthPacket::GetProcessed() const
{
	uint16 packetSize = this->size();
	if (packetSize > 0x7f)
	{
		packetSize = htons(packetSize | 0x8000);
		ByteBuffer packetThing;
		packetThing << packetSize;
		packetThing.append((const byte*)this->contents(),this->size());

		return packetThing;
	}
	else
	{
		ByteBuffer packetThing;
		packetThing << byte(packetSize);
		packetThing.append((const byte*)this->contents(),this->size());
		return packetThing;
	}
}
