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

#ifndef MXOSIM_SYMMETRICCRYPTO_H
#define MXOSIM_SYMMETRICCRYPTO_H

#include "Crypto.h"
#include "ByteBuffer.h"
#include "Util.h"

template <class CRYPTENGINE>
class SymmetricCryptEngine
{
public:
	SymmetricCryptEngine():valid(false)
	{
	}
	~SymmetricCryptEngine()
	{
		Invalidate();
	}

	bool IsValid() { return valid; }
	void Invalidate()
	{
		valid=false;
		m_encrypt.reset();
		m_decrypt.reset();
	}
	bool Initialize(const byte key[], size_t keySize)
	{
		if (keySize < CRYPTENGINE::BLOCKSIZE || keySize > CRYPTENGINE::MAX_KEYLENGTH)
		{
			Invalidate();
			return false;
		}
		byte blankIV[CRYPTENGINE::BLOCKSIZE];
		memset(blankIV,0,sizeof(blankIV));

		Invalidate();
		m_decrypt.reset(new Decryptor(key,keySize,blankIV));
		m_encrypt.reset(new Encryptor(key,keySize,blankIV));

		valid = true;
		return true;
	}
	bool SetEncryptionIV(const byte iv[], size_t ivSize)
	{
		if (ivSize != CRYPTENGINE::BLOCKSIZE || valid==false || m_encrypt == NULL)
		{
			return false;
		}
		m_encrypt->Resynchronize(iv,(int)ivSize);
		return true;
	}
	bool SetEncryptionIV()
	{
		if (valid == false || m_encrypt == NULL)
		{
			return false;
		}

		byte blankIV[CRYPTENGINE::BLOCKSIZE];
		memset(blankIV,0,sizeof(blankIV));
		m_encrypt->Resynchronize(blankIV,sizeof(blankIV));
		return true;
	}
	bool SetDecryptionIV(const byte iv[],size_t ivSize)
	{
		if (ivSize != CRYPTENGINE::BLOCKSIZE || valid==false || m_decrypt == NULL)
		{
			return false;
		}

		m_decrypt->Resynchronize(iv,(int)ivSize);
		return true;
	}
	bool SetDecryptionIV()
	{
		if (valid == false || m_decrypt == NULL)
		{
			return false;
		}

		byte blankIV[CRYPTENGINE::BLOCKSIZE];
		memset(blankIV,0,sizeof(blankIV));
		m_decrypt->Resynchronize(blankIV,sizeof(blankIV));
		return true;
	}
	ByteBuffer Encrypt(const byte *data, size_t dataSize,bool padding=true)
	{
		if (dataSize < 1 || valid==false)
		{
			return ByteBuffer();
		}

		string inputStr((const char*)data,dataSize);
		string outputStr;

		try
		{
			CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme paddingScheme = CryptoPP::BlockPaddingSchemeDef::NO_PADDING;
			if (padding == true)
			{
				paddingScheme = CryptoPP::BlockPaddingSchemeDef::DEFAULT_PADDING;
			}

			CryptoPP::StringSource(inputStr, true, 
				new CryptoPP::StreamTransformationFilter(
				*m_encrypt, new CryptoPP::StringSink(outputStr),
				paddingScheme));
		}
		catch (...)
		{
			return ByteBuffer();
		}

		return ByteBuffer(outputStr);
	}
	ByteBuffer Decrypt(const byte* data, size_t dataSize,bool padding=true)
	{
		if (dataSize < 1 || valid==false)
		{
			return ByteBuffer();
		}

		string inputStr((const char*)data,dataSize);
		string outputStr;

		try
		{
			CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme paddingScheme = CryptoPP::BlockPaddingSchemeDef::NO_PADDING;
			if (padding == true)
			{
				paddingScheme = CryptoPP::BlockPaddingSchemeDef::DEFAULT_PADDING;
			}

			CryptoPP::StringSource(inputStr, true, 
				new CryptoPP::StreamTransformationFilter(
				*m_decrypt, new CryptoPP::StringSink(outputStr),
				paddingScheme));
		}
		catch (...)
		{
			return ByteBuffer();
		}

		return ByteBuffer(outputStr);
	}

private:
	typedef typename CryptoPP::CBC_Mode<CRYPTENGINE>::Decryption Decryptor;
	typedef typename CryptoPP::CBC_Mode<CRYPTENGINE>::Encryption Encryptor;
	typedef shared_ptr<Decryptor> DecryptorPtr;
	typedef shared_ptr<Encryptor> EncryptorPtr;

	DecryptorPtr m_decrypt;
	EncryptorPtr m_encrypt;

	bool valid;
};

typedef CryptoPP::Twofish TwofishCryptMethod;
typedef SymmetricCryptEngine<TwofishCryptMethod> TwofishCryptEngine;

#endif