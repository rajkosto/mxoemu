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
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

PlayerObject::PlayerObject( GameClient &parent,uint64 charUID ) :m_parent(parent),m_characterUID(charUID),m_spawnedInWorld(false)
{
	//grab data from characters table
	{
		std::string sql;
		std::stringstream out;
		

		out << "SELECT `handle`,\
													 `firstName`, `lastName`, `background`,\
													 `x`, `y`, `z`, `rot`, \
													 `healthC`, `healthM`, `innerStrC`, `innerStrM`,\
													 `level`, `profession`, `alignment`, `pvpflag`, `exp`, `cash`, `district`, `adminFlags`\
													 FROM `characters` WHERE `charId` = '";
		out << m_characterUID;
		out << "' LIMIT 1";

		sql = out.str();

		scoped_ptr<QueryResult> result(sDatabase.Query(sql));

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
	//sGame.AnnounceCommand(&m_parent,make_shared<SystemChatMsg>((format("Player %1% connected with object id %2%")%m_handle%m_goId).str()));
	m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Player %1% connected with object id %2%")%m_handle%m_goId).str()));
}

PlayerObject::~PlayerObject()
{
	if (m_spawnedInWorld == true)
	{
		//commit position changes
		saveDataToDB();

		INFO_LOG(format("Player object for %1%:%2% deconstructing") % m_handle % m_goId);
		//sGame.AnnounceStateUpdate(&m_parent,make_shared<DeletePlayerMsg>(m_goId));
		m_parent.QueueState(make_shared<DeletePlayerMsg>(m_goId));
		//sGame.AnnounceCommand(&m_parent,make_shared<SystemChatMsg>((format("Player %1% with object id %2% disconnected")%m_handle%m_goId).str()));
		m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Player %1% with object id %2% disconnected")%m_handle%m_goId).str()));
		
		m_spawnedInWorld=false;

		
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

void PlayerObject::Update()
{
		bool storeSuccess = sDatabase.Execute(format("UPDATE `characters` SET `x` = '%1%', `y` = '%2%', `z` = '%3%', `rot` = '%4%' WHERE `charId` = '%5%'")
		% double(this->getPosition().x)
		% double(this->getPosition().y)
		% double(this->getPosition().z)
		% double(this->getPosition().rot)
		% m_characterUID );
//m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00}?update has been disabled cuz it crashes the server with too much use.  For now you have to log out and back in for each outfit change till I get it fixed -- TW{/c}"));

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
			
		m_parent.QueueCommand(make_shared<HexGenericMsg>("06"));
		InitializeWorld();
		m_spawnedInWorld = false;
		this->SpawnSelf();
		this->PopulateWorld();
}

void PlayerObject::GoDownTown()
{
	//InitializeWorld();
	m_spawnedInWorld = false;
	this->SpawnSelf();
	this->PopulateWorld();

	return;	
	//announce our presence to others
	sGame.AnnounceStateUpdate(&m_parent,make_shared<PlayerSpawnMsg>(m_goId));

	//Update DB for character  Set District 
	
	m_spawnedInWorld = false;
	m_district++; 
	if (m_district > 17) m_district = 0;
	m_parent.FlushQueue();
	string SQL = (format("UPDATE `characters` SET `District`='%1%' WHERE `charId`='%2%' LIMIT 1")
		% (int)m_district
		% (int)m_characterUID).str();
	sDatabase.Execute(SQL);
	m_parent.QueueCommand(make_shared<SystemChatMsg>(SQL));
	//m_parent.QueueCommand(make_shared<HexGenericMsg>("06"));
	//sGame.AnnounceStateUpdate(&m_parent,make_shared<DeletePlayerMsg>(m_goId));
	this->SpawnSelf();
	//
	//InitializeWorld();
	//m_parent.Reconnect();
	//m_parent.Reconnect();
	
	//m_parent.QueueCommand(make_shared<HexGenericMsg>("060e001200000022c3114901560046007265736f757263652f776f726c64732f66696e616c5f776f726c642f636f6e737472756374732f617263686976652f617263686976655f736174692f736174692e6d65747200280048616c6c6f7765656e5f4576656e742c57696e7465723348616c6c6f7765656e466c7954534543000a80e5e7cbc012000000000a80e43b6fda0000000000"));
	
/*
	m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"SatiSky"));
	m_parent.QueueCommand(make_shared<SetExperienceCmd>(m_exp));
	m_parent.QueueCommand(make_shared<SetInformationCmd>(m_cash));
*/	

	//m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"SatiSky"));
	//m_spawnedInWorld = false;
	//m_parent.m_characterSpawned = false;
	//m_parent.m_worldLoaded = false;
	//
	//m_parent.HandlePacket("0x038c9208", 43);
	//m_clients[IPStr]->HandlePacket(pData, len);

	
	//m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"SatiSky"));
	//this->getClient().QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"SatiSky"));

	//this->getClient().QueueCommand(make_shared<WhisperMsg>("TW","Pushing in Hex Test"));
	/*
	

	
	string hexString = "";
	//_itoa(m_lastTestedCommand,hexString,16);
	
	int i = m_lastTestedCommand;
	std::string s;
	std::stringstream out;
	std::stringstream outDec;
	outDec << i;
	out << hex << i;
	s = out.str();

	string debugMsg = "Dec:" + outDec.str();	
	this->getClient().QueueCommand(make_shared<WhisperMsg>("TW",debugMsg));

	bool odd = !!(s.length() & 1);

	if (odd)
	{
		s = "0" + s;
	}
	debugMsg = "HEX:" + s;


	this->getClient().QueueCommand(make_shared<WhisperMsg>("TW",debugMsg));

	m_parent.QueueCommand(make_shared<HexGenericMsg>(s));

	m_lastTestedCommand++;
	if (m_lastTestedCommand == 6) m_lastTestedCommand++; //6 is part of world load stuff.. freezes..
*/
	//EventURLCmd("http://www.johnhasson.com");
/*
	string line;
	ifstream myfile ("D:\\mxoTest.txt");
	if (myfile.is_open())
	{
		while (! myfile.eof() )
		{
			getline (myfile,line);		
			if (line.length() > 0)
			{
				erase_all(line, " ");
				//line = line.replace(" ", "");
				//m_parent.m_buf.
				m_parent.QueueCommand(make_shared<HexGenericMsg>(line));
				//m_parent.FlushQueue(true);
			}	
		}
		myfile.close();
	}*/
	//this->getClient().QueueCommand(make_shared<WhisperMsg>("TW","Done"));
	
	//m_district = 5;
	//m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)5,"SatiSky"));

	
	//m_parent.QueueCommand(make_shared<LoadWorldCmd>((LoadWorldCmd::mxoLocation)m_district,"SatiSky"));
	
	//m_parent.QueueCommand(make_shared<HexGenericMsg>("060e001200000022c3114901560046007265736f757263652f776f726c64732f66696e616c5f776f726c642f636f6e737472756374732f617263686976652f617263686976655f736174692f736174692e6d65747200280048616c6c6f7765656e5f4576656e742c57696e7465723348616c6c6f7765656e466c7954534543"));
	//m_parent.QueueCommand(make_shared<SetExperienceCmd>(m_exp));
	//m_parent.QueueCommand(make_shared<SetInformationCmd>(m_cash));
	/*m_parent.QueueCommand(make_shared<HexGenericMsg>("42"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("42"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("42"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("42"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("42"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("8222c3114904010000038080060e001200000022c3114901560046007265736f757263652f776f726c64732f66696e616c5f776f726c642f636f6e737472756374732f617263686976652f617263686976655f736174692f736174692e6d65747200280048616c6c6f7765656e5f4576656e742c57696e7465723348616c6c6f7765656e466c7954534543000a80e5e7cbc012000000000a80e43b6fda0000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("020401000302342e0700000000000000000000006400002e002400000000000000000000000000000000000e00416674657257686f72754e656f00418167170027001c2200c6011100000000000000370000080e00416674657257686f72754e656f000e00416674657257686f72754e656f0002004b00000000000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("020301000c0c008acdab239b81ff71020000416e646572736f6e00000000000000000000000000000000000000000000000001d054686f6d617300000000000000000000000000000000000000000000000000006666e63e850ad72340ffd8280a2500000801ce81fff700315e000002c66700416674657257686f72754e656f00000000000000000000000000000000000000280afd64010000670075d401004bb100000000231ae99080b00402028e000000f700dd0000004806d4d9400000000000c0574000000000c0f7d0c081ff32125606002890001089b8ca9323e31c011100000067000200000000"));
	m_parent.QueueCommand(make_shared<HexGenericMsg>("020301000c0c008acdab239b81ff71020000416e646572736f6e00000000000000000000000000000000000000000000000001d054686f6d617300000000000000000000000000000000000000000000000000006666e63e850ad72340ffd8280a2500000801ce81fff700315e000002c66700416674657257686f72754e656f00000000000000000000000000000000000000280afd64010000670075d401004bb100000000231ae99080b00402028e000000f700dd0000004806d4d9400000000000c0574000000000c0f7d0c081ff32125606002890001089b8ca9323e31c011100000067000200000000"));
*/
	//this->SpawnSelf();
	//this->PopulateWorld();


	

}

void PlayerObject::SpawnSelf()
{
	if (m_spawnedInWorld == false)
	{

		//first object spawn in world the client likes to control, so we have to self spawn first
		m_parent.QueueState(make_shared<PlayerSpawnMsg>(m_goId));
		
		//sGame.AnnounceStateUpdate(NULL,make_shared<PlayerSpawnMsg>(m_goId));
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
	DEBUG_LOG(format(" HandleCommand: %1%")% Bin2Hex(srcCmd) );

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
			//m_parent.QueueCommand(make_shared<PlayerChatMsg>(m_handle,theMessage));
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

			string debugStr = (format("(%1%) %2%:%3% interacting with object id %4% interaction %5%")
				% m_parent.Address()
				% m_handle
				% m_goId
				% Bin2Hex((const byte*)&staticObjId,sizeof(staticObjId),0)
				% uint32(interaction)).str();

				m_parent.QueueCommand(make_shared<SystemChatMsg>(debugStr));

			if (interaction == 0x03) //open door
			{
				//this->GoAhead(2);
#pragma region Add door if it isnt in the DB.. for now just use clients position soon we will add 1 U based on rotation to it..
			double X,Y,Z,O;
			X = this->getPosition().x;
			Y = this->getPosition().y;
			Z = this->getPosition().z;
			O = this->getPosition().rot;

			string sqlDoorExists = (format("Select * from Doors Where `DistrictId` = '%1%' And `DoorId` = '%2%' Limit 1") % (int)m_district % (int)staticObjId ).str();
			
			scoped_ptr<QueryResult> resultDoorExists(sDatabase.Query(sqlDoorExists));
			if (resultDoorExists == NULL)
			{
				m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}You are using a door not in the database yet, lets add it{/c}"));	
				string sqlDoorInsert = (format("INSERT INTO `Doors` SET  `DistrictId` = '%1%', `DoorId` = '%2%', X = '%3%', Y = '%4%', Z = '%5%', ROT = '%6%', FirstUser = '%7%'") % (int)m_district % (int)staticObjId % X % Y % Z % O % this->getHandle()).str();
				if (sDatabase.Execute(sqlDoorInsert))
				{
					string msg1 = (format("{c:00FF00}Door:%1% in District %2% Set to Location X:%3% Y:%4% Z:%5% O:%6%{/c}") % (int)staticObjId % (int)m_district % X % Y % Z % O).str();
					m_parent.QueueCommand(make_shared<SystemChatMsg>(msg1));
				}
				else
				{
					m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New Door location set, FAILED On INSERT.{/c}"));
				}

			}			
#pragma endregion


				sObjMgr.OpenDoor(staticObjId, &m_parent);		
				//sGame.AnnounceStateUpdate(NULL,make_shared<DeleteDoorMsg>(staticObjId));
				//sGame.AnnounceStateUpdate(NULL,make_shared<DeleteDoorMsg>(staticObjId));
				//uint16 viewId = m_goId;// sObjMgr.getViewForGO(&m_parent,staticObjId);
				//sGame.AnnounceStateUpdate(&m_parent,make_shared<DoorAnimationMsg>(staticObjId, viewId));
				this->GoAhead(1);
				return;
			}
			else if (interaction == 0x00) //sit
			{
				sGame.AnnounceStateUpdate(NULL,make_shared<SitDownMsg>(staticObjId));
				//m_parent.QueueState(make_shared<HexGenericMsg>("0311000108044044b5124700208ac47db717c60300020e045eccf1f846008098c3371d5ac60000"));
				//saiking siting response?
				//"03 11 00 01 08 04 40 44 b5 12 47 00 20 8a c4 7d b7 17 c6 03 00 02 0e 04 5e cc f1 f8 46 00 80 98 c3 37 1d 5a c6 00 00 ";
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
		else if (secondByte == 0x8e) //hardline
		{
			uint8 hardlineYouAreUsing;
			uint8 districtYouAreIn;
			uint8 hardlineLocation;
			uint8 hardlineDistrict;

			//Get hardlineYouAreUsing
			if (srcCmd.remaining() < sizeof(hardlineYouAreUsing)) return;
			srcCmd >> hardlineYouAreUsing;

			srcCmd.rpos(6);
			//Get District you are currently in
			if (srcCmd.remaining() < sizeof(districtYouAreIn)) return;
			srcCmd >> districtYouAreIn;
			
			srcCmd.rpos(10);
			//Get Hardline location
			if (srcCmd.remaining() < sizeof(hardlineLocation)) return;
			srcCmd >> hardlineLocation;

			srcCmd.rpos(14);
			//Get Hardline District
			if (srcCmd.remaining() < sizeof(hardlineDistrict)) return;
			srcCmd >> hardlineDistrict;

			string debugMsg = (format("You want to go to Hardline:%1% District:%2% From District:%3% Hardline:%4%") % (int)hardlineLocation % (int)hardlineDistrict % (int)districtYouAreIn % (int)hardlineYouAreUsing).str();
  			m_parent.QueueCommand(make_shared<SystemChatMsg>(debugMsg));

			//See if we need to add this HL to the DB
			double X,Y,Z,O;
			X = this->getPosition().x;
			Y = this->getPosition().y;
			Z = this->getPosition().z;
			O = this->getPosition().rot;

			string sqlHLExists = (format("Select * from Hardlines Where `DistrictId` = '%1%' And `HardlineId` = '%2%' Limit 1") % (int)districtYouAreIn % (int)hardlineYouAreUsing ).str();
			scoped_ptr<QueryResult> resultHLExists(sDatabase.Query(sqlHLExists));
			if (resultHLExists == NULL)
			{
				m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}You are at a hardline not in the database yet, lets add it so all can use it :){/c}"));	
				string sqlHLInsert = (format("INSERT INTO `hardlines` SET `DistrictId` = '%1%', `HardlineId` = '%2%', X = '%3%', Y = '%4%', Z = '%5%', HardlineName = 'Tagged By %6%', ROT = '%7%'") % (int)districtYouAreIn % (int)hardlineYouAreUsing % X % Y % Z % this->getHandle() % O).str();
				if (sDatabase.Execute(sqlHLInsert))
				{
					string msg1 = (format("{c:00FF00}HardlineId:%1% in District %7% Set to Tagged By %2% at X:%3% Y:%4% Z:%5% O:%6%{/c}") % (int)hardlineYouAreUsing % this->getHandle() % X % Y % Z % O % (int)districtYouAreIn).str();
					m_parent.QueueCommand(make_shared<SystemChatMsg>(msg1));
				}
				else
				{
					m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New hardline set, FAILED On INSERT.{/c}"));
				}

			}			


			string sql = (format("SELECT `X`,`Y`,`Z`, `ROT`, `HardlineName` FROM `hardlines` Where `DistrictId` = '%1%' And `HardlineId` = '%2%' LIMIT 1") % (int)hardlineDistrict % (int)hardlineLocation ).str();
			scoped_ptr<QueryResult> result(sDatabase.Query(sql));
			if (result == NULL)
			{
				m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF0000}The hardline you selected is not in the database yet, go tag it...{/c}"));	

			}
			else
			{
				Field *field = result->Fetch();
				double newX = field[0].GetDouble();
				double newY = field[1].GetDouble();
				double newZ = field[2].GetDouble();
				double newRot = field[3].GetDouble();
				string newlocationName = field[4].GetString();


				string message1 = (format("{c:00FFFF}Welcome to %1%.{/c}") % newlocationName ).str();
				m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:00FFFF}You may have to wait a min or two for the client and server to sync if the area is complex...{/c}"));
				
				LocationVector derp(newX, newY, newZ);
				derp.rot = newRot;
				this->setPosition(derp);
				//sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
				m_parent.QueueState(make_shared<PositionStateMsg>(m_goId));
				this->PopulateWorld();

				//this->Update();

				return;
			}

			//Pull new coords.

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
			playerObj->PopulateWorld();

		}
		return;
	}
	else if (iequals(command, "set"))
	{
		string area;
		cmdStream >> area;

		if (cmdStream.fail()) 
			return;


		std::string s;
		std::stringstream out;
		out << int(m_district);
		s = out.str();

		double X,Y,Z;
		X = this->getPosition().x;
		Y = this->getPosition().y;
		Z = this->getPosition().z;

		string sql1 = (format("DELETE FROM `locations` Where `District` = '%1%' And `Command` = '%2%'") % s % area ).str();
		string sql2 = (format("INSERT INTO `locations` SET `District` = '%1%', `Command` = '%2%', X = '%3%', Y = '%4%', Z = '%5%'") % s % area % X % Y % Z ).str();
		if (sDatabase.Execute(sql1))
		{
			if (sDatabase.Execute(sql2))
			{
			   m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}New location set, test it out.{/c}"));
			}
			else
			{
				m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New location set, FAILED On INSERT.{/c}"));
			}
		}
		else
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New location set, FAILED On DELETE.{/c}"));
		}

		return;
	

	}
	else if (iequals(command, "setHL"))
	{
		string hardlineId;
		cmdStream >> hardlineId;

		if (cmdStream.fail()) 
			return;

		string hardlineName;
		cmdStream >> hardlineName;

		if (cmdStream.fail()) 
			return;

		std::string s;
		std::stringstream out;
		out << int(m_district);
		s = out.str();

		double X,Y,Z,O;
		X = this->getPosition().x;
		Y = this->getPosition().y;
		Z = this->getPosition().z;
		O = this->getPosition().rot;

		string sql1 = (format("DELETE FROM `hardlines` Where `DistrictId` = '%1%' And `HardlineId` = '%2%'") % s % hardlineId ).str();
		string sql2 = (format("INSERT INTO `hardlines` SET `DistrictId` = '%1%', `HardlineId` = '%2%', X = '%3%', Y = '%4%', Z = '%5%', HardlineName = '%6%', ROT = '%7%'") % s % hardlineId % X % Y % Z % hardlineName % O).str();
		if (sDatabase.Execute(sql1))
		{
			if (sDatabase.Execute(sql2))
			{
				string msg1 = (format("{c:FFFF00}HardlineId:%1% Set to %2% at X:%3% Y:%4% Z:%5% O:%6%{/c}") % hardlineId % hardlineName % X % Y % Z % O ).str();
				m_parent.QueueCommand(make_shared<SystemChatMsg>(msg1));
			}
			else
			{
				m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New hardline set, FAILED On INSERT.{/c}"));
			}
		}
		else
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New hardline set, FAILED On DELETE.{/c}"));
		}

		return;
	

	}
	else
	{
		m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Unrecognized server command %1%")%command).str()));
		return;
	}
}

void PlayerObject::GoAhead(double distanceToGo)
{		
	//double angle = this->getPosition().getMxoRot();
	//string debugMsg = (format("X:%1% Y:%2% Z:%3% Rotation:%4% Angle:%5%") % this->getPosition().x % this->getPosition().y % this->getPosition().z % this->getPosition().rot % angle).str();		
	//this->getClient().QueueCommand(make_shared<WhisperMsg>("TW",debugMsg));

	double newX,newY,newZ, newO;
	newX = this->getPosition().x;
	newY = this->getPosition().y;
	newZ = this->getPosition().z;

	double xInc = 0;
	double zInc = 0;

	
	double sAngle = sin(this->getPosition().rot);
	xInc = distanceToGo * sAngle;
	zInc = sqrt(distanceToGo * distanceToGo - xInc * xInc);
	xInc *= 100;
	zInc *= 100;
	newX -= xInc;
	if (abs(this->getPosition().rot) > M_PI/2)
	{
		newZ += zInc;
	}
	else
	{
		newZ -= zInc;
	}

	LocationVector newPos(newX,newY,newZ);
	this->setPosition(newPos);
	m_parent.QueueState(make_shared<PositionStateMsg>(m_goId));
	
	//InitializeWorld();
	//m_parent.QueueCommand(make_shared<HexGenericMsg>("06"));
	//m_spawnedInWorld = false;
	//this->SpawnSelf();
	//this->PopulateWorld();
	//sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
	
	//targetPlayer->getClient().QueueCommand(make_shared<WhisperMsg>(m_handle,theMessage));

	//angle = this->getPosition().getMxoRot();
	//debugMsg = (format("X:%1% Y:%2% Z:%3% Rotation:%4% Angle:%5%") % this->getPosition().x % this->getPosition().y % this->getPosition().z % this->getPosition().rot % angle).str();		
	//this->getClient().QueueCommand(make_shared<WhisperMsg>("TW",debugMsg));
	return;
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

		m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}Teleported...{/c}"));
		LocationVector derp(x,y,z);
		this->setPosition(derp);

		//m_parent.QueueCommand(make_shared<HexGenericMsg>("06"));
		//m_spawnedInWorld = false;
		//this->SpawnSelf();
		m_parent.QueueState(make_shared<PositionStateMsg>(m_goId));
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
		//sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		m_parent.QueueState(make_shared<PositionStateMsg>(m_goId));
		return;
	}
	else if (iequals(command, "goThru"))
	{
		double incrementAmount=0;

		if (cmdStream.eof())
			incrementAmount = 2;

		cmdStream >> incrementAmount;

		this->GoAhead(incrementAmount);
		return;
	}
	else if (iequals(command, "downtown"))
	{
		this->GoDownTown();
		return;
	}
	else if (iequals(command, "state"))
	{
		string line;
		ifstream myfile ("D:\\mxoTest.txt");
		if (myfile.is_open())
		{
			while (! myfile.eof() )
			{
				getline (myfile,line);		
				if (line.length() > 0)
				{
					erase_all(line, " ");
					//line = line.replace(" ", "");
					//m_parent.m_buf.
					m_parent.QueueState(make_shared<HexGenericMsg>(line));
					//m_parent.FlushQueue(true);
				}	
			}
			myfile.close();
		}
	}
	else if (iequals(command, "command"))
	{
		string line;
		ifstream myfile ("D:\\mxoTest.txt");
		if (myfile.is_open())
		{
			while (! myfile.eof() )
			{
				getline (myfile,line);		
				if (line.length() > 0)
				{
					erase_all(line, " ");
					//line = line.replace(" ", "");
					//m_parent.m_buf.
					m_parent.QueueCommand(make_shared<HexGenericMsg>(line));
					//m_parent.FlushQueue(true);
				}	
			}
			myfile.close();
		}
	}
	else if (iequals(command, "announcestate"))
	{
		string line;
		ifstream myfile ("D:\\mxoTest.txt");
		if (myfile.is_open())
		{
			while (! myfile.eof() )
			{
				getline (myfile,line);		
				if (line.length() > 0)
				{
					erase_all(line, " ");
					//line = line.replace(" ", "");
					//m_parent.m_buf.
					sGame.AnnounceStateUpdate(NULL,make_shared<HexGenericMsg>(line));
					//m_parent.FlushQueue(true);
				}	
			}
			myfile.close();
		}
	}
	else if (iequals(command, "random"))
	{
		//Random Object Id
		uint32 randObjId = rand() % 0xFFFFFFFF;
		uint16 randViewId = rand() % 0xFFFF;

		//randObjId = 1310720002;   //mara church middle door

		sObjMgr.RandomObject(randObjId, &m_parent,this->getPosition().x, this->getPosition().y, this->getPosition().z, this->getPosition().rot );	

		//m_parent.QueueState(make_shared<DoorAnimationMsg>(randObjId, randViewId, this->getPosition().x, this->getPosition().y, this->getPosition().z, this->getPosition().rot, 1));

		
		return;
	}
	else if (iequals(command, "update"))
	{

		//m_parent.Reconnect();
		this->Update();
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
		//sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		m_parent.QueueState(make_shared<PositionStateMsg>(m_goId));
		this->PopulateWorld();

		//this->Update();
		return;
	}
	else if (iequals(command, "go"))
	{
		string area;
		cmdStream >> area;

		if (cmdStream.fail()) //Get list and whisper it but for now we just fail
			return;


		std::string s;
		std::stringstream out;
		out << int(m_district);
		s = out.str();

		string sql = (format("SELECT `X`,`Y`,`Z` FROM `locations` Where `District` = '%1%' And `Command` = '%2%' LIMIT 1") % s % area ).str();
		scoped_ptr<QueryResult> result(sDatabase.Query(sql));
		if (result == NULL)
		{
			return;
		}
		else
		{
			Field *field = result->Fetch();
			double newX = field[0].GetDouble();
			double newY = field[1].GetDouble();
			double newZ = field[2].GetDouble();

			string message1 = (format("{c:00FF00}Welcome to %1%.{/c}") % area ).str();
			m_parent.QueueCommand(make_shared<SystemChatMsg>(message1));
			m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}You may have to wait a min or two for the client and server to sync if the area is complex...{/c}"));
			LocationVector derp(newX,newY,newZ);
			this->setPosition(derp);

			//sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
			m_parent.QueueState(make_shared<PositionStateMsg>(m_goId));
			this->PopulateWorld();

			//this->Update();

			


			return;
		}

	}	
	else
	{
		m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Unrecognized server command %1%")%command).str()));
		return;
	}
}