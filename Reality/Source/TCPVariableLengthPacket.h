#ifndef MXOSIM_TCPVARIABLELENGTHPACKET_H
#define MXOSIM_TCPVARIABLELENGTHPACKET_H

#include "ByteBuffer.h"

class TCPVariableLengthPacket : public ByteBuffer
{
public:
	TCPVariableLengthPacket();
	TCPVariableLengthPacket(const ByteBuffer &srcBuf);
	~TCPVariableLengthPacket();
	ByteBuffer GetProcessed() const;
};

#endif
