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

#ifndef MXOSIM_MESSAGETYPES_H
#define MXOSIM_MESSAGETYPES_H

#include "Common.h"
#include "ByteBuffer.h"
#include "Util.h"
#include "Crypto.h"

class MsgBaseClass
{
public:
	class PacketNoLongerValid {};

	MsgBaseClass() {m_buf.clear();}
	virtual ~MsgBaseClass() {}
	virtual const ByteBuffer& toBuf() = 0;
protected:
	ByteBuffer m_buf;
};

typedef shared_ptr<MsgBaseClass> msgBaseClassPtr;

class ObjectUpdateMsg : public MsgBaseClass
{
public:
	ObjectUpdateMsg(uint32 objectId);
	~ObjectUpdateMsg();
	virtual void setReceiver(class GameClient *toWho);
protected:
	uint32 m_objectId;
	class GameClient *m_toWho;
};

class DeletePlayerMsg : public ObjectUpdateMsg
{
public:
	DeletePlayerMsg(uint32 objectId);
	~DeletePlayerMsg();
	const ByteBuffer& toBuf();
	void setReceiver(class GameClient *toWho);
};


class PlayerSpawnMsg : public ObjectUpdateMsg
{
public:
	PlayerSpawnMsg(uint32 objectId);
	~PlayerSpawnMsg();
	const ByteBuffer& toBuf();
};

class StateUpdateMsg : public ObjectUpdateMsg
{
public:
	StateUpdateMsg(uint32 objectId, ByteBuffer stateData);
	~StateUpdateMsg();
	const ByteBuffer& toBuf();
private:
	ByteBuffer restOfData;
};

class StaticMsg : public MsgBaseClass
{
public:
	StaticMsg() {}
	StaticMsg(const ByteBuffer &inputBuf) {m_buf = inputBuf;}
	~StaticMsg() {}
	const ByteBuffer& toBuf() {return m_buf;}
};

class EmptyMsg : public StaticMsg
{
public:
	EmptyMsg() {m_buf.clear();}
	~EmptyMsg() {}
};

class SystemChatMsg : public StaticMsg
{
public:
	SystemChatMsg(string theChatMsg) 
	{
		m_buf.clear();
		//2E0700000000000000000024000000000000000000000000000000000000000000000000 DATALEN(UINT16) ZEROSTR
		const byte headerData[] =
		{
			0x2E, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00
		} ;

		m_buf.append(headerData,sizeof(headerData));
		uint16 dataSize = theChatMsg.length()+1;
		m_buf << uint16(dataSize);
		m_buf.append(theChatMsg.c_str(),dataSize);
	}
	~SystemChatMsg() {}
};

class PlayerChatMsg : public StaticMsg
{
public:
	PlayerChatMsg(string charHandle,string theChatMsg) 
	{
		m_buf.clear();
		
		m_buf << swap16(0x2E10);
		m_buf << uint8(0); //could be 1 as well ?
		m_buf << uint32(swap32(0x12610200)); //some id, different, ill just use something
		size_t injectHandleLenPosHere = m_buf.wpos();
		uint32 handleLenPos=0;
		m_buf << handleLenPos; //will come back here and overwrite later
		size_t injectMessageLenPosHere = m_buf.wpos();
		uint32 messageLenPos=0;
		m_buf << messageLenPos;
		m_buf.wpos(m_buf.wpos()+0x15);
		handleLenPos=m_buf.wpos();
		uint16 handleLen=charHandle.length()+1;
		m_buf << handleLen;
		m_buf.append(charHandle.c_str(),handleLen);
		messageLenPos = m_buf.wpos();
		uint16 messageLen=theChatMsg.length()+1;
		m_buf << messageLen;
		m_buf.append(theChatMsg.c_str(),messageLen);

		//overwrite the pos we didnt before
		m_buf.wpos(injectHandleLenPosHere);
		m_buf << handleLenPos;
		m_buf.wpos(injectMessageLenPosHere);
		m_buf << messageLenPos;
	}
	~PlayerChatMsg() {}
};

class HexGenericMsg : public StaticMsg
{
public:
	HexGenericMsg(string hexadecimalData)
	{
		m_buf.clear();
		string output;
		CryptoPP::HexDecoder decoder;
		decoder.Attach( new CryptoPP::StringSink( output ) );
		decoder.Put( (const byte*)hexadecimalData.c_str(), hexadecimalData.length() );
		decoder.MessageEnd();
		m_buf.append(output);
	}
	~HexGenericMsg() {}
};

class LoadWorldCmd : public StaticMsg
{
public:
	typedef enum
	{
		SLUMS = 0x01,
		DOWNTOWN = 0x02,
		INTERNATIONAL = 0x03
	} mxoLocation;

	LoadWorldCmd(mxoLocation theLoc,string theSky)
	{
		m_buf.clear();
		locs[SLUMS] = "resource/worlds/final_world/slums_barrens_full.metr";
		locs[DOWNTOWN] = "resource/worlds/final_world/downtown/dt_world.metr";
		locs[INTERNATIONAL] = "resource/worlds/final_world/international/it.metr";

		m_buf << uint16(swap16(0x060E))
			  << uint8(0)
			  << uint32(theLoc)
			  //<< uint32(swap32(0x4B61BD47)) //dunno lol
			  << uint32(swap32(0xd868c847))
			  << uint8(1);
		uint16 bytesSoFar = (uint16)m_buf.wpos();
		string metrFile = locs[theLoc];
		uint16 metrFileLen = (uint16)metrFile.length()+1;
		m_buf << uint16(bytesSoFar+sizeof(uint16)+sizeof(uint16)+metrFileLen) //offset to sky length byte
			  << uint16(metrFileLen); //length of string (including null term)
		m_buf.append(metrFile.c_str(),metrFileLen); //string itself (including null term)
		uint16 skyLen = (uint16)theSky.length()+1;
		m_buf << uint16(skyLen);
		m_buf.append(theSky.c_str(),skyLen);
	}
	~LoadWorldCmd() {}
private:
	map<mxoLocation,string> locs;
};

class SetExperienceCmd : public StaticMsg
{
public:
	SetExperienceCmd(uint64 theExp)
	{
		m_buf.clear();
		m_buf << uint16(swap16(0x80e5))
			<< uint64(theExp);
	}
	~SetExperienceCmd() {}
};

class SetInformationCmd : public StaticMsg
{
public:
	SetInformationCmd(uint64 theCash)
	{
		m_buf.clear();
		m_buf << uint16(swap16(0x80e4))
			<< uint64(theCash);
	}
	~SetInformationCmd() {}
};

class EventURLCmd : public StaticMsg
{
public:
	EventURLCmd(string eventURL)
	{
		m_buf.clear();
		//81 a5 00 00 07 00 05
		m_buf << uint16(swap16(0x81a5)) // can also be 81a9
			  << uint16(0)
			  << uint16(7)
			  << uint8(5);
		uint16 eventURLSize = (uint16)eventURL.length()+1;
		m_buf << uint16(eventURLSize);
		m_buf.append(eventURL.c_str(),eventURLSize);
	}
	~EventURLCmd() {}
};

struct MsgBlock 
{
	uint16 sequenceId;
	list<ByteBuffer> subPackets;

	bool FromBuffer(ByteBuffer &source)
	{
		{
			uint16 theId;
			if (source.remaining() < sizeof(theId))
				return false;

			source >> theId;
			sequenceId = swap16(theId);
		}
		uint8 numSubPackets;
		if (source.remaining() < sizeof(numSubPackets))
			return false;

		source >> numSubPackets;
		subPackets.clear();
		for (uint8 i=0;i<numSubPackets;i++)
		{
			uint8 firstTwoBytes[2];
			if (source.remaining() < sizeof(uint8))
				return false;
			source >> firstTwoBytes[0];

			int sizeOfPacketSize = 1;
			if (firstTwoBytes[0] > 0x7F)
			{
				sizeOfPacketSize = 2;
				firstTwoBytes[0] -= 0x80;

				if (source.remaining() < sizeof(uint8))
					return false;

				source >> firstTwoBytes[1];
			}
			uint16 subPacketSize = 0;
			if (sizeOfPacketSize == 1)
			{
				subPacketSize = firstTwoBytes[0];
			}
			else if (sizeOfPacketSize == 2)
			{
				memcpy(&subPacketSize,firstTwoBytes,sizeof(subPacketSize));
				subPacketSize = swap16(subPacketSize);
			}

			if (subPacketSize<1)
				return false;

			vector<byte> dataBuf(subPacketSize);
			if (source.remaining() < dataBuf.size())
				return false;

			source.read(&dataBuf[0],dataBuf.size());
			subPackets.push_back(ByteBuffer(&dataBuf[0],dataBuf.size()));
		}
		return true;
	}
	bool FromBuffer(const byte* buf,size_t size)
	{
		ByteBuffer derp;
		derp.append(buf,size);
		derp.wpos(0);
		derp.rpos(0);
		return FromBuffer(derp) && (derp.remaining() == 0);
	}
	void ToBuffer(ByteBuffer &destination)
	{
		destination << uint16(swap16(sequenceId));

		uint8 numSubPackets = subPackets.size();
		destination << uint8(numSubPackets);
		for (list<ByteBuffer>::iterator it=subPackets.begin();it!=subPackets.end();++it)
		{
			uint16 packetSize = it->size();
			if (packetSize > 0x7f)
			{
				packetSize = htons(packetSize | 0x8000);
				destination << uint16(packetSize);
			}
			else
			{
				destination << uint8(packetSize);
			}
			destination.append(it->contents(),it->size());
		}
	}
	uint32 GetTotalSize()
	{
		uint32 startSize = sizeof(uint16)+sizeof(uint8);
		for (list<ByteBuffer>::iterator it=subPackets.begin();it!=subPackets.end();++it)
		{
			startSize += it->size();
		}		
		return startSize;
	}
};

class OrderedPacket : public StaticMsg
{
public:
	OrderedPacket() {m_buf.clear();}
	OrderedPacket(ByteBuffer &source)
	{
		m_buf.clear();
		FromBuffer(source);
	}
	OrderedPacket(const byte* buf,size_t size)
	{
		m_buf.clear();
		FromBuffer(buf,size);
	}
	OrderedPacket(MsgBlock justOneBlock)
	{
		m_buf.clear();
		msgBlocks.push_back(justOneBlock);
	}
	bool FromBuffer(ByteBuffer &source)
	{
		uint8 zeroFourId=0;
		if (source.remaining() < sizeof(zeroFourId))
			return false;

		source >> zeroFourId;

		if (zeroFourId != 0x04)
			return false;

		uint8 numOrderPackets=0;
		if (source.remaining() < sizeof(numOrderPackets))
			return false;

		source >> numOrderPackets;

		if (numOrderPackets < 1)
			return false;

		msgBlocks.clear();
		for (uint8 i=0;i<numOrderPackets;i++)
		{
			MsgBlock thePacket;
			if (thePacket.FromBuffer(source) == false)
				return false;

			msgBlocks.push_back(thePacket);
		}
		return true;
	}
	bool FromBuffer(const byte* buf,size_t size)
	{
		ByteBuffer derp;
		derp.append(buf,size);
		derp.wpos(0);
		derp.rpos(0);
		return FromBuffer(derp) && (derp.remaining()==0);
	}
private:
	void ToBuffer(ByteBuffer &destination)
	{
		destination << uint8(0x04);
		destination << uint8(msgBlocks.size());

		for (list<MsgBlock>::iterator it=msgBlocks.begin();it!=msgBlocks.end();++it)
		{
			it->ToBuffer(destination);
		}
	}
public:
	const ByteBuffer& toBuf()
	{
		m_buf.clear();
		ToBuffer(m_buf);
		return m_buf;
	}
	list<MsgBlock> msgBlocks;
};


#endif