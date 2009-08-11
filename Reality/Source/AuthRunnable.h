#ifndef MXOSIM_AUTHRUNNABLE_H
#define MXOSIM_AUTHRUNNABLE_H

#include "Util.h"
#include "Threading/ThreadStarter.h"
#include "AuthServer.h"

class AuthRunnable : public ThreadContext
{
public:
	bool run() 
	{
		SetThreadName("Auth Server Thread");

		// Initiate the server. Start the loop
		new AuthServer();
		sAuth.Start();
		while(m_threadRunning)
		{
			sAuth.Loop();
		}
		sAuth.Stop();
		// Loop stopped. server shutdown. Delete object
		delete AuthServer::getSingletonPtr();

		return true;
	}
};

#endif
