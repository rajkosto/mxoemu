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
													 `level`, `profession`, `alignment`, `pvpflag`, `exp`, `cash`, `district`, `adminFlags`\
													 FROM `characters` WHERE `charId` = '%1%' LIMIT 1") % m_characterUID) );

		if (result == NULL)
		{
			throw CharacterNotFound();
		}

		Field *field = result->Fetch();
		if (field[0].GetString() != NULL)
			m_handle = field[0].GetString();
		else
			throw CharacterNotFound();

		if (field[1].GetString() != NULL)
			m_firstName = field[1].GetString();
		else
			m_firstName = "NOFIRST";

		if (field[2].GetString() != NULL)
			m_lastName = field[2].GetString();
		else
			m_lastName = "NOLAST";

		if (field[3].GetString() != NULL)
			m_background = field[3].GetString();
		else
			m_background = "";

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
		m_prof = field[13].GetUInt32();
		m_alignment = field[14].GetUInt8();
		m_pvpflag = field[15].GetBool();
		m_exp = field[16].GetUInt64();
		m_cash = field[17].GetUInt64();
		m_district = field[18].GetUInt8();
		m_isAdmin = field[19].GetBool();
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
	m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Your Object Id is %1%")%m_goId).str()));
	sGame.AnnounceCommand(&m_parent,make_shared<SystemChatMsg>((format("Player %1% connected with object id %2%")%m_handle%m_goId).str()));
}

PlayerObject::~PlayerObject()
{
	if (m_spawnedInWorld == true)
	{
		INFO_LOG(format("Player object for %1%:%2% deconstructing") % m_handle % m_goId);
		sGame.AnnounceStateUpdate(&m_parent,make_shared<DeletePlayerMsg>(m_goId));
		sGame.AnnounceCommand(&m_parent,make_shared<SystemChatMsg>((format("Player %1% with object id %2% disconnected")%m_handle%m_goId).str()));
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
	m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"WinterSky3"));
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

/*	m_parent.QueueCommand(make_shared<SetOptionCmd>("PvPServer",bool(true)));
	m_parent.QueueCommand(make_shared<SetOptionCmd>("PvPMaxSafeLevel",uint32(15)));

	vector<string> theEvents;
	theEvents.push_back("Halloween_Event");
	theEvents.push_back("Winter3HalloweenFlyEyeTSEC");

	m_parent.QueueCommand(make_shared<SetOptionCmd>("WR_RezEvents",theEvents));

	m_parent.QueueCommand(make_shared<SetOptionCmd>("EventSlot1_Effect",string()));
	m_parent.QueueCommand(make_shared<SetOptionCmd>("EventSlot2_Effect",string("flyman_idle3")));
	m_parent.QueueCommand(make_shared<SetOptionCmd>("EventSlot3_Effect",string("fly_virus")));
	m_parent.QueueCommand(make_shared<SetOptionCmd>("FixedBinkIDOverride",uint16(0x20)));*/
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
	foreach(uint32 theGoId, sObjMgr.getAllGOIds())
	{
		//we already spawned
		if (theGoId == m_goId)
			continue;

		PlayerObject *theOtherObject = NULL;
		try
		{
			theOtherObject = sObjMgr.getGOPtr(theGoId);;
		}
		catch (ObjectMgr::ObjectNotAvailable)
		{
			continue;
		}

		if (theOtherObject == this)
			continue;

		foreach(const msgBaseClassPtr &thePacket, theOtherObject->getCurrentStatePackets())
		{
			m_parent.QueueState(thePacket);
		}
	}

	//open doors that are opened
/*	vector<msgBaseClassPtr> openedDoorPackets = sObjMgr.GetAllOpenDoors();
	for (vector<msgBaseClassPtr>::iterator it=openedDoorPackets.begin();it!=openedDoorPackets.end();++it)
	{
		m_parent.QueueState(*it);
	}*/
}

void PlayerObject::HandleStateUpdate( ByteBuffer &srcData )
{
/*	testCount++;
	m_parent.QueueCommand(make_shared<SystemChatMsg>( (format("CMD %1%")%testCount).str() ));*/
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
		WARNING_LOG(format("Client %1% Player %2%:%3% 03 doesn't have number 1 after viewId, packet: %4%") % m_parent.Address() % m_handle % m_goId % Bin2Hex(srcData));
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

#include <boost/algorithm/string.hpp>
using boost::iequals;

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
				WARNING_LOG(format("(%1%) Chat packet stringLenPos not 8 but %2%, packet %3%") % m_parent.Address() % stringLenPos % Bin2Hex(srcCmd));
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

			if (m_isAdmin && theMessage[0] == '!')
			{
				ParseAdminCommand(theMessage.substr(1));
				return;
			}
			else if (theMessage[0] == '&')
			{
				ParsePlayerCommand(theMessage.substr(1));
				return;
			}

			INFO_LOG(format("%1% says %2%") % m_handle % theMessage);
			m_parent.QueueCommand(make_shared<SystemChatMsg>((format("You said %1%") % theMessage).str()));
			sGame.AnnounceCommand(&m_parent,make_shared<PlayerChatMsg>(m_handle,theMessage));
			return;
		}
	}
	else if (firstByte == 0x29)
	{
		uint8 secondByte;
		if (srcCmd.remaining() < sizeof(secondByte))
			return;
		srcCmd >> secondByte;

		if (secondByte == 0x07) //whisper
		{
			uint8 thirdByte;
			if (srcCmd.remaining() < sizeof(thirdByte))
				return;
			srcCmd >> thirdByte;

			if (thirdByte != 0)
				WARNING_LOG(format("(%1%) Whisper packet third byte not 0 but %2%, packet %3%") % m_parent.Address() % uint32(thirdByte) % Bin2Hex(srcCmd));

			uint16 messageLenPos;
			if (srcCmd.remaining() < sizeof(messageLenPos))
				return;
			srcCmd >> messageLenPos;

			if (srcCmd.size() < messageLenPos)
				return;

			uint16 whisperCount;
			if (srcCmd.remaining() < sizeof(whisperCount))
				return;
			srcCmd >> whisperCount;
			whisperCount = swap16(whisperCount); //big endian in packet

			uint16 recipientStrLen;
			if (srcCmd.remaining() < sizeof(recipientStrLen))
				return;
			srcCmd >> recipientStrLen;

			if (srcCmd.remaining() < recipientStrLen)
				return;
			vector<byte> tempBuf(recipientStrLen);
			srcCmd.read(&tempBuf[0],tempBuf.size());

			string theRecipient = string((const char*)&tempBuf[0],tempBuf.size()-1);
			string serverPrefix = "SOE+MXO+Reality+";
			string::size_type prefixPos = theRecipient.find_first_of(serverPrefix);
			if (prefixPos != string::npos)
			{
				theRecipient = theRecipient.substr(prefixPos+serverPrefix.length());
			}

			if (srcCmd.rpos() != messageLenPos)
				WARNING_LOG(format("(%1%) Whisper packet size byte mismatch, packet %2%") % m_parent.Address() % Bin2Hex(srcCmd));

			uint16 messageStrLen;
			if (srcCmd.remaining() < sizeof(messageStrLen))
				return;
			srcCmd >> messageStrLen;

			if (srcCmd.remaining() < messageStrLen)
				return;
			tempBuf.resize(messageStrLen);
			srcCmd.read(&tempBuf[0],tempBuf.size());
			string theMessage = string((const char*)&tempBuf[0],tempBuf.size()-1);

			bool sentProperly=false;
			vector<uint32> objectLists = sObjMgr.getAllGOIds();
			for (int i=0;i<objectLists.size();i++)
			{
				PlayerObject* targetPlayer = NULL;
				try
				{
					targetPlayer = sObjMgr.getGOPtr(objectLists[i]);
				}
				catch (ObjectMgr::ObjectNotAvailable)
				{
					continue;
				}
				
				if (targetPlayer == NULL)
					continue;

				if (iequals(targetPlayer->getHandle(),theRecipient))
				{
					targetPlayer->getClient().QueueCommand(make_shared<WhisperMsg>(m_handle,theMessage));
					sentProperly=true;
				}
			}
			if (sentProperly)
			{
				INFO_LOG(format("%1% whispered to %2%: %3%") % m_handle % theRecipient % theMessage);
				m_parent.QueueCommand(make_shared<SystemChatMsg>((format("You whispered %1% to %2%")%theMessage%theRecipient).str()));
			}
			else
			{
				INFO_LOG(format("%1% sent whisper to disconnected player %2%: %3%") % m_handle % theRecipient % theMessage);
				m_parent.QueueCommand(make_shared<SystemChatMsg>((format("%1% is not online")%theRecipient).str()));
			}
			return;
		}
	}
	else if (firstByte == 0x33) //stop animation
	{
		m_currAnimation = 0;
		sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
		return;
	}
	else if (firstByte == 0x34) //start animation
	{
		uint8 newAnimation;
		if (srcCmd.size() < sizeof(newAnimation))
			return;
		srcCmd >> newAnimation;
		m_currAnimation = newAnimation;
		sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
		return;
	}
	else if (firstByte == 0x35) //change mood
	{
		uint8 newMood;
		if (srcCmd.size() < sizeof(newMood))
			return;
		srcCmd >> newMood;
		m_currMood = newMood;	
		sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
		return;
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
		sGame.AnnounceStateUpdate(NULL,make_shared<EmoteMsg>(m_goId,emoteId,m_emoteCounter));

		DEBUG_LOG(format("(%1%) %2%:%3% doing emote %4% on target %5% at coords %6%,%7%,%8%")
			% m_parent.Address()
			% m_handle
			% m_goId
			% Bin2Hex((const byte*)&emoteId,sizeof(emoteId),0)
			% Bin2Hex((const byte*)&emoteTarget,sizeof(emoteTarget),0)
			% m_pos.x % m_pos.y % m_pos.z );
		return;
	}
	else if (firstByte == 0x80)
	{
		uint8 secondByte;
		if (srcCmd.remaining() < sizeof(secondByte))
			return;
		srcCmd >> secondByte;

		if (secondByte == 0xc8) //interact with static object
		{
			uint32 staticObjId;
			uint16 interaction;

			if (srcCmd.remaining() < sizeof(staticObjId))
				return;
			srcCmd >> staticObjId;

			if (srcCmd.remaining() < sizeof(interaction))
				return;
			srcCmd >> interaction;

			INFO_LOG(format("(%1%) %2%:%3% interacting with object id %4% interaction %5%")
				% m_parent.Address()
				% m_handle
				% m_goId
				% Bin2Hex((const byte*)&staticObjId,sizeof(staticObjId),0)
				% uint32(interaction) );

			if (interaction == 0x03) //open door
			{
				//sObjMgr.OpenDoor(staticObjId);
				return;
			}
			else
			{
				return;
			}
		}
		else if (secondByte == 0xc2) //jump
		{
			LocationVector endPos;
			if (endPos.fromDoubleBuf(srcCmd) == false)
			{
				WARNING_LOG(format("(%1%) %2%:%3% jump packet doesn't have endPos: %4%")
					% m_parent.Address()
					% m_handle
					% m_goId
					% Bin2Hex(srcCmd) );
				return;
			}

			vector<byte> extraData(0x0B);
			if (srcCmd.remaining() < extraData.size())
			{
				WARNING_LOG(format("(%1%) %2%:%3% jump packet doesn't have extraData: %4%")
					% m_parent.Address()
					% m_handle
					% m_goId
					% Bin2Hex(srcCmd) );
				return;
			}
			srcCmd.read(&extraData[0],extraData.size());

			uint32 theTimeStamp = 0;
			if (srcCmd.remaining() < sizeof(theTimeStamp))
			{
				WARNING_LOG(format("(%1%) %2%:%3% jump packet doesn't have timestamp: %4%")
					% m_parent.Address()
					% m_handle
					% m_goId
					% Bin2Hex(srcCmd) );
				return;				
			}
			srcCmd >> theTimeStamp;

			DEBUG_LOG(format("(%1%) %2%:%3% jumping to %4%,%5%,%6% extra data %7% timestamp %8%")
				% m_parent.Address()
				% m_handle
				% m_goId
				% endPos.x % endPos.y % endPos.z
				% Bin2Hex(&extraData[0],extraData.size())
				% theTimeStamp );

			this->setPosition(endPos);
			sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
			return;
		}
		else if (secondByte == 0xc9)
		{
			//some weird coords inside ?
			return;
		}
	}
	else if (firstByte == 0x81)
	{
		uint8 secondByte;
		if (srcCmd.remaining() < sizeof(secondByte))
			return;
		srcCmd >> secondByte;

		if (secondByte == 0x08) //readyforworldchange
		{
			uint32 shouldBeZero;
			if (srcCmd.remaining() < sizeof(shouldBeZero))
				return;
			srcCmd >> shouldBeZero;

			if (shouldBeZero != 0)
				DEBUG_LOG(format("ReadyForWorldChange uint32 is %1%")%shouldBeZero);

			m_district = 0x03;
			InitializeWorld();
			SpawnSelf();
			return;
		}
		else if (secondByte == 0x54) //whereami
		{
			LocationVector clientSidePos;
			if (clientSidePos.fromFloatBuf(srcCmd) == false)
				return;

			bool byte1Valid = false;
			bool byte2Valid = false;
			bool byte3Valid = false;
			uint8 byte1,byte2,byte3;
			
			if (srcCmd.remaining() >= sizeof(byte1))
			{
				byte1Valid=true;
				srcCmd >> byte1;
			}
			if (srcCmd.remaining() >= sizeof(byte2))
			{
				byte2Valid=true;
				srcCmd >> byte2;
			}
			if (srcCmd.remaining() >= sizeof(byte3))
			{
				byte3Valid=true;
				srcCmd >> byte3;
			}

			INFO_LOG(format("(%1%) %2%:%3% requesting whereami clientPos %4%,%5%,%6% %7%:%8% %9%:%10% %11%:%12%")
				% m_parent.Address()
				% m_handle
				% m_goId
				% clientSidePos.x % clientSidePos.y % clientSidePos.z
				% byte1Valid % uint32(byte1)
				% byte2Valid % uint32(byte2)
				% byte3Valid % uint32(byte3) );

			m_parent.QueueCommand(make_shared<WhereAmIResponse>(m_pos));
			//m_parent.QueueCommand(make_shared<HexGenericMsg>("8107"));
			return;
		}
		else if (secondByte == 0x92) //get player details
		{
			uint32 zeroInt;
			if (srcCmd.remaining() < sizeof(zeroInt))
				return;
			srcCmd >> zeroInt;

			if (zeroInt != 0)
			{
				WARNING_LOG(format("Get player details zero int is %1%") % zeroInt );
				return;
			}

			uint16 playerNameStrLenPos;
			if (srcCmd.remaining() < sizeof(playerNameStrLenPos))
				return;
			srcCmd >> playerNameStrLenPos;

			if (playerNameStrLenPos != srcCmd.rpos())
			{
				WARNING_LOG(format("Get player details strlenpos not %1% but %2%") % srcCmd.rpos() % playerNameStrLenPos );
				return;
			}

			srcCmd.rpos(playerNameStrLenPos);
			uint16 playerNameStrLen;
			if (srcCmd.remaining() < sizeof(playerNameStrLen))
				return;
			srcCmd >> playerNameStrLen;
			if (playerNameStrLen < 1 || srcCmd.remaining() < playerNameStrLen)
				return;
			vector<byte> rawPlayerName(playerNameStrLen);
			srcCmd.read(&rawPlayerName[0],rawPlayerName.size());
			string thePlayerName = string((const char*)&rawPlayerName[0],rawPlayerName.size()-1);

			vector<uint32> objectList = sObjMgr.getAllGOIds();
			foreach(uint32 objId, objectList)
			{
				PlayerObject* targetPlayer = NULL;
				try
				{
					targetPlayer = sObjMgr.getGOPtr(objId);
				}
				catch (ObjectMgr::ObjectNotAvailable)
				{
					continue;
				}

				if (targetPlayer == NULL)
					continue;

				if (targetPlayer->getHandle() == thePlayerName)
				{
					m_parent.QueueCommand(make_shared<PlayerDetailsMsg>(targetPlayer));
					m_parent.QueueCommand(make_shared<PlayerBackgroundMsg>(targetPlayer->getBackground()));
					break;
				}
			}
			return;
		}
		else if (secondByte == 0x94) //get background
		{
			m_parent.QueueCommand(make_shared<BackgroundResponseMsg>(this->getBackground()));
			return;
		}
		else if (secondByte == 0x96) //set background
		{
			uint16 backgroundStrLenPos = 0;
			if (srcCmd.remaining() < sizeof(backgroundStrLenPos))
				return;
			srcCmd >> backgroundStrLenPos;

			if (srcCmd.size() < backgroundStrLenPos)
				return;
			srcCmd.rpos(backgroundStrLenPos);
			uint16 backgroundStrLen = 0;

			if (srcCmd.remaining() < sizeof(backgroundStrLen))
				return;
			srcCmd >> backgroundStrLen;

			if (backgroundStrLen < 1 || srcCmd.remaining() < backgroundStrLen)
				return;
			vector<byte> backgroundRawBuf(backgroundStrLen);
			srcCmd.read(&backgroundRawBuf[0],backgroundRawBuf.size());
			string theNewBackground = string((const char*)&backgroundRawBuf[0],backgroundRawBuf.size()-1);

			bool success = this->setBackground(theNewBackground);
			if (success)
			{
				INFO_LOG(format("(%1%) %2%:%3% changed background to |%4%|")
					% m_parent.Address()
					% m_handle
					% m_goId
					% this->getBackground() );
			}
			else
			{
				WARNING_LOG(format("(%1%) %2%:%3% background sql query update failed")
					% m_parent.Address()
					% m_handle
					% m_goId );
			}
			return;
		}
	}

	srcCmd.rpos(0);
	DEBUG_LOG(format("(%1%) unknown 04 command: %2%") % m_parent.Address() % Bin2Hex(srcCmd) );
}

bool PlayerObject::setBackground(string newBackground)
{
	m_background = newBackground;

	return sDatabase.Execute(format("UPDATE `characters` SET `background` = '%1%' WHERE `charId` = '%2%'")
		% sDatabase.EscapeString(this->getBackground())
		% m_characterUID );
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

void PlayerObject::ParseAdminCommand( string theCmd )
{
	stringstream cmdStream;
	cmdStream.str(theCmd);

	string command;
	cmdStream >> command;

	if (cmdStream.fail())
		return;

	if (iequals(command, "teleportPlayer") || iequals(command, "bringPlayer"))
	{
		string playerName;
		cmdStream >> playerName;

		if (cmdStream.fail() || playerName.length() < 1)
			return;

		if (iequals(command, "teleportPlayer") && cmdStream.eof())
			return;

		PlayerObject* theTargetPlayer = NULL;
		{
			vector<uint32> allObjects = sObjMgr.getAllGOIds();
			foreach(uint32 objId, allObjects)
			{
				PlayerObject* playerObj = NULL;
				try
				{
					playerObj = sObjMgr.getGOPtr(objId);
				}
				catch (ObjectMgr::ObjectNotAvailable)
				{
					continue;
				}

				if (iequals(playerName,playerObj->getHandle()))
				{
					theTargetPlayer = playerObj;
					break;
				}
			}
		}

		if (theTargetPlayer == NULL)
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Player %1% is not online")%playerName).str()));
			return;
		}

		LocationVector derp;

		if (iequals(command, "teleportPlayer"))
		{
			double x,y,z;
			cmdStream >> x;
			if (cmdStream.eof() || cmdStream.fail())
				return;
			cmdStream >> y;
			if (cmdStream.eof() || cmdStream.fail())
				return;
			cmdStream >> z;
			if (cmdStream.fail())
				return;

			x*=100;
			y*=100;
			z*=100;

			derp.ChangeCoords(x,y,z);
		}
		else if (iequals(command, "bringPlayer"))
		{
			LocationVector newPos = this->getPosition();
			derp.ChangeCoords(newPos.x,newPos.y,newPos.z,newPos.getMxoRot());
		}

		theTargetPlayer->setPosition(derp);
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(sObjMgr.getGOId(theTargetPlayer)));
		return;
	}
	else if (iequals(command, "teleportAll") || iequals(command, "bringAll"))
	{
		LocationVector derp;

		if (iequals(command, "teleportAll"))
		{
			double x,y,z;
			cmdStream >> x;
			if (cmdStream.eof() || cmdStream.fail())
				return;
			cmdStream >> y;
			if (cmdStream.eof() || cmdStream.fail())
				return;
			cmdStream >> z;
			if (cmdStream.fail())
				return;

			x*=100;
			y*=100;
			z*=100;

			derp.ChangeCoords(x,y,z);
		}
		else if (iequals(command, "bringAll"))
		{
			LocationVector newPos = this->getPosition();
			derp.ChangeCoords(newPos.x,newPos.y,newPos.z,newPos.getMxoRot());
		}

		vector<uint32> allObjects = sObjMgr.getAllGOIds();
		foreach(uint32 objId, allObjects)
		{
			PlayerObject* playerObj = NULL;
			try
			{
				playerObj = sObjMgr.getGOPtr(objId);
			}
			catch (ObjectMgr::ObjectNotAvailable)
			{
				continue;
			}

			if (playerObj == NULL)
				continue;

			playerObj->setPosition(derp);
			sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(objId));
		}
		return;
	}
	else
	{
		m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Unrecognized server command %1%")%command).str()));
		return;
	}
}

void PlayerObject::ParsePlayerCommand( string theCmd )
{
	stringstream cmdStream;
	cmdStream.str(theCmd);

	string command;
	cmdStream >> command;

	if (cmdStream.fail())
		return;

	if (iequals(command, "gotoPos"))
	{
		double x,y,z;
		cmdStream >> x;
		if (cmdStream.eof() || cmdStream.fail())
			return;
		cmdStream >> y;
		if (cmdStream.eof() || cmdStream.fail())
			return;
		cmdStream >> z;
		if (cmdStream.fail())
			return;

		x*=100;
		y*=100;
		z*=100;

		LocationVector derp(x,y,z);
		this->setPosition(derp);
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		return;
	}
	else if (iequals(command, "incX") || iequals(command, "incY") || iequals(command, "incZ"))
	{
		double incrementAmount=0;

		if (cmdStream.eof())
			incrementAmount = 1;

		cmdStream >> incrementAmount;

		if (cmdStream.fail())
			incrementAmount = 1;

		if (incrementAmount==0)
			return;

		incrementAmount*=100;

		double newX,newY,newZ;
		newX = this->getPosition().x;
		newY = this->getPosition().y;
		newZ = this->getPosition().z;

		if (iequals(command, "incX"))
			newX+=incrementAmount;
		else if (iequals(command, "incY"))
			newY+=incrementAmount;
		else if (iequals(command,"incZ"))
			newZ+=incrementAmount;
		
		LocationVector newPos(newX,newY,newZ);
		this->setPosition(newPos);
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		return;
	}
	else if (iequals(command, "gotoPlayer"))
	{
		string playerName;
		cmdStream >> playerName;

		if (cmdStream.fail())
			return;

		PlayerObject* theTargetPlayer = NULL;
		{
			vector<uint32> allObjects = sObjMgr.getAllGOIds();
			foreach(uint32 objId, allObjects)
			{
				PlayerObject* playerObj = NULL;
				try
				{
					playerObj = sObjMgr.getGOPtr(objId);
				}
				catch (ObjectMgr::ObjectNotAvailable)
				{
					continue;
				}

				if (iequals(playerName,playerObj->getHandle()))
				{
					theTargetPlayer = playerObj;
					break;
				}
			}
		}

		if (theTargetPlayer == NULL)
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Player %1% is not online")%playerName).str()));
			return;
		}

		this->setPosition(theTargetPlayer->getPosition());
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
	}
	else
	{
		m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Unrecognized server command %1%")%command).str()));
		return;
	}
}