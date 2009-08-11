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
	uint32 packetNum;
	string username;

	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption Decryptor;
	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption Encryptor;

	auto_ptr<Decryptor> TFDecrypt;
	auto_ptr<Encryptor> TFEncrypt;

	static const byte blankIV[16];
	byte challenge[16];
	byte finalChallenge[16];

	string zeUsername;
};


#endif
