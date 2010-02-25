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
#include "LocationVector.h"

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
//	string testHandle = lexical_cast<string,uint32>(m_objectId);
//	memcpy(handlePos,testHandle.c_str(),testHandle.length());
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
		m_emotesMap[swap32(0xe6020058)]=0x01; // /beckon
		m_emotesMap[swap32(0x9b010058)]=0x02; // /bigwave
		m_emotesMap[swap32(0xe7020058)]=0x03; // /bow
		m_emotesMap[swap32(0x0d00003a)]=0x04; // /clap
		m_emotesMap[swap32(0x1000003a)]=0x05; // /crossarms
		m_emotesMap[swap32(0x1100003a)]=0x06; // /nod
		m_emotesMap[swap32(0x570d0058)]=0x07; // /agree
		m_emotesMap[swap32(0x590d0058)]=0x08; // /yes
		m_emotesMap[swap32(0x1600003a)]=0x09; // /orangutan
		m_emotesMap[swap32(0x10050004)]=0x0a; // /point
		m_emotesMap[swap32(0xd1020058)]=0x0b; // /pointback
		m_emotesMap[swap32(0x1300003a)]=0x0c; // /pointleft
		m_emotesMap[swap32(0x1400003a)]=0x0d; // /pointright
		m_emotesMap[swap32(0xf50c0058)]=0x0e; // /pointup
		m_emotesMap[swap32(0xfa0c0058)]=0x0f; // /pointdown
		m_emotesMap[swap32(0x0e00003a)]=0x10; // /salute
		m_emotesMap[swap32(0x1200003a)]=0x11; // /shakehead
		m_emotesMap[swap32(0x580d0058)]=0x12; // /disagree
		m_emotesMap[swap32(0x0d00003e)]=0x13; // /no
		m_emotesMap[swap32(0xe9020058)]=0x14; // /stomp
		m_emotesMap[swap32(0x0f00003a)]=0x15; // /tapfoot
		m_emotesMap[swap32(0x0c00003a)]=0x16; // /wave
		m_emotesMap[swap32(0xef0c0058)]=0x17; // /dangerarea
		m_emotesMap[swap32(0x130e0058)]=0x18; // /comeforward
		m_emotesMap[swap32(0xf00c0058)]=0x19; // /enemyinsight
		m_emotesMap[swap32(0xf00c0058)]=0x1a; // /enemy
		m_emotesMap[swap32(0xf20c0058)]=0x1b; // /disperse
		m_emotesMap[swap32(0x1500003a)]=0x1c; // /lookaround
		m_emotesMap[swap32(0xf60c0058)]=0x1d; // /takecover
		m_emotesMap[swap32(0xf70c0058)]=0x1e; // /cover
		m_emotesMap[swap32(0xf30c0058)]=0x1f; // /mapcheck
		m_emotesMap[swap32(0xf80c0058)]=0x20; // /onehandedhandstand
		m_emotesMap[swap32(0xfb0c0058)]=0x21; // /giggle
		m_emotesMap[swap32(0xf40c0058)]=0x22; // /handstand
		m_emotesMap[swap32(0xfc0c0058)]=0x23; // /hearnoevil
		m_emotesMap[swap32(0xfd0c0058)]=0x24; // /seenoevil
		m_emotesMap[swap32(0xfe0c0058)]=0x25; // /speaknoevil
		m_emotesMap[swap32(0x000d0058)]=0x26; // /coverears
		m_emotesMap[swap32(0x010d0058)]=0x27; // /covermouth
		m_emotesMap[swap32(0xff0c0058)]=0x28; // /covereyes
		m_emotesMap[swap32(0x130d0058)]=0x29; // /blowkiss
		m_emotesMap[swap32(0x1f0d0058)]=0x2a; // /blush
		m_emotesMap[swap32(0x270d0058)]=0x2b; // /cheer
		m_emotesMap[swap32(0x650d0058)]=0x2c; // /crackknuckles
		m_emotesMap[swap32(0x650d0058)]=0x2d; // /crackknuckles
		m_emotesMap[swap32(0x7a0d0058)]=0x2e; // /cry
		m_emotesMap[swap32(0x7b0d0058)]=0x2f; // /curtsey
		m_emotesMap[swap32(0x7c0d0058)]=0x30; // /formalbow
		m_emotesMap[swap32(0x7d0d0058)]=0x31; // /formalcurtsey
		m_emotesMap[swap32(0x810d0058)]=0x32; // /bowhead
		m_emotesMap[swap32(0x810d0058)]=0x33; // /bowhead
		m_emotesMap[swap32(0x7e0d0058)]=0x34; // /insult
		m_emotesMap[swap32(0xa9150058)]=0x35; // /scream
		m_emotesMap[swap32(0xa5150058)]=0x36; // /anguish
		m_emotesMap[swap32(0x820d0058)]=0x37; // /karatepower
		m_emotesMap[swap32(0xb90d0058)]=0x38; // /karatepower2
		m_emotesMap[swap32(0xba0d0058)]=0x39; // /karatepower3
		m_emotesMap[swap32(0x830d0058)]=0x3a; // /karatespeed
		m_emotesMap[swap32(0x840d0058)]=0x3b; // /karatespeed2
		m_emotesMap[swap32(0x850d0058)]=0x3c; // /karatespeed3
		m_emotesMap[swap32(0xbb0d0058)]=0x3d; // /karatedefense
		m_emotesMap[swap32(0xbc0d0058)]=0x3e; // /karatedefense2
		m_emotesMap[swap32(0xbd0d0058)]=0x3f; // /karatedefense3
		m_emotesMap[swap32(0x870d0058)]=0x40; // /kneel
		m_emotesMap[swap32(0x880d0058)]=0x41; // /takeaknee
		m_emotesMap[swap32(0x890d0058)]=0x42; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x43; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x44; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x45; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x46; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x47; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x48; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x49; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x4a; // /kungfu
		m_emotesMap[swap32(0xbe0d0058)]=0x4b; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4c; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4d; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4e; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4f; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x50; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x51; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x52; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x53; // /aikido
		m_emotesMap[swap32(0x920d0058)]=0x54; // /laugh
		m_emotesMap[swap32(0x7f0d0058)]=0x55; // /rude
		m_emotesMap[swap32(0xa40d0058)]=0x56; // /loser
		m_emotesMap[swap32(0x010000b4)]=0x57; // /bigtrouble
		m_emotesMap[swap32(0x940d0058)]=0x58; // /okay
		m_emotesMap[swap32(0x940d0058)]=0x59; // /ok
		m_emotesMap[swap32(0x960d0058)]=0x5a; // /peace
		m_emotesMap[swap32(0x770d0058)]=0x5b; // /pullhair
		m_emotesMap[swap32(0x970d0058)]=0x5c; // /rolldice
		m_emotesMap[swap32(0x780d0058)]=0x5d; // /sarcasticclap
		m_emotesMap[swap32(0x790d0058)]=0x5e; // /golfclap
		m_emotesMap[swap32(0x980d0058)]=0x5f; // /scratchhead
		m_emotesMap[swap32(0x990d0058)]=0x60; // /shrug
		m_emotesMap[swap32(0x9a0d0058)]=0x62; // /stretch
		m_emotesMap[swap32(0xf90c0058)]=0x63; // /suckitdown
		m_emotesMap[swap32(0x9b0d0058)]=0x64; // /surrender
		m_emotesMap[swap32(0x9c0d0058)]=0x65; // /thumbsup
		m_emotesMap[swap32(0x1200003a)]=0x66; // /shakehead
		m_emotesMap[swap32(0xa20d0058)]=0x67; // /shameshame
		m_emotesMap[swap32(0x9d0d0058)]=0x68; // /twothumbsup
		m_emotesMap[swap32(0x9f0d0058)]=0x69; // /puke
		m_emotesMap[swap32(0x9e0d0058)]=0x6a; // /vomit
		m_emotesMap[swap32(0xa00d0058)]=0x6b; // /whistle
		m_emotesMap[swap32(0xa60d0058)]=0x6c; // /grovel
		m_emotesMap[swap32(0xe1020058)]=0x6d; // /yawn
		m_emotesMap[swap32(0xa70d0058)]=0x6e; // /plead
		m_emotesMap[swap32(0xa50d0058)]=0x6f; // /shakefist
		m_emotesMap[swap32(0x4e0200b4)]=0x70; // /cool
		m_emotesMap[swap32(0xb70d0058)]=0x71; // /crackneck
		m_emotesMap[swap32(0x150e0058)]=0x72; // /assemble
		m_emotesMap[swap32(0x880200b4)]=0x73; // /mockcry
		m_emotesMap[swap32(0xa70200b4)]=0x74; // /throat
		m_emotesMap[swap32(0x9b0200b4)]=0x75; // /powerpose
		m_emotesMap[swap32(0xbf0200b4)]=0x76; // /thumbsdown
		m_emotesMap[swap32(0xcc0200b4)]=0x77; // /twothumbsdown
		m_emotesMap[swap32(0x250e0058)]=0x78; // /taunt
		m_emotesMap[swap32(0x1f0e0058)]=0x79; // /moveout
		m_emotesMap[swap32(0x1f0e0058)]=0x7a; // /move
		m_emotesMap[swap32(0x1d0e0058)]=0x7b; // /iamready
		m_emotesMap[swap32(0x1b0e0058)]=0x7c; // /rdy
		m_emotesMap[swap32(0x190e0058)]=0x7d; // /ready
		m_emotesMap[swap32(0x0d000080)]=0x7e; // /stop
		m_emotesMap[swap32(0x0d0e0058)]=0x7f; // /bigcheer
		m_emotesMap[swap32(0xd80200b4)]=0x80; // /whoa
		m_emotesMap[swap32(0xd5140058)]=0x81; // /talkrelieved
		m_emotesMap[swap32(0x17140058)]=0x82; // /talk1
		m_emotesMap[swap32(0x19140058)]=0x83; // /talk2
		m_emotesMap[swap32(0x1b140058)]=0x84; // /talk3
		m_emotesMap[swap32(0x1d140058)]=0x85; // /talkangry
		m_emotesMap[swap32(0x21140058)]=0x86; // /talkforceful
		m_emotesMap[swap32(0x1f140058)]=0x87; // /talkexcited
		m_emotesMap[swap32(0x23140058)]=0x88; // /talkscared
		m_emotesMap[swap32(0xd1140058)]=0x89; // /talkchuckle
		m_emotesMap[swap32(0xd3140058)]=0x8a; // /talkhurt
		m_emotesMap[swap32(0xd5140058)]=0x8b; // /talkrelieved
		m_emotesMap[swap32(0xe3150058)]=0x8c; // /talknegative
		m_emotesMap[swap32(0xdd150058)]=0x8d; // /talkpuzzled
		m_emotesMap[swap32(0xd9140058)]=0x8e; // /talkwhisperobvious
		m_emotesMap[swap32(0xdb150058)]=0x8f; // /talkgroup
		m_emotesMap[swap32(0xd7150058)]=0x90; // /talkflirtatious
		m_emotesMap[swap32(0xd9150058)]=0x91; // /talkaffirmative
		m_emotesMap[swap32(0xe1140058)]=0x92; // /overheat
		m_emotesMap[swap32(0xdd140058)]=0x93; // /thewave
		m_emotesMap[swap32(0xdb140058)]=0x94; // /snake
		m_emotesMap[swap32(0xdf140058)]=0x95; // /tsuj
		m_emotesMap[swap32(0x27140058)]=0x96; // /touchearpiece
		m_emotesMap[swap32(0xe1140058)]=0x97; // /overheat
		m_emotesMap[swap32(0x51160058)]=0x98; // /backflop
		m_emotesMap[swap32(0x55160058)]=0x99; // /backflop1
		m_emotesMap[swap32(0x53160058)]=0x9a; // /backflop2
		m_emotesMap[swap32(0x57160058)]=0x9b; // /ballet
		m_emotesMap[swap32(0x59160058)]=0x9c; // /bang
		m_emotesMap[swap32(0x5b160058)]=0x9d; // /cutitout
		m_emotesMap[swap32(0x5d160058)]=0x9e; // /giddyup
		m_emotesMap[swap32(0x5f160058)]=0x9f; // /horns
		m_emotesMap[swap32(0x61160058)]=0xa0; // /mimewall
		m_emotesMap[swap32(0x63160058)]=0xa1; // /mimeelbow
		m_emotesMap[swap32(0x67160058)]=0xa2; // /mimerope
		m_emotesMap[swap32(0x65160058)]=0xa3; // /picknose
		m_emotesMap[swap32(0x6b160058)]=0xa4; // /duh
		m_emotesMap[swap32(0x6d160058)]=0xa5; // /timeout
		m_emotesMap[swap32(0x6f160058)]=0xa6; // /whichway
		m_emotesMap[swap32(0x820200b4)]=0xa9; // /kickdoor
		m_emotesMap[swap32(0x710200b4)]=0xaa; // /examine
		m_emotesMap[swap32(0x8e0200b4)]=0xac; // /pickup
		m_emotesMap[swap32(0xb30200b4)]=0xad; // /takepill
		m_emotesMap[swap32(0x0b0000e0)]=0xaf; // /cough
		m_emotesMap[swap32(0xa10200b4)]=0xb0; // /righton
		m_emotesMap[swap32(0x0c0000e0)]=0xb1; // /sleep
		m_emotesMap[swap32(0xea020058)]=0xb2; // /tiphat
		m_emotesMap[swap32(0xa10000b4)]=0xb3; // /confused
		m_emotesMap[swap32(0x000d0058)]=0xb4; // /coverears
		m_emotesMap[swap32(0x760200b4)]=0xb5; // /eyedrops
		m_emotesMap[swap32(0x8e0200b4)]=0xb6; // /pickup
		m_emotesMap[swap32(0x0e050004)]=0xba; // /talkdepressed
		m_emotesMap[swap32(0x0d050004)]=0xbb; // /throw
		m_emotesMap[swap32(0xc50200b4)]=0xbc; // /toss
		m_emotesMap[swap32(0x4b00003a)]=0xbe; // /shakehands
		m_emotesMap[swap32(0xe4020058)]=0xc1; // /slap
		m_emotesMap[swap32(0xab0d0058)]=0xc7; // /dogsniff
		m_emotesMap[swap32(0xfd0d0058)]=0xca; // /hug
		m_emotesMap[swap32(0xca0d0058)]=0xcd; // /weddingkiss
		m_emotesMap[swap32(0xff0d0058)]=0xd0; // /holdbothhands
		m_emotesMap[swap32(0xfb0d0058)]=0xd3; // /kissthering
		m_emotesMap[swap32(0xf50d0058)]=0xd6; // /manhug
		m_emotesMap[swap32(0x050e0058)]=0xd9; // /pound
		m_emotesMap[swap32(0x030e0058)]=0xdc; // /weddingring
		m_emotesMap[swap32(0x010e0058)]=0xdf; // /propose
		m_emotesMap[swap32(0xc5140058)]=0xe2; // /dap
		m_emotesMap[swap32(0xfb0d0058)]=0xe5; // /kiss
		m_emotesMap[swap32(0x05160058)]=0xe8; // /weddingcake
		m_emotesMap[swap32(0x59160058)]=0xee; // /bangbang
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

WhereAmIResponse::WhereAmIResponse( const LocationVector &currPos )
{
	byte whereamipacket[] =
	{
		0x81, 0x54, 0x00, 0xB4, 0xC3, 0x46, 0x00, 0x80, 0xFC, 0xC3, 0x00, 0xE2, 0x01, 0x47, 0x07, 0x01,	0x00
	} ;

	currPos.toFloatBuf(&whereamipacket[2],sizeof(whereamipacket)-2);
	m_buf.clear();
	m_buf.append(whereamipacket,sizeof(whereamipacket));
}

WhereAmIResponse::~WhereAmIResponse()
{

}