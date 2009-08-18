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
			}

			loc = contents.find( "Spawnanother", 0 );
			if( loc != std::string::npos ) 
			{
				if (PlayerSetupState==0x7F)
					Send(ByteBuffer(rawData2,sizeof(rawData2)));
				else {} //WTF
			}

			loc = contents.find( "SpawnHats", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(8,8,SET_HATS);
				}
				else {} //WTF
			}

			loc = contents.find( "SpawnFaces", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_FACES);
				}
				else {} //WTF
			}

			loc = contents.find( "SpawnGlasses", 0 );
			if (loc != std::string::npos )
			{
				if (PlayerSetupState==0x7F)
				{
					SpawnTroop(4,8,SET_GLASSES);
				}
				else {} //WTF
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
				memset(rawPointer,0,13);
				RsiData playerRsi(rawPointer);
				//read/modify
				playerRsi.setSex(0);
				//no idea what these do
				playerRsi.setShirt(0);
				playerRsi.setUnknown1(1); //seems to be full body suits
				playerRsi.setUnknown2(0);
				playerRsi.setUnknown3(0);				
				playerRsi.setBody(1);
				playerRsi.setHat(10);
				playerRsi.setPants(10);
				//save
				playerRsi.ToBytes(rawPointer);

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

				RsiData theRsiData(rawPointer);
				theRsiData.setSex(derp);
				theRsiData.setBody(1);
				theRsiData.setHat(0);
				theRsiData.setFace(0);
				theRsiData.setUnknown1(0);
				theRsiData.setShirt(0);
				theRsiData.setCoat(0);
				theRsiData.setPants(0);
				theRsiData.setShoes(0);
				theRsiData.setGloves(0);
				theRsiData.setGlasses(0);
				theRsiData.setHair(0);
				theRsiData.setFacialDetail(0);
				theRsiData.setShirtColor(0);
				theRsiData.setPantsColor(0);
				theRsiData.setCoatColor(0);
				theRsiData.setHairColor(0);
				theRsiData.setSkinTone(0);
				theRsiData.setTattoo(0);
				theRsiData.setFacialDetailColor(0);

				switch (typeToSet)
				{
				case SET_HATS:
					theRsiData.setHat(personNumber);
					break;
				case SET_FACES:
					theRsiData.setFace(personNumber);
					break;
				case SET_GLASSES:
					theRsiData.setHair(5);
					theRsiData.setGlasses(personNumber);
					break;
				}
				theRsiData.ToBytes(rawPointer);

				Sleep(100);
				Send(ByteBuffer(personData,sizeof(personData)));
			}
		}
	}
}