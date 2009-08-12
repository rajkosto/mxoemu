#ifndef _REMOTESOCKET_H
#define _REMOTESOCKET_H

#include <Sockets/UdpSocket.h>


class RemoteSocket : public UdpSocket
{
public:
	RemoteSocket(ISocketHandler&);
	~RemoteSocket();

	void OnRawData(const char *,size_t,struct sockaddr *,socklen_t);
	void Replay(const char *,size_t);
private:

	CRITICAL_SECTION CriticalSection; 
};


#endif // _REMOTESOCKET_H