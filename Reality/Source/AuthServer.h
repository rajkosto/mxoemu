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

#ifndef MXOSIM_AUTHSERVER_H
#define MXOSIM_AUTHSERVER_H

#include "Singleton.h"
#include "AuthHandler.h"
#include "AuthSocket.h"
#include "Crypto.h"
#include "ByteBuffer.h"

#include <Sockets/ListenSocket.h>

class AuthServer : public Singleton <AuthServer>
{
public:
	AuthServer();;
	~AuthServer();;
	void Start();
	void Stop();
	void Loop();
	string Encrypt(string input);
	string Decrypt(string input);
	ByteBuffer SignWith1024Bit(byte *message,size_t messageLen);
	bool VerifyWith1024Bit(byte *message,size_t messageLen,byte *signature,size_t signatureLen);
	ByteBuffer GetPubKeyData();
	string HashPassword(const string& salt, const string& password);
	bool CreateAccount(const string& username,const string& password);
	bool ChangePassword(const string& username,const string& newPass);
	bool CreateWorld(const string& worldName);
	bool CreateCharacter(const string& worldName, const string& userName, const string& charHandle, const string& firstName, const string& lastName);
private:
	uint32 getAccountIdForUsername(const string &username);
	uint16 getWorldIdForName(const string &worldName);
	uint64 getCharIdForHandle(const string &handle);
	void GenerateRSAKeys(unsigned int keyLen,CryptoPP::RSA::PublicKey &publicOutput, CryptoPP::RSA::PrivateKey &privateOutput);
	void GenerateSignKeys(string &privKeyOut, string &pubKeyOut);
	void LoadSignKeys();
	ByteBuffer MessageFromPublicKey(CryptoPP::RSA::PublicKey &inputKey);
	void GenerateCryptoKeys(string &privKeyOut, string &pubKeyOut);
	void LoadCryptoKeys();

	string MakeSHA1HashHex(const string& input);
	string GenerateSalt(uint32 length);

	AuthHandler authSocketHandler;
	typedef ListenSocket<AuthSocket> AuthListenSocket;
	AuthListenSocket *listenSocketInst;

	CryptoPP::AutoSeededRandomPool randPool;

	CryptoPP::RSAES_OAEP_SHA_Decryptor rsaDecryptor;
	CryptoPP::RSAES_OAEP_SHA_Encryptor rsaEncryptor;

	typedef CryptoPP::Weak::RSASSA_PKCS1v15_MD5_Signer RSASigner;
	typedef CryptoPP::Weak::RSASSA_PKCS1v15_MD5_Verifier RSAVerifier;
	RSASigner signer1024bit;
	RSAVerifier verifier1024bit;
	RSASigner signer2048bit;
	RSAVerifier verifier2048bit;

	CryptoPP::Integer pubKeyModulus;
	vector<byte> pubKeySignature;
};


#define sAuth AuthServer::getSingleton()

#endif

