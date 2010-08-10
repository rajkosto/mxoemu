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


void PlayerObject::ParsePlayerCommand( string theCmd )
{
	stringstream cmdStream;
	cmdStream.str(theCmd);

	string command;
	cmdStream >> command;

	if (cmdStream.fail())
		return;

	using boost::erase_all;
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

void PlayerObject::RPC_HandleChat( ByteBuffer &srcCmd )
{
	uint16 stringLenPos = srcCmd.read<uint16>();
	stringLenPos = swap16(stringLenPos);

	if (stringLenPos != 8)
		WARNING_LOG(format("(%1%) Chat packet stringLenPos not 8 but %2%, packet %3%") % m_parent.Address() % stringLenPos % Bin2Hex(srcCmd));

	srcCmd.rpos(stringLenPos);
	string theMessage = srcCmd.readString();

	if (!theMessage.length())
		return;

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
}

void PlayerObject::RPC_HandleWhisper( ByteBuffer &srcCmd )
{
	uint8 thirdByte = srcCmd.read<uint8>();

	if (thirdByte != 0)
		WARNING_LOG(format("(%1%) Whisper packet third byte not 0 but %2%, packet %3%") % m_parent.Address() % uint32(thirdByte) % Bin2Hex(srcCmd));

	uint16 messageLenPos = srcCmd.read<uint16>();
	uint16 whisperCount = srcCmd.read<uint16>();
	whisperCount = swap16(whisperCount); //big endian in packet

	string theRecipient = srcCmd.readString();
	string serverPrefix = "SOE+MXO+Reality+";
	string::size_type prefixPos = theRecipient.find_first_of(serverPrefix);
	if (prefixPos != string::npos)
		theRecipient = theRecipient.substr(prefixPos+serverPrefix.length());

	if (srcCmd.rpos() != messageLenPos)
		WARNING_LOG(format("(%1%) Whisper packet size byte mismatch, packet %2%") % m_parent.Address() % Bin2Hex(srcCmd));

	string theMessage = srcCmd.readString();

	bool sentProperly=false;
	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach (int currObj, objectLists)
	{
		PlayerObject* targetPlayer = NULL;
		try
		{
			targetPlayer = sObjMgr.getGOPtr(currObj);
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
}

void PlayerObject::RPC_HandleStopAnimation( ByteBuffer &srcCmd )
{
	m_currAnimation = 0;
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
}

void PlayerObject::RPC_HandleStartAnimtion( ByteBuffer &srcCmd )
{
	uint8 newAnimation = srcCmd.read<uint8>();
	m_currAnimation = newAnimation;
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
}

void PlayerObject::RPC_HandleChangeMood( ByteBuffer &srcCmd )
{
	uint8 newMood = srcCmd.read<uint8>();
	m_currMood = newMood;	
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
	return;
}

void PlayerObject::RPC_HandlePerformEmote( ByteBuffer &srcCmd )
{
	uint32 emoteId = srcCmd.read<uint32>();
	uint32 emoteTarget = srcCmd.read<uint32>();

	m_emoteCounter++;
	sGame.AnnounceStateUpdate(NULL,make_shared<EmoteMsg>(m_goId,emoteId,m_emoteCounter));

	DEBUG_LOG(format("(%1%) %2%:%3% doing emote %4% on target %5% at coords %6%,%7%,%8%")
		% m_parent.Address()
		% m_handle
		% m_goId
		% Bin2Hex((const byte*)&emoteId,sizeof(emoteId),0)
		% Bin2Hex((const byte*)&emoteTarget,sizeof(emoteTarget),0)
		% m_pos.x % m_pos.y % m_pos.z );
}

void PlayerObject::RPC_HandleStaticObjInteraction( ByteBuffer &srcCmd )
{
	uint32 staticObjId = srcCmd.read<uint32>();
	uint16 interaction = srcCmd.read<uint16>();

	format debugStr = 
		format("(%s) %s:%d interacting with object id 0x%08x interaction %d")
		% m_parent.Address()
		% m_handle
		% m_goId
		% staticObjId
		% uint32(interaction);

	INFO_LOG( debugStr );

	m_parent.QueueCommand(make_shared<SystemChatMsg>(debugStr.str()));

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
					format("{c:00FF00}Door:0x%08x in District %d Set to Location X:%f Y:%f Z:%f O:%f{/c}")
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
//		sGame.AnnounceStateUpdate(NULL,make_shared<SitDownMsg>(staticObjId));
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
	srcCmd.read(extraData);

	uint32 theTimeStamp = srcCmd.read<uint32>();

	DEBUG_LOG(format("(%1%) %2%:%3% jumping to %4%,%5%,%6% extra data %7% timestamp %8%")
		% m_parent.Address()
		% m_handle
		% m_goId
		% endPos.x % endPos.y % endPos.z
		% Bin2Hex(&extraData[0],extraData.size())
		% theTimeStamp );

	this->setPosition(endPos);
	sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
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
	uint32 shouldBeZero = srcCmd.read<uint32>();

	if (shouldBeZero != 0)
		DEBUG_LOG(format("ReadyForWorldChange uint32 is %1%")%shouldBeZero);

	m_district = 0x03;
	InitializeWorld();
	SpawnSelf();
}

void PlayerObject::RPC_HandleWhereAmI( ByteBuffer &srcCmd )
{
	LocationVector clientSidePos;
	if (clientSidePos.fromFloatBuf(srcCmd) == false)
		throw ByteBuffer::out_of_range();

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
}

void PlayerObject::RPC_HandleGetPlayerDetails( ByteBuffer &srcCmd )
{
	uint32 zeroInt = srcCmd.read<uint32>();

	if (zeroInt != 0)
		WARNING_LOG(format("Get player details zero int is %1%") % zeroInt );

	uint16 playerNameStrLenPos = srcCmd.read<uint16>();

	if (playerNameStrLenPos != srcCmd.rpos())
	{
		WARNING_LOG(format("Get player details strlenpos not %1% but %2%") % (int)srcCmd.rpos() % playerNameStrLenPos );
		return;
	}

	srcCmd.rpos(playerNameStrLenPos);
	string thePlayerName = srcCmd.readString();

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
}

void PlayerObject::RPC_HandleGetBackground( ByteBuffer &srcCmd )
{
	m_parent.QueueCommand(make_shared<BackgroundResponseMsg>(this->getBackground()));
}

void PlayerObject::RPC_HandleSetBackground( ByteBuffer &srcCmd )
{
	uint16 backgroundStrLenPos = srcCmd.read<uint16>();

	srcCmd.rpos(backgroundStrLenPos);
	string theNewBackground = srcCmd.readString();

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
}

void PlayerObject::RPC_HandleHardlineTeleport( ByteBuffer &srcCmd )
{
	uint8 hardlineYouAreUsing;
	uint8 districtYouAreIn;
	uint8 hardlineLocation;
	uint8 hardlineDistrict;

	//Get hardlineYouAreUsing
	srcCmd >> hardlineYouAreUsing;

	srcCmd.rpos(6);
	//Get District you are currently in
	srcCmd >> districtYouAreIn;

	srcCmd.rpos(10);
	//Get Hardline location
	srcCmd >> hardlineLocation;

	srcCmd.rpos(14);
	//Get Hardline District
	srcCmd >> hardlineDistrict;

	format debugMsg = 
		format("You want to go to Hardline:%1% District:%2% From District:%3% Hardline:%4%")
		% (int)hardlineLocation 
		% (int)hardlineDistrict 
		% (int)districtYouAreIn 
		% (int)hardlineYouAreUsing;

	m_parent.QueueCommand(make_shared<SystemChatMsg>(debugMsg.str()));

	//See if we need to add this HL to the DB
	LocationVector loc = this->getPosition();

	format sqlHLExists = 
		format("Select * from Hardlines Where `DistrictId` = '%1%' And `HardlineId` = '%2%' Limit 1")
		% (int)districtYouAreIn 
		% (int)hardlineYouAreUsing;

	scoped_ptr<QueryResult> resultHLExists(sDatabase.Query(sqlHLExists));
	if (resultHLExists == NULL)
	{
		m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}You are at a hardline not in the database yet, lets add it so all can use it :){/c}"));	
		format sqlHLInsert = 
			format("INSERT INTO `hardlines` SET `DistrictId` = '%1%', `HardlineId` = '%2%', X = '%3%', Y = '%4%', Z = '%5%', ROT = '%6%', HardlineName = 'Tagged By %7%'")
			% (int)districtYouAreIn 
			% (int)hardlineYouAreUsing 
			% loc.x % loc.y % loc.z % loc.rot
			% this->getHandle();

		if (sDatabase.Execute(sqlHLInsert))
		{
			format msg1 = 
				format("{c:00FF00}HardlineId:%1% in District %7% Set to Tagged By %2% at X:%3% Y:%4% Z:%5% O:%6%{/c}")
				% (int)hardlineYouAreUsing 
				% this->getHandle() 
				% loc.x % loc.y % loc.z % loc.rot
				% (int)districtYouAreIn;

			m_parent.QueueCommand(make_shared<SystemChatMsg>(msg1.str()));
		}
		else
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New hardline set, FAILED On INSERT.{/c}"));
		}

	}

	format sql = 
		format("SELECT `X`,`Y`,`Z`, `ROT`, `HardlineName` FROM `hardlines` Where `DistrictId` = '%1%' And `HardlineId` = '%2%' LIMIT 1")
		% (int)hardlineDistrict 
		% (int)hardlineLocation;

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

		format message1 = format("{c:00FFFF}Welcome to %1%.{/c}") % newlocationName;
		m_parent.QueueCommand(make_shared<SystemChatMsg>(message1.str()));

		LocationVector newLoc(newX, newY, newZ);
		newLoc.rot = newRot;
		this->setPosition(newLoc);
		//sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		m_parent.QueueState(make_shared<PositionStateMsg>(m_goId));
		this->PopulateWorld();

		//this->Update();
	}
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