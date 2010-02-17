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

#include "MessageTypes.h"
#include "PlayerObject.h"
#include "GameClient.h"
#include "GameServer.h"
#include "ObjectMgr.h"

ObjectUpdateMsg::ObjectUpdateMsg( uint32 objectId ):m_objectId(objectId),m_toWho(NULL)
{
}

ObjectUpdateMsg::~ObjectUpdateMsg()
{

}

void ObjectUpdateMsg::setReceiver( class GameClient *toWho )
{
	m_toWho = toWho;
}

DeletePlayerMsg::DeletePlayerMsg( uint32 objectId ) :ObjectUpdateMsg(objectId)
{

}

DeletePlayerMsg::~DeletePlayerMsg()
{

}

const ByteBuffer& DeletePlayerMsg::toBuf()
{
	return m_buf;
}

void DeletePlayerMsg::setReceiver( class GameClient *toWho )
{
	m_toWho = toWho;
	m_buf.clear();

	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(m_objectId);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		throw PacketNoLongerValid();
	}

	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,m_objectId);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		throw PacketNoLongerValid();		
	}
	DEBUG_LOG(format("Player %1% delete packet serializing for client %2% with viewID %3%") % m_objectId % m_toWho->Address() % viewId);

	//03 01 00 01 01 00 <OBJECT ID:2bytes>
	const byte rawData[6] =
	{
		0x03, 0x01, 0x00, 0x01, 0x01, 0x00, 
	} ;
	m_buf.append(rawData,sizeof(rawData));
	m_buf << uint16(viewId);
}

PlayerSpawnMsg::PlayerSpawnMsg( uint32 objectId ) :ObjectUpdateMsg(objectId)
{

}

PlayerSpawnMsg::~PlayerSpawnMsg()
{

}

const ByteBuffer& PlayerSpawnMsg::toBuf()
{
	byte sampleSpawnPacket[202] =
	{
		0x03, 0x01, 0x00, 0x0C, 0x0C, 0x00, 0x2F, 0xCD, 0xAB, 0x18, 0x8B, 0xEC, 0xFF, 0x05, 0x00, 0x00, 
		0x00, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 
		0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 
		0x3A, 0x90, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 
		0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 
		0x3B, 0x3B, 0x80, 0x98, 0x5A, 0x5A, 0x04, 0x86, 0x8C, 0xFF, 0x6A, 0x6A, 0xC6, 0xC5, 0xFF, 0x3C, 
		0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 
		0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x5B, 
		0x5B, 0xED, 0x7A, 0x00, 0x00, 0x00, 0xC5, 0xFF, 0x75, 0xD4, 0x01, 0x00, 0x8A, 0x8A, 0x8A, 0x8A, 
		0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x00, 0x00, 0x6B, 0x6B, 0x9D, 0x4A, 0x4A, 
		0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4C, 0x4C, 
		0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x8C, 0xFF, 0x7B, 0x22, 0x80, 0x88, 0x17, 0x1C, 0x7C, 0x00, 
		0x10, 0x00, 0x00, 0xC5, 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 
	} ;

	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(m_objectId);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}

	byte *firstNamePos = &sampleSpawnPacket[0x11];
	memset(firstNamePos,0,32);
	memcpy(firstNamePos,m_player->getFirstName().c_str(),m_player->getFirstName().length());
	byte *lastNamePos = &sampleSpawnPacket[0x32];
	memset(lastNamePos,0,32);
	memcpy(lastNamePos,m_player->getLastName().c_str(),m_player->getLastName().length());
	byte *handlePos = &sampleSpawnPacket[0x5F];
	memset(handlePos,0,32);
	memcpy(handlePos,m_player->getHandle().c_str(),m_player->getHandle().length());

	byte *rsiDataPos = &sampleSpawnPacket[0x8C];
	memset(rsiDataPos,0,15);
	m_player->getRsiData(rsiDataPos,15);

	byte *positionPos = &sampleSpawnPacket[0x9E];
	m_player->getPosition().toDoubleBuf(positionPos,sizeof(double)*3);

	uint16 temp16;
	uint8 temp8;

	byte *healthCpos = &sampleSpawnPacket[0x54];
	temp16 = m_player->getCurrentHealth();
	memcpy(healthCpos,&temp16,sizeof(temp16));

	byte *healthMpos = &sampleSpawnPacket[0x7F];
	temp16 = m_player->getMaximumHealth();
	memcpy(healthMpos,&temp16,sizeof(temp16));

	byte *innerStrCpos = &sampleSpawnPacket[0x5A];
	temp16 = m_player->getCurrentIS();
	memcpy(innerStrCpos,&temp16,sizeof(temp16));

	byte *innerStrMpos = &sampleSpawnPacket[0x9B];
	temp16 = m_player->getMaximumIS();
	memcpy(innerStrMpos,&temp16,sizeof(temp16));

	byte *professionPos = &sampleSpawnPacket[0x82];
	temp8 = m_player->getProfession();
	memcpy(professionPos,&temp8,sizeof(temp8));

	byte *levelPos = &sampleSpawnPacket[0xB8];
	temp8 = m_player->getLevel();
	memcpy(levelPos,&temp8,sizeof(temp8));

	byte *alignmentPos = &sampleSpawnPacket[0xBE];
	temp8 = m_player->getAlignment();
	memcpy(alignmentPos,&temp8,sizeof(temp8));

	byte *viewIdPos = &sampleSpawnPacket[0xC5];
	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,m_objectId);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}
	memcpy(viewIdPos,&viewId,sizeof(viewId));
	DEBUG_LOG(format("Player %1%:%2% spawn packet serializing for client %3% with viewID %4%") % m_player->getHandle() % m_objectId % m_toWho->Address() % viewId);

	m_buf.clear();
	m_buf.append(sampleSpawnPacket,sizeof(sampleSpawnPacket));
	return m_buf;
}

StateUpdateMsg::StateUpdateMsg( uint32 objectId, ByteBuffer stateData ) :ObjectUpdateMsg(objectId)
{
	restOfData=stateData;
}

StateUpdateMsg::~StateUpdateMsg()
{

}

const ByteBuffer& StateUpdateMsg::toBuf()
{
	m_buf.clear();

	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(m_objectId);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}
	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,m_objectId);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}

	m_buf << uint8(0x03);
	m_buf << uint16(viewId);
	m_buf.append(restOfData.contents(),restOfData.size());
	return m_buf;
}

map<uint32,uint8> EmoteMsg::m_emotesMap;

EmoteMsg::EmoteMsg(uint32 objectId, uint32 emoteId, uint8 emoteCount):ObjectUpdateMsg(objectId)
{
	m_emoteCount=emoteCount;
	if (m_emotesMap.empty())
	{
		m_emotesMap[swap32(0xE6020058)]=0x1;
		m_emotesMap[swap32(0xE7020058)]=0x3;
		m_emotesMap[swap32(0x810D0058)]=0x33;
		m_emotesMap[swap32(0x7C0D0058)]=0x30;
		m_emotesMap[swap32(0x7B0D0058)]=0x2f;
		m_emotesMap[swap32(0x7D0D0058)]=0x31;
		m_emotesMap[swap32(0x0E00003A)]=0x10;
		m_emotesMap[swap32(0x0C00003A)]=0x16;
		m_emotesMap[swap32(0x9B010058)]=0x2;
		m_emotesMap[swap32(0x970D0058)]=0x5c;
		m_emotesMap[swap32(0x9C0D0058)]=0x65;
		m_emotesMap[swap32(0xBF0200B4)]=0x76;
		m_emotesMap[swap32(0xCC0200B4)]=0x77;
		m_emotesMap[swap32(0x9D0D0058)]=0x68;
		m_emotesMap[swap32(0x590D0058)]=0x8;
		m_emotesMap[swap32(0x570D0058)]=0x7;
		m_emotesMap[swap32(0x1100003A)]=0x6;
		m_emotesMap[swap32(0x940D0058)]=0x58;
		m_emotesMap[swap32(0x5A0D0058)]=0x13;
		m_emotesMap[swap32(0x580D0058)]=0x12;
		m_emotesMap[swap32(0x1200003A)]=0x66;
		m_emotesMap[swap32(0x150E0058)]=0x72;
		m_emotesMap[swap32(0xEF0C0058)]=0x17;
		m_emotesMap[swap32(0x170E0058)]=0x1b;
		m_emotesMap[swap32(0xF00C0058)]=0x19;
		m_emotesMap[swap32(0xF30C0058)]=0x1f;
		m_emotesMap[swap32(0x1F0E0058)]=0x79;
		m_emotesMap[swap32(0x190E0058)]=0x7d;
		m_emotesMap[swap32(0x110E0058)]=0x7e;
		m_emotesMap[swap32(0xF60C0058)]=0x1d;
		m_emotesMap[swap32(0x0400003A)]=0xa;
		m_emotesMap[swap32(0x1300003A)]=0xc;
		m_emotesMap[swap32(0x1400003A)]=0xd;
		m_emotesMap[swap32(0xD1020058)]=0xb;
		m_emotesMap[swap32(0xF50C0058)]=0xe;
		m_emotesMap[swap32(0xFA0C0058)]=0xf;
		m_emotesMap[swap32(0x0D00003A)]=0x4;
		m_emotesMap[swap32(0xFB0C0058)]=0x21;
		m_emotesMap[swap32(0x920D0058)]=0x54;
		m_emotesMap[swap32(0x270D0058)]=0x2b;
		m_emotesMap[swap32(0x1F0D0058)]=0x2a;
		m_emotesMap[swap32(0x0D0E0058)]=0x7f;
		m_emotesMap[swap32(0x960D0058)]=0x5a;
		m_emotesMap[swap32(0x130D0058)]=0x29;
		m_emotesMap[swap32(0xE9020058)]=0x14;
		m_emotesMap[swap32(0x7A0D0058)]=0x2e;
		m_emotesMap[swap32(0x880200B4)]=0x73;
		m_emotesMap[swap32(0x7E0D0058)]=0x34;
		m_emotesMap[swap32(0xA40D0058)]=0x56;
		m_emotesMap[swap32(0x770D0058)]=0x5b;
		m_emotesMap[swap32(0xA50D0058)]=0x6f;
		m_emotesMap[swap32(0x250E0058)]=0x78;
		m_emotesMap[swap32(0xA9150058)]=0x35;
		m_emotesMap[swap32(0x1000003A)]=0x5;
		m_emotesMap[swap32(0x0F00003A)]=0x15;
		m_emotesMap[swap32(0xF40C0058)]=0x22;
		m_emotesMap[swap32(0x650D0058)]=0x2d;
		m_emotesMap[swap32(0xB70D0058)]=0x71;
		m_emotesMap[swap32(0x990D0058)]=0x60;
		m_emotesMap[swap32(0xFD0D0058)]=0xca;
		m_emotesMap[swap32(0xF50D0058)]=0xd6;
		m_emotesMap[swap32(0xE4020058)]=0xc1;
		m_emotesMap[swap32(0xFB0D0058)]=0xd3;
		m_emotesMap[swap32(0x050E0058)]=0xd9;
		m_emotesMap[swap32(0xFF0D0058)]=0xd0;
		m_emotesMap[swap32(0x010E0058)]=0xdf;
		m_emotesMap[swap32(0xFF0D0058)]=0xd0;
		m_emotesMap[swap32(0x030E0058)]=0xdc;
		m_emotesMap[swap32(0xCA0D0058)]=0xcd;
		m_emotesMap[swap32(0x05160058)]=0xe8;
	}
	if (m_emotesMap.find(emoteId) != m_emotesMap.end() )
		m_emoteAnimation=m_emotesMap[emoteId];
	else
		m_emoteAnimation=0;
}

EmoteMsg::~EmoteMsg()
{

}

const ByteBuffer& EmoteMsg::toBuf()
{
	byte sampleEmoteMsg[] =
	{
		0x03, 0x02, 0x00, 0x01, 0x28, 0xAA, 0x40, 0x00, 0x25, 0x01, 0x00, 0x00, 0x10, 0xBB, 0xBB, 0xBB, 
		0xBB, 0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD, 0x2A, 0x9F, 0x1E, 0x20, 0x00, 0x00, 
	} ;

	sampleEmoteMsg[5] = m_emoteCount;

	if (m_emoteAnimation>0)
	{
		sampleEmoteMsg[9]=m_emoteAnimation;
	}
	else
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}

	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(m_objectId);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}
	m_player->getPosition().toFloatBuf(&sampleEmoteMsg[0x0D],sizeof(float)*3);
	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,m_objectId);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}
	memcpy(&sampleEmoteMsg[1],&viewId,sizeof(viewId));

	m_buf.clear();
	m_buf.append(sampleEmoteMsg,sizeof(sampleEmoteMsg));
	return m_buf;
}

AnimationStateMsg::AnimationStateMsg( uint32 objectId ):ObjectUpdateMsg(objectId)
{
	
}

AnimationStateMsg::~AnimationStateMsg()
{

}

const ByteBuffer& AnimationStateMsg::toBuf()
{
	byte sampleAnimationBuf[9] =
	{
		0x03, 0x02, 0x00, 0x01, 0x01, 0xAA, 0xBB, 0x00, 0x00, 
	} ;

	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(m_objectId);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}
	sampleAnimationBuf[5] = m_player->getCurrentAnimation();
	sampleAnimationBuf[6] = m_player->getCurrentMood();

	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,m_objectId);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}
	memcpy(&sampleAnimationBuf[1],&viewId,sizeof(viewId));

	m_buf.clear();
	m_buf.append(sampleAnimationBuf,sizeof(sampleAnimationBuf));
	return m_buf;
}