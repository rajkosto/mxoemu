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

#pragma pack(1)

uint8 twofishkeyz[16] = {0x6C, 0xAB, 0x8E, 0xCC, 0xE7, 0x3C, 0x22, 0x47, 0xDB, 0xEB, 0xDE, 0x1A, 0xA8, 0xE7, 0x5F, 0xB8};


GameClient::GameClient(struct sockaddr_in address, SOCKET *sock)
{
	_sock = sock;
	_address = address;
	server_sequence = 0;
	client_sequence = 0;
	numPackets = 0;
	PlayerSetupState = 0;

	uint8 hax[CryptoPP::Twofish::BLOCKSIZE];
	memset(hax,0,CryptoPP::Twofish::BLOCKSIZE);
	TFDecrypt=new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(twofishkeyz, CryptoPP::Twofish::DEFAULT_KEYLENGTH, hax);
	TFEncrypt=new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption(twofishkeyz, CryptoPP::Twofish::DEFAULT_KEYLENGTH, hax);
	Valid_Client=true;
	Handled_Session=false;

	uint32 random = sRand.randInt(999999);
	stringstream lol;
	lol << "AgentNum" << random;
	name = lol.str();
}

GameClient::~GameClient()
{
	delete TFDecrypt;
	delete TFEncrypt;
}

void GameClient::HandlePacket(char *pData, uint16 nLength)
{
	_last_activity = getTime();
	numPackets++;

	if (Handled_Session == true && pData[0] != 0x01) // Ping...just reply with the same thing
	{
		int clientlen=sizeof(_address);
		sendto(*_sock, pData, nLength, 0, (struct sockaddr*)&_address,clientlen );
	}
	else
	{
		if (pData[0] == 0x01)
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
				for (unsigned int i=0;i<5;i++)
				{
					int clientlen=sizeof(_address);
					sendto(*_sock, (const char *)GAMEResponseTo1_1_2_3_4_5, sizeof(GAMEResponseTo1_1_2_3_4_5), 0, (struct sockaddr*)&_address, clientlen);
				}

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
				memcpy(nameInPacket,name.c_str(),name.length()+1);

				double playerX,playerY,playerZ;
				playerX = 27800;
				playerY = -5;
				playerZ = -11700;
				memcpy(&GAMEResponseTo6_1Modified[0x92],&playerX,sizeof(playerX));
				memcpy(&GAMEResponseTo6_1Modified[0x9A],&playerY,sizeof(playerY));
				memcpy(&GAMEResponseTo6_1Modified[0xA2],&playerZ,sizeof(playerZ));

				//change rsi data
				byte *rawPointer = &GAMEResponseTo6_1Modified[0x80];

				//load
				RsiDataMale playerRsi; //change this to RsiDataFemale if you want a girl
				playerRsi.FromBytes(rawPointer,13);
				//read/modify
				//read RsiData.h for a list of all the parameters
				playerRsi["Sex"] = 0; //also change this to 1 if you want a girl
				playerRsi["Shirt"] = 2;	
				playerRsi["ShirtColor"] = 14;
				playerRsi["Body"] = 2;
				playerRsi["Hat"] = 10;
				playerRsi["Pants"] = 2;
				playerRsi["PantsColor"] = 13;
				playerRsi["Hair"] = 5;
				playerRsi["Glasses"] = 7;
				playerRsi["Coat"] = 3;
				playerRsi["CoatColor"] = 10;
				//save
				playerRsi.ToBytes(rawPointer,13);

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
				}
				theRsiData.ToBytes(rawPointer,13);

				Sleep(100);
				Send(ByteBuffer(personData,sizeof(personData)));
			}
		}
	}
}