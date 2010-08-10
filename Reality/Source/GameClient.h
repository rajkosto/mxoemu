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
#include "SymmetricCrypto.h"
#include "SequencedPacket.h"
#include "Sockets.h"
#include "MessageTypes.h"
#include "PlayerObject.h"
#include "MessageTypes.h"
#include "Log.h"
#include "Timer.h"
#include <Sockets/Ipv4Address.h>

class GameClient
{
public:		
	GameClient(sockaddr_in inc_addr, class GameSocket *sock);
	~GameClient();

	inline uint32 LastActive() { return m_lastActivity; }
	inline bool IsValid() { return m_validClient; }
	void Invalidate() { m_validClient=false;}
	string Address() { return m_address.Convert(true); }
	uint32 GetSessionId() 
	{
		if (m_encryptionInitialized == true)
			return m_sessionId; 
		else
			return 0;
	}
	uint32 GetWorldCharId() { return m_charWorldId; }

	void HandlePacket(const char *pData, size_t nLength);
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

		m_queuedCommands.push_back(queuedMsg(m_serverCommandsSent,realPtr));
		m_serverCommandsSent++;
	}
	void QueueRaw(msgBaseClassPtr theCmd)
	{
		msgBaseClassPtr &realPtr = theCmd;
		shared_ptr<ObjectUpdateMsg> amIObjectUpdate = dynamic_pointer_cast<ObjectUpdateMsg>(realPtr);
		if (amIObjectUpdate != NULL)
			amIObjectUpdate->setReceiver(this);

		m_queuedCommands.push_back(queuedMsg(m_serverCommandsSent,realPtr));
		m_serverCommandsSent++;
	}
	void FlushQueue(bool alsoResend=false);
	void CheckAndResend();
	void Reconnect();
	void ReconnectBeat();
private:
	bool SendSequencedPacket(msgBaseClassPtr jumboPacket);
	SequencedPacket Decrypt(const char *pData, size_t nLength);
	void FlagsChanged(uint8 oldFlags,uint8 newFlags);
	bool PacketReceived(uint16 clientSeq);
	uint32 AcknowledgePacket(uint16 serverSeq);
	struct queuedMsg
	{
		queuedMsg(uint16 newSeqId, msgBaseClassPtr dataToSend)
		{
			sequenceId=newSeqId;
			theData=dataToSend;
			msLastSent=0;
		}
		~queuedMsg()
		{
		}
		bool operator<(const queuedMsg& rhs) const
		{
			if (this->msLastSent == 0 && rhs.msLastSent > 0)
				return true;
			if (this->msLastSent > 0 && rhs.msLastSent == 0)
				return false;

			return this->sequenceId < rhs.sequenceId;
		}

		uint16 sequenceId;
		msgBaseClassPtr theData;
		vector<uint16> packetsItsIn;
		uint32 msLastSent;
	};
	typedef deque<queuedMsg> msgQueueType;
	msgQueueType m_queuedCommands;
	uint16 m_serverCommandsSent;

	struct queuedState
	{
		queuedState(msgBaseClassPtr theState, bool immediateOnly=false)
		{
			stateData = theState;
			noResend=immediateOnly;
		}
		bool noResend;
		msgBaseClassPtr stateData;
		vector<uint16> packetsItsIn;
		uint32 msLastSent;
	};
	typedef deque<queuedState> stateQueueType;
	stateQueueType m_queuedStates;

	void SendEncrypted(SequencedPacket withSequences);

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
	Ipv4Address m_address;
	uint32 m_lastActivity;
	uint32 m_lastPacketReceivedMS;
	bool m_calculatedInitialLatency;
	uint32 m_lastServerMS;

	//sequences of packets received
	typedef deque<uint16> packetsToAckType;
	packetsToAckType m_packetsToAck;

	uint16 GetAnAck(uint16 serverSeq)
	{
		if (m_packetsToAck.size() > 0)
		{
			uint16 theAck = m_packetsToAck.front();
			m_packetsToAck.pop_front();
			return theAck;
		}
		return m_lastClientSequence;
	}
	typedef map<uint16,ByteBuffer> clientCommandsType;
	clientCommandsType m_clientCommandsReceived;

	// Sequences
	uint8 m_clientFlags;
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
		return	( (biggerSequence > smallerSequence) && (biggerSequence-smallerSequence <= int32(max_sequence/2)) )
				|| ( (smallerSequence > biggerSequence) && (smallerSequence-biggerSequence > int32(max_sequence/2)) );
	}
	uint32 m_playerGoId;

	class MarginSocket *m_marginConn;

	TwofishCryptEngine m_tfEngine;
};

#endif
