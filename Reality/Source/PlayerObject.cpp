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
#include "PlayerObject.h"
#include "RsiData.h"
#include "Database/Database.h"
#include "GameServer.h"
#include "MessageTypes.h"
#include "Log.h"
#include "GameClient.h"
#include "Timer.h"

PlayerObject::PlayerObject( GameClient &parent,uint64 charUID ) :m_parent(parent),m_characterUID(charUID),m_spawnedInWorld(false)
{
	//grab data from characters table
	{
		scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `handle`,\
													 `firstName`, `lastName`, `background`,\
													 `x`, `y`, `z`, `rot`, \
													 `healthC`, `healthM`, `innerStrC`, `innerStrM`,\
													 `level`, `profession`, `alignment`, `pvpflag`, `exp`, `cash`, `district`\
													 FROM `characters` WHERE `charId` = '%1%' LIMIT 1") % m_characterUID) );

		if (result == NULL)
		{
			throw CharacterNotFound();
		}

		Field *field = result->Fetch();
		m_handle = field[0].GetString();
		m_firstName = field[1].GetString();
		m_lastName = field[2].GetString();
		m_background = field[3].GetString();
		m_pos.ChangeCoords(	field[4].GetDouble(),
			field[5].GetDouble(),
			field[6].GetDouble());
		m_pos.rot = field[7].GetDouble();
		m_savedPos = m_pos;
		m_healthC = field[8].GetUInt16();
		m_healthM = field[9].GetUInt16();
		m_innerStrC = field[10].GetUInt16();
		m_innerStrM = field[11].GetUInt16();
		m_lvl = field[12].GetUInt8();
		m_prof = field[13].GetUInt8();
		m_alignment = field[14].GetUInt8();
		m_pvpflag = field[15].GetBool();
		m_exp = field[16].GetUInt64();
		m_cash = field[17].GetUInt64();
		m_district = field[18].GetUInt8();
	}
	//grab data from rsi table
	{
		scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `sex`, `body`, `hat`, `face`, `shirt`,\
													 `coat`, `pants`, `shoes`, `gloves`, `glasses`,\
													 `hair`, `facialdetail`, `shirtcolor`, `pantscolor`,\
													 `coatcolor`, `shoecolor`, `glassescolor`, `haircolor`,\
													 `skintone`, `tattoo`, `facialdetailcolor`, `leggings` FROM `rsivalues` WHERE `charId` = '%1%' LIMIT 1") % m_characterUID) );
		if (result == NULL)
		{
			INFO_LOG(format("SpawnRSI(%1%): Character's RSI doesn't exist") % m_handle );
			m_rsi.reset(new RsiDataMale);
			const byte defaultRsiValues[] = {0x00,0x0C,0x71,0x48,0x18,0x0C,0xE2,0x00,0x23,0x00,0xB0,0x00,0x40,0x00,0x00};
			m_rsi->FromBytes(defaultRsiValues,sizeof(defaultRsiValues));
		}
		else
		{
			Field *field = result->Fetch();
			uint8 sex = field[0].GetUInt8();

			if (sex == 0) //male
				m_rsi.reset(new RsiDataMale);
			else
				m_rsi.reset(new RsiDataFemale);

			RsiData &playerRef = *m_rsi;

			if (sex == 0) //male
				playerRef["Sex"]=0;
			else
				playerRef["Sex"]=1;

			playerRef["Body"] =			field[1].GetUInt8();
			playerRef["Hat"] =			field[2].GetUInt8();
			playerRef["Face"] =			field[3].GetUInt8();
			playerRef["Shirt"] =		field[4].GetUInt8();
			playerRef["Coat"] =			field[5].GetUInt8();
			playerRef["Pants"] =		field[6].GetUInt8();
			playerRef["Shoes"] =		field[7].GetUInt8();
			playerRef["Gloves"] =		field[8].GetUInt8();
			playerRef["Glasses"] =		field[9].GetUInt8();
			playerRef["Hair"] =			field[10].GetUInt8();
			playerRef["FacialDetail"]=	field[11].GetUInt8();
			playerRef["ShirtColor"] =	field[12].GetUInt8();
			playerRef["PantsColor"] =	field[13].GetUInt8();
			playerRef["CoatColor"] =	field[14].GetUInt8();
			playerRef["ShoeColor"] =	field[15].GetUInt8();
			playerRef["GlassesColor"]=	field[16].GetUInt8();
			playerRef["HairColor"] =	field[17].GetUInt8();
			playerRef["SkinTone"] =		field[18].GetUInt8();
			playerRef["Tattoo"] =		field[19].GetUInt8();
			playerRef["FacialDetailColor"] =	field[20].GetUInt8();

			if (sex != 0)
				playerRef["Leggings"] =	field[21].GetUInt8();
		}
	}

	m_goId=0;
	INFO_LOG(format("Player object for %1% constructed") % m_handle);
	testCount=0;
	m_lastStore = getTime();
	m_currAnimation=0;
	m_currMood=0;
	m_emoteCounter=0;
}

void PlayerObject::initGoId(uint32 theGoId)
{
	m_goId = theGoId;
	INFO_LOG(format("Player name %1% has goid %2%") % m_handle % m_goId);
}

PlayerObject::~PlayerObject()
{
	if (m_spawnedInWorld == true)
	{
		INFO_LOG(format("Player object for %1%:%2% deconstructing") % m_handle % m_goId);
		sGame.AnnounceStateUpdate(&m_parent,make_shared<DeletePlayerMsg>(m_goId));
		m_spawnedInWorld=false;

		//commit position changes
		saveDataToDB();
	}
}

uint8 PlayerObject::getRsiData( byte* outputBuf,uint32 maxBufLen ) const
{
	if (m_rsi == NULL)
		return 0;

	return m_rsi->ToBytes(outputBuf,maxBufLen);
}

void PlayerObject::checkAndStore()
{
	if (getTime() - m_lastStore > 60) //every 60 seconds
	{
		saveDataToDB();
		m_lastStore = getTime();
	}
}

void PlayerObject::saveDataToDB()
{
	if (m_savedPos == m_pos)
		return;

	bool storeSuccess = sDatabase.Execute(format("UPDATE `characters` SET `x` = '%1%', `y` = '%2%', `z` = '%3%', `rot` = '%4%' WHERE `charId` = '%5%'")
		% m_pos.x
		% m_pos.y
		% m_pos.z
		% m_pos.rot
		% m_characterUID );

	if (!storeSuccess)
		WARNING_LOG(format("%1%:%2% failed to save data to database") % m_handle % m_goId );
	else
	{
		m_savedPos = m_pos;
		m_parent.QueueCommand(make_shared<SystemChatMsg>( (format("Character data for %1% has been written to the database.") % m_handle).str() ));
	}
}

void PlayerObject::InitializeWorld()
{
	m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"SatiSky"));
	m_parent.QueueCommand(make_shared<SetExperienceCmd>(m_exp));
	m_parent.QueueCommand(make_shared<SetInformationCmd>(m_cash));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80b24e0008000802"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80b2520005000802"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80b2540008000802"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80b24f0008000802"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80b251000b000802"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80b2110001000802"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc4503110000020000001100010000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc450002000002000000cc00000000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc1500030000f70300000802000000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc1500040000f70300000702ecffffff0000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc1500050000f70300005004000000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc1500060000f7030000f403000000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc1500070000f70300005104f6ffffff0000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80bc1500080000f703000052040f0000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("47000004010000000000000000000000000000001a0006000000010000000001010000000000800000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("2e0700000000000000000000005900002e00000000000000000000000000000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80865220060000000000000000000000000000000000000000210000000000230000000000"));
	m_parent.QueueCommand(make_shared<EventURLCmd>("http://mxoemu.info/forum/index.php"));
	m_parent.QueueCommand(make_shared<SystemChatMsg>("COME ON CHECK THIS OUT"));
}

void PlayerObject::SpawnSelf()
{
	if (m_spawnedInWorld == false)
	{
		//first object spawn in world the client likes to control, so we have to self spawn first
		m_parent.QueueState(make_shared<PlayerSpawnMsg>(m_goId));
		//announce our presence to others
		sGame.AnnounceStateUpdate(&m_parent,make_shared<PlayerSpawnMsg>(m_goId));
		m_spawnedInWorld=true;
	}
}

void PlayerObject::PopulateWorld()
{
	//we need to get all other world entities and populate our client with it
	vector<uint32> allWorldObjects = sObjMgr.getAllGOIds();
	for (vector<uint32>::iterator it=allWorldObjects.begin();it!=allWorldObjects.end();++it)
	{
		PlayerObject *theOtherObject = NULL;
		try
		{
			theOtherObject = sObjMgr.getGOPtr(*it);;
		}
		catch (ObjectMgr::ObjectNotAvailable)
		{
			continue;
		}

		//we self spawned already, so no
		if (theOtherObject!=this)
		{
			vector<msgBaseClassPtr> objectsPackets = theOtherObject->getCurrentStatePackets();
			for (vector<msgBaseClassPtr>::iterator it2=objectsPackets.begin();it2!=objectsPackets.end();++it2)
			{
				m_parent.QueueState(*it2);
			}
		}
	}
}

void PlayerObject::HandleStateUpdate( ByteBuffer &srcData )
{
	checkAndStore();

	uint8 zeroThree;
	if (srcData.remaining() < sizeof(zeroThree))
		return;
	srcData >> zeroThree;
	if (zeroThree != 3)
		return;
	uint16 viewIdToUpdate;
	if (srcData.remaining() < sizeof(viewIdToUpdate))
		return;
	srcData >> viewIdToUpdate;
	if (viewIdToUpdate != sObjMgr.getViewForGO(&m_parent,m_goId))
	{
		WARNING_LOG(format("Client %1% Player %2%:%3% trying to update someone else's object view %4%") % m_parent.Address() % m_handle % m_goId % viewIdToUpdate);
		return;
	}
	size_t restOfDataPos = srcData.rpos();
	uint8 shouldBeOne;
	if (srcData.remaining() < sizeof(shouldBeOne))
		return;
	srcData >> shouldBeOne;
	if (shouldBeOne != 1)
	{
		WARNING_LOG(format("Client %1% Player %2%:%3% 03 doesn't have number 1 after viewId") % m_parent.Address() % m_handle % m_goId);
		return;
	}
	uint8 updateType;
	if (srcData.remaining() < sizeof(updateType))
		return;
	srcData >> updateType;
	bool validUpdate=false;
	switch (updateType)
	{
	//change angle
	case 0x04:
		{
			uint8 theRotByte;
			if (srcData.remaining() < sizeof(theRotByte))
				return;
			srcData >> theRotByte;

			m_pos.setMxoRot(theRotByte);
			validUpdate=true;
			break;
		}
	//change angle with extra param
	case 0x06:
		{
			uint8 theAnimation;
			if (srcData.remaining() < sizeof(theAnimation))
				return;
			srcData >> theAnimation;
			//we will just ignore the animation for now
			uint8 theRotByte;
			if (srcData.remaining() < sizeof(theRotByte))
				return;
			srcData >> theRotByte;

			m_pos.setMxoRot(theRotByte);
			validUpdate=true;
			break;
		}
	//update xyz
	case 0x08:
		{
			validUpdate = m_pos.fromFloatBuf(srcData);
			break;
		}
	//update xyz, extra byte before xyz
	case 0x0A:
	case 0x0C:
		{
			uint8 extraByte;
			if (srcData.remaining() < sizeof(extraByte))
				return;
			srcData >> extraByte;
			
			validUpdate = m_pos.fromFloatBuf(srcData);
			break;
		}
	//update xyz, extra 2 bytes before xyz
	case 0x0E:
		{
			uint8 extraByte1,extraByte2;
			if (srcData.remaining() < sizeof(uint8)*2)
				return;
			srcData >> extraByte1;
			srcData >> extraByte2;

			validUpdate = m_pos.fromFloatBuf(srcData);
			break;
		}
	//sometimes happens, no info inside
	case 0x02:
		{
			validUpdate = true;
			break;
		}
	}
	if (validUpdate)
	{
		//propagate state to all other players
		srcData.rpos(restOfDataPos);
		ByteBuffer theStateData;
		theStateData.append(&srcData.contents()[srcData.rpos()],srcData.remaining());
		sGame.AnnounceStateUpdate(&m_parent,make_shared<StateUpdateMsg>(m_goId,theStateData),true);
	}
	else
	{
		srcData.rpos(0);
		DEBUG_LOG(format("(%1%) %2%:%3% 03 data: %4%") % m_parent.Address() % m_handle % m_goId % Bin2Hex(srcData) );
	}
}

void PlayerObject::HandleCommand( ByteBuffer &srcCmd )
{
	checkAndStore();

	uint8 firstByte;
	if (srcCmd.remaining() < sizeof(firstByte) )
		return;
	srcCmd >> firstByte;
	if (firstByte == 0x28)
	{
		uint8 secondByte;
		if (srcCmd.remaining() < sizeof(secondByte))
			return;
		srcCmd >> secondByte;

		if (secondByte == 0x10)
		{
			uint16 stringLenPos;
			if (srcCmd.remaining() < sizeof(stringLenPos))
				return;
			srcCmd >> stringLenPos;
			stringLenPos=swap16(stringLenPos);
			if (stringLenPos != 8)
			{
				WARNING_LOG(format("(%1) Chat packet stringLenPos not 8 but %2%, packet %3%") % m_parent.Address() % stringLenPos % Bin2Hex(srcCmd));
				return;
			}
			if (srcCmd.size() < stringLenPos)
				return;

			srcCmd.rpos(stringLenPos);
			uint16 messageLen;
			if (srcCmd.remaining() < sizeof(messageLen))
				return;
			srcCmd >> messageLen;
			if (srcCmd.remaining() < messageLen)
				return;
			vector<byte> messageBuf(messageLen);
			srcCmd.read(&messageBuf[0],messageBuf.size());
			string theMessage((const char*)&messageBuf[0],messageBuf.size()-1);

			INFO_LOG(format("%1% says %2%") % m_handle % theMessage);
			m_parent.QueueCommand(make_shared<SystemChatMsg>((format("You said %1%") % theMessage).str()));
			sGame.AnnounceCommand(&m_parent,make_shared<PlayerChatMsg>(m_handle,theMessage));
		}
	}
	else if (firstByte == 0x33) //stop animation
	{
		m_currAnimation = 0;
		m_parent.QueueState(make_shared<AnimationStateMsg>(m_goId));
		sGame.AnnounceStateUpdate(&m_parent,make_shared<AnimationStateMsg>(m_goId));
	}
	else if (firstByte == 0x34) //start animation
	{
		uint8 newAnimation;
		if (srcCmd.size() < sizeof(newAnimation))
			return;
		srcCmd >> newAnimation;
		m_currAnimation = newAnimation;
		m_parent.QueueState(make_shared<AnimationStateMsg>(m_goId));
		sGame.AnnounceStateUpdate(&m_parent,make_shared<AnimationStateMsg>(m_goId));
	}
	else if (firstByte == 0x35) //change mood
	{
		uint8 newMood;
		if (srcCmd.size() < sizeof(newMood))
			return;
		srcCmd >> newMood;
		m_currMood = newMood;	
		m_parent.QueueState(make_shared<AnimationStateMsg>(m_goId));
		sGame.AnnounceStateUpdate(&m_parent,make_shared<AnimationStateMsg>(m_goId));
	}
	else if (firstByte == 0x30) //perform emote
	{
		uint32 emoteId;
		uint32 emoteTarget;
		if (srcCmd.size() < sizeof(emoteId))
			return;
		srcCmd >> emoteId;
		if (srcCmd.size() < sizeof(emoteTarget))
			return;
		srcCmd >> emoteTarget;

		m_emoteCounter++;
		m_parent.QueueState(make_shared<EmoteMsg>(m_goId,emoteId,m_emoteCounter));
		sGame.AnnounceStateUpdate(&m_parent,make_shared<EmoteMsg>(m_goId,emoteId,m_emoteCounter));

		DEBUG_LOG(format("(%1%) %2%:%3% doing emote %4% on target %5% at coords %6%,%7%,%8%")
			% m_parent.Address()
			% m_handle
			% m_goId
			% Bin2Hex((const byte*)&emoteId,sizeof(emoteId),0)
			% Bin2Hex((const byte*)&emoteTarget,sizeof(emoteTarget),0)
			% m_pos.x % m_pos.y % m_pos.z );
	}
	else
	{
		srcCmd.rpos(0);
		DEBUG_LOG(format("(%1%) unknown 04 command: %2%") % m_parent.Address() % Bin2Hex(srcCmd) );
	}
}

vector<msgBaseClassPtr> PlayerObject::getCurrentStatePackets()
{
	vector<msgBaseClassPtr> tempVect;
	tempVect.push_back(make_shared<PlayerSpawnMsg>(m_goId));
	if (m_currAnimation != 0 || m_currMood != 0)
	{
		tempVect.push_back(make_shared<AnimationStateMsg>(m_goId));
	}
	return tempVect;
}