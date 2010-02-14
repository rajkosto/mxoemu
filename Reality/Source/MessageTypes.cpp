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
	DEBUG_LOG(format("Player %1% spawn packet serializing for client %2% with viewID %3%") % m_objectId % m_toWho->Address() % viewId);

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