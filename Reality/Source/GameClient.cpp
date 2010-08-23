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
	m_serverSequence = 1;
	m_serverCommandsSent = 0;
	m_clientFlags = 0;
	m_lastClientSequence = 0;
	m_validClient = true;
	m_worldLoaded = false;
	m_characterSpawned = false;
	m_encryptionInitialized = false;
	m_lastSimTimeMS = 0;
	m_playerGoId=0;
	m_calculatedInitialLatency = false;
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
	m_lastPacketReceivedMS = getMSTime32();

	if (m_encryptionInitialized == false && pData[0] == 0 && nLength == 43)
	{
		m_lastServerMS = getMSTime32();

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
	}

	if (m_worldLoaded == true && pData[0] != 0x01) // Ping...just reply with the same thing
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

			if (m_calculatedInitialLatency == false)
			{
				uint64 clientLatency = getMSTime32()-m_lastServerMS;
				DEBUG_LOG(format("CLIENT LATENCY: %1%")%clientLatency);
				m_calculatedInitialLatency=true;
			}

			uint32 pktsAcked = AcknowledgePacket(packetData.getRemoteSeq());

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
			if (m_clientFlags != packetData.getFlags())
			{
				FlagsChanged(m_clientFlags,packetData.getFlags());
			}

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
			if (m_clientCommandsReceived.find(currCmdSequence) != m_clientCommandsReceived.end())
			{
				ByteBuffer &existingPacket = m_clientCommandsReceived[currCmdSequence];
				if (it2->size() != existingPacket.size() || memcmp(it2->contents(),existingPacket.contents(),existingPacket.size()) != 0)
				{
					DEBUG_LOG(format("(%1%) Command %2% already exists, but with different contents, reprocessing.") % Address() % currCmdSequence);
					m_clientCommandsReceived[currCmdSequence] = *it2;

					if (m_playerGoId != 0)
					{
						sObjMgr.getGOPtr(m_playerGoId)->HandleCommand(*it2);
					}
				}
				else
				{
					DEBUG_LOG(format("(%1%) Command %2% already exists, with identical contents, ignoring.") % Address() % currCmdSequence);
				}
			}
			else
			{
//				DEBUG_LOG(format("(%1%) Parsing command %2%.") % Address() % currCmdSequence);
				m_clientCommandsReceived[currCmdSequence] = *it2;

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

void GameClient::FlagsChanged( uint8 oldFlags,uint8 newFlags )
{
//	DEBUG_LOG(format("flags changed from %02x to %02x")%uint32(oldFlags)%uint32(newFlags));

	m_clientFlags = newFlags;
	if (m_clientFlags == 0x01)
	{
		//skip straight to 0x07
		m_clientFlags=0x07;	
		if (m_worldLoaded==false)
		{
			sObjMgr.getGOPtr(m_playerGoId)->SpawnSelf();
			m_worldLoaded = true;
		}
	}
	else if (m_clientFlags == 0x07)
	{
		if (m_worldLoaded==false)
		{
			sObjMgr.getGOPtr(m_playerGoId)->SpawnSelf();
			m_worldLoaded = true;
		}
	}
	else if (m_clientFlags == 0x7F)
	{
		if (m_worldLoaded==false)
		{
			sObjMgr.getGOPtr(m_playerGoId)->SpawnSelf();
			m_worldLoaded = true;
		}
		if (m_characterSpawned == false)
		{
			sObjMgr.getGOPtr(m_playerGoId)->PopulateWorld();
			m_characterSpawned = true;
		}
	}
	else
	{   //break on nonhandled
		ERROR_LOG(format("Unhandled Client Flag: 0x%02x") % uint32(m_clientFlags));
	}

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
		size_t removedPacketsToAck = m_packetsToAck.size();
		m_packetsToAck.clear();

		INFO_LOG(format("(%1%) Purged %2% acks due to client wraparound") % Address() % removedPacketsToAck);
	}

	if (find(m_packetsToAck.begin(),m_packetsToAck.end(),clientSeq) != m_packetsToAck.end())
		return false;

	m_packetsToAck.push_back(clientSeq);
	return true;
}



uint32 GameClient::AcknowledgePacket( uint16 serverSeq )
{
	//		vector<uint16> zeAckedPacketz;
	uint32 eraseCounter=0;
	for (stateQueueType::iterator it=m_queuedStates.begin();it!=m_queuedStates.end();)
	{
		if ( (it->packetsItsIn.size() > 0) &&
			find(it->packetsItsIn.begin(),it->packetsItsIn.end(),serverSeq)!=it->packetsItsIn.end() )
		{
			/*for (int i=0;i<it->packetsItsIn.size();i++)
			{
				zeAckedPacketz.push_back(it->packetsItsIn[i]);
			}*/
			it=m_queuedStates.erase(it);
			eraseCounter++;
		}
		else
		{
			++it;
		}
	}
	for (msgQueueType::iterator it=m_queuedCommands.begin();it!=m_queuedCommands.end();)
	{
		if ( (it->packetsItsIn.size() > 0) &&
			find(it->packetsItsIn.begin(),it->packetsItsIn.end(),serverSeq)!=it->packetsItsIn.end() )
		{
			/*for (int i=0;i<it->packetsItsIn.size();i++)
			{
				zeAckedPacketz.push_back(it->packetsItsIn[i]);
			}*/
			it=m_queuedCommands.erase(it);
			eraseCounter++;
		}
		else
		{
			++it;
		}
	}
	/*stringstream derp;
	derp << "Acking " << serverSeq << " Acked " << eraseCounter << " packets latency " << m_latency << "ms ( ";
	for (int i=0;i<zeAckedPacketz.size();i++)
	{
		derp << zeAckedPacketz[i] << " ";
	}
	vector<uint16> zePacketsLeft;
	for (msgQueueType::iterator it=m_queuedCommands.begin();it!=m_queuedCommands.end();++it)
	{
		for (int i=0;i<it->packetsItsIn.size();i++)
		{
			zePacketsLeft.push_back(it->packetsItsIn[i]);
		}
	}
	for (stateQueueType::iterator it=m_queuedStates.begin();it!=m_queuedStates.end();++it)
	{
		for (int i=0;i<it->packetsItsIn.size();i++)
		{
			zePacketsLeft.push_back(it->packetsItsIn[i]);
		}
	}
	derp << ") " << zePacketsLeft.size() << " left ( ";
	for (int i=0;i<zePacketsLeft.size();i++)
	{
		derp << zePacketsLeft[i] << " ";
	}
	derp << ")";
	DEBUG_LOG(derp.str());*/
	return eraseCounter;
}

bool GameClient::SendSequencedPacket( msgBaseClassPtr jumboPacket )
{
	//serialize data from dynamic packet to a static one
	ByteBuffer serializedData;
	try
	{
		serializedData=jumboPacket->toBuf();
	}
	catch (MsgBaseClass::PacketNoLongerValid)
	{
		return false;
	}

	ByteBuffer outputData;
	//we send simtime synchronization only in the first packet (anything else seems to break the game)
	uint32 currTimeMS = getMSTime32();
	if ( /*currTimeMS-m_lastSimTimeMS>1000 && serializedData.size() > 0*/ m_lastSimTimeMS==0)
	{
		m_lastSimTimeMS = getMSTime32();
		outputData << uint8(0x82);
		//theirs ticks at 64hz, ours ticks at 1000
/*		double preciseInterval = double(m_lastSimTimeMS)/double(1000/64);
		uint32 simTimeToSend = uint32(preciseInterval);
		outputData << uint32(simTimeToSend);*/
		outputData << uint32(sGame.GetSimTime());
	}
	else
	{
		//nosimtime byte
		outputData << uint8(0x02);
	}

	//append that to our simtime or nosimtime byte
	if (serializedData.size() > 0)
	{
		outputData.append(serializedData.contents(),serializedData.size());
	}

	//prepend sequences
	SequencedPacket prepended(m_serverSequence,GetAnAck(m_serverSequence),m_clientFlags,outputData);
	SendEncrypted(prepended);
	increaseServerSequence();
	return true;
}

void GameClient::FlushQueue( bool alsoResend )
{
	//reliable commands first
	{
		shared_ptr<OrderedPacket> jumboPacket(new OrderedPacket());
		MsgBlock currBlock;
		sort(m_queuedCommands.begin(),m_queuedCommands.end());
		for (msgQueueType::iterator it=m_queuedCommands.begin();it!=m_queuedCommands.end();)
		{
			if (it->packetsItsIn.size() > 0 && (currBlock.subPackets.size() == 0 || currBlock.sequenceId+currBlock.subPackets.size() != it->sequenceId))
			{
				if (getMSTime32() - it->msLastSent < 200 || alsoResend == false) //200ms resend
				{
					++it;
					continue;
				}
			}

			ByteBuffer packetStaticBuf;
			try
			{
				packetStaticBuf = it->theData->toBuf();
			}
			catch (MsgBaseClass::PacketNoLongerValid)
			{
				it=m_queuedCommands.erase(it);
				continue;
			}

			if (jumboPacket->GetTotalSize() + packetStaticBuf.size() > 1000)
			{
				SendSequencedPacket(jumboPacket);
				jumboPacket.reset(new OrderedPacket());
				continue;
			}

			if (currBlock.subPackets.size() == 0)
			{
				currBlock.sequenceId = it->sequenceId;
			}
			else if (currBlock.sequenceId + currBlock.subPackets.size() != it->sequenceId)
			{
				jumboPacket->msgBlocks.push_back(currBlock);
				currBlock = MsgBlock();
				currBlock.sequenceId = it->sequenceId;
			}

			it->packetsItsIn.push_back(m_serverSequence);
			it->msLastSent = getMSTime32();
			currBlock.subPackets.push_back(packetStaticBuf);

			++it;
		}
		if (currBlock.subPackets.size() > 0)
		{
			jumboPacket->msgBlocks.push_back(currBlock);
			currBlock = MsgBlock();
		}
		if (jumboPacket->msgBlocks.size() > 0)
		{
			SendSequencedPacket(jumboPacket);
			jumboPacket.reset(new OrderedPacket());
		}
	}

	//03 next
	for (stateQueueType::iterator it=m_queuedStates.begin();it!=m_queuedStates.end();)
	{
		if ((getMSTime32() - it->msLastSent < 500 || alsoResend == false) && it->packetsItsIn.size() > 0) //500ms resend
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
				it=m_queuedStates.erase(it);
				continue;
			}

			if (it->packetsItsIn.size() > 10)
			{
				DEBUG_LOG(format("(%1%) Doesn't want packet %2% of size %3%") % Address() % it->packetsItsIn.front() % serializedData.size());
				it=m_queuedStates.erase(it);
				continue;
			}
		}
		it->packetsItsIn.push_back(m_serverSequence);
		it->msLastSent=getMSTime32();
		SendSequencedPacket(it->stateData);
		if (it->noResend==true)
			it = m_queuedStates.erase(it);
		else
			++it;
	}

	//we ran out of packets but there are still more acks to be sent ?
	while(m_packetsToAck.size() > 0)
	{
		SendSequencedPacket(make_shared<EmptyMsg>());
	}
}


void GameClient::CheckAndResend()
{
	FlushQueue(true);
}