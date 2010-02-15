#include "Common.h"
#include "GameSocket.h"
#include "Log.h"
#include "GameClient.h"
#include "Timer.h"
#include <Sockets/Ipv4Address.h>

GameSocket::GameSocket( ISocketHandler& theHandler ) : UdpSocket(theHandler)
{
	m_lastCleanupTime = getTime();
}

GameSocket::~GameSocket()
{

}

void GameSocket::OnRawData( const char *pData,size_t len,struct sockaddr *sa_from,socklen_t sa_len )
{
	stringstream IP;
	struct sockaddr_in inc_addr;
	memcpy(&inc_addr,sa_from,sa_len);
	shared_ptr<SocketAddress> theAddr(new Ipv4Address(inc_addr));

	if (theAddr->IsValid() == false)
		return;

	string IPStr = theAddr->Convert(true);
	GClientList::iterator it = m_clients.find(IPStr);
	if (it != m_clients.end())
	{
		GameClient *Client = it->second;
		if (Client->IsValid() == false)
		{
			DEBUG_LOG( format("Removing dead client [%1%]") % IPStr );
			m_clients.erase(it);
			delete Client;
		}
		else
		{
			Client->HandlePacket(pData, len);
		}
	}
	else
	{
		m_clients[IPStr] = new GameClient(theAddr, this);
		DEBUG_LOG(format ("Client connected [%1%], now have [%2%] clients")
			% IPStr % Clients_Connected());

		m_clients[IPStr]->HandlePacket(pData, len);
	}
}

void GameSocket::PruneDeadClients()
{
	m_currTime = getTime();
	if ((m_currTime - m_lastCleanupTime) >= 5)
	{
		// Do client cleanup
		for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();)
		{
			GameClient *Client = it->second;
			if (Client->IsValid() == false || (m_currTime - Client->LastActive()) >= 20)
			{
				DEBUG_LOG( format("Routine dead client removal [%1%]") % Client->Address() );
				m_clients.erase(it++);
				delete Client;
			}
			else
			{
				++it;
			}
		}
		m_lastCleanupTime = m_currTime;
	}
}

GameClient * GameSocket::GetClientWithSessionId( uint32 sessionId )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second->GetSessionId() == sessionId)
		{
			return it->second;
		}
	}
	return NULL;
}

void GameSocket::CheckAndResend()
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		it->second->CheckAndResend();
	}
}

void GameSocket::Broadcast( const ByteBuffer &message )
{
	/*for (GClientList::iterator i = m_clients.begin();i != m_clients.end();++i)
	{
		i->second->QueueState(message);
	}*/
}

void GameSocket::AnnounceStateUpdate( GameClient* clFrom,msgBaseClassPtr theMsg, bool immediateOnly/*=false*/ )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueState(theMsg,immediateOnly);
		}
	}
}

void GameSocket::AnnounceCommand( GameClient* clFrom,msgBaseClassPtr theCmd )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueCommand(theCmd);
		}
	}
}