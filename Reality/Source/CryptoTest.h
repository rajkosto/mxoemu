#ifndef CRYPTOTEST_H
#define CRYPTOTEST_H

#define UNITTEST

#include "Common.h"
#include "Crypto.h"
#include "Util.h"

unsigned char key[16] =
{
	0x27, 0x68, 0x78, 0xB0, 0x0B, 0xE2, 0x07, 0xF5, 0x6D, 0x83, 0xA9, 0xE4, 0x48, 0xE0, 0xA0, 0xFC, 
} ;

unsigned char ciphertext[16] =
{
	0x85, 0x8D, 0x98, 0x9D, 0x91, 0x51, 0x38, 0x30, 0x87, 0xE5, 0x13, 0xAB, 0x05, 0xC7, 0x8E, 0x58, 
} ;


void runTest()
{
	byte bullshit[16];
	memset(bullshit,0,sizeof(bullshit));
	string hexadec;
	CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption twofishDec(key,sizeof(key),bullshit);
	string cipherInput((const char*)ciphertext,sizeof(ciphertext));
	string CipherOutput;
	CryptoPP::StringSource(cipherInput, true, new CryptoPP::StreamTransformationFilter(twofishDec, new CryptoPP::StringSink(CipherOutput),CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING));
	((const byte *)CipherOutput.data(),hexadec,CipherOutput.size(),false);
	cout << "Decrypted Data: |" << Bin2Hex(CipherOutput) << "|" << endl;

/*	CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption TFDecrypt(key, sizeof(key),(const byte*)CipherOutput.data());
	string zeInput((const char*)cipherText,sizeof(cipherText));
	string zeOutput;
	CryptoPP::StringSource(zeInput, true, new CryptoPP::StreamTransformationFilter(TFDecrypt, new CryptoPP::StringSink(zeOutput),CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING));

	ConvertBytesintoHex((const byte *)zeOutput.data(),hexadec,zeOutput.size(),false);
	cout << "|" << hexadec << "|" << endl;*/

}

#endif