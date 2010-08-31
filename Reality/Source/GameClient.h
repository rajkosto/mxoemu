// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
// ***************************************************************************

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

	typedef boost::function<void ()> packetAckFunc;

	void QueueState(msgBaseClassPtr theData,bool immediateOnly=false,packetAckFunc callFunc=0)
	{
		msgBaseClassPtr &realPtr = theData;
		shared_ptr<ObjectUpdateMsg> amIObjectUpdate = dynamic_pointer_cast<ObjectUpdateMsg>(realPtr);
		if (amIObjectUpdate != NULL)
			amIObjectUpdate->setReceiver(this);

		m_queuedStates.push_back(queuedState(realPtr,immediateOnly,callFunc));
	}
	void QueueCommand(msgBaseClassPtr theCmd,packetAckFunc callFunc=0)
	{
		msgBaseClassPtr &realPtr = theCmd;
		shared_ptr<ObjectUpdateMsg> amIObjectUpdate = dynamic_pointer_cast<ObjectUpdateMsg>(realPtr);
		if (amIObjectUpdate != NULL)
			amIObjectUpdate->setReceiver(this);

		m_queuedCommands.push_back(queuedMsg(m_serverCommandsSent,theCmd,callFunc));
		m_serverCommandsSent++;
	}
	void FlushQueue(bool alsoResend=false);
	void CheckAndResend();
private:
	uint16 SendSequencedPacket(msgBaseClassPtr jumboPacket);
	SequencedPacket Decrypt(const char *pData, size_t nLength);
	bool PacketReceived(uint16 clientSeq);
	uint32 AcknowledgePacket(uint16 serverSeq, uint8 ackBits);

	struct sentMsgBlock
	{
		sentMsgBlock(MsgBlock theCmds, const queue<packetAckFunc>& theCallbacks)
		{
			commands=theCmds;
			callBacks=theCallbacks;
			msLastSent=0;
			invalidated=false;
		}
		~sentMsgBlock()
		{
		}

		MsgBlock commands;
		vector<uint16> packetsItsIn;
		uint32 msLastSent;
		queue<packetAckFunc> callBacks;
		bool invalidated;
	};
	typedef list<sentMsgBlock> sentMsgBlocksType;
	sentMsgBlocksType m_sentCommands;

	struct queuedMsg
	{
		queuedMsg(uint16 theSeq, msgBaseClassPtr theData, packetAckFunc theCallback=0)
		{
			sequenceId=theSeq;
			data=theData;
			callBack=theCallback;
		}
		~queuedMsg()
		{
		}

		uint16 sequenceId;
		msgBaseClassPtr data;
		packetAckFunc callBack;
	};

	typedef deque<queuedMsg> queuedMsgType;
	queuedMsgType m_queuedCommands;
	uint16 m_serverCommandsSent;

	struct queuedState
	{
		queuedState(msgBaseClassPtr theState, bool immediateOnly, packetAckFunc callFunc )
		{
			stateData=theState;
			noResend=immediateOnly;
			callBack=callFunc;
			invalidated=false;
		}
		bool noResend;
		msgBaseClassPtr stateData;
		vector<uint16> packetsItsIn;
		uint32 msLastSent;
		packetAckFunc callBack;
		bool invalidated;
	};
	typedef deque<queuedState> stateQueueType;
	stateQueueType m_queuedStates;

	void SendEncrypted(SequencedPacket withSequences);

	bool m_encryptionInitialized;
	uint64 m_characterUID;
	uint32 m_charWorldId;
	uint32 m_sessionId;

	float m_lastSimTime;

	// client states
	bool m_validClient;

	// Master Sock handle, client's address structure, last received packet
	class GameSocket *m_sock;
	Ipv4Address m_address;
	uint32 m_lastActivity;
	uint32 m_lastPacketReceivedMS;
	bool m_calculatedInitialLatency;
	uint32 m_lastServerMS;

	//sequences of packets received
	typedef unordered_map<uint16,bool> recvdPacketSeqsType;
	recvdPacketSeqsType m_recvdPacketSeqs;

	uint16 GetAnAck()
	{
		for(recvdPacketSeqsType::iterator it=m_recvdPacketSeqs.begin();it!=m_recvdPacketSeqs.end();++it)
		{
			if (it->second == false)
			{
				it->second=true;
				return it->first;
			}
		}
		
		if (m_lastClientSequence>0)
			m_recvdPacketSeqs[m_lastClientSequence] = true;

		return m_lastClientSequence;
	}
	uint8 GetAckBits(uint16 clientSeq)
	{
		uint8 ackBits=0;
		for(int i=0;i<7;i++)
		{
			if(m_recvdPacketSeqs.count(clientSeq-i) > 0)
				ackBits |= (1 << i);
		}
		/*
		printf("ACK BITS: %02x for sequence %d [",uint32(ackBits),clientSeq);
		for(int i=0;i<7;i++)
		{
			if (m_recvdPacketSeqs.count(clientSeq-i) > 0)
				printf("%d=%d ",clientSeq-i,m_recvdPacketSeqs[clientSeq-i]);
		}
		printf("]\n");*/
		return ackBits;
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
