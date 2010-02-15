// *************************************************************************************************
// --------------------------------------
// Copyright (C) 2006-2010 Rajko Stojadinovic
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************

#ifndef MXOSIM_GAMECLIENT_H
#define MXOSIM_GAMECLIENT_H

#include "Crypto.h"
#include "SequencedPacket.h"
#include "Sockets.h"
#include "MessageTypes.h"
#include "PlayerObject.h"
#include "MessageTypes.h"
#include "Log.h"
#include <Sockets/SocketAddress.h>

class GameClient
{
public:		
	GameClient(shared_ptr<SocketAddress> address, class GameSocket *sock);
	~GameClient();

	inline uint32 LastActive() { return m_lastActivity; }
	inline bool IsValid() { return m_validClient; }
	void Invalidate() { m_validClient=false;}
	string Address() { return m_address->Convert(true); }
	uint32 GetSessionId() 
	{
		if (m_encryptionInitialized == true)
			return m_sessionId; 
		else
			return 0;
	}

	void HandlePacket(const char *pData, uint16 nLength);
	void HandleEncrypted(ByteBuffer &srcData);
	void HandleOther(ByteBuffer &otherData);
	void HandleOrdered(ByteBuffer &orderedData);
	void QueueState(msgBaseClassPtr theData,bool immediateOnly=false)
	{
		msgBaseClassPtr &realPtr = theData;
		shared_ptr<ObjectUpdateMsg> amIObjectUpdate = dynamic_pointer_cast<ObjectUpdateMsg>(realPtr);
		if (amIObjectUpdate != NULL)
			amIObjectUpdate->setReceiver(this);

		m_queuedStates.push_back(queuedState(realPtr,immediateOnly));
	}
	void QueueCommand(msgBaseClassPtr theCmd)
	{
		msgBaseClassPtr &realPtr = theCmd;
		shared_ptr<ObjectUpdateMsg> amIObjectUpdate = dynamic_pointer_cast<ObjectUpdateMsg>(realPtr);
		if (amIObjectUpdate != NULL)
			amIObjectUpdate->setReceiver(this);

		m_queuedCommands.push_back(realPtr);
	}
	void FlushQueue();
	void CheckAndResend();
private:
	SequencedPacket Decrypt(const char *pData, uint16 nLength);
	void PSSChanged(uint8 oldPSS,uint8 newPSS);
	bool PacketReceived(uint16 clientSeq)
	{
		bool wraparound=false;

		if (isSequenceMoreRecent(clientSeq,m_lastClientSequence) == true)
		{
			if ( (m_lastClientSequence > 4096/2) && clientSeq < 4096/2 )
				wraparound=true;

			m_lastClientSequence = clientSeq;
		}

		if (wraparound == true)
		{
			size_t removedPacketsToAck = 0;
			for (deque<uint16>::iterator it=m_packetsToAck.begin();it!=m_packetsToAck.end();)
			{
				if (*it < 4096/2)
				{
					it = m_packetsToAck.erase(it);
					removedPacketsToAck++;
				}
				else
				{
					++it;
				}
			}

			size_t flaggedActualPackets = 0;
			for (sendQueueList::iterator it=m_sendQueue.begin();it!=m_sendQueue.end();++it)
			{
				if (it->client_sequence < 4096/2)
				{
					it->client_sequence = m_lastClientSequence;
					it->ack = false;
					flaggedActualPackets++;
				}
			}

			INFO_LOG(format("(%1) Purged %2% potential and %3% real acks due to client wraparound") % Address() % removedPacketsToAck % flaggedActualPackets);
		}

		if (find(m_packetsToAck.begin(),m_packetsToAck.end(),clientSeq) != m_packetsToAck.end())
			return false;

		m_packetsToAck.push_back(clientSeq);
		return true;
	}
	uint32 AcknowledgePacket(uint16 serverSeq)
	{
		vector<uint16> zeAckedPacketz;
		uint32 eraseCounter=0;
		for (sendQueueList::iterator it=m_sendQueue.begin();it!=m_sendQueue.end();)
		{
			if (it->sent==true && it->server_sequence==serverSeq)
			{
				zeAckedPacketz.push_back(it->server_sequence);
				it=m_sendQueue.erase(it);
				eraseCounter++;
			}
			else
			{
				++it;
			}
		}
	/*	stringstream derp;
		derp << "Acked " << eraseCounter << " packets ( ";
		for (int i=0;i<zeAckedPacketz.size();i++)
		{
			derp << zeAckedPacketz[i] << " ";
		}
		vector<uint16> zePacketsLeft;
		for (sendQueueList::iterator it=m_sendQueue.begin();it!=m_sendQueue.end();++it)
		{
			if (it->sent==true)
			{
				zePacketsLeft.push_back(it->server_sequence);
			}
		}
		derp << ") " << zePacketsLeft.size() << "left ( ";
		for (int i=0;i<zePacketsLeft.size();i++)
		{
			derp << zePacketsLeft[i] << " ";
		}
		derp << ")";
		DEBUG_LOG(derp.str());*/
		return eraseCounter;
	}

	typedef deque<msgBaseClassPtr> queueType;
	queueType m_queuedCommands;
	struct queuedState
	{
		queuedState(msgBaseClassPtr theState, bool immediateOnly=false)
		{
			stateData = theState;
			noResend=immediateOnly;
		}
		bool noResend;
		msgBaseClassPtr stateData;
	};
	typedef deque<queuedState> stateQueueType;
	stateQueueType m_queuedStates;

	struct PacketInQueue
	{
		//the packet will own the data pointer
		PacketInQueue(uint8 thePSS, uint16 serverSeq, uint16 clientSeq, bool ackPacket, msgBaseClassPtr dataToSend, bool immediateOnly=false)
		{
			clientPSS = thePSS;
			server_sequence = serverSeq;
			client_sequence = clientSeq;
			ack=ackPacket;
			theData = dataToSend;
			sent=false;
			msTimeSent=0;
			resentCounter=0;
			noResends=immediateOnly;
		}
		~PacketInQueue() {}

		uint8 clientPSS;
		uint16 server_sequence;
		uint16 client_sequence;
		bool ack;
		msgBaseClassPtr theData;
		bool sent;
		uint32 msTimeSent;
		uint32 resentCounter;
		bool noResends;
	};
	typedef list<PacketInQueue> sendQueueList;
	sendQueueList m_sendQueue;
	void AddPacketToQueue(uint16 clientSeq, bool ackPacket, msgBaseClassPtr dataToSend, bool immediateOnly=false)
	{
		//if its a static packet of 0 bytes and no ack, no reason to send it
		if (ackPacket == false)
		{
			shared_ptr<StaticMsg> amiStatic = dynamic_pointer_cast<StaticMsg>(dataToSend);
			if (amiStatic != NULL)
			{
				uint32 packetLen = amiStatic->toBuf().size();
				if (packetLen < 1)
				{
					return;
				}
			}
		}
		increaseServerSequence();
		uint16 theServerSeq = m_serverSequence;
		ByteBuffer outputData = dataToSend->toBuf();
/*		if (outputData.size() > 0)
		{
			DEBUG_LOG(format("(%s) Queue SSeq: %d CSeq: %d Ack: %d Data: |%s|") % Address() % theServerSeq % clientSeq % ackPacket % Bin2Hex(outputData));
		}
		else
		{
			DEBUG_LOG(format("(%s) Queue SSeq: %d CSeq: %d Ack: %d No Data") % Address() % theServerSeq % clientSeq % ackPacket);
		}*/
		m_sendQueue.push_back(PacketInQueue(m_clientPSS,theServerSeq,clientSeq,ackPacket,dataToSend,immediateOnly));
	}
	void AddPacketToQueue(msgBaseClassPtr dataToSend, bool immediateOnly=false)
	{
		AddPacketToQueue(m_lastClientSequence,false,dataToSend,immediateOnly);
	}

	void MoveMsgsToQueue();
	void SendEncrypted(SequencedPacket withSequences);
	uint16 m_serverCommandsSent;

	bool m_encryptionInitialized;
	uint64 m_characterUID;
	uint32 m_charWorldId;
	uint32 m_sessionId;

	uint32 m_lastSimTimeMS;

	// client states
	bool m_validClient;
	bool m_worldLoaded;
	bool m_characterSpawned;

	// Master Sock handle, client's address structure, last received packet
	class GameSocket *m_sock;
	shared_ptr<SocketAddress> m_address;
	uint32 m_lastActivity;
	uint32 m_lastPacketReceivedMS;
	uint32 m_lastOrderedFlush;

	//Number of packets received
	uint32 m_numPackets;
	deque<uint16> m_packetsToAck;

	uint16 m_clientCommandsReceived;

	// Sequences
	uint8 m_clientPSS;
	uint16 m_serverSequence;
	inline void increaseServerSequence()
	{
		m_serverSequence++;
		if (m_serverSequence == 4096)
		{
			m_serverSequence=0;
		}
	}
	uint16 m_lastClientSequence;
	inline bool isSequenceMoreRecent( uint16 biggerSequence, uint16 smallerSequence, uint32 max_sequence=4096 )
	{
		return	( (biggerSequence > smallerSequence) && (biggerSequence-smallerSequence <= max_sequence/2) )
				|| ( (smallerSequence > biggerSequence) && (smallerSequence-biggerSequence > max_sequence/2) );
	}
	uint32 m_playerGoId;

	class MarginSocket *m_marginConn;

	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption TFDecryption;
	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption TFEncryption;
	typedef shared_ptr<TFDecryption> TFDecryptionPtr;
	typedef shared_ptr<TFEncryption> TFEncryptionPtr;
	TFDecryptionPtr m_TFDecrypt;
	TFEncryptionPtr m_TFEncrypt;	
};

#endif
