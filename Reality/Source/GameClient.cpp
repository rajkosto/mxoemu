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

#include "GameClient.h"
#include "Timer.h"
#include "GameResponses.h"
#include "Util.h"
#include "MersenneTwister.h"
#include "EncryptedPacket.h"
#include "SequencedPacket.h"
#include "RsiData.h"
#include "MarginSocket.h"
#include "MarginServer.h"
#include "Database/Database.h"
#include "Log.h"

#pragma pack(1)

GameClient::GameClient(struct sockaddr_in address, SOCKET *sock)
{
	_sock = sock;
	_address = address;
	server_sequence = 0;
	client_sequence = 0;
	numPackets = 0;
	PlayerSetupState = 0;
	marginConn = NULL;

	TFDecrypt=NULL;
	TFEncrypt=NULL;

	Valid_Client=true;
	Handled_Session=false;
	encryptionInitialized = false;
}

GameClient::~GameClient()
{
	if (TFDecrypt != NULL)
	{
		delete TFDecrypt;
		TFDecrypt = NULL;
	}
	if (TFEncrypt != NULL)
	{
		delete TFEncrypt;
		TFEncrypt = NULL;
	}
}

void GameClient::HandlePacket(char *pData, uint16 nLength)
{
	if (nLength < 1 || Valid_Client == false)
		return;

	_last_activity = getTime();
	numPackets++;

	if (encryptionInitialized == false && pData[0] == 0 && nLength == 43)
	{
		ByteBuffer packetData;
		packetData.append(pData,nLength);
		packetData.rpos(0x0B);
		if (packetData.remaining() < sizeof(characterUID))
		{
			Valid_Client = false;
			return;
		}
		packetData >> characterUID;
		
		//scope for auto_ptr
		{
			auto_ptr<QueryResult> result(sDatabase.Query("SELECT `handle`, `firstName`, `lastName`, `background`, `x`, `y`, `z` FROM `characters` WHERE `charId` = '%ull' LIMIT 1",characterUID) );
			if (result.get() == NULL)
			{
				ERROR_LOG("InitialUDPPacket(%s): Character doesn't exist",Address().c_str());
				Valid_Client = false;
				return;
			}

			Field *field = result->Fetch();
			m_handle = field[0].GetString();
			m_firstName = field[1].GetString();
			m_lastName = field[2].GetString();
			m_background = field[3].GetString();
			m_x = field[4].GetFloat();
			m_y = field[5].GetFloat();
			m_z = field[6].GetFloat();
		}

		marginConn = MarginServer::getSingleton().GetSocketByCharacterUID(characterUID);
		if (marginConn == NULL)
		{
			ERROR_LOG("InitialUDPPacket(%s): Margin session not found",Address().c_str());
			Valid_Client = false;
			return;
		}
		sessionId = marginConn->GetSessionId();
		charWorldId = marginConn->GetWorldCharId();

		//initialize encryptors with key from margin
		vector<byte> twofishKey = marginConn->GetTwofishKey();
		vector<byte> twofishIV(CryptoPP::Twofish::BLOCKSIZE,0);
		if (TFDecrypt!=NULL)
			delete TFDecrypt;
		if (TFEncrypt!=NULL)
			delete TFEncrypt;

		TFDecrypt=new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(&twofishKey[0], twofishKey.size(), &twofishIV[0]);
		TFEncrypt=new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption(&twofishKey[0], twofishKey.size(), &twofishIV[0]);

		//now we can verify if session key in this packet is correct
		packetData.rpos(packetData.size()-CryptoPP::Twofish::BLOCKSIZE);
		if (packetData.remaining() < CryptoPP::Twofish::BLOCKSIZE)
		{
			//wat
			Valid_Client=false;
			marginConn->ForceDisconnect();
			return;
		}
		vector<byte> encryptedSessionId(packetData.remaining());
		packetData.read(&encryptedSessionId[0],encryptedSessionId.size());
		string decryptedOutput;
		CryptoPP::StringSource(string( (const char*)&encryptedSessionId[0],encryptedSessionId.size() ), true, 
			new CryptoPP::StreamTransformationFilter(
			*TFDecrypt, new CryptoPP::StringSink(decryptedOutput),
			CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
		ByteBuffer decryptedData;
		decryptedData.append(decryptedOutput.data(),decryptedOutput.size());
		uint32 recoveredSessionId=0;
		decryptedData >> recoveredSessionId;

		if (recoveredSessionId != sessionId)
		{
			ERROR_LOG("InitialUDPPacket(%s): Session Key Mismatch",Address().c_str());
			Valid_Client = false;
			marginConn->ForceDisconnect();
			return;
		}

		encryptionInitialized=true;

		//send latency/loss calibration heartbeats
		const int numberOfBeats = 5;
		for (int i=0;i<numberOfBeats;i++)
		{
			ByteBuffer beatPacket;
			for (int j=0;j<numberOfBeats;j++)
			{
				beatPacket << uint8(0);
			}
			beatPacket << uint16(swap16(numberOfBeats));

			int clientlen=sizeof(_address);
			sendto(*_sock, beatPacket.contents(), beatPacket.size(), 0, (struct sockaddr*)&_address, clientlen);
		}

		//notify margin that udp session is established
		if (marginConn->UdpReady() == false)
		{
			ERROR_LOG("InitialUDPPacket(%s): Margin not ready for UDP connection",Address().c_str());
			encryptionInitialized=false;
			Valid_Client = false;
			marginConn->ForceDisconnect();
			return;
		}
	}

	if (Handled_Session == true && pData[0] != 0x01) // Ping...just reply with the same thing
	{
		int clientlen=sizeof(_address);
		sendto(*_sock, pData, nLength, 0, (struct sockaddr*)&_address,clientlen );
	}
	else
	{
		if (pData[0] == 0x01 && encryptionInitialized==true)
		{
			SequencedPacket packetData=Decrypt(&pData[1],nLength-1);
			client_sequence = packetData.getLocalSeq();

			cout << "CSeq: " << packetData.getLocalSeq() << " SSeq: " << packetData.getRemoteSeq();
			cout << " " << Bin2Hex(packetData) << endl;

			string contents = string(packetData.contents(),packetData.size());

			string::size_type loc = contents.find( "Spawnalot", 0 );
			if( loc != std::string::npos ) 
			{
				if (PlayerSetupState==0x7F)
					Send(ByteBuffer(rawData,sizeof(rawData)));
				else {} //WTF
				return;
			}

			loc = contents.find( "Spawnanother", 0 );
			if( loc != std::string::npos ) 
			{
				if (PlayerSetupState==0x7F)
					Send(ByteBuffer(rawData2,sizeof(rawData2)));
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnHats", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(8,8,SET_HATS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnFaces", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_FACES);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnGlassesColors", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(2,8,SET_GLASSESCOLORS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnGlasses", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_GLASSES);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnHairColors", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_HAIRCOLORS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnHairs", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_HAIRS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnFacialDetailColors", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(1,8,SET_FACIALDETAILCOLORS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnFacialDetails", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(2,8,SET_FACIALDETAILS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnLeggings", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(2,8,SET_LEGGINGS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnShirtColors", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(8,8,SET_SHIRTCOLORS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnShirts", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_SHIRTS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnPantsColors", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_PANTSCOLORS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnCoatColors", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_COATCOLORS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnCoats", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(8,8,SET_COATS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnPants", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_PANTS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnShoeColors", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(2,8,SET_SHOECOLORS);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnShoes", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(8,8,SET_SHOES);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnGloves", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(8,8,SET_GLOVES);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnSkinTones", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_SKINTONES);
				}
				else {} //WTF
				return;
			}

			loc = contents.find( "SpawnTattoos", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(1,8,SET_TATTOOS);
				}
				else {} //WTF
				return;
			}

			/*
			loc = contents.find( "Move", 0 );
			if( loc != std::string::npos ) 
			{
			if (PlayerSetupState==0x7F)
			Send(std::string((const char *)move,sizeof(move)));
			else {} //WTF
			}*/
		}

		switch(numPackets)
		{
		case 1:
			{
				Send(ByteBuffer(GAMEResponseTo1_6,sizeof(GAMEResponseTo1_6)));
				Send(ByteBuffer(GAMEResponseTo1_7,sizeof(GAMEResponseTo1_7)));
				Send(ByteBuffer(GAMEResponseTo1_8,sizeof(GAMEResponseTo1_8)));
				Send(ByteBuffer(GAMEResponseTo1_9,sizeof(GAMEResponseTo1_9)));
				Send(ByteBuffer(GAMEResponseTo1_10,sizeof(GAMEResponseTo1_10)));
				Send(ByteBuffer(GAMEResponseTo1_11,sizeof(GAMEResponseTo1_11)));
				Send(ByteBuffer(GAMEResponseTo1_12,sizeof(GAMEResponseTo1_12)));
				Send(ByteBuffer(GAMEResponseTo1_13,sizeof(GAMEResponseTo1_13)));
				Send(ByteBuffer(GAMEResponseTo1_14,sizeof(GAMEResponseTo1_14)));
				Send(ByteBuffer(GAMEResponseTo1_15,sizeof(GAMEResponseTo1_15)));
				Send(ByteBuffer(GAMEResponseTo1_16,sizeof(GAMEResponseTo1_16)));
				Send(ByteBuffer(GAMEResponseTo1_17,sizeof(GAMEResponseTo1_17)));
				Send(ByteBuffer(GAMEResponseTo1_18,sizeof(GAMEResponseTo1_18)));
				Send(ByteBuffer(GAMEResponseTo1_19,sizeof(GAMEResponseTo1_19)));
				Send(ByteBuffer(GAMEResponseTo1_20,sizeof(GAMEResponseTo1_20)));
				break;
			}
		case 2:
			{
				PlayerSetupState = 1;
				Send(ByteBuffer(GAMEResponseTo2,sizeof(GAMEResponseTo2)));
				break;
			}
		case 5:
			{
				Send(ByteBuffer(GAMEResponseTo5,sizeof(GAMEResponseTo5)));
				break;
			}
		case 6:
			{
				PlayerSetupState = 0x1F;

				byte GAMEResponseTo6_1Modified[200];
				memcpy(GAMEResponseTo6_1Modified,GAMEResponseTo6_1,sizeof(GAMEResponseTo6_1Modified));

				byte *nameInPacket = &GAMEResponseTo6_1Modified[0x55];
				memset(nameInPacket,0,32);
				memcpy(nameInPacket,m_handle.c_str(),m_handle.length()+1);

				double playerX,playerY,playerZ;
				playerX = m_x;
				playerY = m_y;
				playerZ = m_z;
				memcpy(&GAMEResponseTo6_1Modified[0x92],&playerX,sizeof(playerX));
				memcpy(&GAMEResponseTo6_1Modified[0x9A],&playerY,sizeof(playerY));
				memcpy(&GAMEResponseTo6_1Modified[0xA2],&playerZ,sizeof(playerZ));

				//change rsi data
				byte *rawPointer = &GAMEResponseTo6_1Modified[0x80];

				//load
				RsiData *playerRsi = NULL;
				//scope for auto_ptr
				{
					auto_ptr<QueryResult> result(sDatabase.Query("SELECT `sex`, `body`, `hat`, `face`, `shirt`,\
																 `coat`, `pants`, `shoes`, `gloves`, `glasses`,\
																 `hair`, `facialdetail`, `shirtcolor`, `pantscolor`,\
																 `coatcolor`, `shoecolor`, `glassescolor`, `haircolor`,\
																 `skintone`, `tattoo`, `facialdetailcolor`, `leggings` FROM `rsivalues` WHERE `charId` = '%ull' LIMIT 1",characterUID) );
					if (result.get() == NULL)
					{
						INFO_LOG("SpawnRSI(%s): Character's RSI doesn't exist",Address().c_str());
						playerRsi = new RsiDataMale;
						playerRsi->FromBytes(rawPointer,15);
					}
					else
					{
						Field *field = result->Fetch();
						uint8 sex = field[0].GetUInt8();

						if (sex == 0) //male
							playerRsi = new RsiDataMale;
						else
							playerRsi = new RsiDataFemale;

						RsiData &playerRef = *playerRsi;

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
						{
							playerRef["Leggings"] =	field[21].GetUInt8();
						}
					}
				}

				//save and delete
				playerRsi->ToBytes(rawPointer,15);
				delete playerRsi;

				Send(ByteBuffer(GAMEResponseTo6_1Modified,sizeof(GAMEResponseTo6_1Modified)));
				Send(ByteBuffer(GAMEResponseTo6_2,sizeof(GAMEResponseTo6_2)));
				Send(ByteBuffer(GAMEResponseTo6_4,sizeof(GAMEResponseTo6_4)));
				Send(ByteBuffer(GAMEResponseTo6_6,sizeof(GAMEResponseTo6_6)));
				break;
			}
		case 9:
			{
				PlayerSetupState = 0x7F;
				/*			sendme.FromString(std::string((const char *)GAMEResponseTo9_1_header,sizeof(GAMEResponseTo9_1_header)));
				sendme.Append(name);
				sendme.Append(std::string((const char *)GAMEResponseTo9_1_footer,sizeof(GAMEResponseTo9_1_footer)));
				Send(sendme);*/
				Send(std::string((const char *)GAMEResponseTo9_2,sizeof(GAMEResponseTo9_2)));
				Handled_Session=true;
				break;
			}
		}

		if (Handled_Session)
		{
			Send(std::string((const char *)GAMEResponseTo9_2,sizeof(GAMEResponseTo9_2)));
		}
	}
}

SequencedPacket GameClient::Decrypt(char *pData, uint16 nLength)
{
	EncryptedPacket decryptedData(ByteBuffer(pData,nLength),TFDecrypt);
	return SequencedPacket(decryptedData);
}

void GameClient::Send(const ByteBuffer &contents)
{
	if (!TFEncrypt)
		return;

	server_sequence++;

	if (server_sequence == 4096)
		server_sequence=0;

	SequencedPacket withSequences(server_sequence,client_sequence,PlayerSetupState,contents);
	EncryptedPacket withEncryption(withSequences.getDataWithHeader());
	ByteBuffer sendMe;
	sendMe << uint8(1);
	sendMe.append(withEncryption.toCipherText(TFEncrypt));

	int clientlen=sizeof(_address);
    sendto(*_sock, sendMe.contents(), (int)sendMe.size(), 0, (struct sockaddr*)&_address,clientlen);
}

void GameClient::SpawnTroop( int rows, int columns,WhatToSet typeToSet )
{
	for (int derp=0;derp < 2;derp++)
	{
		for (int i=0;i<rows;i++)
		{
			for (int j=0;j<columns;j++)
			{
				byte personData[243];
				memcpy(personData,rawData,sizeof(personData));
				int personNumber = (i*columns)+j;
				stringstream personName;
				switch (typeToSet)
				{
				case SET_HATS:
					personName << "Hat";
					break;
				case SET_FACES:
					personName << "Face";
					break;
				case SET_GLASSES:
					personName << "Glasses";
					break;
				case SET_HAIRS:
					personName << "Hair";
					break;
				case SET_SHIRTS:
					personName << "Shirt";
					break;
				case SET_FACIALDETAILS:
					personName << "FacialDetail";
					break;
				case SET_LEGGINGS:
					personName << "Leggings";
					break;
				case SET_SHIRTCOLORS:
					personName << "ShirtColor";
					break;
				case SET_PANTSCOLORS:
					personName << "PantsColor";
					break;
				case SET_COATS:
					personName << "Coat";
					break;
				case SET_PANTS:
					personName << "Pants";
					break;
				case SET_SHOES:
					personName << "Shoes";
					break;
				case SET_GLOVES:
					personName << "Gloves";
					break;
				case SET_COATCOLORS:
					personName << "CoatColor";
					break;
				case SET_HAIRCOLORS:
					personName << "HairColor";
					break;
				case SET_SKINTONES:
					personName << "SkinTone";
					break;
				case SET_TATTOOS:
					personName << "Tattoo";
					break;
				case SET_FACIALDETAILCOLORS:
					personName << "FacialDetailColor";
					break;
				case SET_GLASSESCOLORS:
					personName << "GlassesColor";
					break;
				case SET_SHOECOLORS:
					personName << "ShoeColor";
					break;
				}
				if (derp == 0)
				{
					personName << "Guy";
				}
				else
				{
					personName << "Gal";
				}
				personName << dec << personNumber;

				memcpy(&personData[0x6A],personName.str().c_str(),personName.str().size()+1);
				double personX,personY,personZ;
				personX = 27800 - (200*j);
				personY = -5;
				personZ = (-11700) - (200*(i+rows*derp));
				memcpy(&personData[0xC1],&personX,sizeof(personX));
				memcpy(&personData[0xC9],&personY,sizeof(personY));
				memcpy(&personData[0xD1],&personZ,sizeof(personZ));
				uint16 personHalfObjId = 6400 + random(0,59135);
				memcpy(&personData[0xF1],&personHalfObjId,sizeof(personHalfObjId));

				byte *rawPointer = &personData[0xAF];

				shared_ptr<RsiData> genderSpecificRsiData;
				if (derp == 0)
				{
					genderSpecificRsiData.reset(new RsiDataMale());
				}
				else if (derp == 1)
				{
					genderSpecificRsiData.reset(new RsiDataFemale());
				}

				RsiData &theRsiData = *genderSpecificRsiData;

				theRsiData.FromBytes(rawPointer,13);
				theRsiData["Sex"] = derp;
				theRsiData["Body"] = 1;
				theRsiData["Hat"] = 0;
				theRsiData["Face"] = 0;
				theRsiData["Shirt"] = 0;
				theRsiData["Coat"] = 0;
				theRsiData["Pants"] = 0;
				theRsiData["Shoes"] = 0;
				theRsiData["Gloves"] = 0;
				theRsiData["Glasses"] = 0;
				theRsiData["Hair"] = 0;
				theRsiData["FacialDetail"] = 0;
				theRsiData["ShirtColor"] = 0;
				theRsiData["PantsColor"] = 0;
				theRsiData["CoatColor"] = 0;
				theRsiData["HairColor"] = 0;
				theRsiData["SkinTone"] = 0;
				theRsiData["Tattoo"] = 0;
				theRsiData["FacialDetailColor"] = 0;

				switch (typeToSet)
				{
				case SET_HATS:
					theRsiData["Hat"] = personNumber;
					break;
				case SET_FACES:
					theRsiData["Face"] = personNumber;
					break;
				case SET_GLASSES:
					theRsiData["Hair"] = 5;
					theRsiData["Glasses"] = personNumber;
					break;
				case SET_HAIRS:
					theRsiData["Glasses"] = 0;
					theRsiData["Hair"] = personNumber;
					break;
				case SET_SHIRTS:
					theRsiData["Coat"] = 0;
					theRsiData["Shirt"] = personNumber;
					break;
				case SET_FACIALDETAILS:
					theRsiData["Glasses"] = 0;
					theRsiData["Hat"] = 0;
					theRsiData["FacialDetail"] = personNumber;
					break;
				case SET_LEGGINGS:
					theRsiData["Pants"] = 0;
					theRsiData["Shoes"] = 0;
					if (derp == 1) // cant do this for guys, invalid var
						theRsiData["Leggings"] = personNumber;
					break;
				case SET_SHIRTCOLORS:
					theRsiData["Coat"] = 0;
					theRsiData["Shirt"] = 2;
					theRsiData["ShirtColor"] = personNumber;
					break;
				case SET_PANTSCOLORS:
					theRsiData["Coat"] = 0;
					theRsiData["Shirt"] = 0;
					theRsiData["Pants"] = 1;
					theRsiData["PantsColor"] = personNumber;
					break;
				case SET_COATS:
					theRsiData["Coat"] = personNumber;
					break;
				case SET_PANTS:
					theRsiData["Coat"] = 0;
					theRsiData["Pants"] = personNumber;
					break;
				case SET_SHOES:
					theRsiData["Shirt"] = 0;
					theRsiData["Coat"] = 0;
					theRsiData["Shoes"] = personNumber;
					break;
				case SET_GLOVES:
					theRsiData["Shirt"] = 0;
					theRsiData["Coat"] = 0;
					theRsiData["Gloves"] = personNumber;
					break;
				case SET_COATCOLORS:
					theRsiData["Coat"] = 1;
					theRsiData["CoatColor"] = personNumber;
					break;
				case SET_HAIRCOLORS:
					theRsiData["Glasses"] = 0;
					theRsiData["Hair"] = 1;
					theRsiData["HairColor"] = personNumber;
					break;
				case SET_SKINTONES:
					theRsiData["Pants"] = 0;
					theRsiData["Coat"] = 0;
					theRsiData["Shirt"] = 0;
					theRsiData["SkinTone"] = personNumber;
					break;
				case SET_TATTOOS:
					theRsiData["Coat"] = 0;
					theRsiData["Shirt"] = 0;
					theRsiData["Pants"] = 0;
					theRsiData["SkinTone"] = 1;
					theRsiData["Tattoo"] = personNumber;
					break;
				case SET_FACIALDETAILCOLORS:
					theRsiData["Glasses"] = 0;

					theRsiData["Hat"] = 0;
					theRsiData["FacialDetail"] = 10;
					theRsiData["FacialDetailColor"] = personNumber;
					break;
				case SET_GLASSESCOLORS:
					theRsiData["Hat"] = 0;
					theRsiData["Glasses"] = 12;
					theRsiData["GlassesColor"] = personNumber;
					break;
				case SET_SHOECOLORS:
					theRsiData["Pants"] = 0;
					theRsiData["Shoes"] = 27;
					theRsiData["ShoeColor"] = personNumber;
					break;
				}
				theRsiData.ToBytes(rawPointer,13);

				Sleep(100);
				Send(ByteBuffer(personData,sizeof(personData)));
			}
		}
	}
}