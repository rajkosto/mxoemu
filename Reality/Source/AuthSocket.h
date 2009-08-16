#ifndef MXOSIM_AUTHSOCKET_H
#define MXOSIM_AUTHSOCKET_H

#include "TCPVarLenSocket.h"
#include "Common.h"
#include "Crypto.h"

class AuthSocket : public TCPVarLenSocket
{
public:
	AuthSocket(ISocketHandler& );
	~AuthSocket();
private:
	void ProcessData(const byte *buf,size_t len);
	bool VerifyPassword( const string& plaintextPass, const string& passwordSalt, const string& passwordHash );
	uint32 packetNum;
	string username;

	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption Decryptor;
	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption Encryptor;

	auto_ptr<Decryptor> TFDecrypt;
	auto_ptr<Encryptor> TFEncrypt;

	static const byte blankIV[16];
	byte challenge[16];
	byte finalChallenge[16];

	uint32 m_userId;
	string m_username;
	string m_passwordSalt;
	string m_passwordHash;
	uint16 m_publicExponent;
	string m_publicModulus;
	string m_privateExponent;
	uint32 m_timeCreated;
};


#endif
