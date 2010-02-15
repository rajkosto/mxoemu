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

#include "Common.h"

#include "GameClient.h"
#include "Timer.h"
#include "Util.h"
#include "MersenneTwister.h"
#include "EncryptedPacket.h"
#include "SequencedPacket.h"
#include "RsiData.h"
#include "MarginSocket.h"
#include "MarginServer.h"
#include "Database/Database.h"
#include "Log.h"
#include "GameServer.h"
#include "GameSocket.h"

#pragma pack(1)

GameClient::GameClient(shared_ptr<SocketAddress> address, GameSocket *sock)
{
	m_sock = sock;
	m_address = address;
	m_serverSequence = 0;
	m_serverCommandsSent = 0;
	m_clientCommandsReceived = 0;
	m_numPackets = 0;
	m_clientPSS = 0;
	m_lastClientSequence = 0;
	m_validClient = true;
	m_worldLoaded = false;
	m_characterSpawned = false;
	m_encryptionInitialized = false;
	m_lastSimTimeMS = 0;
	m_lastOrderedFlush = 0;
	m_playerGoId=0;
}

GameClient::~GameClient()
{
	if (m_playerGoId != 0)
	{
		sObjMgr.deallocatePlayer(m_playerGoId);
	}
	sObjMgr.clientSigningOff(this);

	MarginSocket *marginConn = sMargin.GetSocketByCharacterUID(m_characterUID);
	if (marginConn != NULL)
	{
		marginConn->ForceDisconnect();
	}
}

void GameClient::HandlePacket( const char *pData, uint16 nLength )
{
	if (nLength < 1 || m_validClient == false)
		return;

	m_lastActivity = getTime();
	m_lastPacketReceivedMS = getMSTime();
	m_numPackets++;

	if (m_encryptionInitialized == false && pData[0] == 0 && nLength == 43)
	{
		ByteBuffer packetData;
		packetData.append(pData,nLength);
		packetData.rpos(0x0B);
		if (packetData.remaining() < sizeof(m_characterUID))
		{
			m_validClient = false;
			return;
		}
		packetData >> m_characterUID;
		try
		{
			m_playerGoId = sObjMgr.allocatePlayer(this,m_characterUID);
		}
		catch (ObjectMgr::ObjectNotAvailable)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Character doesn't exist") % Address() );
			m_validClient = false;
			return;
		}

		MarginSocket *marginConn = sMargin.GetSocketByCharacterUID(m_characterUID);
		if (marginConn == NULL)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Margin session not found") % Address() );
			m_validClient = false;
			return;
		}
		m_sessionId = marginConn->GetSessionId();
		m_charWorldId = marginConn->GetWorldCharId();

		//initialize encryptors with key from margin
		vector<byte> twofishKey = marginConn->GetTwofishKey();
		vector<byte> twofishIV(CryptoPP::Twofish::BLOCKSIZE,0);

		m_TFDecrypt.reset(new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(&twofishKey[0], twofishKey.size(), &twofishIV[0]));
		m_TFEncrypt.reset(new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption(&twofishKey[0], twofishKey.size(), &twofishIV[0]));

		//now we can verify if session key in this packet is correct
		packetData.rpos(packetData.size()-CryptoPP::Twofish::BLOCKSIZE);
		if (packetData.remaining() < CryptoPP::Twofish::BLOCKSIZE)
		{
			//wat
			m_validClient=false;
			marginConn->ForceDisconnect();
			return;
		}
		vector<byte> encryptedSessionId(packetData.remaining());
		packetData.read(&encryptedSessionId[0],encryptedSessionId.size());
		string decryptedOutput;
		CryptoPP::StringSource(string( (const char*)&encryptedSessionId[0],encryptedSessionId.size() ), true, 
			new CryptoPP::StreamTransformationFilter(
			*m_TFDecrypt, new CryptoPP::StringSink(decryptedOutput),
			CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
		ByteBuffer decryptedData;
		decryptedData.append(decryptedOutput.data(),decryptedOutput.size());
		uint32 recoveredSessionId=0;
		decryptedData >> recoveredSessionId;

		if (recoveredSessionId != m_sessionId)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Session Key Mismatch") % Address() );
			m_validClient = false;
			marginConn->ForceDisconnect();
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

			m_sock->SendToBuf(*m_address, beatPacket.contents(), beatPacket.size(), 0);
		}

		//notify margin that udp session is established
		if (marginConn->UdpReady(this) == false)
		{
			ERROR_LOG(format("InitialUDPPacket(%1%): Margin not ready for UDP connection") % Address() );
			m_encryptionInitialized=false;
			m_validClient = false;
			marginConn->ForceDisconnect();
			return;
		}
		sObjMgr.getGOPtr(m_playerGoId)->InitializeWorld();
		FlushQueue();
	}

	if (m_worldLoaded == true && pData[0] != 0x01) // Ping...just reply with the same thing
	{
		m_sock->SendToBuf(*m_address, pData, nLength, 0);
	}
	else
	{
		if (pData[0] == 0x01 && m_encryptionInitialized==true)
		{
			SequencedPacket packetData=Decrypt(&pData[1],nLength-1);
			ByteBuffer dataToParse;

			uint32 pktsAcked = AcknowledgePacket(packetData.getRemoteSeq());
			assert(pktsAcked <= 1);

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

			//DEBUG_LOG( format("(%s) Recv PSS: %x CSeq: %d SSeq: %d |%s|") % Address() % uint32(packetData.getPSS()) % packetData.getLocalSeq() % packetData.getRemoteSeq() % Bin2Hex(dataToParse) );

			//add to need to ack list
			PacketReceived(packetData.getLocalSeq());						
			if (m_clientPSS != packetData.getPSS())
			{
				PSSChanged(m_clientPSS,packetData.getPSS());
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

	//try to find 04 block
	while (dataCopy.remaining() > 0)
	{
		uint8 theByte;
		dataCopy >> theByte;
		if (theByte == 0x04)
		{
			//reverse read of byte
			dataCopy.rpos(dataCopy.rpos()-sizeof(theByte));
			int32 thePos = dataCopy.rpos();

			//try and parse
			OrderedPacket testPacket;
			bool parseSuccessfull = testPacket.FromBuffer(dataCopy);
			if (parseSuccessfull == true && dataCopy.remaining() == 0)
			{
				dataCopy.rpos(thePos);
				zeroFourBlock = ByteBuffer(&dataCopy.contents()[dataCopy.rpos()],dataCopy.remaining());
				commandOffset=thePos;
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

bool msgblock_sequence_lessthan(MsgBlock s2, MsgBlock s1)
{
	return	( (s1.sequenceId > s2.sequenceId) && (s1.sequenceId-s2.sequenceId <= 65536/2) ) ||
			( (s2.sequenceId > s1.sequenceId) && (s2.sequenceId-s1.sequenceId > 65536/2) );
}

bool msgblock_sequence_greaterthan(MsgBlock s1, MsgBlock s2)
{
	return	( (s1.sequenceId > s2.sequenceId) && (s1.sequenceId-s2.sequenceId <= 65536/2) ) ||
		( (s2.sequenceId > s1.sequenceId) && (s2.sequenceId-s1.sequenceId > 65536/2) );
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

	if (bigPacket.msgBlocks.size() > 1)
	{
		bigPacket.msgBlocks.sort(msgblock_sequence_lessthan);
	}

	for (list<MsgBlock>::iterator it1=bigPacket.msgBlocks.begin();it1!=bigPacket.msgBlocks.end();++it1)
	{
		DEBUG_LOG( format("(%s) 04 client order: %d, our order %d") % Address() % it1->sequenceId % m_clientCommandsReceived);
		if ( isSequenceMoreRecent(it1->sequenceId,m_clientCommandsReceived,65536) == true || it1->sequenceId == m_clientCommandsReceived)
		{
			for (list<ByteBuffer>::iterator it2=it1->subPackets.begin();it2!=it1->subPackets.end();++it2)
			{
				if (m_playerGoId != 0)
				{
					sObjMgr.getGOPtr(m_playerGoId)->HandleCommand(*it2);
					m_clientCommandsReceived++;
				}
			}
		}
		else
		{
			WARNING_LOG(format("(%1%) Client order %2% smaller than server order %3%") % Address() % it1->sequenceId % m_clientCommandsReceived);
		}
	}
}

SequencedPacket GameClient::Decrypt(const char *pData, uint16 nLength)
{
	EncryptedPacket decryptedData(ByteBuffer(pData,nLength),m_TFDecrypt.get());
	return SequencedPacket(decryptedData);
}

void GameClient::SendEncrypted(SequencedPacket withSequences)
{
	if (!m_TFEncrypt)
		return;

	EncryptedPacket withEncryption(withSequences.getDataWithHeader());
	ByteBuffer sendMe;
	sendMe << uint8(1);
	sendMe.append(withEncryption.toCipherText(m_TFEncrypt.get()));

	m_sock->SendToBuf(*m_address, sendMe.contents(), sendMe.size(), 0);
}

void GameClient::PSSChanged( uint8 oldPSS,uint8 newPSS )
{
	m_clientPSS = newPSS;
	if (m_clientPSS == 0x01)
	{
		//skip straight to 0x07
		m_clientPSS=0x07;	
		if (m_worldLoaded==false)
		{
			sObjMgr.getGOPtr(m_playerGoId)->SpawnSelf();
			m_worldLoaded = true;
		}
	}
	else if (m_clientPSS == 0x07)
	{
		if (m_worldLoaded==false)
		{
			sObjMgr.getGOPtr(m_playerGoId)->SpawnSelf();
			m_worldLoaded = true;
		}
	}
	else if (m_clientPSS == 0x7F)
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
}

void GameClient::MoveMsgsToQueue()
{
	//first we send out all the 03
	for (stateQueueType::iterator it=m_queuedStates.begin();it!=m_queuedStates.end();++it)
	{
		//consume an ack if we can
		if (m_packetsToAck.size() > 0)
		{			
			uint16 theClientSeq = m_packetsToAck.front();
			m_packetsToAck.pop_front();

			AddPacketToQueue(theClientSeq,true,it->stateData,it->noResend);
		}
		else
		{
			AddPacketToQueue(it->stateData,it->noResend);
		}
	}
	//all queued 03s have been transferred to packet queue, clear the 03 queue
	m_queuedStates.clear();
	
	if (/*getMSTime() - m_lastOrderedFlush > 50*/1) //we will not send 04 packets more than 20 times a second
	{
		//send out subpackets, as much as we got, consuming as much acks as we can
		while (m_queuedCommands.size() > 0)
		{
			MsgBlock currBlock;
			currBlock.sequenceId = m_serverCommandsSent;

			while (m_queuedCommands.size() > 0)
			{
				ByteBuffer packetStaticBuf;
				try
				{
					packetStaticBuf = m_queuedCommands.front()->toBuf();
				}
				catch (MsgBaseClass::PacketNoLongerValid)
				{
					//just remove the command and go to the next one
					m_queuedCommands.front().reset();
					m_queuedCommands.pop_front();
				}

				currBlock.subPackets.push_back(packetStaticBuf);
				m_serverCommandsSent++;

				m_queuedCommands.front().reset();
				m_queuedCommands.pop_front();

				if (m_queuedCommands.size() < 1 || 
					(currBlock.GetTotalSize() + packetStaticBuf.size() >= 1000) )
				{
					break;
				}
			}

			//consume an ack if we can
			if (m_packetsToAck.size() > 0)
			{
				uint16 theClientSeq = m_packetsToAck.front();
				m_packetsToAck.pop_front();

				AddPacketToQueue(theClientSeq,true,make_shared<OrderedPacket>(currBlock));
			}
			else
			{
				AddPacketToQueue(make_shared<OrderedPacket>(currBlock));
			}
		}
		m_lastOrderedFlush = getMSTime();
	}	

	//we ran out of packets but there are still more acks to be sent ?
	if (m_packetsToAck.size() > 0)
	{
		for (deque<uint16>::iterator it=m_packetsToAck.begin();it!=m_packetsToAck.end();++it)
		{
			AddPacketToQueue(*it,true,make_shared<EmptyMsg>());
		}
		m_packetsToAck.clear();
	}
}

void GameClient::FlushQueue()
{
	MoveMsgsToQueue();

	for(sendQueueList::iterator it=m_sendQueue.begin();it!=m_sendQueue.end();)
	{
		//skip sent packets
		if (it->sent==true)
		{
			++it;
			continue;
		}

		//serialize data from dynamic packet to a static one
		ByteBuffer serializedData;
		try
		{
			serializedData=it->theData->toBuf();
		}
		catch (MsgBaseClass::PacketNoLongerValid)
		{
			serializedData.clear();
			//remove data from packet in queue, this will leave it to still have an ack if needed
			it->theData.reset(new EmptyMsg());
		}

		ByteBuffer outputData;
		//we send simtime synchronization only in the first packet (anything else seems to break the game)
		uint32 currTimeMS = getMSTime();
		if ( /*(*/m_lastSimTimeMS==0 /*|| currTimeMS-m_lastSimTimeMS>1000) 
			&& serializedData.size() > 0
			&& dynamic_pointer_cast<OrderedPacket>(it->theData) != NULL*/)
		{
			m_lastSimTimeMS = getMSTime();
			outputData << uint8(0x82);
			//theirs ticks at 64hz, ours ticks at 1000
			double preciseInterval = double(m_lastSimTimeMS)/double(1000/64);
			uint32 simTimeToSend = uint32(preciseInterval);
			outputData << uint32(simTimeToSend);
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
		else if (it->ack == false)
		{
			//if no ack, and data is 0, no reason to send this packet, so remove from queue and carry on
			it=m_sendQueue.erase(it);
			continue;
		}

		//prepend sequences
		SequencedPacket prepended(it->server_sequence,it->client_sequence,it->clientPSS,outputData);
		SendEncrypted(prepended);

//		DEBUG_LOG(format("(%s) Flush PSS: %x SSeq: %d CSeq: %d |%s|") % Address() % uint32(prepended.getPSS()) % prepended.getLocalSeq() % prepended.getRemoteSeq() % Bin2Hex(prepended));

		//mark packet as sent
		it->sent=true;
		it->msTimeSent=getMSTime();
		//if packet is non resendable, remove it
		if (it->noResends==true)
			it=m_sendQueue.erase(it);
		else
			++it;
	}
}

void GameClient::CheckAndResend()
{
	bool modified = false;
	for(sendQueueList::iterator it=m_sendQueue.begin();it!=m_sendQueue.end();)
	{
		//skip unsent packets
		if (it->sent == false)
		{
			++it;
			continue;
		}
		bool deleted = false;

		uint32 currTime = getMSTime();
		if (currTime - it->msTimeSent > 200) //200 is timeout for resend
		{
			//client obviously doesnt want to ack this packet
			if (it->resentCounter > 10)
			{
				ByteBuffer resentPacketDumpedData;
				try
				{
					resentPacketDumpedData = it->theData->toBuf();
				}
				catch (MsgBaseClass::PacketNoLongerValid)
				{
					INFO_LOG(format("(%1%) Resent packet %2%'s data is no longer valid") % Address() % it->server_sequence);
				}

//				INFO_LOG(format("(%1%) Doesn't want packet %2% of size %3%") % Address() % it->server_sequence % resentPacketDumpedData.size());

				if (resentPacketDumpedData.size() > 1000)
				{
					WARNING_LOG(format("(%1%) WTF, WHY IS THIS SO BIG, %2%") % Address() % Bin2Hex(resentPacketDumpedData) );
				}

				it=m_sendQueue.erase(it);
				deleted=true;
			}
			else
			{
				bool subdivided=false;
				//is it a 04 packet ?
				shared_ptr<OrderedPacket> daPacket = dynamic_pointer_cast<OrderedPacket>(it->theData);
				if (daPacket != NULL && daPacket->msgBlocks.size() == 1)
				{
					DEBUG_LOG(format("(%1%) Packet %2% is a 04 packet, with %3% msgblocks %4% msgs(1)") % Address() % it->server_sequence % daPacket->msgBlocks.size() % daPacket->msgBlocks.front().subPackets.size());
					shared_ptr<OrderedPacket> newPacket(new OrderedPacket());
					for (list<MsgBlock>::iterator blockPtr=daPacket->msgBlocks.begin();blockPtr!=daPacket->msgBlocks.end();++blockPtr)
					{
						MsgBlock currBlock = *blockPtr;
						//send the same subpackets only in smaller chunks
						uint16 altCommandsSent = currBlock.sequenceId;
						if (currBlock.subPackets.size() > 4)
						{
							int numAtATime = 2;
							if (currBlock.subPackets.size() >= 12)
							{
								numAtATime = 4;
							}
							else if (currBlock.subPackets.size() >= 6)
							{
								numAtATime = 3;
							}

							for (;;)
							{
								MsgBlock newBlock;
								newBlock.sequenceId = altCommandsSent;
								for (int i=0;i<numAtATime;i++)
								{
									if (currBlock.subPackets.size() < 1)
										break;

									newBlock.subPackets.push_back(currBlock.subPackets.front());
									altCommandsSent++;
									currBlock.subPackets.pop_front();
								}

								if (newBlock.subPackets.size() < 1)
									break;

								newPacket->msgBlocks.push_back(newBlock);
							}
						}
					}
					if (newPacket->msgBlocks.size() > 1)
					{
						uint16 oldServerSeq = it->server_sequence;
						uint16 oldClientSeq = it->client_sequence;
						uint8 oldClientPSS = it->clientPSS;
						bool oldAck = it->ack;

						it = m_sendQueue.erase(it);
						deleted=true;

						INFO_LOG(format("(%1%) Subdivided packet sseq %2% from 1 msgblocks to %3% msgblocks") % Address() % oldServerSeq % newPacket->msgBlocks.size());
						//reverse iterator because push_front inserts into reverse anyway
						for (list<MsgBlock>::reverse_iterator hurr=newPacket->msgBlocks.rbegin();hurr!=newPacket->msgBlocks.rend();++hurr)
						{
							uint16 oldSeqz = m_serverSequence;
							increaseServerSequence();
							m_sendQueue.push_front( PacketInQueue(oldClientPSS,m_serverSequence,oldClientSeq,oldAck,shared_ptr<OrderedPacket>(new OrderedPacket(*hurr))) );
						}
						subdivided=true;
					}
				}
				
				if (subdivided==false)
				{
					it->resentCounter++;
					it->sent = false;
					it->msTimeSent=0;
					uint16 oldSeq = it->server_sequence;
					increaseServerSequence();
					it->server_sequence = m_serverSequence;
				}

				//DEBUG_LOG(format("(%1%) Resending packet sseq %2% under new sseq %3%") % Address() % oldSeq % it->server_sequence);
				modified = true;
			}
		}

		if (deleted==false)
		{
			++it;
		}
	}
	if (modified == true)
	{
		FlushQueue();
	}
}