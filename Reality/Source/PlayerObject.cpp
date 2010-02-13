#include "Common.h"
#include "PlayerObject.h"
#include "RsiData.h"
#include "Database/Database.h"
#include "GameServer.h"
#include "MessageTypes.h"
#include "Log.h"
#include "GameClient.h"

PlayerObject::PlayerObject( GameClient &parent,uint64 charUID ) :m_parent(parent),m_characterUID(charUID),m_spawnedInWorld(false)
{
	//grab data from characters table
	{
		scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `handle`,\
													 `firstName`, `lastName`, `background`,\
													 `x`, `y`, `z`,\
													 `healthC`, `healthM`, `innerStrC`, `innerStrM`,\
													 `level`, `profession`, `alignment`, `pvpflag`, `exp`, `cash`, `district`\
													 FROM `characters` WHERE `charId` = '%1%' LIMIT 1") % m_characterUID) );

		if (result.get() == NULL)
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
		m_healthC = field[7].GetUInt16();
		m_healthM = field[8].GetUInt16();
		m_innerStrC = field[9].GetUInt16();
		m_innerStrM = field[10].GetUInt16();
		m_lvl = field[11].GetUInt8();
		m_prof = field[12].GetUInt8();
		m_alignment = field[13].GetUInt8();
		m_pvpflag = field[14].GetBool();
		m_exp = field[15].GetUInt64();
		m_cash = field[16].GetUInt64();
		m_district = field[17].GetUInt8();
	}
	//grab data from rsi table
	{
		scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `sex`, `body`, `hat`, `face`, `shirt`,\
													 `coat`, `pants`, `shoes`, `gloves`, `glasses`,\
													 `hair`, `facialdetail`, `shirtcolor`, `pantscolor`,\
													 `coatcolor`, `shoecolor`, `glassescolor`, `haircolor`,\
													 `skintone`, `tattoo`, `facialdetailcolor`, `leggings` FROM `rsivalues` WHERE `charId` = '%1%' LIMIT 1") % m_characterUID) );
		if (result.get() == NULL)
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
		sGame.AnnounceStateUpdate(&m_parent,new DeletePlayerMsg(m_goId));

		m_spawnedInWorld=false;
	}
}

uint8 PlayerObject::getRsiData( byte* outputBuf,uint32 maxBufLen ) const
{
	if (m_rsi.get() == NULL)
		return 0;

	return m_rsi->ToBytes(outputBuf,maxBufLen);
}

void PlayerObject::InitializeWorld()
{
	m_parent.QueueCommand(new LoadWorldCmd((LoadWorldCmd::mxoLocation)m_district,"SatiSky"));
	m_parent.QueueCommand(new SetExperienceCmd(m_exp));
	m_parent.QueueCommand(new SetInformationCmd(m_cash));
	m_parent.QueueCommand(new HexGenericMsg("80b24e0008000802"));
	m_parent.QueueCommand(new HexGenericMsg("80b2520005000802"));
	m_parent.QueueCommand(new HexGenericMsg("80b2540008000802"));
	m_parent.QueueCommand(new HexGenericMsg("80b24f0008000802"));
	m_parent.QueueCommand(new HexGenericMsg("80b251000b000802"));
	m_parent.QueueCommand(new HexGenericMsg("80b2110001000802"));
	m_parent.QueueCommand(new HexGenericMsg("80bc4503110000020000001100010000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80bc450002000002000000cc00000000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80bc1500030000f70300000802000000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80bc1500040000f70300000702ecffffff0000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80bc1500050000f70300005004000000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80bc1500060000f7030000f403000000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80bc1500070000f70300005104f6ffffff0000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80bc1500080000f703000052040f0000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("47000004010000000000000000000000000000001a0006000000010000000001010000000000800000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("2e0700000000000000000000005900002e00000000000000000000000000000000000000"));
	m_parent.QueueCommand(new HexGenericMsg("80865220060000000000000000000000000000000000000000210000000000230000000000"));
	m_parent.QueueCommand(new EventURLCmd("http://mxoemu.info/forum/index.php"));
	m_parent.QueueCommand(new SystemChatMsg("COME ON CHECK THIS OUT"));
}

void PlayerObject::SpawnSelf()
{
	if (m_spawnedInWorld == false)
	{
		m_parent.QueueState(new SelfSpawnMsg(m_goId));
		sGame.AnnounceStateUpdate(&m_parent,new PlayerSpawnMsg(m_goId));

		m_spawnedInWorld=true;
	}
}

void PlayerObject::HandleStateUpdate( ByteBuffer &srcData )
{
	srcData.rpos(0);
	DEBUG_LOG(format("unknown 03 command: %1%") % Bin2Hex(srcData) );
}

void PlayerObject::HandleCommand( ByteBuffer &srcCmd )
{
	uint16 cmdOpcode;
	if (srcCmd.remaining() < sizeof(cmdOpcode) )
		return;
	srcCmd >> cmdOpcode;
	switch (swap16(cmdOpcode))
	{
	case 0x2810:
		{
			uint16 chatType;
			if (srcCmd.remaining() < sizeof(chatType))
				return;
			srcCmd >> chatType;
			uint32 shouldBeZero;
			if (srcCmd.remaining() < sizeof(shouldBeZero))
				return;
			srcCmd >> shouldBeZero;
			uint16 messageLen;
			if (srcCmd.remaining() < sizeof(messageLen))
				return;
			srcCmd >> messageLen;
			if (srcCmd.remaining() < messageLen)
				return;
			vector<byte> messageBuf(messageLen);
			srcCmd.read(&messageBuf[0],messageBuf.size());
			string theMessage((const char*)&messageBuf[0],messageBuf.size()-1);

			INFO_LOG(format("%1% chat type %2% zeroes %3% says %4%") % m_handle % swap16(chatType) % shouldBeZero % theMessage);
			m_parent.QueueCommand(new SystemChatMsg(theMessage));

			break;
		}
	default:
		{
			srcCmd.rpos(0);
			DEBUG_LOG(format("unknown 04 command: %1%") % Bin2Hex(srcCmd) );
			break;
		}
	}
}