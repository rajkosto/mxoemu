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
	uint64 GetCharacterId() 
	{
		return m_characterUID;
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
	string GetNetStats();
private:
	//RCC Start
	void ResetRCC();
	uint16 SendSequencedPacket(msgBaseClassPtr jumboPacket);
	SequencedPacket Decrypt(const char *pData, size_t nLength);
	bool PacketReceived(uint16 clientSeq);
	uint32 AcknowledgePacket(uint16 serverSeq, uint8 ackBits);

	// RCC Constants
	static const uint NUM_SAVED_PINGS = 16;
	static const uint PING_MULTIPLIER_RELIABLE = 1;
	static const uint PING_MULTIPLIER_UNRELIABLE = 2;
	static const uint MINIMUM_RESEND_TIME = 30;
	static const uint MAXIMUM_RESEND_TIME = 1000;
	static const uint INITIAL_PING = 200;
	static const uint MAX_ACKED_PACKETS = 7;
	static const uint MAX_SAVED_PACKETS = 32;

	typedef deque<SequencedPacket> savedPacketsType;
	savedPacketsType m_savedPackets;

	deque<uint32> m_pingHistory;
	float m_currentPing;

	//netstat statistics
	uint32 m_guarSent;
	uint32 m_guarResent;
	uint32 m_guarsInvalid;
	uint32 m_guarsSkipped;
	uint32 m_unguarSent;
	uint32 m_unguarResent;
	uint32 m_unguarsInvalid;
	uint32 m_duplicateCmdsReceived;
	uint32 m_unguarRejected;
	uint32 m_morePacketRequests;
	uint32 m_rawPacketsResent;

	void AddtoPingHistory(uint32 msTime)
	{
		m_pingHistory.push_back(msTime);
		while (m_pingHistory.size()>NUM_SAVED_PINGS)
			m_pingHistory.pop_front();

		m_currentPing = 0;
		foreach(uint32 thePing, m_pingHistory)
		{
			m_currentPing += thePing;
		}

		m_currentPing /= m_pingHistory.size();
	}

	struct sentMsgBlock
	{
		sentMsgBlock(MsgBlock theCmds, const queue<packetAckFunc>& theCallbacks)
		{
			commands=theCmds;
			callBacks=theCallbacks;
			invalidated=false;
		}
		~sentMsgBlock()
		{
		}

		uint32 getLastTimeSent()
		{
			if (packetsItsIn.size() < 1)
				return 0;

			uint32 highestMsSent=0;
			for(map<uint16,uint32>::iterator it=packetsItsIn.begin();it!=packetsItsIn.end();++it)
			{
				if (it->second > highestMsSent)
					highestMsSent=it->second;
			}
			return highestMsSent;
		}

		MsgBlock commands;
		map<uint16,uint32> packetsItsIn;
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

		uint32 getLastTimeSent()
		{
			if (packetsItsIn.size() < 1)
				return 0;

			uint32 highestMsSent=0;
			for(map<uint16,uint32>::iterator it=packetsItsIn.begin();it!=packetsItsIn.end();++it)
			{
				if (it->second > highestMsSent)
					highestMsSent=it->second;
			}
			return highestMsSent;
		}

		bool noResend;
		msgBaseClassPtr stateData;
		map<uint16,uint32> packetsItsIn;
		packetAckFunc callBack;
		bool invalidated;
	};
	typedef deque<queuedState> stateQueueType;
	stateQueueType m_queuedStates;

	void SendEncrypted(SequencedPacket withSequences);

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
		for(uint i=0;i<MAX_ACKED_PACKETS;i++)
		{
			if(m_recvdPacketSeqs.count(clientSeq-i) > 0)
				ackBits |= (1 << i);
		}
		/*
		printf("ACK BITS: %02x for sequence %d [",uint32(ackBits),clientSeq);
		for(int i=0;i<MAX_ACKED_PACKETS;i++)
		{
			if (m_recvdPacketSeqs.count(clientSeq-i) > 0)
				printf("%d=%d ",clientSeq-i,m_recvdPacketSeqs[clientSeq-i]);
		}
		printf("]\n");*/
		return ackBits;
	}
	typedef vector<uint16> clientCommandsType;
	clientCommandsType m_clientCommandsReceived;

	// Sequences
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

	float m_lastSimTimeUpdate;
	//RCC end

	bool m_encryptionInitialized;
	uint64 m_characterUID;
	uint32 m_charWorldId;
	uint32 m_sessionId;

	// client states
	bool m_validClient;

	// Master Sock handle, client's address structure, last received packet
	class GameSocket *m_sock;
	Ipv4Address m_address;
	uint32 m_lastActivity;
	uint32 m_lastPacketReceivedMS;
	uint32 m_lastServerMS;

	uint32 m_playerGoId;
	class MarginSocket *m_marginConn;
	TwofishCryptEngine m_tfEngine;
};

#endif
