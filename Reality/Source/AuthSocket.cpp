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

#include "AuthSocket.h"
#include "Common.h"
#include "Util.h"
#include "ByteBuffer.h"
#include "AuthServer.h"
#include "Log.h"
#include "Timer.h"
#include "TCPVariableLengthPacket.h"
#include "Database/DatabaseEnv.h"
#include "SignedDataStruct.h"

AuthSocket::AuthSocket(ISocketHandler& h) : TCPVarLenSocket(h)
{
	matrixVersion = 0;
	packetNum = 0;
	memset(finalChallenge,0,sizeof(finalChallenge));
	memset(challenge,0,sizeof(challenge));
}

AuthSocket::~AuthSocket()
{
}

enum AuthOpcode
{
	AS_GetPublicKeyRequest = 0x06,
	AS_GetPublicKeyReply = 0x07,
	AS_AuthRequest = 0x08,
	AS_AuthChallenge = 0x09,
	AS_AuthChallengeResponse = 0x0A,
	AS_AuthReply = 0x0B,
	AS_CreateCharacterRequest = 0x0C,
	AS_CreateCharacterReply = 0x0D,
	AS_DeleteCharacterRequest = 0x0E,
	AS_DeleteCharacterReply = 0x0F,
	AS_LockAccountRequest = 0x10,
	AS_LockAccountReply = 0x11,
	AS_UnlockAccountRequest = 0x12,
	AS_UnlockAccountReply = 0x13,
	AS_PSAuthenticateRequest = 0x14,
	AS_WorldIdAndStatus = 0x15,
	AS_PSAuthenticateReply = 0x16,
	AS_PSCreateCharacterRequest = 0x17,
	AS_PSCreateCharacterReply = 0x18,
	AS_PSGetWorldListRequest = 0x19,
	AS_PSGetWorldListReply = 0x1A,
	AS_PSGetWorldPopulationsRequest = 0x1B,
	AS_PSGetWorldPopulationsReply = 0x1C,
	AS_GetAccountInfoRequest = 0x1D,
	AS_GetAccountInfoReply = 0x1E,
	AS_ProxyConnectRequest = 0x1F,
	AS_ProxyConnectReply = 0x20,
	AS_WorldShuttingDown = 0x21,
	AS_PSResetAccountInUseRequest = 0x22,
	AS_SetTransSessionPenaltyRequest = 0x23,
	AS_PSSetTransSessionPenaltyRequest = 0x24,
	AS_PSSetTransSessionPenaltyReply = 0x25,
	AS_SetWorldStatusRequest = 0x26,
	AS_SetWorldStatusReply = 0x27,
	AS_PSSetWorldStatusRequest = 0x28,
	AS_PSSetWorldStatusReply = 0x29,
	AS_UnlockAllAccountsRequest = 0x2A,
	AS_LockRecoveringAccountRequest = 0x2B,
	AS_LockRecoveringAccountReply = 0x2C,
	AS_RefreshCertificateRequest = 0x2D,
	AS_RefreshCertificateReply = 0x2E,
	AS_PSCertRefreshRequest = 0x2F,
	AS_PSCertRefreshReply = 0x30,
	AS_PSSetWorldVersionRequest = 0x31,
	AS_PSSetWorldVersionReply = 0x32,
	AS_IAmProxyLeader = 0x33,
	AS_RouteToAuthServer = 0x34,
	AS_GetWorldListRequest = 0x35,
	AS_GetWorldListReply = 0x36
};

void AuthSocket::ProcessData( const byte *buf,size_t len )
{
	ByteBuffer packetContents(buf,len);

	DEBUG_LOG(format("Auth Receieved |%1%|") % Bin2Hex(packetContents));

	byte packetOpcode;
	packetContents >> packetOpcode;
	AuthOpcode opcode = AuthOpcode(packetOpcode);

	switch (opcode)
	{
	default:
		{
			break;
		}
	case AS_GetPublicKeyRequest:
		{
			HandleGetPublicKeyRequest(packetContents);
			break;
		}
	case AS_AuthRequest:
		{
			HandleAuthRequest(packetContents);
			break;
		}
	case AS_AuthChallengeResponse:
		{
			HandleAuthChallengeResponse(packetContents);
			break;
		}
	}
}

bool AuthSocket::VerifyPassword( const string& plaintextPass, const string& passwordSalt, const string& passwordHash )
{
	if (sAuth.HashPassword(passwordSalt,plaintextPass) == passwordHash)
		return true;

	return false;
}

void AuthSocket::HandleGetPublicKeyRequest( ByteBuffer &packet )
{
	packet >> matrixVersion;
	string clientVersionStr = ClientVersionString(matrixVersion);
	if (clientVersionStr != "7.5668" )
	{
		WARNING_LOG(format("Auth client connected with unknown version %1%") % clientVersionStr );
	}
	uint32 rsaVersion;
	packet >> rsaVersion;

	TCPVariableLengthPacket awesome;

	if (rsaVersion == 4)
	{
		awesome		
			<< byte(AS_GetPublicKeyReply)
			<< uint32(0) 
			<< uint32(getTime())	//time
			<< uint32(0x04)			//rsa method
			<< byte(0) 
			<< uint32(0);
	}
	else
	{
		awesome
			<< byte(AS_GetPublicKeyReply)
			<< uint32(0)
			<< uint32(getTime())	//time
			<< uint32(0x04)			//rsa method
			<< uint8(0x12)
			<< uint8(0x00)
			<< uint8(0x11) //public exponent
			<< uint8(0x94)
			<< uint8(0x00);

		awesome.append(sAuth.GetPubKeyData());
	}

	SendPacket(awesome);

	DEBUG_LOG(format("Sending AS_GetPublicKeyReply: |%1%|") % Bin2Hex(awesome));
}

void AuthSocket::HandleAuthRequest( ByteBuffer &packet )
{
#pragma pack(push,1)
	typedef struct 
	{
		uint32 rsaType;
		uint32 unknownz1;
		char unknownz2[31];
		uint16 blobLen;
	}requestHdr;
#pragma pack(pop)

	requestHdr requestHeader;
	memset(&requestHeader,0,sizeof(requestHeader));
	packet.read((uint8 *)&requestHeader,sizeof(requestHeader));

	vector<byte> encryptedBlob;
	encryptedBlob.resize(requestHeader.blobLen);
	packet.read(&encryptedBlob[0],encryptedBlob.size());

	string decryptedBlob;

	try
	{
		decryptedBlob = sAuth.Decrypt(string((const char*)&encryptedBlob[0],encryptedBlob.size()));
	}
	catch (CryptoPP::InvalidCiphertext)
	{
		ERROR_LOG("Invalid RSA ciphertext, client used bad pubkey.dat, disconnecting.");
		SetCloseAndDelete(true);
		return;
	}

	DEBUG_LOG(format("Got encrypted Blob: |%1%|") % Bin2Hex(decryptedBlob));

	ByteBuffer rsaBlobBuffer(decryptedBlob.substr(sizeof(byte)));

	//should error out if not 4
	uint32 rsaBlobMethod;
	rsaBlobBuffer >> rsaBlobMethod;
	if (rsaBlobMethod != 4)
	{
		WARNING_LOG("rsaMethod in rsaBlob not 4!");
	}
	uint16 someShort;
	rsaBlobBuffer >> someShort;
	DEBUG_LOG(format("someShort is %1%") % someShort);

	byte twofishKey[16];
	rsaBlobBuffer.read(twofishKey,sizeof(twofishKey));
	DEBUG_LOG(format("Auth TF key: |%s|") % Bin2Hex(twofishKey,sizeof(twofishKey)));

	//instantiate ciphers
	m_tfEngine.Initialize(twofishKey,sizeof(twofishKey));

	//should check this time to be the same as the one we sent it in previous packet
	uint32 theTime;
	rsaBlobBuffer >> theTime;
	time_t expandedTime = theTime;
	DEBUG_LOG(format("time is: %1%") % ctime(&expandedTime));

	uint16 usernameLen;
	rsaBlobBuffer >> usernameLen;
	vector<char> usernameVect;
	usernameVect.resize(usernameLen);
	rsaBlobBuffer.read((uint8 *)&usernameVect[0],usernameVect.size());
	string theUsername(&usernameVect[0],usernameVect.size()-1);
	DEBUG_LOG(format("Username: |%1%|") % theUsername);

	//copy to variable
	m_username = theUsername;

	//scope for db ptr
	{
		scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `userId`, `username`, `passwordSalt`, `passwordHash`, `publicExponent`, `publicModulus`, `privateExponent`, `timeCreated` FROM `users` WHERE `username` = '%1%' LIMIT 1") % sDatabase.EscapeString(m_username) ) );
		if (result == NULL)
		{
			INFO_LOG(format("Username %1% doesn't exist, disconnecting.") % m_username );
			SetCloseAndDelete(true);
			return;
		}

		Field *field = result->Fetch();
		m_userId = field[0].GetUInt32();
		m_username = field[1].GetString();
		m_passwordSalt = field[2].GetString();
		m_passwordHash = field[3].GetString();
		m_publicExponent = field[4].GetUInt16();

		const char *pubModulusStr = field[5].GetString();
		if (pubModulusStr != NULL)
			m_publicModulus = string(pubModulusStr,96);
		else
			m_publicModulus.clear();

		const char *privExponentStr = field[6].GetString();
		if (privExponentStr != NULL)
			m_privateExponent = string(field[6].GetString(),96);
		else
			m_privateExponent.clear();

		m_timeCreated = field[7].GetUInt32();
	}


	//now, for the reply
	//reply consists entirely of challenge, which is used to make twofish IV and encrypt challenge response

	//create challenge
	byte decryptedChallenge[16];
	CryptoPP::AutoSeededRandomPool randPool;
	randPool.GenerateBlock(decryptedChallenge,sizeof(decryptedChallenge));

	//to create challenge, we encrypt the random bytes			
	//encrypt our random bytes using twofish and 0 IV
	m_tfEngine.SetEncryptionIV(); // just in case
	ByteBuffer ourChallenge = m_tfEngine.Encrypt(decryptedChallenge,sizeof(decryptedChallenge),false);
	DEBUG_LOG(format("TF Encrypted Challenge (as sent) |%1%|") % Bin2Hex(ourChallenge));
	//copy to variable
	memcpy(challenge,ourChallenge.contents(),sizeof(challenge));

	DEBUG_LOG(format("Decrypted Challenge (feed to md5) |%1%|") % Bin2Hex(decryptedChallenge,sizeof(decryptedChallenge)) );

	//do md5 transform to decrypted challenge to make final iv
	byte MD5Digest[16];
	CryptoPP::Weak::MD5 md5Transformer;
	md5Transformer.Update(decryptedChallenge,sizeof(decryptedChallenge));
	md5Transformer.Final(MD5Digest);

	DEBUG_LOG(format("Processed Challenge: |%1%|") % Bin2Hex(MD5Digest,sizeof(MD5Digest)) );

	//store this challenge
	memcpy(finalChallenge,MD5Digest,sizeof(finalChallenge));

	//make reply
	TCPVariableLengthPacket reply;
	reply << byte(AS_AuthChallenge);
	//send encrypted tf bytes, client will decrypt then md5 to generate IV
	reply.append(ourChallenge.contents(),ourChallenge.size());

	SendPacket(reply);

	DEBUG_LOG(format("Sending AS_AuthChallenge: |%1%|") % Bin2Hex(reply) );
}

void AuthSocket::HandleAuthChallengeResponse( ByteBuffer &packet )
{
	uint16 someShort;
	packet >> someShort;
	DEBUG_LOG(format("someShort is: %d") % someShort);

	uint16 cipherTextLen;
	packet >> cipherTextLen;

	vector<byte> cipherText;
	cipherText.resize(cipherTextLen);
	packet.read(&cipherText[0],cipherText.size());

	//now we have the ciphertext, lets decrypt
	//challenge response from client is encrypted with IV set to 0
	m_tfEngine.SetDecryptionIV();
	ByteBuffer chRspPlain = m_tfEngine.Decrypt(&cipherText[0],cipherText.size(),false);
	DEBUG_LOG(format("Challenge response decrypted: |%1%|") % Bin2Hex(chRspPlain) );

	size_t packetSize = 0;

	uint8 someByte;
	chRspPlain >> someByte;
	packetSize += sizeof(someByte);

	byte processedChallenge[16];
	chRspPlain.read(processedChallenge,sizeof(processedChallenge));
	packetSize += sizeof(processedChallenge);

	if (memcmp(processedChallenge,finalChallenge,sizeof(processedChallenge)))
	{
		WARNING_LOG(format("Processed challenge mismatch: got |%1%|, expected |%2%|, disconnecting.")
			% Bin2Hex(processedChallenge,sizeof(processedChallenge))
			% Bin2Hex(finalChallenge,sizeof(finalChallenge)) );

		SetCloseAndDelete(true);
		return;
	}

	uint16 unknown1;
	chRspPlain >> unknown1;
	packetSize += sizeof(unknown1);

	uint16 unknown2;
	chRspPlain >> unknown2;
	packetSize += sizeof(unknown2);

	uint16 unknown3;
	chRspPlain >> unknown3;
	packetSize += sizeof(unknown3);

	uint16 passwordLen;
	chRspPlain >> passwordLen;
	packetSize += sizeof(passwordLen);

	vector<char> password;
	password.resize(passwordLen);
	chRspPlain.read((byte*)&password[0],password.size());
	packetSize += password.size();

	uint16 soePassLen;
	chRspPlain >> soePassLen;
	packetSize += sizeof(soePassLen);

	vector<char> soePass;
	soePass.resize(soePassLen);
	chRspPlain.read((byte*)&soePass[0],soePass.size());
	packetSize += soePass.size();

	uint16 paddingLen;
	chRspPlain >> paddingLen;
	packetSize += sizeof(paddingLen);

	vector<byte> padding;
	padding.resize(paddingLen);
	chRspPlain.read(&padding[0],paddingLen);
	packetSize += padding.size();

	if (packetSize != chRspPlain.size())
	{
		WARNING_LOG(format("Challenge response blob size mismatch !, expected %1%, received %2%, disconnecting.") %packetSize % chRspPlain.size());
		SetCloseAndDelete(true);
		return;
	}

	DEBUG_LOG(format("Parsed contents: 1(const 23): |%1%|, 2(size): |%2%|, 3(size): |%3%|, User Password: |%4%|, SOE Password: |%5%|") % unknown1 % unknown2 % unknown3 % &password[0] % &soePass[0]);

	if (VerifyPassword(string(&password[0]),m_passwordSalt,m_passwordHash) == false)
	{
		WARNING_LOG(format("User %1% supplied an invalid password, disconnecting.") % m_username );
		SetCloseAndDelete(true);
		return;
	}

	if (m_publicExponent != 17 || m_publicModulus.size() != 96 || m_privateExponent.size() != 96)
	{
		INFO_LOG(format("Invalid RSA keys for user %1%, regenerating.") % m_username );

		for (;;)
		{
			//generate RSA keypair for client
			CryptoPP::AutoSeededRandomPool randPool;
			CryptoPP::InvertibleRSAFunction params;
			params.GenerateRandomWithKeySize( randPool, 768 );
			CryptoPP::RSA::PublicKey userPubKey(params);
			CryptoPP::RSA::PrivateKey userPrivKey(params);
			m_publicExponent = uint16(userPubKey.GetPublicExponent().ConvertToLong()); 
			byte tempBuf[96];
			userPubKey.GetModulus().Encode(tempBuf,sizeof(tempBuf));
			m_publicModulus = string((const char*)tempBuf,sizeof(tempBuf));
			m_privateExponent.clear();
			CryptoPP::StringSink privateExponentSink(m_privateExponent);
			userPrivKey.GetPrivateExponent().Encode(privateExponentSink,userPrivKey.GetPrivateExponent().MinEncodedSize());

			if (m_publicExponent == 17 && m_publicModulus.size() == 96 && m_privateExponent.size() == 96)
			{
				break;
			}
		}

		//update db info
		sDatabase.Execute(format("UPDATE `users` SET `publicExponent` = %1%, `publicModulus` = %2%, `privateExponent` = %3% WHERE `userId` = %4%") 
			% m_publicExponent
			% Bin2Hex(m_publicModulus,BIN2HEX_ZEROES)
			% Bin2Hex(m_privateExponent,BIN2HEX_ZEROES)
			% m_userId );
	}

	signedDataStruct signedData;
	memset(&signedData,0,sizeof(signedData));

	signedData.unknownByte = 1;
	signedData.userId1 = m_userId;
	strncpy(signedData.userName,m_username.c_str(),sizeof(signedData.userName)-1);
	signedData.unknownShort = 256;
	signedData.expiryTime = getTime() + 60 * 10; //10 minutes ahead, just like on real server
	signedData.publicExponent = swap16(m_publicExponent); //almost forgot the swap16 here :D
	memcpy(signedData.modulus,m_publicModulus.data(),sizeof(signedData.modulus));
	signedData.timeCreated = m_timeCreated;

	//sign the data that needs to be signed
	//its not that actual part which is signed, but the md5 of it, and sign makes a md5 of that, i think wb is just fucking with us
	CryptoPP::Weak::MD5 md5Object;
	md5Object.Update((const byte*)&signedData,sizeof(signedData));
	byte signMePlease[16];
	md5Object.Final(signMePlease);
	ByteBuffer signature = sAuth.SignWith1024Bit(signMePlease,sizeof(signMePlease));

	//the encrypted data is the private exponent of user's RSA key
	//to encrypt 96 byte exponent, use auth_key as key and challenge as IV
	m_tfEngine.SetEncryptionIV(challenge,sizeof(challenge));
	ByteBuffer encryptedPrivateExponent=m_tfEngine.Encrypt((const byte*)m_privateExponent.data(),m_privateExponent.size(),false);

	if (encryptedPrivateExponent.size() != 96)
	{
		ERROR_LOG(format("Encrypted private exponent ended up being something other than 96 bytes (%1% bytes), disconnecting.")
			% encryptedPrivateExponent.size());
		SetCloseAndDelete(true);
		return;
	}

	//send reply
	TCPVariableLengthPacket worldPacket;

#pragma pack(push,1)
	typedef struct 
	{
		uint8 opcode; //AS_AuthReply
		byte unknown1[10]; //10 zero bytes
		uint16 offsetAuthData; //offset into packet where worldList data ends, and authData starts
		uint16 offsetEncryptedData; //offset into packet where authData ends, and encrypted pkey starts
		uint32 unknown2; //always 1F 00 00 00, aka 0x1F 31d
		uint16 offsetCharData; //offset into packet where this header ends, and charData starts (should be always 0x21, sizeof(AuthReplyHeader))
		uint32 unknown3; //always 6E D1 00 00, aka 53614d
		uint32 offsetServerData; //offset into packet where charData ends, and serverData begins
		uint32 offsetUsername; //offset into packet where authData ends, and userName at the end begins
	} AuthReplyHeader;
#pragma pack(pop)

	AuthReplyHeader packetHeader;
	memset(&packetHeader,0,sizeof(packetHeader));
	packetHeader.opcode = AS_AuthReply;
	packetHeader.unknown2 = 0x1F;
	packetHeader.unknown3 = 0x0000D16E;
	packetHeader.offsetCharData = sizeof(packetHeader);

	worldPacket.append((const byte*)&packetHeader,sizeof(packetHeader)); //we will have to get back and overwrite this later with the proper values

	uint16 numCharacters;

	scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `charId`, `worldId`, `status`, `handle` FROM `characters` WHERE `userId` = %1%") % m_userId));
	if(result == NULL)
		numCharacters = 0;
	else
		numCharacters = result->GetRowCount();

	//number of characters user has
	worldPacket << uint16(numCharacters);

	if (numCharacters > 0)
	{
#pragma pack(push,1)
		typedef struct  
		{
			uint8 unknown1; //always 0
			uint16 handleStrOffset; //offset from start of this charDataItem to string
			uint64 charId; //64bit globally unique character id
			uint8 status; //ok,in transit,banned
			uint16 worldId; //world the char is in
		} CharacterData;
#pragma pack(pop)

		ByteBuffer characterDatas;
		ByteBuffer characterStrings;

		for (uint i=0;i<numCharacters;i++)
		{
			Field *field = result->Fetch();

			CharacterData currCharacter;
			currCharacter.unknown1 = 0;
			currCharacter.charId = field[0].GetUInt64();
			currCharacter.worldId = field[1].GetUInt16();
			currCharacter.status = field[2].GetUInt8();
			//how many bytes untill we get to character strings start + offset into character strings
			currCharacter.handleStrOffset = (numCharacters - i)*sizeof(CharacterData) + characterStrings.wpos();

			string guidHex = Bin2Hex((const byte*)&currCharacter.charId,sizeof(currCharacter.charId));
			DEBUG_LOG(format("Character GUID: %1%") % guidHex );

			//add the character to character datas
			characterDatas.append((const byte*)&currCharacter,sizeof(currCharacter));

			string characterString = field[3].GetString();
			characterStrings.writeString(characterString);

			//fetch next row
			if (result->NextRow() == false)
				break;
		}

		//add both chars and strings to worldpacket
		worldPacket.append(characterDatas);
		worldPacket.append(characterStrings);
	}

	//character data has ended, server data starts at this pos
	packetHeader.offsetServerData = worldPacket.wpos();	

	//fetch server list data
	result.reset(sDatabase.Query("SELECT `worldId`, `name`, `type`, `status`, `numPlayers` FROM `worlds`"));
	if(result == NULL || result->GetRowCount() < 1)
	{
		ERROR_LOG("No worlds in db, disconnecting.");
		SetCloseAndDelete(true);
		return;
	}

	uint16 numWorlds = result->GetRowCount();
	worldPacket << uint16(numWorlds);

	do 
	{
		Field *field = result->Fetch();

#pragma pack(push,1)
		typedef struct  
		{
			uint8 unknown1; //always 0
			uint16 worldId; 
			char worldName[20];
			uint8 status; //down,open,etc
			uint8 type; //pvp or non pvp
			uint32 clientVersion; //always D9 21 07 00 (7.xxxx)
			uint16 unknown4; //always 1
			uint8 load; //31,32,33
		} WorldData;
#pragma pack(pop)

		WorldData currWorld;
		memset(&currWorld,0,sizeof(currWorld));

		currWorld.unknown1 = 0;
		currWorld.worldId = field[0].GetUInt16();
		string worldNameStr = field[1].GetString();
		strncpy(currWorld.worldName,worldNameStr.c_str(),worldNameStr.length());
		currWorld.type = field[2].GetUInt8();
		currWorld.status = field[3].GetUInt8();
		currWorld.clientVersion = matrixVersion;
		currWorld.unknown4 = 1;

		//determine load
		{
			uint32 numPlayers = field[4].GetUInt32();
			if (numPlayers < 50)
				currWorld.load = 0x31; //low load
			else if (numPlayers < 100)
				currWorld.load = 0x32; //medium load
			else 
				currWorld.load = 0x33; //high load
		}

		worldPacket.append((const byte*)&currWorld,sizeof(currWorld));
	}
	while(result->NextRow());

	//worldlist data ended, now starts auth data
	packetHeader.offsetAuthData = worldPacket.wpos();

	worldPacket << uint16(swap16(0x3601)); //indicates start of auth data
	//then the signature of the signed data
	worldPacket.append(signature);
	//then the signed data itself
	worldPacket.append((const byte*)&signedData,sizeof(signedData));

	//auth data ended, now starts key data
	packetHeader.offsetEncryptedData = worldPacket.wpos();

	//then the size of the encrypted blob
	worldPacket << uint16(encryptedPrivateExponent.size());
	//then the encrypted blob itself
	worldPacket.append(encryptedPrivateExponent.contents(),encryptedPrivateExponent.size());

	//key data ended, now starts username data
	packetHeader.offsetUsername = worldPacket.wpos();

	//then the username string as mxo string
	worldPacket.writeString(m_username);

	//we need to rewrite the header back to the front now
	worldPacket.put(0,(const byte*)&packetHeader,sizeof(packetHeader));

	//for debugging, lets dump the packet we just created
	DEBUG_LOG(format("Sending AS_AuthReply: %1%") % Bin2Hex(worldPacket) );

	SendPacket(worldPacket);
}