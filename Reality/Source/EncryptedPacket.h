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

#ifndef MXOSIM_ENCRYPTEDPACKET_H
#define MXOSIM_ENCRYPTEDPACKET_H

#include "Common.h"
#include "SymmetricCrypto.h"
#include "ByteBuffer.h"

class InvalidCRCException {};

template <class CRYPTENGINE>
class EncryptedPacket : public ByteBuffer
{
public:
	EncryptedPacket() : ByteBuffer()
	{
	}
	EncryptedPacket(const ByteBuffer &copyMe) : ByteBuffer(copyMe)
	{
	}
	EncryptedPacket(ByteBuffer &cipherText,SymmetricCryptEngine<CRYPTENGINE> &decryptor)
	{
		fromCipherText(cipherText,decryptor);
	}
	~EncryptedPacket()
	{

	}

	void fromCipherText(ByteBuffer &cipherText,SymmetricCryptEngine<CRYPTENGINE> &decryptEngine)
	{
		if (cipherText.remaining() < CRYPTENGINE::BLOCKSIZE)
		{
			throw runtime_error("Invalid ciphertext");
		}

		if (decryptEngine.IsValid() == false)
		{
			throw invalid_argument("Invalid decryptor");
		}

		vector<byte> theVector(CRYPTENGINE::BLOCKSIZE);
		cipherText.read(&theVector[0],theVector.size());
		vector<byte> encrypted(cipherText.remaining());
		cipherText.read(&encrypted[0],encrypted.size());

		decryptEngine.SetDecryptionIV(&theVector[0],theVector.size());
		ByteBuffer decrypBuf = decryptEngine.Decrypt(&encrypted[0],encrypted.size(),true);

		uint32 ph_crc32; //crc32 of the whole packet after crc32
		uint16 ph_length; //length of whole packet after timestamp (includes sequences)
		uint32 ph_timeStamp; //32bit time_t

		decrypBuf >> ph_crc32;

		//strip crc32 (this gives us buffer of what we calculate crc against)
		string decrypted = string(&decrypBuf.contents()[decrypBuf.rpos()],decrypBuf.remaining());

		CryptoPP::CRC32 hashMethod;
		string outputCrc;
		CryptoPP::StringSource(decrypted, true,new CryptoPP::HashFilter(hashMethod,new CryptoPP::StringSink(outputCrc)));

		//check computed crc against one in packet
		if (memcmp(&ph_crc32,outputCrc.c_str(),sizeof(ph_crc32)))
		{
			throw InvalidCRCException();
		}

		//strip length and timestamp
		decrypBuf >> ph_length;
		decrypBuf >> ph_timeStamp;

		//now we SHOULD have a packet of exactly LENGTH
		if (decrypBuf.remaining() != ph_length)
		{
			throw out_of_range("Invalid packet length");
		}

		//this is our plaintext data
		this->clear();
		this->append((const byte*)&decrypBuf.contents()[decrypBuf.rpos()],ph_length);
	}
	ByteBuffer toCipherText(SymmetricCryptEngine<CRYPTENGINE> &encryptEngine)
	{
		if (this->size() < 1)
		{
			throw runtime_error("Nothing to encrypt");
		}

		if (encryptEngine.IsValid() == false)
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
		encryptEngine.SetEncryptionIV(vector,sizeof(vector));
		//encrypt
		ByteBuffer cryptedBuf = encryptEngine.Encrypt((byte*)encryptMe.data(),encryptMe.size(),true);

		//packet output should be vector + encrypted data
		ByteBuffer packetOutput;
		packetOutput.append(vector,sizeof(vector));
		packetOutput.append(cryptedBuf);

		return packetOutput;
	}
};

typedef EncryptedPacket<TwofishCryptMethod> TwofishEncryptedPacket;

#endif
