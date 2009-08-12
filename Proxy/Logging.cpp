#include "Logging.h"
#include "Util.h"
#include "TCPVariableLengthPacket.h"
#include "EncryptedPacket.h"
#include "SequencedPacket.h"

#pragma pack(1)

void LogPacket(const char *pData,size_t pSize,PacketDirection direction,string comment="")
{
	string dirStr;
	if (direction == CLIENT_TO_AUTH)
	{
		dirStr = "Client->AUTH";
	}
	else if (direction == AUTH_TO_CLIENT)
	{
		dirStr = "AUTH->Client";
	}
	else if (direction == CLIENT_TO_MARGIN)
	{
		dirStr = "Client->MARGIN";
	}
	else if (direction == MARGIN_TO_CLIENT)
	{
		dirStr = "MARGIN->Client";
	}
	else if (direction == CLIENT_TO_WORLD)
	{
		dirStr = "Client->WORLD";
	}
	else if (direction == WORLD_TO_CLIENT)
	{
		dirStr = "WORLD->Client";
	}

	char dateStr [9];
	char timeStr [9];
	_strdate_s(dateStr,9);
	_strtime_s(timeStr,9);
	std::ofstream File;
	File.open("Packets.log",std::ios::app);
	File << dirStr << " " << "[" << timeStr << " " << dateStr  << "]" << " " << "packet size: " << pSize << endl;
	if (comment.length() > 0)
	{
		File << comment << endl;
	}
	string text;
	ConvertBytesintoHex((byte *)pData,text,(unsigned int)pSize);
	File << text << endl;
	int j = 0;

	for (unsigned int i = 0;i < pSize;i++)
	{
		j++;
		if (j == 33)
		{
			File << endl;
			j = 1;
		}
		if (pData[i] > 31 && pData[i] < 127)
			File << pData[i] << "  ";
		else
			File << "." << "  ";
	}
	File << endl << endl;
	File.close();
}

void LogWorldPacket(const char *pData,size_t pSize,PacketDirection direction,uint16 localSeq,uint16 remoteSeq,uint8 flags)
{
	stringstream comment;
	comment << "PSS: " << hex << (int)flags << " LocalSeq: " << dec << localSeq << " RemoteSeq: " << dec << remoteSeq;
	LogPacket(pData,pSize,direction,comment.str());
}

void LogString(string &title,string &contents)
{
	char dateStr [9];
	char timeStr [9];
	_strdate(dateStr);
	_strtime(timeStr);
	std::ofstream File;
	File.open("Packets.log",std::ios::app);
	File << "[" << timeStr << " " << dateStr  << "]" << " " << title << endl << contents << endl;
	File.close();
}

enum
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

const string GetStringForAuthOpcode(int theOpcode)
{
	switch (theOpcode)
	{
	case AS_GetPublicKeyRequest:
		return "AS_GetPublicKeyRequest";
	case AS_GetPublicKeyReply:
		return "AS_GetPublicKeyReply";
	case AS_AuthRequest:
		return "AS_AuthRequest";
	case AS_AuthChallenge:
		return "AS_AuthChallenge";
	case AS_AuthChallengeResponse:
		return "AS_AuthChallengeResponse";
	case AS_AuthReply:
		return "AS_AuthReply";
	case AS_CreateCharacterRequest:
		return "AS_CreateCharacterRequest";
	case AS_CreateCharacterReply:
		return "AS_CreateCharacterReply";
	case AS_DeleteCharacterRequest:
		return "AS_DeleteCharacterRequest";
	case AS_DeleteCharacterReply:
		return "AS_DeleteCharacterReply";
	case AS_LockAccountRequest:
		return "AS_LockAccountRequest";
	case AS_LockAccountReply:
		return "AS_LockAccountReply";
	case AS_UnlockAccountRequest:
		return "AS_UnlockAccountRequest";
	case AS_UnlockAccountReply:
		return "AS_UnlockAccountReply";
	case AS_PSAuthenticateRequest:
		return "AS_PSAuthenticateRequest";
	case AS_WorldIdAndStatus:
		return "AS_WorldIdAndStatus";
	case AS_PSAuthenticateReply:
		return "AS_PSAuthenticateReply";
	case AS_PSCreateCharacterRequest:
		return "AS_PSCreateCharacterRequest";
	case AS_PSCreateCharacterReply:
		return "AS_PSCreateCharacterReply";
	case AS_PSGetWorldListRequest:
		return "AS_PSGetWorldListRequest";
	case AS_PSGetWorldListReply:
		return "AS_PSGetWorldListReply";
	case AS_PSGetWorldPopulationsRequest:
		return "AS_PSGetWorldPopulationsRequest";
	case AS_PSGetWorldPopulationsReply:
		return "AS_PSGetWorldPopulationsReply";
	case AS_GetAccountInfoRequest:
		return "AS_GetAccountInfoRequest";
	case AS_GetAccountInfoReply:
		return "AS_GetAccountInfoReply";
	case AS_ProxyConnectRequest:
		return "AS_ProxyConnectRequest";
	case AS_ProxyConnectReply:
		return "AS_ProxyConnectReply";
	case AS_WorldShuttingDown:
		return "AS_WorldShuttingDown";
	case AS_PSResetAccountInUseRequest:
		return "AS_PSResetAccountInUseRequest";
	case AS_SetTransSessionPenaltyRequest:
		return "AS_SetTransSessionPenaltyRequest";
	case AS_PSSetTransSessionPenaltyRequest:
		return "AS_PSSetTransSessionPenaltyRequest";
	case AS_PSSetTransSessionPenaltyReply:
		return "AS_PSSetTransSessionPenaltyReply";
	case AS_SetWorldStatusRequest:
		return "AS_SetWorldStatusRequest";
	case AS_SetWorldStatusReply:
		return "AS_SetWorldStatusReply";
	case AS_PSSetWorldStatusRequest:
		return "AS_PSSetWorldStatusRequest";
	case AS_PSSetWorldStatusReply:
		return "AS_PSSetWorldStatusReply";
	case AS_UnlockAllAccountsRequest:
		return "AS_UnlockAllAccountsRequest";
	case AS_LockRecoveringAccountRequest:
		return "AS_LockRecoveringAccountRequest";
	case AS_LockRecoveringAccountReply:
		return "AS_LockRecoveringAccountReply";
	case AS_RefreshCertificateRequest:
		return "AS_RefreshCertificateRequest";
	case AS_RefreshCertificateReply:
		return "AS_RefreshCertificateReply";
	case AS_PSCertRefreshRequest:
		return "AS_PSCertRefreshRequest";
	case AS_PSCertRefreshReply:
		return "AS_PSCertRefreshReply";
	case AS_PSSetWorldVersionRequest:
		return "AS_PSSetWorldVersionRequest";
	case AS_PSSetWorldVersionReply:
		return "AS_PSSetWorldVersionReply";
	case AS_IAmProxyLeader:
		return "AS_IAmProxyLeader";
	case AS_RouteToAuthServer:
		return "AS_RouteToAuthServer";
	case AS_GetWorldListRequest:
		return "AS_GetWorldListRequest";
	case AS_GetWorldListReply:
		return "AS_GetWorldListReply";
	}

	return "AS_UNDEFINED";
}

CryptoPP::AutoSeededRandomPool randPool;

CryptoPP::RSA::PrivateKey LoadRSAPrivateKey()
{
	ifstream f_privateKey;
	f_privateKey.open("privkey.dat",ios::binary);
	f_privateKey.seekg(0,ios::end);
	ifstream::pos_type keySize = f_privateKey.tellg();
	vector<byte> storage;
	storage.resize(keySize);
	f_privateKey.seekg(0,ios::beg);
	f_privateKey.read((char*)&storage[0],storage.size());
	f_privateKey.close();

	string pkeyStr = string((const char*)&storage[0],storage.size());

	CryptoPP::InvertibleRSAFunction params;
	CryptoPP::StringSource pkeySource(pkeyStr,true);
	params.BERDecodePrivateKey(pkeySource,false,pkeySource.MaxRetrievable());
	CryptoPP::RSA::PrivateKey privateKey( params );

	return privateKey;
}

CryptoPP::RSAES_OAEP_SHA_Decryptor LoadRSADecryptor()
{
	return CryptoPP::RSAES_OAEP_SHA_Decryptor(LoadRSAPrivateKey());
}

ByteBuffer SignWith1024Bit( byte *message,size_t messageLen )
{
	CryptoPP::Weak::RSASSA_PKCS1v15_MD5_Signer signer1024bit(LoadRSAPrivateKey());

	//generate signature
	ByteBuffer signMe;
	signMe.append(message,messageLen);
	signMe.rpos(0);	
	vector<byte> signature;
	signature.resize(signer1024bit.MaxSignatureLength());
	size_t actualSignatureSize = signer1024bit.SignMessage(randPool,(byte*)signMe.contents(),signMe.size(),&signature[0]);
	signature.resize(actualSignatureSize);

	ByteBuffer toReturn;
	toReturn.append(&signature[0],signature.size());
	toReturn.rpos(0);
	toReturn.wpos(0);

	return toReturn;
}

CryptoPP::RSAES_OAEP_SHA_Encryptor LoadMxORSAEncryptor()
{
	ifstream f_mxoPublic;
	f_mxoPublic.open("mxopub.dat",ios::binary);
	f_mxoPublic.seekg(0,ios::end);
	ifstream::pos_type keySize = f_mxoPublic.tellg();
	vector<byte> storage;
	storage.resize(keySize);
	f_mxoPublic.seekg(0,ios::beg);
	f_mxoPublic.read((char*)&storage[0],storage.size());
	f_mxoPublic.close();
	string keyString = string((const char*)&storage[0],storage.size());
	keyString = keyString.substr(4);

	CryptoPP::StringSource keySource(keyString,true);
	CryptoPP::Integer modulus;
	modulus.BERDecode(keySource);
	CryptoPP::Integer exponent;
	exponent.BERDecode(keySource);

	CryptoPP::RSA::PublicKey publicKey;
	publicKey.Initialize(modulus,exponent);

	return CryptoPP::RSAES_OAEP_SHA_Encryptor(publicKey);	
}

ByteBuffer GetPubKeyPacket()
{
	ifstream f_publicKey;
	f_publicKey.open("pubkey.dat",ios::binary);
	f_publicKey.seekg(0,ios::end);
	ifstream::pos_type keySize = f_publicKey.tellg();
	vector<byte> storage;
	storage.resize(keySize);
	f_publicKey.seekg(0,ios::beg);
	f_publicKey.read((char*)&storage[0],storage.size());
	f_publicKey.close();

	string keyString = string((const char*)&storage[0],storage.size());
	keyString = keyString.substr(4);

	CryptoPP::StringSource keySource(keyString,true);
	CryptoPP::Integer modulus;
	modulus.BERDecode(keySource);

	ByteBuffer keySourceBuf;
	keySourceBuf.append((const byte*)keyString.data(),keyString.size());

	byte theSignature[256];
	keySourceBuf.rpos(keySourceBuf.size()-sizeof(theSignature));
	keySourceBuf.read(theSignature,sizeof(theSignature));

	string pubKeyModulusStr;
	CryptoPP::StringSink pubKeyModulusSink(pubKeyModulusStr);
	modulus.Encode(pubKeyModulusSink,modulus.MinEncodedSize());

	ByteBuffer result;
	result << uint16(pubKeyModulusStr.size());
	result.append(pubKeyModulusStr);

	result << uint16(sizeof(theSignature));
	result.append(theSignature,sizeof(theSignature));

	return result;
}

string RSAMxOEncrypt(string input)
{
	string output;
	CryptoPP::StringSource(input,true, new CryptoPP::PK_EncryptorFilter(randPool, LoadMxORSAEncryptor(), new CryptoPP::StringSink(output)));
	return output;
}

string RSADecrypt(string input)
{
	string output;
	CryptoPP::StringSource(input,true, new CryptoPP::PK_DecryptorFilter(randPool, LoadRSADecryptor(), new CryptoPP::StringSink(output)));
	return output;
}

byte auth_Key[16];
byte challenge[16];
const byte blankIV[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


bool clientRequiresPubKeySent = false;

string ClientToAuth(const char* pData,size_t pSize)
{
	string hexadecm;

	TCPVariableLengthPacket varLenPacket((const byte*)pData,pSize);

	ByteBuffer thePacket;
	thePacket.append((const byte*)varLenPacket.contents(),varLenPacket.size());
	thePacket.rpos(0);


	byte packetOpcode;
	thePacket >> packetOpcode;

	switch(packetOpcode)
	{
	default:
		{
			stringstream theComment;
			theComment << GetStringForAuthOpcode(packetOpcode) << endl;

			LogPacket(thePacket.contents(),thePacket.size(),CLIENT_TO_AUTH,theComment.str());

			TCPVariableLengthPacket withAddedLen;
			withAddedLen.append((const byte*)thePacket.contents(),thePacket.size());
			ByteBuffer sendMe = withAddedLen.GetContentsWithSize();
			return string(sendMe.contents(),sendMe.size());
		}
	case AS_GetPublicKeyRequest:
		{
			uint32 clientVersion;
			thePacket >> clientVersion;
			uint32 rsaVersion;
			thePacket >> rsaVersion;

			ConvertBytesintoHex((const byte*)thePacket.contents(),hexadecm,thePacket.size());
			cout << "Received AS_GetPublicKeyRequest from client: |" << hexadecm << "|" << endl;

			stringstream comments;
			comments << GetStringForAuthOpcode(AS_GetPublicKeyRequest) << endl;
			comments << "clientVersion: " << clientVersion << endl;
			comments << "rsaVersion: " << rsaVersion << endl;

			if (rsaVersion != 4)
			{
				clientRequiresPubKeySent = true;
			}
			else
			{
				clientRequiresPubKeySent = false;
			}

			//we will always send OK to REAL server
			TCPVariableLengthPacket awesome;
			awesome << uint8(AS_GetPublicKeyRequest);
			awesome << clientVersion;
			awesome << uint32(4);

			LogPacket(thePacket.contents(),thePacket.size(),CLIENT_TO_AUTH,comments.str());

			ByteBuffer sendMe = awesome.GetContentsWithSize();

			ConvertBytesintoHex((const byte*)sendMe.contents(),hexadecm,sendMe.size());
			cout << "Sending AS_GetPublicKeyRequest to server: |" << hexadecm << "|" << endl;

			return string(sendMe.contents(),sendMe.size());
		}
	case AS_AuthRequest:
		{
			typedef struct 
			{
				uint32 rsaType;
				uint32 unknownz1;
				char unknownz2[31];
				uint16 blobLen;
			}requestHdr;

			ConvertBytesintoHex((const byte*)thePacket.contents(),hexadecm,thePacket.size());
			cout << "Received AS_AuthRequest from client: |" << hexadecm << "|" << endl;

			stringstream comments;

			comments << GetStringForAuthOpcode(AS_AuthRequest) << endl;

			requestHdr requestHeader;
			memset(&requestHeader,0,sizeof(requestHeader));
			thePacket.read((uint8 *)&requestHeader,sizeof(requestHeader));

			vector<byte> encryptedBlob;
			encryptedBlob.resize(requestHeader.blobLen);
			thePacket.read(&encryptedBlob[0],encryptedBlob.size());

			string decryptedBlob = RSADecrypt(string((const char*)&encryptedBlob[0],encryptedBlob.size()));
			ConvertBytesintoHex((const byte*)decryptedBlob.data(),hexadecm,decryptedBlob.size());
			comments << "RSA Blob:|" << hexadecm << "|" << endl;

			ByteBuffer blobBuf(decryptedBlob);

			blobBuf.read(1);

			uint32 rsaMethod;
			blobBuf >> rsaMethod;
			comments << "rsaMethod is " << rsaMethod << endl;

			uint16 someShort;
			blobBuf >> someShort;
			comments << "someShort is " << someShort << endl;

			memset(auth_Key,0,sizeof(auth_Key));
			blobBuf.read(auth_Key,sizeof(auth_Key));
			ConvertBytesintoHex(auth_Key,hexadecm,sizeof(auth_Key));
			comments << "Auth TF key: |" << hexadecm << "|" << endl;

			uint32 theTime;
			blobBuf >> theTime;
			comments << "time is: " << hex << theTime << endl;

			uint16 usernameLen;
			blobBuf >> usernameLen;
			vector<char> usernameVect;
			usernameVect.resize(usernameLen);
			blobBuf.read((uint8 *)&usernameVect[0],usernameVect.size());
			string theUsername(&usernameVect[0],usernameVect.size());
			comments << "Username: |" << theUsername << "|" << endl;

			string reEncryptedBlob = RSAMxOEncrypt(decryptedBlob);
			TCPVariableLengthPacket awesome;
			awesome << byte(AS_AuthRequest);
			awesome.append((const byte*)&requestHeader,sizeof(requestHeader));
			awesome.append(reEncryptedBlob);

			LogPacket(thePacket.contents(),thePacket.size(),CLIENT_TO_AUTH,comments.str());

			ByteBuffer sendMe = awesome.GetContentsWithSize();

			ConvertBytesintoHex((const byte*)sendMe.contents(),hexadecm,sendMe.size());
			cout << "Sending AS_AuthRequest to server: |" << hexadecm << "|" << endl;

			return string(sendMe.contents(),sendMe.size());
		}
	case AS_AuthChallengeResponse:
		{
			ConvertBytesintoHex((const byte*)thePacket.contents(),hexadecm,thePacket.size());
			cout << "Received AS_AuthChallengeResponse from client: |" << hexadecm << "|" << endl;

			stringstream comments;
			comments << GetStringForAuthOpcode(AS_AuthChallengeResponse) << endl;

			uint16 someShort;
			thePacket >> someShort;
			comments << "someShort is " << someShort << endl;

			uint16 cipherTextLen;
			thePacket >> cipherTextLen;

			vector<byte> cipherText;
			cipherText.resize(cipherTextLen);
			thePacket.read(&cipherText[0],cipherText.size());

			//now we have the ciphertext, lets decrypt
			string cipherInput((const char*)&cipherText[0],cipherText.size());
			string plainOutput;

			//this message is done with 0 as iv
			CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption TFDecrypt(auth_Key,sizeof(auth_Key),blankIV);
			CryptoPP::StringSource(cipherInput, true, 
				new CryptoPP::StreamTransformationFilter(
				TFDecrypt, new CryptoPP::StringSink(plainOutput),
				CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING));

			size_t packetSize = 0;
			ByteBuffer chRspPlain(plainOutput);

			uint8 someByte;
			chRspPlain >> someByte;
			packetSize += sizeof(someByte);

			byte processedChallenge[16];
			chRspPlain.read(processedChallenge,sizeof(processedChallenge));
			packetSize += sizeof(processedChallenge);

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
				comments << "Challenge response blob size mismatch, expected " << dec << packetSize << ", received " << chRspPlain.size() << endl;
			}

			ConvertBytesintoHex(processedChallenge,hexadecm,sizeof(processedChallenge));
			{
				char someCharsHere[1024];
				sprintf(someCharsHere,"Parsed contents: 1(const 23): |%d|, 2(size): |%d|, 3(size): |%d|, User Password: |%s|, SOE Password: |%s|",unknown1,unknown2,unknown3,"CENSORED",&soePass[0]);
				comments << someCharsHere << endl;
			}

			LogPacket(thePacket.contents(),thePacket.size(),CLIENT_TO_AUTH,comments.str());

			TCPVariableLengthPacket withAddedLen;
			withAddedLen.append((const byte*)thePacket.contents(),thePacket.size());
			ByteBuffer sendMe = withAddedLen.GetContentsWithSize();

			cout << "Sending AS_AuthChallengeResponse to server unchanged" << endl;

			return string(sendMe.contents(),sendMe.size());
		}
	}
}

typedef struct  
{
	uint8 unknownByte; //always 01
	uint32 userId1; //4 bytes, last byte is 00 on real server, unique per user
	char userName[33]; //32 for actual text, 0 for null terminator (if max len)
	uint16 unknownShort; //always 256
	uint32 padding1; //4 bytes of 0 padding
	uint32 timeStamp; //10 minutes ahead of current time
	byte padding2[32]; //32 bytes of 0 padding
	uint16 publicExponent; //17 for cryptopp, but big endian format here
	byte modulus[96]; //768bit public modulus of user RSA key
	uint32 userId2; //4 bytes, unique per user, users registered later have this number higher
} signedDataStruct;

byte savedSignature[128];
signedDataStruct savedSignedData;

CryptoPP::RSAES_OAEP_SHA_Decryptor marginBlobDecryptor;

ByteBuffer RSAMarginBlobDecrypt(const byte* pData,size_t pLen)
{
	string input = string((const char*)pData,pLen);
	string output;
	CryptoPP::StringSource(input,true, new CryptoPP::PK_DecryptorFilter(randPool, marginBlobDecryptor, new CryptoPP::StringSink(output)));
	ByteBuffer returnMe;
	returnMe.append((const byte*)output.data(),output.size());
	return returnMe;
}

string AuthToClient(const char* pData,size_t pSize)
{
	string hexadecm;

	TCPVariableLengthPacket varLenPacket((const byte*)pData,pSize);

	ByteBuffer thePacket;
	thePacket.append((const byte*)varLenPacket.contents(),varLenPacket.size());
	thePacket.rpos(0);

	byte packetOpcode;
	thePacket >> packetOpcode;

	switch(packetOpcode)
	{
	default:
		{
			stringstream theComment;
			theComment << GetStringForAuthOpcode(packetOpcode) << endl;

			LogPacket(thePacket.contents(),thePacket.size(),AUTH_TO_CLIENT,theComment.str());

			TCPVariableLengthPacket withAddedLen;
			withAddedLen.append((const byte*)thePacket.contents(),thePacket.size());
			ByteBuffer sendMe = withAddedLen.GetContentsWithSize();
			return string(sendMe.contents(),sendMe.size());
		}
	case AS_GetPublicKeyReply:
		{
			ConvertBytesintoHex((const byte*)thePacket.contents(),hexadecm,thePacket.size());
			cout << "Received AS_GetPublicKeyReply from server: |" << hexadecm << "|" << endl;

			stringstream comments;
			comments << GetStringForAuthOpcode(AS_GetPublicKeyReply) << endl;

			uint32 padding0;
			thePacket >> padding0;
			uint32 serverTime;
			thePacket >> serverTime;
			comments << "serverTime: " << dec << serverTime << endl;
			uint32 rsaMethod;
			thePacket >> rsaMethod;
			comments << "serverRsaMethod: " << dec << rsaMethod << endl;

			TCPVariableLengthPacket awesome;
			if (clientRequiresPubKeySent == true)
			{
				awesome
					<< byte(AS_GetPublicKeyReply)
					<< uint32(0)
					<< uint32(_time32(NULL))		//time
					<< uint32(0x04)			//rsa method
					<< uint8(0x12)
					<< uint8(0x00)
					<< uint8(0x11)			//public exponent
					<< uint8(0x94)
					<< uint8(0x00);

				awesome.append(GetPubKeyPacket());
			}
			else
			{
				awesome		
					<< byte(AS_GetPublicKeyReply)
					<< uint32(0) 
					<< uint32(_time32(NULL))		//time
					<< uint32(0x04)			//rsa method
					<< byte(0) 
					<< uint32(0);
			}

			LogPacket(thePacket.contents(),thePacket.size(),AUTH_TO_CLIENT,comments.str());

			ByteBuffer sendMe = awesome.GetContentsWithSize();

			ConvertBytesintoHex((const byte*)sendMe.contents(),hexadecm,sendMe.size());
			cout << "Sending AS_GetPublicKeyReply to client: |" << hexadecm << "|" << endl;

			return string(sendMe.contents(),sendMe.size());
		}
	case AS_AuthChallenge:
		{
			ConvertBytesintoHex((const byte*)thePacket.contents(),hexadecm,thePacket.size());
			cout << "Received AS_AuthChallenge from server: |" << hexadecm << "|" << endl;

			stringstream comments;
			comments << GetStringForAuthOpcode(AS_AuthChallenge) << endl;

			//this is just 16 bytes of data (the CHALLENGE)
			byte theChallenger[16];
			thePacket.read(theChallenger,sizeof(theChallenger));
			string cipherChallenge = string((char*)theChallenger,sizeof(theChallenger));

			memcpy(challenge,theChallenger,sizeof(challenge));

			ConvertBytesintoHex(challenge,hexadecm,sizeof(challenge));
			comments << "Original challenge from server: |" << hexadecm << "|" << endl;

			//to get finalChallenge from the challenge, we need to decrypt with auth key and 0 iv, then hash with md5

			CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption TFDecrypt(auth_Key,sizeof(auth_Key),blankIV);
			string plainChallenge;
			CryptoPP::StringSource(cipherChallenge, true, 
				new CryptoPP::StreamTransformationFilter(
				TFDecrypt, new CryptoPP::StringSink(plainChallenge),
				CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING));	

			ConvertBytesintoHex((const byte*)plainChallenge.data(),hexadecm,plainChallenge.size());
			comments << "Decrypted Challenge: |" << hexadecm << "|" << endl;

			byte finalChallenge[16];

			CryptoPP::Weak::MD5 md5object;
			//feed decrypted challenge to md5 object
			md5object.Update((const byte*)plainChallenge.data(),plainChallenge.size());
			//end hash goes to finalCallenge
			md5object.Final(finalChallenge);

			ConvertBytesintoHex(finalChallenge,hexadecm,sizeof(finalChallenge));
			comments << "Processed Challenge: |" << hexadecm << "|" << endl;

			LogPacket(thePacket.contents(),thePacket.size(),AUTH_TO_CLIENT,comments.str());

			TCPVariableLengthPacket withAddedLen;
			withAddedLen.append((const byte*)thePacket.contents(),thePacket.size());
			ByteBuffer sendMe = withAddedLen.GetContentsWithSize();

			cout << "Sending AS_AuthChallenge to client unchanged" << endl;

			return string(sendMe.contents(),sendMe.size());
		}
	case AS_AuthReply:
		{
			ConvertBytesintoHex((const byte*)thePacket.contents(),hexadecm,thePacket.size());
			cout << "Received AS_AuthReply from server: |" << hexadecm << "|" << endl;

			stringstream comments;
			comments << GetStringForAuthOpcode(AS_AuthReply) << endl;

			int authDataOffset = -1;
			int encryptedBlobOffset = -1;

			for (int i=0;i<thePacket.size();i++)
			{
				byte *thePacketPtr = (byte*)(&thePacket.contents()[i]);
				static byte bytesToFind[2] = { 0x36, 0x01 };

				if (!memcmp(thePacketPtr,bytesToFind,sizeof(bytesToFind)))
				{
					authDataOffset = i;
					break;
				}
			}
			for (int i=0;i<thePacket.size();i++)
			{
				byte *thePacketPtr = (byte*)(&thePacket.contents()[i]);
				static byte bytesToFind[2] = { 0x60, 0x00 };

				if (!memcmp(thePacketPtr,bytesToFind,sizeof(bytesToFind)))
				{
					encryptedBlobOffset = i;
					break;
				}
			}

			vector<byte> worldListData;

			if (authDataOffset != -1)
			{
				thePacket.rpos(0);
				worldListData.resize(authDataOffset);
				thePacket.read(&worldListData[0],worldListData.size());

				thePacket.rpos(authDataOffset);
				uint16 thirtySixZeroOne;
				thePacket >> thirtySixZeroOne;
				thePacket.read(savedSignature,sizeof(savedSignature));
				thePacket.read((byte*)&savedSignedData,sizeof(savedSignedData));
			}
			else
			{
				cout << "VERY BIG ERROR HERE BOSS" << endl;
			}

			signedDataStruct newSignedData;
			memcpy(&newSignedData,&savedSignedData,sizeof(newSignedData));
			//just change time
			newSignedData.timeStamp = _time32(NULL) + (60*10);

			string zeUsername = string(newSignedData.userName);
			comments << "Username: " << zeUsername << " UserId1: " << dec << newSignedData.userId1 << " UserId2: " << dec << newSignedData.userId2 << endl;

			//resign
			//its not that actual part which is signed, but the md5 of it, and sign makes a md5 of that, i think wb is just fucking with us
			byte signMePlease[16];
			CryptoPP::Weak::MD5 md5Object;
			md5Object.Update((const byte*)&newSignedData,sizeof(newSignedData));
			md5Object.Final(signMePlease);
			ByteBuffer newSignature = SignWith1024Bit(signMePlease,sizeof(signMePlease));

			vector<byte> encryptedBlob;

			if (encryptedBlobOffset != -1)
			{
				thePacket.rpos(encryptedBlobOffset);
				uint16 blobSize;
				thePacket >> blobSize;
				encryptedBlob.resize(blobSize);
				thePacket.read(&encryptedBlob[0],encryptedBlob.size());
			}
			else
			{
				cout << "ANOTHER BIG ERROR HERE BOSS" << endl;
			}

			//to decrypt 96 byte exponent, use auth_key as key and challenge as IV
			CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption TFDecrypt(auth_Key,sizeof(auth_Key),challenge);
			string decryptedPrivateExponent;

			CryptoPP::StringSource( string( (const char*) &encryptedBlob[0],encryptedBlob.size() ) ,
				true, 
				new CryptoPP::StreamTransformationFilter(
				TFDecrypt, new CryptoPP::StringSink(decryptedPrivateExponent),
				CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING));

			//extract keys
			CryptoPP::Integer publicExponent(uint32(swap16(newSignedData.publicExponent)));
			CryptoPP::Integer modulus;
			modulus.Decode(newSignedData.modulus,sizeof(newSignedData.modulus));
			//and private exponent is inside the now decrypted blo
			CryptoPP::Integer privateExponent;
			privateExponent.Decode((const byte*)decryptedPrivateExponent.data(),decryptedPrivateExponent.size());

			//comment the keys
			comments << "e: " << hex << publicExponent << " n: " << hex << modulus << " d: " << hex << privateExponent << endl;

			//initialize our margin blob decryptor with this key
			CryptoPP::RSA::PrivateKey marginPrivateKey;
			marginPrivateKey.Initialize(modulus,publicExponent,privateExponent);
			marginBlobDecryptor = CryptoPP::RSAES_OAEP_SHA_Decryptor(marginPrivateKey);

			//reassemble our new packet

			TCPVariableLengthPacket awesome;
			//non touched worldlist data
			awesome.append(&worldListData[0],worldListData.size());
			//36 01 indicates start of auth data
			awesome << uint16(swap16(0x3601));
			//then the signature of the signed data
			awesome.append(newSignature);
			//then the signed data itself
			awesome.append((const byte*)&newSignedData,sizeof(newSignedData));
			//then the size of the encrypted blob
			awesome << uint16(encryptedBlob.size());
			//then the encrypted blob itself
			awesome.append(&encryptedBlob[0],encryptedBlob.size());
			//then the size of username including null end character
			uint16 userNameLength = uint16(strlen(zeUsername.c_str())+1);
			awesome << userNameLength;
			//then the username including null end character
			awesome.append((const byte*)zeUsername.c_str(),userNameLength);

			LogPacket(thePacket.contents(),thePacket.size(),AUTH_TO_CLIENT,comments.str());

			ByteBuffer sendMe = awesome.GetContentsWithSize();

			ConvertBytesintoHex((const byte*)sendMe.contents(),hexadecm,sendMe.size());
			cout << "Sending AS_AuthReply to client: |" << hexadecm << "|" << endl;
			return string(sendMe.contents(),sendMe.size());
		}
	}
}

typedef enum MarginOpcode
{
	CERT_ConnectRequest = 0x01,
	CERT_Challenge = 0x02,
	CERT_ChallengeResponse = 0x03,
	CERT_ConnectReply = 0x04,
	CERT_NewSessionKey = 0x05,
	MS_ConnectRequest = 0x06,
	MS_ConnectChallenge = 0x07,
	MS_ConnectChallengeResponse = 0x08,
	MS_ConnectReply = 0x09,
	MS_ClaimCharacterNameRequest = 0x0A,
	MS_ClaimCharacterNameReply = 0x0B,
	MS_CreateCharacterRequest = 0x0C,
	MS_DeleteCharacterRequest = 0x0D,
	MS_DeleteCharacterReply = 0x0E,
	MS_LoadCharacterRequest = 0x0F,
	MS_LoadCharacterReply = 0x10,
	MS_EstablishUDPSessionReply = 0x11,
	MS_RefreshSessionKeyRequest = 0x12,
	MS_RefreshSessionKeyReply = 0x13,
	MS_ServerShuttingDown = 0x14,
	MS_FailoverRequest = 0x15,
	MS_FailoverReply = 0x16,
	MS_AdjustFloodgateRequest = 0x17,
	MS_AdjustFloodgateReply = 0x18,
	MS_GetPlayerListRequest = 0x19,
	MS_GetPlayerListReply = 0x1A,
	MS_FakePopulationRequest = 0x1B,
	MS_UnloadCharacterRequest = 0x1C,
	MS_GetClientIPRequest = 0x1D,
	MS_GetClientIPReply  = 0x1E
};

const string GetStringForMarginOpcode(int theOpcode)
{
	switch (theOpcode)
	{
	case CERT_ConnectRequest:
		return "CERT_ConnectRequest";
	case CERT_Challenge:
		return "CERT_Challenge";
	case CERT_ChallengeResponse:
		return "CERT_ChallengeResponse";
	case CERT_ConnectReply:
		return "CERT_ConnectReply";
	case CERT_NewSessionKey:
		return "CERT_NewSessionKey";
	case MS_ConnectRequest:
		return "MS_ConnectRequest";
	case MS_ConnectChallenge:
		return "MS_ConnectChallenge";
	case MS_ConnectChallengeResponse:
		return "MS_ConnectChallengeResponse";
	case MS_ConnectReply:
		return "MS_ConnectReply";
	case MS_ClaimCharacterNameRequest:
		return "MS_ClaimCharacterNameRequest";
	case MS_ClaimCharacterNameReply:
		return "MS_ClaimCharacterNameReply";
	case MS_CreateCharacterRequest:
		return "MS_CreateCharacterRequest";
	case MS_DeleteCharacterRequest:
		return "MS_DeleteCharacterRequest";
	case MS_DeleteCharacterReply:
		return "MS_DeleteCharacterReply";
	case MS_LoadCharacterRequest:
		return "MS_LoadCharacterRequest";
	case MS_LoadCharacterReply:
		return "MS_LoadCharacterReply";
	case MS_EstablishUDPSessionReply:
		return "MS_EstablishUDPSessionReply";
	case MS_RefreshSessionKeyRequest:
		return "MS_RefreshSessionKeyRequest";
	case MS_RefreshSessionKeyReply:
		return "MS_RefreshSessionKeyReply";
	case MS_ServerShuttingDown:
		return "MS_ServerShuttingDown";
	case MS_FailoverRequest:
		return "MS_FailoverRequest";
	case MS_FailoverReply:
		return "MS_FailoverReply";
	case MS_AdjustFloodgateRequest:
		return "MS_AdjustFloodgateRequest";
	case MS_AdjustFloodgateReply:
		return "MS_AdjustFloodgateReply";
	case MS_GetPlayerListRequest:
		return "MS_GetPlayerListRequest";
	case MS_GetPlayerListReply:
		return "MS_GetPlayerListReply";
	case MS_FakePopulationRequest:
		return "MS_FakePopulationRequest";
	case MS_UnloadCharacterRequest:
		return "MS_UnloadCharacterRequest";
	case MS_GetClientIPRequest:
		return "MS_GetClientIPRequest";
	case MS_GetClientIPReply:
		return "MS_GetClientIPReply";
	}

	return "MS_UNDEFINED";
}

CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *TFEncryptCTM = NULL;
CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptCTM = NULL;

CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *TFEncryptMTC = NULL;
CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptMTC = NULL;

CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptCTW = NULL;
CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *TFEncryptCTW = NULL;

CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptWTC = NULL;
CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *TFEncryptWTC = NULL;

byte theMarginBlobChallenge[16];

string ClientToMargin( const char* pData,size_t pSize )
{
	stringstream comment;
	string hexadecm;
	TCPVariableLengthPacket packetContents((const byte*)pData,pSize);

	bool encrypted = true;

	byte firstByte;
	packetContents >> firstByte;
	if (firstByte >= CERT_ConnectRequest && firstByte <= CERT_NewSessionKey)
	{
		uint16 firstShort;
		packetContents >> firstShort;

		if (firstShort == 3)
		{
			encrypted = false;
		}
	}

	//reset position (since we just read 3 bytes)
	packetContents.rpos(0);
	ByteBuffer packetData;

	if (encrypted == true && TFDecryptCTM != NULL)
	{
		try
		{
			EncryptedPacket packetTodecrypt(packetContents,TFDecryptCTM);
			packetData.append((const byte*)packetTodecrypt.contents(),packetTodecrypt.size());
			comment << "encrypted=true ";
		}
		catch (...)
		{
			ConvertBytesintoHex((const byte*)packetContents.contents(),hexadecm,packetContents.size());
			cout << "Decryption exception for CTM packet: |" << hexadecm << "|" << endl;
			encrypted = false;
		}
	}

	if (encrypted == false || TFDecryptCTM == NULL)
	{
		packetData.append((const byte*)packetContents.contents(),packetContents.size());
		comment << "encrypted=false ";
	}

	byte packetOpcode;
	packetData >> packetOpcode;

	ByteBuffer packetToSend;

	switch (packetOpcode)
	{
	default:
		{
			comment << GetStringForMarginOpcode(packetOpcode);
			LogPacket(packetData.contents(),packetData.size(),CLIENT_TO_MARGIN,comment.str());
			packetToSend = packetData;
			packetToSend.rpos(0);
			break;
		}
	case CERT_ConnectRequest:
		{
			//log the original packet
			comment << GetStringForMarginOpcode(CERT_ConnectRequest);
			LogPacket(packetData.contents(),packetData.size(),CLIENT_TO_MARGIN,comment.str());

			//this packet contains clients copy of the signature & signed data, however it is wrong since we tampered with it before for our pubkey
			//so, just replace it with the cached copy from real server we gathered earlier
			packetToSend.clear();
			packetToSend << uint8(CERT_ConnectRequest);
			packetToSend << uint16(3);
			packetToSend << uint16(swap16(0x3601)); //auth data start, next is signature
			packetToSend.append(savedSignature,sizeof(savedSignature)); //next is signed data, also cached
			packetToSend.append((const byte*)&savedSignedData,sizeof(savedSignedData));

			//thats it for the packet, break, function epilog will add header as necessary
			break;
		}
	case CERT_ChallengeResponse:
		{
			comment << GetStringForMarginOpcode(CERT_ChallengeResponse) << endl;

			//we are not going to modify this packet, just snoop around :D , so just copy input packet as output
			packetToSend.clear();
			packetToSend.append((const byte*)packetData.contents(),packetData.size());

			byte clientsChallenge[16];
			packetData.read(clientsChallenge,sizeof(clientsChallenge));

			ConvertBytesintoHex(clientsChallenge,hexadecm,sizeof(clientsChallenge));
			comment << "Client's challenge: |" << hexadecm << "|, which ";

			if (!memcmp(clientsChallenge,theMarginBlobChallenge,sizeof(clientsChallenge)))
			{
				comment << "matches the server copy." << endl;
			}
			else
			{
				comment << "doesn't match the server copy!" << endl;
			}

			LogPacket(packetData.contents(),packetData.size(),CLIENT_TO_MARGIN,comment.str());
			break;
		}
	}

	if (encrypted == true && TFEncryptCTM != NULL)
	{
		EncryptedPacket giveMeDataToEncrypt;
		giveMeDataToEncrypt.append((const byte*)packetToSend.contents(),packetToSend.size());
		ByteBuffer encryptedData = giveMeDataToEncrypt.toCipherText(TFEncryptCTM);
		TCPVariableLengthPacket giveMeDataToAddLengthTo;
		giveMeDataToAddLengthTo.append((const byte*)encryptedData.contents(),encryptedData.size());
		ByteBuffer sendMe = giveMeDataToAddLengthTo.GetContentsWithSize();
		return string(sendMe.contents(),sendMe.size());
	}
	else
	{
		TCPVariableLengthPacket giveMeDataToAddLengthTo;
		giveMeDataToAddLengthTo.append((const byte*)packetToSend.contents(),packetToSend.size());
		ByteBuffer sendMe = giveMeDataToAddLengthTo.GetContentsWithSize();
		return string(sendMe.contents(),sendMe.size());
	}
}

string MarginToClient( const char* pData,size_t pSize )
{
	stringstream comment;
	string hexadecm;
	TCPVariableLengthPacket packetContents((const byte*)pData,pSize);

	bool encrypted = true;

	byte firstByte;
	packetContents >> firstByte;
	if (firstByte >= CERT_ConnectRequest && firstByte <= CERT_NewSessionKey)
	{
		uint16 firstShort;
		packetContents >> firstShort;

		if (firstShort == 3)
		{
			encrypted = false;
		}
	}

	//reset position (since we just read 3 bytes)
	packetContents.rpos(0);
	ByteBuffer packetData;

	if (encrypted == true && TFDecryptMTC != NULL)
	{
		try
		{
			EncryptedPacket packetTodecrypt(packetContents,TFDecryptMTC);
			packetData.append((const byte*)packetTodecrypt.contents(),packetTodecrypt.size());
			comment << "encrypted=true ";
		}
		catch (...)
		{
			ConvertBytesintoHex((const byte*)packetContents.contents(),hexadecm,packetContents.size());
			cout << "Decryption exception for MTC packet: |" << hexadecm << "|" << endl;
			encrypted = false;
		}
	}

	if (encrypted == false || TFDecryptMTC == NULL)
	{
		packetData.append((const byte*)packetContents.contents(),packetContents.size());
		comment << "encrypted=false ";
	}

	byte packetOpcode;
	packetData >> packetOpcode;

	ByteBuffer packetToSend;

	switch (packetOpcode)
	{
	default:
		{
			comment << GetStringForMarginOpcode(packetOpcode);
			LogPacket(packetData.contents(),packetData.size(),MARGIN_TO_CLIENT,comment.str());
			packetToSend = packetData;
			packetToSend.rpos(0);
			break;
		}
	case CERT_Challenge:
		{
			comment << GetStringForMarginOpcode(CERT_Challenge) << endl;

			//we are not going to modify this packet, just snoop around :D , so just copy input packet as output
			packetToSend.clear();
			packetToSend.append((const byte*)packetData.contents(),packetData.size());

			uint16 numberThree;
			packetData >> numberThree;
			uint16 rsaBlobSize;
			packetData >> rsaBlobSize;
			vector<byte> rsaBlob;
			rsaBlob.resize(rsaBlobSize);
			packetData.read(&rsaBlob[0],rsaBlob.size());

			ByteBuffer blobContents = RSAMarginBlobDecrypt(&rsaBlob[0],rsaBlob.size());

			uint8 alwaysZero;
			blobContents >> alwaysZero;

			if (alwaysZero != 0)
			{
				cout << "First byte of CERT_Challenge not right !" << endl;
			}

			byte theTwofishKey[16];
			blobContents.read(theTwofishKey,sizeof(theTwofishKey));
			blobContents.read(theMarginBlobChallenge,sizeof(theMarginBlobChallenge));

			//since we have twofish key, we can initialize all those twofish encryptors/decryptors (its the same for all margin/world)
			if (TFEncryptCTM != NULL)
				delete TFEncryptCTM;
			if (TFDecryptCTM != NULL)
				delete TFDecryptCTM;
			if (TFEncryptMTC != NULL)
				delete TFEncryptMTC;
			if (TFDecryptMTC != NULL)
				delete TFDecryptMTC;

			TFEncryptCTM = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption(theTwofishKey,sizeof(theTwofishKey),blankIV);
			TFDecryptCTM = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(theTwofishKey,sizeof(theTwofishKey),blankIV);
			TFEncryptMTC = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption(theTwofishKey,sizeof(theTwofishKey),blankIV);
			TFDecryptMTC = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(theTwofishKey,sizeof(theTwofishKey),blankIV);

			if (TFEncryptCTW != NULL)
				delete TFEncryptCTW;
			if (TFDecryptCTW != NULL)
				delete TFDecryptCTW;
			if (TFEncryptWTC != NULL)
				delete TFEncryptWTC;
			if (TFDecryptWTC != NULL)
				delete TFDecryptWTC;

			TFEncryptCTW = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption(theTwofishKey,sizeof(theTwofishKey),blankIV);
			TFDecryptCTW = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(theTwofishKey,sizeof(theTwofishKey),blankIV);
			TFEncryptWTC = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption(theTwofishKey,sizeof(theTwofishKey),blankIV);
			TFDecryptWTC = new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(theTwofishKey,sizeof(theTwofishKey),blankIV);

			ConvertBytesintoHex(theTwofishKey,hexadecm,sizeof(theTwofishKey));
			comment << "Margin/World Twofish Key: |" << hexadecm << "| TF Crypto Engines initialized!" << endl;

			ConvertBytesintoHex(theMarginBlobChallenge,hexadecm,sizeof(theMarginBlobChallenge));
			comment << "Margin CERT Challenge: |" << hexadecm << "|" << endl;

			LogPacket(packetData.contents(),packetData.size(),MARGIN_TO_CLIENT,comment.str());

			break;
		}
	}

	if (encrypted == true && TFEncryptMTC != NULL)
	{
		EncryptedPacket giveMeDataToEncrypt;
		giveMeDataToEncrypt.append((const byte*)packetToSend.contents(),packetToSend.size());
		ByteBuffer encryptedData = giveMeDataToEncrypt.toCipherText(TFEncryptMTC);
		TCPVariableLengthPacket giveMeDataToAddLengthTo;
		giveMeDataToAddLengthTo.append((const byte*)encryptedData.contents(),encryptedData.size());
		ByteBuffer sendMe = giveMeDataToAddLengthTo.GetContentsWithSize();
		return string(sendMe.contents(),sendMe.size());
	}
	else
	{
		TCPVariableLengthPacket giveMeDataToAddLengthTo;
		giveMeDataToAddLengthTo.append((const byte*)packetToSend.contents(),packetToSend.size());
		ByteBuffer sendMe = giveMeDataToAddLengthTo.GetContentsWithSize();
		return string(sendMe.contents(),sendMe.size());
	}
}

uint16 CTW_remoteSeq = 0;
uint16 CTW_localSeq = 0;
uint8 CTW_flags = 0;

uint16 WTC_remoteSeq = 0;
uint16 WTC_localSeq = 0;
uint8 WTC_flags = 0;

string ClientToWorld( const char* pData,size_t pSize )
{
	stringstream comment;
	if (pData[0] != 0x01 || TFDecryptCTW == NULL)
	{
		//data is not encrypted, no idea what to do with it, just log as is
		comment << "Not Encrypted";
		LogPacket(pData,pSize,CLIENT_TO_WORLD,comment.str());
		return string(pData,pSize);
	}
	else
	{
		//packet is encrypted, lets decrypt, log the plaintext, then reencrypt
		ByteBuffer encryptedContents;
		encryptedContents.append((const byte*)&pData[1],pSize-1); //skip the first byte
		EncryptedPacket encryptionless(encryptedContents,TFDecryptCTW);
		SequencedPacket headerless(encryptionless);

		CTW_remoteSeq = headerless.getRemoteSeq();
		CTW_localSeq = headerless.getLocalSeq();
		CTW_flags = headerless.getPSS();

		LogWorldPacket(headerless.contents(),headerless.size(),CLIENT_TO_WORLD,CTW_localSeq,CTW_remoteSeq,CTW_flags);

		//reencrypt
		ByteBuffer reencrypted = encryptionless.toCipherText(TFEncryptCTW);
		ByteBuffer withLeadingZeroOne;
		withLeadingZeroOne << uint8(0x01);
		withLeadingZeroOne.append((const byte*)reencrypted.contents(),reencrypted.size());

		return string(withLeadingZeroOne.contents(),withLeadingZeroOne.size());
	}
}

string WorldToClient( const char* pData,size_t pSize )
{
	stringstream comment;
	if (pData[0] != 0x01 || TFDecryptWTC == NULL)
	{
		//data is not encrypted, no idea what to do with it, just log as is
		comment << "Not Encrypted";
		LogPacket(pData,pSize,WORLD_TO_CLIENT,comment.str());
		return string(pData,pSize);
	}
	else
	{
		//packet is encrypted, lets decrypt, log the plaintext, then reencrypt
		ByteBuffer encryptedContents;
		encryptedContents.append((const byte*)&pData[1],pSize-1); //skip the first byte
		EncryptedPacket encryptionless(encryptedContents,TFDecryptWTC);
		SequencedPacket headerless(encryptionless);

		WTC_remoteSeq = headerless.getRemoteSeq();
		WTC_localSeq = headerless.getLocalSeq();
		WTC_flags = headerless.getPSS();

		LogWorldPacket(headerless.contents(),headerless.size(),WORLD_TO_CLIENT,WTC_localSeq,WTC_remoteSeq,WTC_flags);

		//reencrypt
		ByteBuffer reencrypted = encryptionless.toCipherText(TFEncryptWTC);
		ByteBuffer withLeadingZeroOne;
		withLeadingZeroOne << uint8(0x01);
		withLeadingZeroOne.append((const byte*)reencrypted.contents(),reencrypted.size());

		return string(withLeadingZeroOne.contents(),withLeadingZeroOne.size());
	}
}