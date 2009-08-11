#ifndef _REMOTESOCKET_H
#define _REMOTESOCKET_H

#include <UdpSocket.h>


class RemoteSocket : public UdpSocket
{
public:
	RemoteSocket(ISocketHandler&);

	void OnRawData(const char *,size_t,struct sockaddr *,socklen_t);
};


#endif // _REMOTESOCKET_H