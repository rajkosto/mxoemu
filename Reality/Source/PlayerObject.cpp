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
#include "PlayerObject.h"
#include "RsiData.h"
#include "Database/Database.h"
#include "GameServer.h"
#include "MessageTypes.h"
#include "Log.h"
#include "GameClient.h"
#include "Timer.h"
#include <boost/algorithm/string.hpp>

PlayerObject::PlayerObject( GameClient &parent,uint64 charUID ) :m_parent(parent),m_characterUID(charUID),m_spawnedInWorld(false)
{
	loadFromDB(true);

	m_goId=0;
	INFO_LOG(format("Player object for %1% constructed") % m_handle);
	testCount=0;
	m_lastStore = getTime();
	m_currAnimation=0;
	m_currMood=0;
	m_emoteCounter=0;
	m_jackoutRequested=false;

	setOnlineStatus(true);
}

void PlayerObject::loadFromDB( bool updatePos )
{
	//grab data from characters table
	{
		format sql = format("SELECT `handle`, `firstName`, `lastName`, `background`,\
							`x`, `y`, `z`, `rot`, \
							`healthC`, `healthM`, `innerStrC`, `innerStrM`,\
							`level`, `profession`, `alignment`, `pvpflag`, `exp`, `cash`, `district`, `adminFlags`\
							FROM `characters` WHERE `charId` = '%1%' LIMIT 1") % m_characterUID;

		scoped_ptr<QueryResult> result(sDatabase.Query(sql));

		if (!result)
			throw CharacterNotFound();

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

		if (updatePos)
		{
			m_pos.ChangeCoords(	field[4].GetDouble(),
				field[5].GetDouble(),
				field[6].GetDouble());
			m_pos.rot = field[7].GetDouble();
			m_savedPos = m_pos;
		}

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
		//commit position changes
		saveDataToDB();
		setOnlineStatus(false);

		INFO_LOG(format("Player object for %1%:%2% deconstructing") % m_handle % m_goId);
		sGame.AnnounceStateUpdate(&m_parent,make_shared<DeletePlayerMsg>(m_goId));
		sGame.AnnounceCommand(&m_parent,make_shared<SystemChatMsg>((format("Player %1% with object id %2% disconnected")%m_handle%m_goId).str()));
		
		m_spawnedInWorld=false;
	}
}

uint8 PlayerObject::getRsiData( byte* outputBuf, size_t maxBufLen ) const
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

	bool storeSuccess = sDatabase.Execute(format("UPDATE `characters` SET `x` = '%1%', `y` = '%2%', `z` = '%3%', `rot` = '%4%', `lastOnline` = NOW() WHERE `charId` = '%5%'")
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

void PlayerObject::setOnlineStatus( bool isOnline )
{
	sDatabase.Execute(format("UPDATE `characters` SET `lastOnline` = NOW(), `isOnline` = '%1%' WHERE `charId` = '%2%'")
		% int(isOnline)
		% m_characterUID );
}

void PlayerObject::InitializeWorld()
{
	m_lastTestedCommand = 1;
//m_district = 18;
	if (m_district > 17) m_district = 0;
	m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"Massive"));
	m_parent.QueueCommand(make_shared<SetExperienceCmd>(m_exp));
	m_parent.QueueCommand(make_shared<SetInformationCmd>(m_cash));
/*	m_parent.QueueCommand(make_shared<HexGenericMsg>("80b24e0008000802"));
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
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80865220060000000000000000000000000000000000000000210000000000230000000000"));*/
	//m_parent.QueueCommand(make_shared<EventURLCmd>("http://mxoemu.info/forum/index.php"));
	
	m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF0000}W{/c}{c:DD0000}e{/c}{c:EE0000}l{/c}{c:CC0000}c{/c}{c:BB0000}o{/c}{c:AA0000}m{/c}{c:990000}e{/c} {c:880000}B{/c}{c:770000}a{/c}{c:660000}c{/c}{c:550000}k{/c}{c:440000}.{/c}{c:330000}.{/c}{c:220000}.{/c}"));

}

void PlayerObject::UpdateAppearance()
{
	loadFromDB(false);
	sGame.AnnounceStateUpdate(NULL,make_shared<PlayerAppearanceMsg>(m_goId));
}

void PlayerObject::SpawnSelf()
{
	if (m_spawnedInWorld == false)
	{
		sGame.AnnounceStateUpdate(NULL,make_shared<PlayerSpawnMsg>(m_goId));
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

	//open doors that are opened
	vector<msgBaseClassPtr> openedDoorPackets = sObjMgr.GetAllOpenDoors(&m_parent);
	for (vector<msgBaseClassPtr>::iterator it=openedDoorPackets.begin();it!=openedDoorPackets.end();++it)
	{
		m_parent.QueueState(*it);
	}
}

void PlayerObject::HandleStateUpdate( ByteBuffer &srcData )
{
/*	testCount++;
	m_parent.QueueCommand(make_shared<SystemChatMsg>( (format("CMD %1%")%testCount).str() ));*/

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
		//m_parent.QueueState(make_shared<StateUpdateMsg>(m_goId,theStateData));
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
//	DEBUG_LOG(format(" HandleCommand: %1%")% Bin2Hex(srcCmd) );

	//set up handler ptrs
	if (!m_RPCbyte.size())
	{
		m_RPCbyte[0x33] = &PlayerObject::RPC_HandleStopAnimation;
		m_RPCbyte[0x34] = &PlayerObject::RPC_HandleStartAnimtion;
		m_RPCbyte[0x35] = &PlayerObject::RPC_HandleChangeMood;
		m_RPCbyte[0x30] = &PlayerObject::RPC_HandlePerformEmote;
	}
	if (!m_RPCshort.size())
	{
		m_RPCshort[0x2810] = &PlayerObject::RPC_HandleChat;
		m_RPCshort[0x2907] = &PlayerObject::RPC_HandleWhisper;
		m_RPCshort[0x80c7] = &PlayerObject::RPC_HandleDynamicObjInteraction;
		m_RPCshort[0x80c8] = &PlayerObject::RPC_HandleStaticObjInteraction;
		m_RPCshort[0x80c2] = &PlayerObject::RPC_HandleJump;
		m_RPCshort[0x80c9] = &PlayerObject::RPC_HandleRegionLoadedNotification;
		m_RPCshort[0x8108] = &PlayerObject::RPC_HandleReadyForWorldChange;
		m_RPCshort[0x8152] = &PlayerObject::RPC_HandleWho;
		m_RPCshort[0x8154] = &PlayerObject::RPC_HandleWhereAmI;
		m_RPCshort[0x8192] = &PlayerObject::RPC_HandleGetPlayerDetails;
		m_RPCshort[0x8194] = &PlayerObject::RPC_HandleGetBackground;
		m_RPCshort[0x8196] = &PlayerObject::RPC_HandleSetBackground;
		m_RPCshort[0x818e] = &PlayerObject::RPC_HandleHardlineTeleport;
		m_RPCshort[0x8151] = &PlayerObject::RPC_HandleObjectSelected;
		m_RPCshort[0x80fc] = &PlayerObject::RPC_HandleJackoutRequest;
		m_RPCshort[0x80fe] = &PlayerObject::RPC_HandleJackoutFinished;
	}

	uint8 firstByte = srcCmd.read<uint8>();

	try
	{
		if (m_RPCbyte.count(firstByte))
		{
			CALL_METHOD_PTR(this,m_RPCbyte[firstByte])(srcCmd);
			return;
		}
		else
		{
			if (srcCmd.remaining())
			{
				uint8 secondByte = srcCmd.read<uint8>();
				uint16 shortCommand = (uint16(firstByte) << 8) | (secondByte & 0xFF);
				if (m_RPCshort.count(shortCommand))
				{
					CALL_METHOD_PTR(this,m_RPCshort[shortCommand])(srcCmd);
					return;
				}
			}
		}
	}
	catch ( ByteBuffer::out_of_range )
	{
		srcCmd.rpos(0);
		DEBUG_LOG(format("(%1%) Out of range error processing RPC data: %2%") % m_parent.Address() % Bin2Hex(srcCmd) );
		return;
	}

	srcCmd.rpos(0);
	DEBUG_LOG(format("(%1%) unhandled RPC data: %2%") % m_parent.Address() % Bin2Hex(srcCmd) );
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

void PlayerObject::GoAhead(double distanceToGo)
{		
	//double angle = this->getPosition().getMxoRot();
	//string debugMsg = (format("X:%1% Y:%2% Z:%3% Rotation:%4% Angle:%5%") % this->getPosition().x % this->getPosition().y % this->getPosition().z % this->getPosition().rot % angle).str();		
	//this->getClient().QueueCommand(make_shared<WhisperMsg>("TW",debugMsg));

	LocationVector newLoc = this->getPosition();

	double xInc = 0;
	double zInc = 0;
	
	double sAngle = sin(newLoc.rot);
	xInc = distanceToGo * sAngle;
	zInc = sqrt(distanceToGo * distanceToGo - xInc * xInc);
	xInc *= 100;
	zInc *= 100;
	newLoc.x -= xInc;
	if (abs(newLoc.rot) > M_PI/2)
	{
		newLoc.z += zInc;
	}
	else
	{
		newLoc.z -= zInc;
	}
	this->setPosition(newLoc);
	sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId),true);
}

void PlayerObject::Update()
{
	checkAndStore();
	if (m_jackoutRequested && getTime() - m_jackoutRequestedTime >= 10) // 10 seconds for jackout
	{
		m_parent.QueueCommand(make_shared<HexGenericMsg>("80fd000000000000"));
		m_parent.FlushQueue();
		m_jackoutRequested=false;
		//hack, should see why client doesnt send jackout complete msg, instead of invalidating here
		m_parent.Invalidate();
	}
}