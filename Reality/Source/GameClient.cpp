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

#include "Common.h"

#include "GameClient.h"
#include "Timer.h"
#include "Util.h"
#include "MersenneTwister.h"
#include "SequencedPacket.h"
#include "RsiData.h"
#include "MarginSocket.h"
#include "MarginServer.h"
#include "Database/Database.h"
#include "Log.h"
#include "GameServer.h"
#include "GameSocket.h"
#include "EncryptedPacket.h"

GameClient::GameClient(sockaddr_in inc_addr, GameSocket *sock):m_address(inc_addr),m_sock(sock)
{
	m_validClient = true;
	m_encryptionInitialized = false;

	ResetRCC();

	m_characterUID = 0;
	m_playerGoId=0;
}

GameClient::~GameClient()
{
	if (m_playerGoId != 0)
	{
		sObjMgr.destroyObject(m_playerGoId);
	}
	sObjMgr.releaseRelevantSet(this);

	MarginSocket* marginConn = sMargin.GetSocketBySessionId(m_sessionId);
	if(marginConn)
		marginConn->ForceDisconnect();
}

void GameClient::HandlePacket( const char *pData, size_t nLength )
{
	if (nLength < 1 || !IsValid())
		return;

	m_lastActivity = getTime();
	m_lastPacketReceivedMS = getMSTime();

	if (m_encryptionInitialized == false && pData[0] == 0 && nLength == 43)
	{
		m_lastServerMS = getMSTime();

		ByteBuffer packetData;
		packetData.append(pData,nLength);
		packetData.rpos(0x0B);
		if (packetData.remaining() < sizeof(m_characterUID))
		{
			Invalidate();
			return;
		}
		packetData >> m_characterUID;
		try
		{
			m_playerGoId = sObjMgr.constructPlayer(this,m_characterUID);
		}
		catch (ObjectMgr::ObjectNotAvailable)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Character doesn't exist") % Address() );
			Invalidate();
			return;
		}

		vector<MarginSocket*> marginConns = sMargin.GetSocketsForCharacterUID(m_characterUID);
		if (marginConns.size() < 1)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Margin session for character not found") % Address() );
			Invalidate();
			return;
		}
		//we need to test every margin session that has the same charId, for a matching sessionId
		MarginSocket* marginConn = NULL;
		foreach(marginConn, marginConns)
		{
			m_sessionId = marginConn->GetSessionId();
			m_charWorldId = marginConn->GetWorldCharId();

			//initialize encryptors with key from margin
			vector<byte> twofishKey = marginConn->GetTwofishKey();
			m_tfEngine.Initialize(&twofishKey[0], twofishKey.size());

			//now we can verify if session key in this packet is correct
			packetData.rpos(packetData.size()-TwofishCryptMethod::BLOCKSIZE);
			if (packetData.remaining() < TwofishCryptMethod::BLOCKSIZE)
			{
				//wat, should never happen
				Invalidate();
				marginConn->ForceDisconnect();
				return;
			}
			vector<byte> encryptedSessionId(packetData.remaining());
			packetData.read(encryptedSessionId);
			ByteBuffer decryptedData = m_tfEngine.Decrypt(&encryptedSessionId[0],encryptedSessionId.size(),false);
			if (decryptedData.size() != TwofishCryptMethod::BLOCKSIZE)
			{
				//invalid key, try another connection
				continue;
			}
			uint32 recoveredSessionId=0;
			decryptedData >> recoveredSessionId;

			if (recoveredSessionId != m_sessionId)
			{
				//invalid sessionId, try another connection
				continue;
			}
		}

		if (!marginConn)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Margin session for character not found") % Address() );
			Invalidate();
			return;
		}

		m_encryptionInitialized=true;

		//send latency/loss calibration heartbeats
		const int numberOfBeats = 5;
		for (int i=0;i<numberOfBeats;i++)
		{
			ByteBuffer beatPacket;
			for (int j=0;j<numberOfBeats;j++)
			{
				beatPacket << uint8(0);
			}
			beatPacket << uint16(swap16(numberOfBeats));

			m_sock->SendToBuf(m_address, beatPacket.contents(), beatPacket.size(), 0);
		}

		//notify margin that udp session is established
		if (marginConn->UdpReady(this) == false)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Margin not ready for UDP connection") % Address() );
			m_encryptionInitialized=false;
			Invalidate();
			marginConn->ForceDisconnect();
			return;
		}
		sObjMgr.getGOPtr(m_playerGoId)->InitializeWorld();
		FlushQueue();
		return;
	}

	if (m_encryptionInitialized == true && pData[0] != 0x01) // Ping...just reply with the same thing
	{		
		const byte pingHeader[8] =	{0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x08};
		if ( (nLength != sizeof(pingHeader)+sizeof(uint32)) || 
			memcmp(pingHeader,pData,sizeof(pingHeader)) != 0)
		{
			WARNING_LOG(format("UNENCRYPTED (BUT NOT PING): %1%") % Bin2Hex(pData,nLength,0));
		}
		else
		{
			m_sock->SendToBuf(m_address, pData, int(nLength), 0);
		}
	}
	else
	{
		if (pData[0] == 0x01 && m_encryptionInitialized==true)
		{
			SequencedPacket packetData=Decrypt(&pData[1],nLength-1);
			ByteBuffer dataToParse;

			uint32 pktsAcked = AcknowledgePacket(packetData.getRemoteSeq(), packetData.getAckBits());
			if (packetData.getAckBits() & 0x80)
			{
				m_morePacketRequests++;

				DEBUG_LOG(format("(%s) Set first bit of ackBits (%02x)! SSeq: %d CSeq: %d Packet %s")
					%Address()
					%uint32(packetData.getAckBits())
					%packetData.getRemoteSeq()
					%packetData.getLocalSeq()
					%Bin2Hex(packetData));
			}

			uint8 firstByte = packetData.contents()[0];
			//normal packet byte
			if (firstByte==0x02)
			{
				dataToParse.append(&packetData.contents()[1],packetData.size()-1);
			}
			else
			{
				WARNING_LOG(format("(%1%) First byte of client packet isnt 0x02 !") % Address());
				dataToParse.append(&packetData.contents()[0],packetData.size());
			}

		/*	if (dataToParse.size() > 0)
			{
				DEBUG_LOG( format("(%s) Recv FLAGS: %x CSeq: %d SSeq: %d |%s|") % Address() % uint32(packetData.getFlags()) % packetData.getLocalSeq() % packetData.getRemoteSeq() % Bin2Hex(dataToParse) );
			}
			else
			{
				DEBUG_LOG( format("(%s) Recv FLAGS: %x CSeq: %d SSeq: %d") % Address() % uint32(packetData.getFlags()) % packetData.getLocalSeq() % packetData.getRemoteSeq() );
			}*/

			//add to need to ack list
			PacketReceived(packetData.getLocalSeq());						

			//TODO: hack,we should update things irrespective of packets received
			if (m_playerGoId != 0)
			{
				sObjMgr.getGOPtr(m_playerGoId)->Update();
			}

			if (dataToParse.size() > 0)
			{
				HandleEncrypted(dataToParse);
			}

			FlushQueue();
		}
	}
}

void GameClient::HandleEncrypted( ByteBuffer &srcData )
{
	ByteBuffer dataCopy(&srcData.contents()[srcData.rpos()],srcData.remaining());
	int32 commandOffset = -1;
	ByteBuffer zeroFourBlock;
	ByteBuffer zeroFiveBlock;

	//try to find 04 block
	while (dataCopy.remaining() > 0)
	{
		uint8 theByte;
		dataCopy >> theByte;
		if (theByte == 0x04)
		{
			//reverse read of byte
			dataCopy.rpos(dataCopy.rpos()-sizeof(theByte));
			size_t thePos = dataCopy.rpos();

			//try and parse
			OrderedPacket testPacket;
			bool parseSuccessfull = testPacket.FromBuffer(dataCopy);
			if (parseSuccessfull == true)
			{
				size_t zeroFiveBlockSize = dataCopy.remaining();
				if (zeroFiveBlockSize > 0)
				{
					zeroFiveBlock = ByteBuffer(&dataCopy.contents()[dataCopy.rpos()],zeroFiveBlockSize);
				}
				dataCopy.rpos(thePos);
				zeroFourBlock = ByteBuffer(&dataCopy.contents()[dataCopy.rpos()],dataCopy.remaining()-zeroFiveBlockSize);
				commandOffset=(int32)thePos;

				break;
			}
		}
	}

	ByteBuffer otherBlock;
	if (commandOffset < 0 || zeroFourBlock.size() < 1)
	{
		otherBlock = ByteBuffer(dataCopy.contents(),dataCopy.size());
	}
	else if (commandOffset > 0)
	{
		otherBlock = ByteBuffer(dataCopy.contents(),commandOffset);
	}

	if (zeroFiveBlock.size() > 0)
	{
		HandleOther(zeroFiveBlock);
	}
	if (otherBlock.size() > 0)
	{
		HandleOther(otherBlock);
	}
	if (zeroFourBlock.size() > 0)
	{
		HandleOrdered(zeroFourBlock);
	}
}

void GameClient::HandleOther( ByteBuffer &otherData )
{
	uint8 packetType;
	otherData >> packetType;
	otherData.rpos(otherData.rpos()-sizeof(packetType));

	if (packetType == 0x03)
	{
		if (m_playerGoId == 0)
		{
			WARNING_LOG(format("HandleOther(%1%): 03 received but no player object to handle it") % this->Address() );
			return;
		}
		sObjMgr.getGOPtr(m_playerGoId)->HandleStateUpdate(otherData);
	}
	else
	{
		WARNING_LOG(format("HandleOther(%1%): Received unknown packet type %2%") % this->Address() % Bin2Hex(otherData) );
	}
}

void GameClient::HandleOrdered( ByteBuffer &orderedData )
{
	OrderedPacket bigPacket;
	if (bigPacket.FromBuffer(orderedData) == false)
	{
		ERROR_LOG(format("(%1%) Error creating bigpacket from bytes %2%") % Address() % Bin2Hex(orderedData) );
		return;
	}

	if (bigPacket.msgBlocks.size() < 1)
	{
		ERROR_LOG(format("(%1%) Bigpacket has no msgblocks") % Address());
		return;
	}

	for (list<MsgBlock>::iterator it1=bigPacket.msgBlocks.begin();it1!=bigPacket.msgBlocks.end();++it1)
	{
		uint16 currCmdSequence = it1->sequenceId;
		for (list<ByteBuffer>::iterator it2=it1->subPackets.begin();it2!=it1->subPackets.end();++it2)
		{
			if (std::count(m_clientCommandsReceived.begin(),m_clientCommandsReceived.end(),currCmdSequence) > 0)
			{
				DEBUG_LOG(format("(%1%) Command %2% already exists, ignoring.") % Address() % currCmdSequence);
				m_duplicateCmdsReceived++;
			}
			else
			{
//				DEBUG_LOG(format("(%1%) Parsing command %2%.") % Address() % currCmdSequence);
				m_clientCommandsReceived.push_back(currCmdSequence);

				if (m_playerGoId != 0)
				{
					sObjMgr.getGOPtr(m_playerGoId)->HandleCommand(*it2);
				}
			}
			currCmdSequence++;
		}
	}
}

SequencedPacket GameClient::Decrypt( const char *pData, size_t nLength )
{
	ByteBuffer tempBuf(pData,nLength);
	TwofishEncryptedPacket decryptedData(tempBuf,m_tfEngine);
	return SequencedPacket(decryptedData);
}

void GameClient::SendEncrypted(SequencedPacket withSequences)
{
	if (!m_tfEngine.IsValid())
		return;

	TwofishEncryptedPacket withEncryption(withSequences.getDataWithHeader());
	ByteBuffer sendMe;
	sendMe << uint8(1);
	sendMe.append(withEncryption.toCipherText(m_tfEngine));

	m_sock->SendToBuf(m_address, sendMe.contents(), sendMe.size(), 0);
}

bool GameClient::PacketReceived( uint16 clientSeq )
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
		size_t removedPacketsToAck=0;
		typedef std::pair<uint16,bool> ackedType;
		foreach(ackedType acked,m_recvdPacketSeqs)
			if(!acked.second)removedPacketsToAck++;

		m_recvdPacketSeqs.clear();

		if (removedPacketsToAck)
			INFO_LOG(format("(%1%) Purged %2% unsent acks due to client wraparound") % Address() % removedPacketsToAck);
	}

	if (m_recvdPacketSeqs.count(clientSeq) != 0)
		return false;

	//insert as not acked
	m_recvdPacketSeqs[clientSeq] = false;
	return true;
}



uint32 GameClient::AcknowledgePacket( uint16 serverSeq, uint8 ackBits )
{
	vector<uint16> ackSequences;
	ackSequences.reserve(8);
	//process bits to see which sequences to ack
	{
		for(int i=0;i<MAX_ACKED_PACKETS;i++)
		{
			uint8 mask = 1 << i;
			if (ackBits & mask)
				ackSequences.push_back(serverSeq-i);
		}
	}
/*	printf("serverSeq %d ackBits %02x [",uint32(serverSeq),uint32(ackBits));
	foreach(uint16 seq, ackSequences)
	{
		printf("%d ",seq);
	}
	printf("]\n");*/

	for (stateQueueType::iterator it=m_queuedStates.begin();it!=m_queuedStates.end();)
	{
		if (it->invalidated || it->packetsItsIn.size() < 1)
		{
			++it;
			continue;
		}

		bool remove=false;
		foreach(uint16 ack, ackSequences)
		{
			if (it->packetsItsIn.count(ack) > 0)
			{
				//update latency
				AddtoPingHistory(getMSTime() - it->packetsItsIn[ack]);
				remove=true;
				break;
			}
		}
		if ( remove )
		{
			it->invalidated=true;

			if (!it->callBack.empty())
				it->callBack();

			it=m_queuedStates.begin();
		}
		else
		{
			++it;
		}
	}
	for (sentMsgBlocksType::iterator it=m_sentCommands.begin();it!=m_sentCommands.end();)
	{
		if (it->invalidated || it->packetsItsIn.size() < 1)
		{
			++it;
			continue;
		}

		bool remove=false;
		foreach(uint16 ack, ackSequences)
		{
			if (it->packetsItsIn.count(ack) > 0)
			{
				//update latency
				AddtoPingHistory(getMSTime() - it->packetsItsIn[ack]);
				remove=true;
				break;
			}
		}
		if ( remove )
		{
			it->invalidated=true;

			while(!it->callBacks.empty())
			{
				const packetAckFunc& func = it->callBacks.front();
				if(!func.empty())
					func();

				it->callBacks.pop();
			}

			it=m_sentCommands.begin();
		}
		else
		{
			++it;
		}
	}
	uint32 eraseCounter=0;
	for (stateQueueType::iterator it=m_queuedStates.begin();it!=m_queuedStates.end();)
	{
		if (it->invalidated)
		{
			it=m_queuedStates.erase(it);
			eraseCounter++;
		}
		else
		{
			++it;
		}
	}
	for (sentMsgBlocksType::iterator it=m_sentCommands.begin();it!=m_sentCommands.end();)
	{
		if (it->invalidated)
		{
			it=m_sentCommands.erase(it);
			eraseCounter++;
		}
		else
		{
			++it;
		}
	}
	return eraseCounter;
}



string GameClient::GetNetStats()
{
	format summary = format("Latency: %1%ms GuarQ: %2% UnGuarQ: %3%\n");
	summary % int(m_currentPing) % uint32(m_sentCommands.size()) % uint32(m_queuedStates.size());
	stringstream details;
	if (m_sentCommands.size())
	{
		details << "Guaranteed:\n";
		int cnt=1;
		foreach(const sentMsgBlock& sentCmd, m_sentCommands)
		{
			const MsgBlock& currBlock = sentCmd.commands;
			details << cnt << ". " << currBlock.sequenceId << "-" << currBlock.sequenceId+currBlock.subPackets.size()
				<< "(" << currBlock.GetTotalSize() << "B): ";
			for(map<uint16,uint32>::const_iterator it=sentCmd.packetsItsIn.begin();it!=sentCmd.packetsItsIn.end();++it)
			{
				details << it->first << "(" << getMSTime()-it->second << "ms) ";
			}
			details << "\n";
			cnt++;
		}
	}
	if (m_queuedStates.size())
	{
		details << "Unguaranteed:\n";
		int cnt=1;
		foreach(const queuedState& sentState, m_queuedStates)
		{
			ByteBuffer msgBuf;
			try
			{
				msgBuf = sentState.stateData->toBuf();
			}
			catch(MsgBaseClass::PacketNoLongerValid)
			{
				continue;
			}

			uint16 viewId=0;
			if (msgBuf.read<uint8>() == 3)
			{
				msgBuf >> viewId;
			}

			details << cnt << ". View: " << viewId << "(" << msgBuf.count() << "B): ";
			for(map<uint16,uint32>::const_iterator it=sentState.packetsItsIn.begin();it!=sentState.packetsItsIn.end();++it)
			{
				details << it->first << "(" << getMSTime()-it->second << "ms) ";
			}
			details << "\n";
			cnt++;
		}
	}

	stringstream stats;
	stats << "guarBlkSent: " << m_guarSent << " guarBlkResent: " << m_guarResent << " guarsInvalid: " << m_guarsInvalid << " guarsSkipped: " << m_guarsSkipped << "\n";
	stats << "unGuarSent: " << m_unguarSent << " unGuarResent: " << m_unguarResent << " unGuarsInvalid: " << m_unguarsInvalid << " unGuarsRejected: " << m_unguarRejected << "\n";
	stats << "duplicateCmdsRecvd: " << m_duplicateCmdsReceived << " additionalPcktRqsts: " << m_morePacketRequests << " rawPcktsResent: " << m_rawPacketsResent;

	return	summary.str()+details.str()+stats.str();
}

void GameClient::ResetRCC()
{
	m_serverSequence = 1;
	m_serverCommandsSent = 0;
	m_lastClientSequence = 0;

	m_currentPing = INITIAL_PING;
	m_pingHistory.clear();

	m_sentCommands.clear();
	while (!m_queuedCommands.empty()) m_queuedCommands.pop_back();

	m_guarSent = 0;
	m_guarResent = 0;
	m_guarsInvalid = 0;
	m_guarsSkipped = 0;

	while (!m_queuedStates.empty()) m_queuedStates.pop_back();

	m_unguarSent = 0;
	m_unguarResent = 0;
	m_unguarsInvalid = 0;
	m_duplicateCmdsReceived = 0;
	m_unguarRejected = 0;
	m_morePacketRequests = 0;
	m_rawPacketsResent = 0;

	m_recvdPacketSeqs.clear();
	m_clientCommandsReceived.clear();

	m_lastSimTimeUpdate = -1;
}

uint16 GameClient::SendSequencedPacket( msgBaseClassPtr jumboPacket )
{
	//serialize data from dynamic packet to a static one
	ByteBuffer serializedData;
	try
	{
		serializedData=jumboPacket->toBuf();
	}
	catch (MsgBaseClass::PacketNoLongerValid)
	{
		return 0;
	}

	enum
	{
		SERVERFLAGS_NORMAL = (1 << 1),
		SERVERFLAGS_RESETRCC = (1 << 6),
		SERVERFLAGS_SIMTIME = (1 << 7),
	};

	uint8 serverFlags = SERVERFLAGS_NORMAL;

	if ( m_lastSimTimeUpdate <= 0 || getFloatTime()-m_lastSimTimeUpdate >= 5.0f) //send simtime every 5 seconds
	{
		m_lastSimTimeUpdate = getFloatTime();
		serverFlags |= SERVERFLAGS_SIMTIME;
	}

	ByteBuffer outputData;
	outputData << uint8(serverFlags);
	if (serverFlags &= SERVERFLAGS_SIMTIME)
		outputData << float(sGame.GetSimTime());
	if (serializedData.size() > 0)
		outputData.append(serializedData.contents(),serializedData.size());

	//prepend sequences
	uint16 clientSeq = GetAnAck(); //get sequence number of a packet we havent acked
	uint8 ackBits = GetAckBits(clientSeq);
	uint16 serverSeq = m_serverSequence;
	SequencedPacket prepended(serverSeq,clientSeq,ackBits,outputData);
	SendEncrypted(prepended);
	increaseServerSequence();
	return serverSeq;
}

void GameClient::FlushQueue( bool alsoResend )
{
	//reliable commands first
	{
		uint32 reliableResendMS = min(max((uint32)m_currentPing, MINIMUM_RESEND_TIME)*PING_MULTIPLIER_RELIABLE, MAXIMUM_RESEND_TIME);

		shared_ptr<OrderedPacket> jumboPacket(new OrderedPacket());
		vector<sentMsgBlocksType::iterator> msgBlocksInJumbo;
		if (alsoResend || !m_queuedCommands.empty()) //always resend previous guaranteed msgs before sending new ones
		{
			for(sentMsgBlocksType::iterator it=m_sentCommands.begin();it!=m_sentCommands.end();++it)
			{
				if (it->invalidated)
					continue;

				if( (getMSTime() - it->getLastTimeSent() < reliableResendMS) && m_queuedCommands.empty()) //don't resend like crazy
					continue;

				//if this msgblock wont fit, flush the earlier ones
				if (jumboPacket->GetTotalSize() + it->commands.GetTotalSize() > 1300)
				{
					uint16 jumboServerSeq = SendSequencedPacket(jumboPacket);
					foreach(sentMsgBlocksType::iterator msgBlkIt,msgBlocksInJumbo)
					{
						msgBlkIt->packetsItsIn[jumboServerSeq]=getMSTime();
					}
					jumboPacket.reset(new OrderedPacket());
					msgBlocksInJumbo.clear();
				}

				jumboPacket->msgBlocks.push_back(it->commands);
				msgBlocksInJumbo.push_back(it);

				m_guarResent++;
			}
		}
		MsgBlock currBlock;
		queue<packetAckFunc> currBlockCallbacks;
		while(!m_queuedCommands.empty())
		{
			const queuedMsg& currMsg = m_queuedCommands.front();
			ByteBuffer packetStaticBuf;
			try
			{
				packetStaticBuf = currMsg.data->toBuf();
			}
			catch (MsgBaseClass::PacketNoLongerValid)
			{
				m_guarsInvalid++;
				WARNING_LOG(format("(%1%) Reliable msg %2% no longer valid, this shouldn't happen!") % Address() % currMsg.sequenceId);

				m_queuedCommands.pop_front();
				for(queuedMsgType::iterator iter=m_queuedCommands.begin();iter!=m_queuedCommands.end();++iter)
				{
					iter->sequenceId--;
				}
				continue;
			}

			//no more room to send this msg, finalize block and send packet
			if ( (jumboPacket->GetTotalSize() + currBlock.GetTotalSize() + packetStaticBuf.size()) > 1300)
			{
				if (currBlock.subPackets.size() > 0)
				{
					//add currently formed msgblock to jumbo
					jumboPacket->msgBlocks.push_back(currBlock);

					//add msgblock to sent list
					m_sentCommands.push_back(sentMsgBlock(currBlock,currBlockCallbacks));
					sentMsgBlocksType::iterator newSent = m_sentCommands.end();
					msgBlocksInJumbo.push_back(--newSent);

					//empty msgblock
					currBlock = MsgBlock();
					while(!currBlockCallbacks.empty()) currBlockCallbacks.pop();

					m_guarSent++;
				}
				//send jumbo and update all related msgblocks
				uint16 jumboServerSeq = SendSequencedPacket(jumboPacket);
				foreach(sentMsgBlocksType::iterator msgBlkIt,msgBlocksInJumbo)
				{
					msgBlkIt->packetsItsIn[jumboServerSeq] = getMSTime();
				}

				//empty jumbo
				jumboPacket.reset(new OrderedPacket());
				msgBlocksInJumbo.clear();
			}

			//starting new block
			if (currBlock.subPackets.size() == 0)
			{
				currBlock.sequenceId = currMsg.sequenceId;
			}
			else if (currBlock.sequenceId + currBlock.subPackets.size() != currMsg.sequenceId) //msg ids not continuous ?!
			{
				m_guarsSkipped++;
				WARNING_LOG(format("(%1%) While adding msg %2% skipped some ids (should be %3%), might cause desync !") % Address() % currMsg.sequenceId % (currBlock.sequenceId+(uint32)currBlock.subPackets.size()) );

				if (currBlock.subPackets.size() > 0)
				{
					//add currently formed msgblock to jumbo
					jumboPacket->msgBlocks.push_back(currBlock);

					//add msgblock to sent list
					m_sentCommands.push_back(sentMsgBlock(currBlock,currBlockCallbacks));
					sentMsgBlocksType::iterator newSent = m_sentCommands.end();
					msgBlocksInJumbo.push_back(--newSent);
				}
				//empty msgblock
				currBlock = MsgBlock();
				while(!currBlockCallbacks.empty()) currBlockCallbacks.pop();

				m_guarSent++;

				//go back to top of loop to check size again
				continue;
			}

			currBlock.subPackets.push_back(packetStaticBuf);
			currBlockCallbacks.push(currMsg.callBack);

			m_queuedCommands.pop_front();
		}
		if (currBlock.subPackets.size() > 0)
		{
			//add currently formed msgblock to jumbo
			jumboPacket->msgBlocks.push_back(currBlock);

			//add msgblock to sent list
			m_sentCommands.push_back(sentMsgBlock(currBlock,currBlockCallbacks));
			sentMsgBlocksType::iterator newSent = m_sentCommands.end();
			msgBlocksInJumbo.push_back(--newSent);

			//empty msgblock
			currBlock = MsgBlock();
			while(!currBlockCallbacks.empty()) currBlockCallbacks.pop();

			m_guarSent++;
		}
		if (jumboPacket->msgBlocks.size() > 0)
		{
			//send jumbo and update all related msgblocks
			uint16 jumboServerSeq = SendSequencedPacket(jumboPacket);
			foreach(sentMsgBlocksType::iterator msgBlkIt,msgBlocksInJumbo)
			{
				msgBlkIt->packetsItsIn[jumboServerSeq] = getMSTime();
			}

			//empty jumbo
			jumboPacket.reset(new OrderedPacket());
			msgBlocksInJumbo.clear();
		}

		assert(m_queuedCommands.empty());
	}

	//03 next
	uint32 unreliableResendMS = min(max((uint32)m_currentPing, MINIMUM_RESEND_TIME)*PING_MULTIPLIER_UNRELIABLE, MAXIMUM_RESEND_TIME);
	for (stateQueueType::iterator it=m_queuedStates.begin();it!=m_queuedStates.end();)
	{
		if ((getMSTime() - it->getLastTimeSent() < 500 || alsoResend == false) && it->packetsItsIn.size() > 0) //500ms resend
		{
			++it;
			continue;
		}
		//see if packet is still valid, if not, erase and carry on
		{
			ByteBuffer serializedData;
			try
			{
				serializedData=it->stateData->toBuf();
			}
			catch (MsgBaseClass::PacketNoLongerValid)
			{
				m_unguarsInvalid++;

				it=m_queuedStates.erase(it);
				continue;
			}

			if (it->packetsItsIn.size() > 20)
			{
				m_unguarRejected++;
				DEBUG_LOG(format("(%1%) Doesn't want packet %2% of size %3%") % Address() % it->packetsItsIn.begin()->first % serializedData.size());
				it=m_queuedStates.erase(it);
				continue;
			}
		}

		if (it->packetsItsIn.size() > 0)
		{
			m_unguarResent++;
		}
		else
		{
			m_unguarSent++;
		}

		uint16 serverSeq = SendSequencedPacket(it->stateData);
		it->packetsItsIn[serverSeq] = getMSTime();
		if (it->noResend==true)
			it = m_queuedStates.erase(it);
		else
			++it;
	}

	//we ran out of packets but there are still more acks to be sent ?
	size_t numUnacked=0;
	typedef std::pair<uint16,bool> ackedType;
	foreach(ackedType acked,m_recvdPacketSeqs)
	{
		if (!acked.second)
			numUnacked++;
	}

	for(size_t i=0;i<numUnacked;i++)
		SendSequencedPacket(make_shared<EmptyMsg>());
}


void GameClient::CheckAndResend()
{
	FlushQueue(true);
}