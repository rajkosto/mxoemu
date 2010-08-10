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


class CloseDoorMsg : public ObjectUpdateMsg
{
public:
	CloseDoorMsg(uint32 objectId );
	~CloseDoorMsg();
	const ByteBuffer& toBuf();
	void setReceiver(class GameClient *toWho);
};

class SitDownMsg : public ObjectUpdateMsg
{
public:
	SitDownMsg(uint32 objectId);
	~SitDownMsg();
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

class EmoteMsg : public ObjectUpdateMsg
{
public:
	EmoteMsg(uint32 objectId, uint32 emoteId, uint8 emoteCount);
	~EmoteMsg();
	const ByteBuffer& toBuf();
private:
	uint8 m_emoteCount;
	static map<uint32,uint8> m_emotesMap;
	uint8 m_emoteAnimation;
};

class AnimationStateMsg : public ObjectUpdateMsg
{
public:
	AnimationStateMsg(uint32 objectId);
	~AnimationStateMsg();
	const ByteBuffer& toBuf();
};

class PositionStateMsg : public ObjectUpdateMsg
{
public:
	PositionStateMsg(uint32 objectId);
	~PositionStateMsg();
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

class DoorAnimationMsg : public StaticMsg
{
public:

	DoorAnimationMsg(uint32 doorId, uint16 viewId, double X, double Y, double Z, double ROT, int doorType);
	~DoorAnimationMsg();
};

class WhereAmIResponse : public StaticMsg
{
public:
	WhereAmIResponse(const class LocationVector &currPos);
	~WhereAmIResponse();
};

class WhisperMsg : public StaticMsg
{
public:
	WhisperMsg(string sender, string message)
	{
		m_buf << uint16(swap16(0x2E11));
		m_buf << uint8(0);
		m_buf << uint32(0);

		size_t putSenderStrLenPosHere = m_buf.wpos();
		m_buf << uint32(0); //will overwrite this later
		size_t putMessageStrLenPosHere = m_buf.wpos();
		m_buf << uint32(0); //will overwrite this later

		//0x15 bytes of 0s
		for (int i=0;i<0x15;i++)
			m_buf << uint8(0);

		size_t senderStrLenPos = m_buf.wpos();

		sender = "SOE+MXO+Reality+" + sender;
		uint16 senderStrLen = sender.length()+1;
		m_buf << uint16(senderStrLen);
		m_buf.append(sender.c_str(),senderStrLen);
		
		size_t messageStrLenPos = m_buf.wpos();
		uint16 messageStrLen = message.length()+1;
		m_buf << uint16(messageStrLen);
		m_buf.append(message.c_str(),messageStrLen);

		//go back and put positions there
		m_buf.wpos(putSenderStrLenPosHere);
		m_buf << uint32(senderStrLenPos);
		m_buf.wpos(putMessageStrLenPosHere);
		m_buf << uint32(messageStrLenPos);
	}
	~WhisperMsg()
	{

	}
};

class SystemMsg : public StaticMsg
{
public:
	SystemMsg(uint16 opcode,string message)
	{
		m_buf.clear();

		m_buf << uint16(swap16(opcode));
		m_buf << uint8(0);
		m_buf << uint32(0);

		m_buf << uint32(0); //no sender
		size_t putMessageStrLenPosHere = m_buf.wpos();
		m_buf << uint32(0); //we will overwrite this later

		//0x15 bytes of 0s
		for (int i=0;i<0x15;i++)
			m_buf << uint8(0);

		size_t messageStrLenPos = m_buf.wpos();
		uint16 messageStrLen = message.length()+1;
		m_buf << uint16(messageStrLen);
		m_buf.append(message.c_str(),messageStrLen);

		//go back and put position there
		m_buf.wpos(putMessageStrLenPosHere);
		m_buf << uint32(messageStrLenPos);
	}
	~SystemMsg() {}
};

class BroadcastMsg : public SystemMsg
{
public:
	BroadcastMsg(string message) : SystemMsg(0x2EC7,message) {}
	~BroadcastMsg() {}
};

class ModalMsg : public SystemMsg
{
public:
	ModalMsg(string message) : SystemMsg(0x2ED7,message) {}
	~ModalMsg() {}
};

class SystemChatMsg : public SystemMsg
{
public:
	SystemChatMsg(string message) : SystemMsg(0x2E07,message) {}
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
		size_t handleLenPos=0;
		m_buf << handleLenPos; //will come back here and overwrite later
		size_t injectMessageLenPosHere = m_buf.wpos();
		size_t messageLenPos=0;
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
		m_buf << uint32(handleLenPos);
		m_buf.wpos(injectMessageLenPosHere);
		m_buf << uint32(messageLenPos);
	}
	~PlayerChatMsg() {}
};

class BackgroundResponseMsg : public StaticMsg
{
public:
	BackgroundResponseMsg(string playerBackground)
	{
		m_buf.clear();

		m_buf << uint16(swap16(0x8195));
		size_t insertBackgroundStrLenPosHere = m_buf.wpos();
		m_buf << uint16(0); //placeholder
		m_buf << uint8(0);
		uint16 backgroundStrLenPos = m_buf.wpos();
		uint16 backgroundStrLen = playerBackground.length()+1;
		m_buf << uint16(backgroundStrLen);
		m_buf.append(playerBackground.c_str(),backgroundStrLen);

		//go back and put position there
		m_buf.wpos(insertBackgroundStrLenPosHere);
		m_buf << uint16(backgroundStrLenPos);
	}
	~BackgroundResponseMsg() {}
};

class PlayerDetailsMsg : public StaticMsg
{
public:
	PlayerDetailsMsg(class PlayerObject *thePlayer);
	~PlayerDetailsMsg();
};

class PlayerBackgroundMsg : public StaticMsg
{
public:
	PlayerBackgroundMsg(string playerBackground)
	{
		m_buf.clear();

		m_buf << uint16(swap16(0x8198));
		size_t insertBackgroundStrLenPosHere = m_buf.wpos();
		m_buf << uint16(0); //placeholder
		m_buf << uint8(1);
		uint16 backgroundStrLenPos = m_buf.wpos();
		uint16 backgroundStrLen = playerBackground.length()+1;
		m_buf << uint16(backgroundStrLen);
		m_buf.append(playerBackground.c_str(),backgroundStrLen);

		//go back and put position there
		m_buf.wpos(insertBackgroundStrLenPosHere);
		m_buf << uint16(backgroundStrLenPos);
	}
	~PlayerBackgroundMsg() {}
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
		TUTORIAL = 0x00,
		SLUMS = 0x01,
		DOWNTOWN = 0x02,
		INTERNATIONAL = 0x03,
		ARCHIVE01 = 0x04,
		ARCHIVE02 = 0x05,
		ASHENCOURT = 0x06,
		DATAMINE = 0x07,
		SAKURA = 0x08,
		SATI = 0x09,
		WIDOWSMOOR = 0x0A,
		YUKI = 0x0B,
		LARGE01 = 0x0C,
		LARGE02 = 0x0D,
		MEDIUM01 = 0x0E,
		MEDIUM02 = 0x0F,
		MEDIUM03 = 0x10,
		SMALL03 = 0x11,
		CAVES = 0x12,
	} mxoLocation;

	LoadWorldCmd(mxoLocation theLoc,string theSky);
	~LoadWorldCmd();
private:
	map<mxoLocation,string> locs;
};

class SetOptionCmd : public StaticMsg
{
public:
	SetOptionCmd(string optionName,bool booleanVal)
	{
		InitialSetUp(optionName,TYPE_BOOL);
		uint32 intermediaryVal = 2;
		if (booleanVal)
			intermediaryVal=1;

		m_buf << sizeof(intermediaryVal);
		m_buf << intermediaryVal;
	}
	SetOptionCmd(string optionName,uint16 shortIntVal)
	{
		InitialSetUp(optionName,TYPE_UINT16);

		m_buf << sizeof(shortIntVal);
		m_buf << shortIntVal;
	}
	SetOptionCmd(string optionName,uint32 intVal)
	{
		InitialSetUp(optionName,TYPE_UINT32);

		m_buf << sizeof(intVal);
		m_buf << intVal;
	}
	SetOptionCmd(string optionName,string stringVal)
	{
		InitialSetUp(optionName,TYPE_STRING);

		if (stringVal.length() < 1)
			m_buf << uint16(0);
		else
		{
			uint16 stringValLen = stringVal.length()+1;
			m_buf << uint16(stringValLen);
			m_buf.append(stringVal.c_str(),stringValLen);
		}
	}
	SetOptionCmd(string optionName,vector<string> stringVals)
	{
		InitialSetUp(optionName,TYPE_STRING);

		stringstream cummulative;

		foreach(const string &theStr,stringVals)
		{
			if (cummulative.str().length() > 1)
				cummulative << ",";

			cummulative << theStr;
		}

		string stringVal = cummulative.str();

		if (stringVal.length() < 1)
			m_buf << uint16(0);
		else
		{
			uint16 stringValLen = stringVal.length()+1;
			m_buf << uint16(stringValLen);
			m_buf.append(stringVal.c_str(),stringValLen);
		}
	}
	~SetOptionCmd() {}
private:
	typedef enum
	{
		TYPE_BOOL = 0x11,
		TYPE_UINT16 = 0x1B,
		TYPE_UINT32 = 0x17,
		TYPE_STRING = 0x19,
		TYPE_STRINGLIST = 0x14,
	} varType;
	void InitialSetUp(string optionName,varType theType)
	{
		m_buf.clear();
		m_buf << uint16(swap16(0x3A05));
		m_buf << uint8(0);
		m_buf << uint16(theType);
		uint16 optionNameStrLen = optionName.length()+1;
		m_buf << uint16(optionNameStrLen);
		m_buf.append(optionName.c_str(),optionNameStrLen);
	}
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
	uint32 GetTotalSize()
	{
		uint32 startSize = sizeof(uint8)*2;
		for (list<MsgBlock>::iterator it=msgBlocks.begin();it!=msgBlocks.end();++it)
		{
			startSize += it->GetTotalSize();
		}		
		return startSize;
	}
	list<MsgBlock> msgBlocks;
};


#endif
