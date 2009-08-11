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
