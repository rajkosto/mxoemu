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

#include "EncryptedPacket.h"

#pragma pack(1)

typedef struct  
{
	uint32 crc32; //crc32 of the whole packet after crc32
	uint16 length; //length of whole packet after timestamp (includes sequences)
	uint32 timeStamp; //32bit time_t
} encryptedPacketHeader_t;

void EncryptedPacket::fromCipherText(ByteBuffer cipherText,CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *decryptor)
{
	if (cipherText.size() < 1)
	{
		throw runtime_error("Invalid ciphertext");
	}

	if (decryptor == NULL)
	{
		throw invalid_argument("Invalid decryptor");
	}

	cipherText.rpos(0);
	byte theVector[16];
	cipherText.read(theVector,sizeof(theVector));
	vector<byte> encrypted;
	encrypted.resize(cipherText.size() - cipherText.rpos());
	cipherText.read(&encrypted[0],encrypted.size());

	decryptor->Resynchronize(theVector);
	
	string decrypted;
	CryptoPP::StringSource( string( (const char*)&encrypted[0],encrypted.size() ),
		true,
		new CryptoPP::StreamTransformationFilter(*decryptor, new CryptoPP::StringSink(decrypted)));

	encryptedPacketHeader_t packetHeader;
	memcpy(&packetHeader,decrypted.data(),sizeof(packetHeader));

	//strip crc32 (this gives us buffer of what we calculate crc against)
	decrypted = decrypted.substr(sizeof(packetHeader.crc32));

	CryptoPP::CRC32 hashMethod;
	string outputCrc;
	CryptoPP::StringSource(decrypted, true,new CryptoPP::HashFilter(hashMethod,new CryptoPP::StringSink(outputCrc)));

	//check computed crc against one in packet
	if (memcmp(&packetHeader.crc32,outputCrc.c_str(),sizeof(packetHeader.crc32)))
	{
		throw InvalidCRCException();
	}

	//strip length
	decrypted = decrypted.substr(sizeof(packetHeader.length));
	//strip timestamp
	decrypted = decrypted.substr(sizeof(packetHeader.timeStamp));

	//now we SHOULD have a packet of exactly LENGTH
	if (decrypted.size() != packetHeader.length)
	{
		throw out_of_range("Invalid packet length");
	}

	//this is our plaintext data
	this->clear();
	this->append((const byte*)decrypted.data(),decrypted.size());
}

ByteBuffer EncryptedPacket::toCipherText(CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *encryptor) const
{
	if (this->size() < 1)
	{
		throw runtime_error("Nothing to encrypt");
	}

	if (encryptor == NULL)
	{
		throw invalid_argument("Invalid encryptor");
	}

	uint32 timeStamp = (uint32)time(NULL);

	//this is what we need length of
	uint16 length = (unsigned short)this->size();

	//this is what we need crc32 of
	string crcMe =	string((const char*)&length,sizeof(length)) +
		string((const char*)&timeStamp,sizeof(timeStamp)) +
		string(this->contents(),this->size());

	string crcValue;
	CryptoPP::CRC32 hashMethod;
	CryptoPP::StringSource(crcMe, true,new CryptoPP::HashFilter(hashMethod,new CryptoPP::StringSink(crcValue)));

	//this is what we need to encrypt
	string encryptMe = crcValue + crcMe;

	//generate random IV
	byte vector[16];
	CryptoPP::AutoSeededRandomPool rng;
	rng.GenerateBlock( vector, sizeof(vector) );

	//set the IV
	encryptor->Resynchronize(vector);

	//encrypt
	string encrypted;
	CryptoPP::StringSource(encryptMe, true, new CryptoPP::StreamTransformationFilter(*encryptor, new CryptoPP::StringSink(encrypted)));

	//packet output should be vector + encrypted data
	ByteBuffer packetOutput;
	packetOutput.append(vector,sizeof(vector));
	packetOutput.append(encrypted);

	return packetOutput;
}
