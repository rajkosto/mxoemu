#include "Util.h"
#include "TCPVariableLengthPacket.h"

TCPVariableLengthPacket::TCPVariableLengthPacket() : ByteBuffer()
{
}

TCPVariableLengthPacket::TCPVariableLengthPacket( const byte* pData,size_t pSize )
{
	string wholePacket = string((const char*)pData,pSize);

	int bytesToRemove = 1;

	byte firstByte = wholePacket.data()[0];
	if (firstByte > 0x7F)
	{
		bytesToRemove = 2;
		firstByte-=0x80;
	}

	this->append(wholePacket.substr(bytesToRemove));
	this->rpos(0);
}

ByteBuffer TCPVariableLengthPacket::GetContentsWithSize()
{
	uint16 packetSize = this->size();
	if (packetSize > 0x7f)
	{
		packetSize = swap16(packetSize | 0x8000);
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

TCPVariableLengthPacket::~TCPVariableLengthPacket()
{

}