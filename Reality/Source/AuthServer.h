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
private:
	void GenerateRSAKeys(unsigned int keyLen,CryptoPP::RSA::PublicKey &publicOutput, CryptoPP::RSA::PrivateKey &privateOutput);
	void GenerateSignKeys(string &privKeyOut, string &pubKeyOut);
	void LoadSignKeys();
	ByteBuffer MessageFromPublicKey(CryptoPP::RSA::PublicKey &inputKey);
	void GenerateCryptoKeys(string &privKeyOut, string &pubKeyOut);
	void LoadCryptoKeys();

	AuthHandler authSocketHandler;
	typedef ListenSocket<AuthSocket> AuthListenSocket;
	AuthListenSocket *listenSocketInst;

	CryptoPP::AutoSeededRandomPool randPool;

	auto_ptr<CryptoPP::RSAES_OAEP_SHA_Decryptor> rsaDecryptor;
	auto_ptr<CryptoPP::RSAES_OAEP_SHA_Encryptor> rsaEncryptor;
	auto_ptr<CryptoPP::Weak::RSASSA_PKCS1v15_MD5_Signer> signer1024bit;
	auto_ptr<CryptoPP::Weak::RSASSA_PKCS1v15_MD5_Verifier> verifier1024bit;
	auto_ptr<CryptoPP::Weak::RSASSA_PKCS1v15_MD5_Signer> signer2048bit;
	auto_ptr<CryptoPP::Weak::RSASSA_PKCS1v15_MD5_Verifier> verifier2048bit;

	CryptoPP::Integer pubKeyModulus;
	vector<byte> pubKeySignature;
};


#define sAuth AuthServer::getSingleton()

#endif

