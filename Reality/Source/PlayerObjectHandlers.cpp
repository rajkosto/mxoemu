#include "Common.h"
#include "PlayerObject.h"
#include "Log.h"
#include "Database/Database.h"
#include "GameServer.h"
#include "GameClient.h"

#include <boost/algorithm/string.hpp>
using boost::iequals;

void PlayerObject::RPC_NullHandle( ByteBuffer &srcCmd )
{
	return;
}

void PlayerObject::RPC_HandleChat( ByteBuffer &srcCmd )
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

void PlayerObject::RPC_HandleWhisper( ByteBuffer &srcCmd )
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

void PlayerObject::RPC_HandleStopAnimation( ByteBuffer &srcCmd )
{
	m_currAnimation = 0;
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
	return;
}

void PlayerObject::RPC_HandleStartAnimtion( ByteBuffer &srcCmd )
{
	uint8 newAnimation;
	if (srcCmd.size() < sizeof(newAnimation))
		return;
	srcCmd >> newAnimation;
	m_currAnimation = newAnimation;
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
	return;
}

void PlayerObject::RPC_HandleChangeMood( ByteBuffer &srcCmd )
{
	uint8 newMood;
	if (srcCmd.size() < sizeof(newMood))
		return;
	srcCmd >> newMood;
	m_currMood = newMood;	
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
	return;
}

void PlayerObject::RPC_HandlePerformEmote( ByteBuffer &srcCmd )
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

void PlayerObject::RPC_HandleStaticObjInteraction( ByteBuffer &srcCmd )
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
		LocationVector loc = this->getPosition();

		scoped_ptr<QueryResult> resultDoorExists(sDatabase.Query(format("Select * from Doors Where `DistrictId` = '%1%' And `DoorId` = '%2%' Limit 1") % (int)getDistrict() % staticObjId));
		if (resultDoorExists == NULL)
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}You are using a door not in the database yet, lets add it{/c}"));	
			format sqlDoorInsert = 
				format("INSERT INTO `Doors` SET  `DistrictId` = '%1%', `DoorId` = '%2%', X = '%3%', Y = '%4%', Z = '%5%', ROT = '%6%', FirstUser = '%7%'")
				% (int)m_district
				% staticObjId
				% loc.x	% loc.y	% loc.z	% loc.rot
				% this->getHandle();

			if (sDatabase.Execute(sqlDoorInsert))
			{
				format msg1 = 
					format("{c:00FF00}Door:%1% in District %2% Set to Location X:%3% Y:%4% Z:%5% O:%6%{/c}")
					% staticObjId 
					% (int)m_district 
					% loc.x	% loc.y	% loc.z	% loc.rot;

				m_parent.QueueCommand(make_shared<SystemChatMsg>(msg1.str()));
			}
		}			
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

void PlayerObject::RPC_HandleJump( ByteBuffer &srcCmd )
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



void PlayerObject::RPC_HandleRegionLoadedNotification( ByteBuffer &srcCmd )
{
	vector<byte> fourBytes(4);
	srcCmd.read(fourBytes);
	LocationVector loc;
	if (!loc.fromFloatBuf(srcCmd))
		throw ByteBuffer::out_of_range();

	DEBUG_LOG(format("(%1%) %2%:%3% loaded region X:%4% Y:%5% Z:%6% extra: %7%")
		% m_parent.Address()
		% m_handle
		% m_goId
		% loc.x % loc.y % loc.z
		% Bin2Hex(fourBytes));
}

void PlayerObject::RPC_HandleReadyForWorldChange( ByteBuffer &srcCmd )
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

void PlayerObject::RPC_HandleWhereAmI( ByteBuffer &srcCmd )
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

void PlayerObject::RPC_HandleGetPlayerDetails( ByteBuffer &srcCmd )
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

void PlayerObject::RPC_HandleGetBackground( ByteBuffer &srcCmd )
{
	m_parent.QueueCommand(make_shared<BackgroundResponseMsg>(this->getBackground()));
	return;
}

void PlayerObject::RPC_HandleSetBackground( ByteBuffer &srcCmd )
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

void PlayerObject::RPC_HandleHardlineTeleport( ByteBuffer &srcCmd )
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
	}

	return;
}

void PlayerObject::RPC_HandleObjectSelected( ByteBuffer &srcCmd )
{
	uint32 objectId = srcCmd.read<uint32>();
	if (objectId)
	{
		DEBUG_LOG(format("(%s) %s:%d selected dynamic object data %08x")
			% m_parent.Address()
			% m_handle
			% m_goId
			% swap32(objectId));
	}
}