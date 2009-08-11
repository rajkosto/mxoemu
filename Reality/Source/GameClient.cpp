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
#include "InPacket.h"
#include "GameResponses.h"
#include "Util.h"
#include "MersenneTwister.h"

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
	std::stringstream lol;
	lol << "AgentNum" << std::setfill('0') << std::setw(6) << random;
	name = std::string(lol.str().c_str(),lol.str().length());
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
			std::string decrypted;
			if (Decrypt(pData,nLength,decrypted))
			{
				client_sequence++;
				if (client_sequence == 4096)
					client_sequence=0;

				IncomingPacket thepaxor(decrypted);

				std::cout << "CSeq: " << thepaxor.client_sequence << " SSeq: " << thepaxor.server_sequence;
				std::string contents = thepaxor.ToString();
				std::cout << " " << Bin2Hex(contents) << std::endl;

				std::string::size_type loc = contents.find( "Spawnalot", 0 );
				if( loc != std::string::npos ) 
				{
					if (PlayerSetupState==0x7F)
						Send(std::string((const char *)rawData,sizeof(rawData)));
					else {} //WTF
				}

				loc = contents.find( "Spawnanother", 0 );
				if( loc != std::string::npos ) 
				{
					if (PlayerSetupState==0x7F)
						Send(std::string((const char *)rawData2,sizeof(rawData2)));
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
		}

		switch(numPackets)
		{
		case 1:
			for (unsigned int i=0;i<5;i++)
			{
				int clientlen=sizeof(_address);
				sendto(*_sock, (const char *)GAMEResponseTo1_1_2_3_4_5, sizeof(GAMEResponseTo1_1_2_3_4_5), 0, (struct sockaddr*)&_address, clientlen);
			}
			
			Send(std::string((const char *)GAMEResponseTo1_6,sizeof(GAMEResponseTo1_6)));
			Send(std::string((const char *)GAMEResponseTo1_7,sizeof(GAMEResponseTo1_7)));
			Send(std::string((const char *)GAMEResponseTo1_8,sizeof(GAMEResponseTo1_8)));
			Send(std::string((const char *)GAMEResponseTo1_9,sizeof(GAMEResponseTo1_9)));
			Send(std::string((const char *)GAMEResponseTo1_10,sizeof(GAMEResponseTo1_10)));
			Send(std::string((const char *)GAMEResponseTo1_11,sizeof(GAMEResponseTo1_11)));
			Send(std::string((const char *)GAMEResponseTo1_12,sizeof(GAMEResponseTo1_12)));
			Send(std::string((const char *)GAMEResponseTo1_13,sizeof(GAMEResponseTo1_13)));
			Send(std::string((const char *)GAMEResponseTo1_14,sizeof(GAMEResponseTo1_14)));
			Send(std::string((const char *)GAMEResponseTo1_15,sizeof(GAMEResponseTo1_15)));
			Send(std::string((const char *)GAMEResponseTo1_16,sizeof(GAMEResponseTo1_16)));
			Send(std::string((const char *)GAMEResponseTo1_17,sizeof(GAMEResponseTo1_17)));
			Send(std::string((const char *)GAMEResponseTo1_18,sizeof(GAMEResponseTo1_18)));
			Send(std::string((const char *)GAMEResponseTo1_19,sizeof(GAMEResponseTo1_19)));
			Send(std::string((const char *)GAMEResponseTo1_20,sizeof(GAMEResponseTo1_20)));
			break;
		case 2:
			PlayerSetupState = 1;
			Send(std::string((const char *)GAMEResponseTo2,sizeof(GAMEResponseTo2)));
			break;
		case 5:
			Send(std::string((const char *)GAMEResponseTo5,sizeof(GAMEResponseTo5)));
			break;
		case 6:
			PlayerSetupState = 0x1F;
			Send(std::string((const char *)GAMEResponseTo6_1_header,sizeof(GAMEResponseTo6_1_header)) +
				name + std::string((const char *)GAMEResponseTo6_1_footer,sizeof(GAMEResponseTo6_1_footer)));
			Send(std::string((const char *)GAMEResponseTo6_2,sizeof(GAMEResponseTo6_2)));
			Send(std::string((const char *)GAMEResponseTo6_4,sizeof(GAMEResponseTo6_4)));
			Send(std::string((const char *)GAMEResponseTo6_6,sizeof(GAMEResponseTo6_6)));
			break;
		case 9:
			PlayerSetupState = 0x7F;
/*			sendme.FromString(std::string((const char *)GAMEResponseTo9_1_header,sizeof(GAMEResponseTo9_1_header)));
			sendme.Append(name);
			sendme.Append(std::string((const char *)GAMEResponseTo9_1_footer,sizeof(GAMEResponseTo9_1_footer)));
			Send(sendme);*/
			Send(std::string((const char *)GAMEResponseTo9_2,sizeof(GAMEResponseTo9_2)));
			Handled_Session=true;
			break;
		}

		if (Handled_Session)
		{
			Send(std::string((const char *)GAMEResponseTo9_2,sizeof(GAMEResponseTo9_2)));
		}
	}
}

bool GameClient::Decrypt(char *pData, uint16 nLength,std::string &output)
{
	std::string vector(pData+1,16);
	TFDecrypt->Resynchronize((const byte *)vector.data());
	std::string input(pData+17,nLength-17);
	output.clear();
	CryptoPP::StringSource(input, true, new CryptoPP::StreamTransformationFilter(*TFDecrypt, new CryptoPP::StringSink(output)));
	//output now contains crc len time data
	std::string crc32 = std::string(output.data(),sizeof(uint32));
	uint16 length;
	memcpy(&length,output.data()+sizeof(uint32),sizeof(uint16));

	std::string computedcrc;
	CryptoPP::CRC32 hash;
	CryptoPP::StringSource(std::string(output.data()+sizeof(uint32),length+sizeof(uint32)+sizeof(uint16)), true,new CryptoPP::HashFilter(hash,new CryptoPP::StringSink(computedcrc)));

	if (memcmp(crc32.data(),computedcrc.data(),sizeof(uint32)))
		return false;
	
	output = std::string(output.data()+sizeof(uint32)+sizeof(uint16)+sizeof(uint32),length);
	return true;
}

void GameClient::Send(const std::string &contents)
{
	server_sequence++;

	if (server_sequence == 4096)
		server_sequence=0;

	OutgoingPacket packet(PlayerSetupState,client_sequence,server_sequence);
	packet.FromString(contents);

	std::string buffer = packet.ToString();
	uint16 lenght = (uint16)buffer.size();
	uint32 time = getTime();
	std::string input = std::string((const char*)&lenght,sizeof(lenght)) + std::string((const char*)&time,sizeof(time)) + buffer; //len+time+data
	std::string crc32;
	std::string output;

	CryptoPP::CRC32 hash;
	CryptoPP::StringSource(input, true,new CryptoPP::HashFilter(hash,new CryptoPP::StringSink(crc32))); //output2 now has crc
	output = crc32 + input; //add the rest onto the crc,output now contains crc len time data

	byte vector[16];

	for (uint32 i=0;i < sizeof(vector); i++)
	{
		vector[i] = rand()%255;
	}

	TFEncrypt->Resynchronize(vector);
	input.clear();
	CryptoPP::StringSource(output, true, new CryptoPP::StreamTransformationFilter(*TFEncrypt, new CryptoPP::StringSink(input))); // output now has encrypted shit
	input = char(0x01) + std::string((const char*)vector,sizeof(vector)) + input; // input has whole packet now

	int clientlen=sizeof(_address);
    sendto(*_sock, input.data(), (int)input.size(), 0, (struct sockaddr*)&_address,clientlen);
}
