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

///////////////////////////////////////////////////////////////////////////////
//
// mxohax.cpp
//

#include <windows.h>
#include "..\detours\include\detours.h"

#include "stdafx.h"
#include "mxohax.h"


HMODULE hModule = NULL;
HMODULE hProcessModule = NULL;

bool bKeySent = FALSE;

CRITICAL_SECTION KeySentCriticalSection;

GetKey_ GetKey = NULL;
CBCEncryption_ProcessBlocks_ CBCEncryption_ProcessBlocks = NULL;
CBCDecryption_ProcessBlocks_ CBCDecryption_ProcessBlocks = NULL;
VerifyMessage_ VerifyMessage = NULL;
HashUpdate_ HashUpdate = NULL;
GetMaterial_ GetMaterial = NULL;
PK_Decryptor_Decrypt_ PK_Decryptor_Decrypt = NULL;
GetMaterial_ GetMaterial2 = NULL;

///////////////////////////////////////////////////////////////////////////////
//
// Microsoft's Detours library requires an exported function with ordinal #1.
//
void ExportedOrdinal1()
{
    return;
}

///////////////////////////////////////////////////////////////////////////////
//
// FindGetKey()
//
GetKey_ FindGetKey (DWORD moduleBase)
{
    DWORD address = NULL;

    // TODO: Limit this search to the memory range of the module.

    for ( DWORD i = moduleBase; ; ++i )
    {
        if ( 0x051C418B == (*reinterpret_cast<DWORD*>(i)) &&
             0x00000085 == (*reinterpret_cast<DWORD*>(i+4)) &&
             0xC3 == (*reinterpret_cast<BYTE*>(i+8)) )
        {
			//found the function
            address = i;
            break;
        }
    }

    return ( reinterpret_cast<GetKey_>(address) );
}

//
// FindCBCDecryption_ProcessBlocks()
//
CBCDecryption_ProcessBlocks_ FindCBCDecryption_ProcessBlocks (DWORD moduleBase)
{
	DWORD address = NULL;

	const byte functionStart[] = 
	{
		0x55, 0x8B, 0xEC, 0x51, 0x53, 0x8B, 0xD9, 0x8B, 0x43, 0x10, 0x89, 0x45, 0xFC, 0x8B, 0x45, 0x10,
	} ;

	for ( DWORD i = moduleBase; ;++i )
	{
		byte *dataPointer = reinterpret_cast<byte *>(i);
		if (!memcmp(dataPointer,functionStart,sizeof(functionStart)))
		{
			//found the function
			address = i;
			break;
		}
	}

	return ( reinterpret_cast<CBCDecryption_ProcessBlocks_>(address) );
}

//
// FindCBCEncryption_ProcessBlocks()
//
CBCEncryption_ProcessBlocks_ FindCBCEncryption_ProcessBlocks (DWORD moduleBase)
{
	DWORD address = NULL;

	const byte functionStart[] = 
	{
		0x55, 0x8B, 0xEC, 0x51, 0x8B, 0x45, 0x10, 0x85, 0xC0, 0x53, 0x8B, 0xD9, 0x56, 0x8B, 0x73, 0x10, 
	} ;

	for ( DWORD i = moduleBase; ;++i )
	{
		byte *dataPointer = reinterpret_cast<byte *>(i);
		if (!memcmp(dataPointer,functionStart,sizeof(functionStart)))
		{
			//found the function
			address = i;
			break;
		}
	}

	return ( reinterpret_cast<CBCEncryption_ProcessBlocks_>(address) );
}

//
// FindVerifyMessage()
//
VerifyMessage_ FindVerifyMessage (DWORD moduleBase)
{
	DWORD address = NULL;

	const byte functionStart[16] =
	{
		0x55, 0x8B, 0xEC, 0x53, 0x56, 0x8B, 0xF1, 0x8B, 0x06, 0x57, 0xFF, 0x50, 0x1C, 0x8B, 0x4D, 0x10, 
	} ;

	for ( DWORD i = moduleBase; ;++i )
	{
		byte *dataPointer = reinterpret_cast<byte *>(i);
		if (!memcmp(dataPointer,functionStart,sizeof(functionStart)))
		{
			//found the function
			address = i;
			break;
		}
	}

	return ( reinterpret_cast<VerifyMessage_>(address) );
}

//
// FindHashUpdate()
//
HashUpdate_ FindHashUpdate (DWORD moduleBase)
{
	DWORD address = NULL;

	const byte functionStart[16] =
	{
		0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x0C, 0x53, 0x56, 0x8B, 0xD9, 0x8B, 0x73, 0x1C, 0x57, 0x8B, 0x7D, 
	} ;

	for ( DWORD i = moduleBase; ;++i )
	{
		byte *dataPointer = reinterpret_cast<byte *>(i);
		if (!memcmp(dataPointer,functionStart,sizeof(functionStart)))
		{
			//found the function
			address = i;
			break;
		}
	}

	return ( reinterpret_cast<HashUpdate_>(address) );
}

//
// FindGetMaterial()
//
GetMaterial_ FindGetMaterial (DWORD moduleBase)
{
	DWORD address = NULL;

	const byte functionStart[16] =
	{
		0x8B, 0x01, 0xFF, 0x50, 0x18, 0x85, 0xC0, 0x75, 0x01, 0xC3, 0x8B, 0x08, 0x8B, 0x49, 0x04, 0x03, 
	} ;


	for ( DWORD i = moduleBase; ;++i )
	{
		byte *dataPointer = reinterpret_cast<byte *>(i);
		if (!memcmp(dataPointer,functionStart,sizeof(functionStart)))
		{
			//found the function
			address = i;
			break;
		}
	}

	return ( reinterpret_cast<GetMaterial_>(address) );
}

//
// FindPK_Decryptor_Decrypt()
//
PK_Decryptor_Decrypt_ FindPK_Decryptor_Decrypt (DWORD moduleBase)
{
	DWORD address = NULL;

	const byte functionStart[16] =
	{
		0x55, 0x8B, 0xEC, 0x56, 0x8B, 0xF1, 0x8B, 0x46, 0x04, 0x8B, 0x48, 0x08, 0x8B, 0x54, 0x31, 0x04, 
	} ;

	for ( DWORD i = moduleBase; ;++i )
	{
		byte *dataPointer = reinterpret_cast<byte *>(i);
		if (!memcmp(dataPointer,functionStart,sizeof(functionStart)))
		{
			//found the function
			address = i;
			break;
		}
	}

	return ( reinterpret_cast<PK_Decryptor_Decrypt_>(address) );
}

//
// FindGetMaterial2()
//
GetMaterial_ FindGetMaterial2 (DWORD moduleBase)
{
	DWORD address = NULL;

	const byte functionStart[16] =
	{
		0x8B, 0x01, 0xFF, 0x50, 0x18, 0x85, 0xC0, 0x75, 0x01, 0xC3, 0x8B, 0x48, 0x04, 0x8B, 0x51, 0x04, 
	} ;

	for ( DWORD i = moduleBase; ;++i )
	{
		byte *dataPointer = reinterpret_cast<byte *>(i);
		if (!memcmp(dataPointer,functionStart,sizeof(functionStart)))
		{
			//found the function
			address = i;
			break;
		}
	}

	return ( reinterpret_cast<GetMaterial_>(address) );
}

void LogString(string &title,string &contents);

///////////////////////////////////////////////////////////////////////////////
//
// SendKey()
//
BOOL SendKey (void* key)
{
	BOOL	bResult = TRUE;
	HANDLE	hPipe = CreateFile(SEND_KEY_PIPE_NAME,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,0);

	if (hPipe != INVALID_HANDLE_VALUE)
	{
		DWORD numBytes = 0;
		if ( WriteFile(hPipe,key,TWOFISH_KEY_LENGTH,&numBytes,NULL) == FALSE || numBytes != TWOFISH_KEY_LENGTH)
		{
			LogString(string("Major Error"),string("Failed to send the world key to proxy!\n"));
			bResult = FALSE;
		}
		CloseHandle(hPipe);
	}
	else
	{
		ostringstream nub;
		nub << "Pipe error: ( 0x" << hex << GetLastError() << " ) !" << endl;
		LogString(string("Error creating pipe"),nub.str());
		bResult = FALSE;
	}

    return (bResult);
}

string AddKey(const char* name,const byte* data,size_t size);

///////////////////////////////////////////////////////////////////////////////
//
// DetouredGetKey()
//
void* __stdcall DetouredGetKey ()
{
    DWORD thisPtr;
    void* key;

    // This instruction must precede all other statements, with the exception
    // of uninitialized local variable definitions.
    __asm { mov thisPtr, ecx } // Save GetKey's 'this' pointer.

    // We have to manually set up the call to this function, because it
    // requires the 'this' pointer in ECX, which could otherwise be destroyed
    // by compiler register usage.
    __asm
    { 
        mov ecx, thisPtr    // Retrieve GetKey's 'this' pointer.
        call GetKey
        mov key, eax    // Save the result of the call.
    }

    EnterCriticalSection (&KeySentCriticalSection);

    if ( FALSE == bKeySent )
    {
		// Add key to cache
		AddKey("WORLD_KEY",(byte*)key,16);
        // Send out the key.
        SendKey (key);
        bKeySent = TRUE;
    }

    LeaveCriticalSection (&KeySentCriticalSection);

    return (key);
}

void LogString(string &title,string &contents)
{
	char dateStr [9];
	char timeStr [9];
	_strdate(dateStr);
	_strtime(timeStr);
	std::ofstream File;
	File.open("Crypto.log",std::ios::app);
	File << "[" << timeStr << " " << dateStr  << "]" << " " << title << endl << contents << endl;
	File.close();
}

void ConvertBytesintoHex(const byte *data,string &string,unsigned int count)
{
	if (data != NULL)
	{
		ostringstream myStream;

		int i = 0;
		for (unsigned int j = 0;j < count;j++)
		{
			i++;
			if (i == 33)
			{
				myStream << endl;
				i = 1;
			}
			byte n = data[j];
			if (n <= 15)
				myStream << "0";
			myStream << hex << (int)n;
			myStream << " ";
		}
		myStream.flush();
		string = myStream.str();
	}
}
typedef map< string,vector<byte> > keyMap;
keyMap symmetricKeys;

int keyCounter = 1;

string AddKey(const char* name,const byte* data,size_t size)
{
	string nameStr;
	if (name != NULL)
	{
		nameStr = string(name);
	}
	else
	{
		stringstream derp;
		derp << "KEY_" << dec << keyCounter;
		nameStr = derp.str();
		keyCounter++;
	}

	vector<byte> keyData;
	keyData.resize(size);
	memcpy(&keyData[0],data,keyData.size());
	symmetricKeys[nameStr] = keyData;

	string title = "New Key Added";
	stringstream contents;
	string hexadecm;
	ConvertBytesintoHex(&keyData[0],hexadecm,keyData.size());
	contents << "[" << nameStr << "]" << " = { " << hexadecm << "}" << endl;
	LogString(title,contents.str());

	return nameStr;
}

typedef enum
{
	CRYPTO_DECRYPTION,
	CRYPTO_ENCRYPTION
} CryptoType;

void LogCrypto(const byte *cipher,const byte *plain,size_t pSize,const byte *key,CryptoType direction)
{
	string dirStr;
	if (direction == CRYPTO_DECRYPTION)
	{
		dirStr = "CBC Decrypt";
	}
	else if (direction == CRYPTO_ENCRYPTION)
	{
		dirStr = "CBC Encrypt";
	}

	string KeyName;

	keyMap::iterator keyNode;

	for (keyNode = symmetricKeys.begin();keyNode != symmetricKeys.end();keyNode++)
	{
		vector<byte> keyData = keyNode->second;
		
		if (!memcmp(&keyData[0],key,keyData.size()))
		{
			KeyName = keyNode->first;
			break;
		}
	}

	if (KeyName.length() < 1)
	{
		KeyName = AddKey(NULL,key,16);
	}

	ostringstream contents;

	string cipherHex,plainHex;
	ConvertBytesintoHex(cipher,cipherHex,pSize);
	ConvertBytesintoHex(plain,plainHex,pSize);

	contents << "Key: " << KeyName << endl;

	if (direction == CRYPTO_DECRYPTION)
	{
		contents << "Cipher (" << dec << pSize << " bytes):" << endl;
		contents << cipherHex << endl;
		contents << "Plain (" << dec << pSize << " bytes):" << endl;
		contents << plainHex << endl;
	}
	else if (direction == CRYPTO_ENCRYPTION)
	{		
		contents << "Plain (" << dec << pSize << " bytes):" << endl;
		contents << plainHex << endl;
		contents << "Cipher (" << dec << pSize << " bytes):" << endl;
		contents << cipherHex << endl;
	}

	LogString(dirStr,contents.str());
}

bool firstTime = true;

#include <CryptoPP/md5.h>

void AddFirstKeys(const byte* authKey,const byte *challenge,const byte *challengeDecrypted)
{
	AddKey("AUTH_KEY",authKey,16);
	AddKey("CHALLENGE",challenge,16);
	AddKey("CHALLENGE_DEC",challengeDecrypted,16);

	CryptoPP::MD5 md5object;
	//feed decrypted challenge to md5 object
	md5object.Update(challengeDecrypted,16);
	//end hash goes to md5Hashed
	byte md5Hashed[16];
	md5object.Final(md5Hashed);

	AddKey("CHALLENGE_DEC_MD5",md5Hashed,sizeof(md5Hashed));
}

///////////////////////////////////////////////////////////////////////////////
//
// DetouredCBCDecryption()
//
void __stdcall DetouredCBCDecryption_ProcessBlocks(byte *outBuffer,byte *inBuffer,unsigned int numBlocks)
{
	DWORD thisPtr;

	// This instruction must precede all other statements, with the exception
	// of uninitialized local variable definitions.
	__asm { mov thisPtr, ecx } // Save the 'this' pointer.

	byte *copyOutBuffer = outBuffer;
	byte *copyInBuffer = inBuffer;
	unsigned int copyNumBlocks = numBlocks;

	//save inBuffer to our own buffer in case the transform is in place
	vector<byte> copiedInBuffer;
	copiedInBuffer.resize(copyNumBlocks*16);
	memcpy(&copiedInBuffer[0],copyInBuffer,copiedInBuffer.size());

	//save key
	byte *objInstance = (byte*)thisPtr;
	byte *keyInObj = &objInstance[0xBC];
	byte theKey[16];
	memcpy(theKey,keyInObj,sizeof(theKey));

	// We have to manually set up the call to this function, because it
	// requires the 'this' pointer in ECX, which could otherwise be destroyed
	// by compiler register usage.
	__asm
	{ 
		//since params are popped, we need to push them back, in reverse order
		mov ecx, copyNumBlocks // Copy 3rd var to ecx
		push ecx // Then push it
		mov ecx, copyInBuffer // Copy 2nd var to ecx
		push ecx // Then push it
		mov ecx, copyOutBuffer // Copy 1st var to ecx
		push ecx // Then Push it
		//the stack should be in the exact same state it was before we detoured

		mov ecx, thisPtr    // Put saved 'this' pointer into the place it should be
		call CBCDecryption_ProcessBlocks // call the original function
	}

	if (firstTime == true)
	{
		firstTime = false;
		AddFirstKeys(theKey,&copiedInBuffer[0],copyOutBuffer);
	}

	LogCrypto(&copiedInBuffer[0],copyOutBuffer,copyNumBlocks*16,theKey,CRYPTO_DECRYPTION);
}

///////////////////////////////////////////////////////////////////////////////
//
// DetouredCBCEncryption()
//
void __stdcall DetouredCBCEncryption_ProcessBlocks(byte *outBuffer,byte *inBuffer,unsigned int numBlocks)
{
	DWORD thisPtr;

	// This instruction must precede all other statements, with the exception
	// of uninitialized local variable definitions.
	__asm { mov thisPtr, ecx } // Save the 'this' pointer.

	byte *copyOutBuffer = outBuffer;
	byte *copyInBuffer = inBuffer;
	unsigned int copyNumBlocks = numBlocks;

	//save inBuffer to our own buffer in case the transform is in place
	vector<byte> copiedInBuffer;
	copiedInBuffer.resize(copyNumBlocks*16);
	memcpy(&copiedInBuffer[0],copyInBuffer,copiedInBuffer.size());

	//save key
	byte *objInstance = (byte*)thisPtr;
	byte *keyInObj = &objInstance[0xB0];
	byte theKey[16];
	memcpy(theKey,keyInObj,sizeof(theKey));

	// We have to manually set up the call to this function, because it
	// requires the 'this' pointer in ECX, which could otherwise be destroyed
	// by compiler register usage.
	__asm
	{ 
		//since params are popped, we need to push them back, in reverse order
		mov ecx, copyNumBlocks // Copy 3rd var to ecx
		push ecx // Then push it
		mov ecx, copyInBuffer // Copy 2nd var to ecx
		push ecx // Then push it
		mov ecx, copyOutBuffer // Copy 1st var to ecx
		push ecx // Then Push it
		//the stack should be in the exact same state it was before we detoured

		mov ecx, thisPtr    // Put saved 'this' pointer into the place it should be
		call CBCEncryption_ProcessBlocks // call the original function
	}

	LogCrypto(copyOutBuffer,&copiedInBuffer[0],copyNumBlocks*16,theKey,CRYPTO_ENCRYPTION);
}

#include <CryptoPP/cryptlib.h>
#include <CryptoPP/pubkey.h>
#include <CryptoPP/rsa.h>

///////////////////////////////////////////////////////////////////////////////
//
// VerifyMessage()
//
bool __stdcall DetouredVerifyMessage(byte *message,
									 unsigned int messageLen,
									 byte *signature,
									 unsigned int signatureLength)
{
	DWORD thisPtr;

	// This instruction must precede all other statements, with the exception
	// of uninitialized local variable definitions.
	__asm { mov thisPtr, ecx } // Save the 'this' pointer.

	// this pops the parameters
	byte *copyMessage = message;
	unsigned int copyMessageLen = messageLen;
	byte *copySignature = signature;
	unsigned int copySignatureLength = signatureLength;

	DWORD returnValue;

	// We have to manually set up the call to this function, because it
	// requires the 'this' pointer in ECX, which could otherwise be destroyed
	// by compiler register usage.
	__asm
	{ 
		//since params are popped, we need to push them back, in reverse order
		mov ecx, copySignatureLength // Copy 4th var to ecx
		push ecx // Then push it
		mov ecx, copySignature // Copy 3rd var to ecx
		push ecx // Then push it
		mov ecx, copyMessageLen // Copy 2nd var to ecx
		push ecx // Then push it
		mov ecx, copyMessage // Copy 1st var to ecx
		push ecx // Then Push it
		//the stack should be in the exact same state it was before we detoured

		mov ecx, thisPtr    // Put saved 'this' pointer into the place it should be
		call VerifyMessage // call the original function
		mov returnValue, eax
	}

	string messageHex;
	ConvertBytesintoHex(copyMessage,messageHex,copyMessageLen);
	string signatureHex;
	ConvertBytesintoHex(copySignature,signatureHex,copySignatureLength);
	string thisHex;
	ConvertBytesintoHex((byte*)thisPtr,thisHex,512);

	void *GetMaterialReturnValue = NULL;

	string materialHex;

	// We have to manually set up the call to this function, because it
	// requires the 'this' pointer in ECX, which could otherwise be destroyed
	// by compiler register usage.
	__asm
	{ 
			mov ecx, thisPtr    // Put saved 'this' pointer into the place it should be
			call GetMaterial // call the original function
			mov GetMaterialReturnValue, eax
	}

	if (GetMaterialReturnValue == NULL)
	{
		LogString(string("GetMaterial was null"),string(""));
	}
	else
	{
		CryptoPP::CryptoMaterial* someMaterial = (CryptoPP::CryptoMaterial*)(GetMaterialReturnValue);
		string outputz;
		CryptoPP::StringSink outputSink(outputz);
		someMaterial->Save(outputSink);
		if (outputz.size() > 0)
		{
			ConvertBytesintoHex((const byte*)outputz.data(),materialHex,outputz.size());
		}
	}

	stringstream derp;

	if (materialHex.size() > 0)
	{
		derp << "Public Key DER: " << "|" << materialHex << "|" << endl;
	}

	derp << "Signature: " << "|" << signatureHex << "|" << endl;
	derp << "Message: " << "|" << messageHex << "|" << endl;
	derp << "Return value: " << dec << returnValue << endl;

	LogString(string("VerifyMessage"),derp.str());

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// DetouredHashUpdate()
//
void __stdcall DetouredHashUpdate(byte *input, unsigned int len)
{
	DWORD thisPtr;

	// This instruction must precede all other statements, with the exception
	// of uninitialized local variable definitions.
	__asm { mov thisPtr, ecx } // Save the 'this' pointer.

	// this pops the parameters
	byte *copyInput = input;
	unsigned int copyLen = len;

	//copy input in case something happens to it
	byte *someData = new byte[copyLen];
	memcpy(someData,copyInput,copyLen);

	// We have to manually set up the call to this function, because it
	// requires the 'this' pointer in ECX, which could otherwise be destroyed
	// by compiler register usage.
	__asm
	{ 
		//since params are popped, we need to push them back, in reverse order
		mov ecx, copyLen // Copy 2nd var to ecx
		push ecx // Then push it
		mov ecx, copyInput // Copy 1st var to ecx
		push ecx // Then Push it
		//the stack should be in the exact same state it was before we detoured

		mov ecx, thisPtr    // Put saved 'this' pointer into the place it should be
		call HashUpdate // call the original function
	}

	string inputHex;

	if (copyLen < 2048)
	{
		ConvertBytesintoHex(someData,inputHex,copyLen);
	}

	delete [] someData;

	if (copyLen < 2048)
	{
		LogString(string("HashUpdate"),inputHex);
	}
	else
	{
		stringstream derp;
		derp << "Len too big: " << dec << copyLen;
		LogString(string("HashUpdate"),derp.str());
	}
}

void LogPK_Decryptor_Decrypt(DWORD thisPointer,const CryptoPP::DecodingResult *decodingResult,
							 const byte *cipherText,
							 unsigned int cipherLen,
							 const byte *plainText
							 )
{
	stringstream theContents;

	theContents << "status: " << decodingResult->isValidCoding << endl;

	string cipherHex;
	ConvertBytesintoHex(cipherText,cipherHex,cipherLen);
	theContents << "cipherText: " << "|" << cipherHex << "|" << endl;

	if (decodingResult->isValidCoding == true)
	{
		string plainHex;
		ConvertBytesintoHex(plainText,plainHex,decodingResult->messageLength);
		theContents << "plainText: " << "|" << plainHex << "|" << endl;
	}

	DWORD GetMaterialReturnValue = 0;

	// We have to manually set up the call to this function, because it
	// requires the 'this' pointer in ECX, which could otherwise be destroyed
	// by compiler register usage.
	__asm
	{ 
		mov ecx, thisPointer    // Put saved 'this' pointer into the place it should be
		call GetMaterial2 // call the GetMaterial2 function
		mov GetMaterialReturnValue, eax
	}

	theContents << "getMaterial: " << hex << GetMaterialReturnValue << endl;
	
	string materialHex;
	CryptoPP::CryptoMaterial *theMaterialz = (CryptoPP::CryptoMaterial*)(GetMaterialReturnValue);
	string outputz;
	CryptoPP::StringSink outputSink(outputz);
	theMaterialz->Save(outputSink);
	if (outputz.size() > 0)
	{
		ConvertBytesintoHex((const byte*)outputz.data(),materialHex,outputz.size());
	}

	theContents << "material: " << "|" << materialHex << "|" << endl;

	CryptoPP::RSA::PrivateKey thePkeyBro;
	CryptoPP::StringSource inputSource(outputz,true);
	thePkeyBro.Load(inputSource);

	theContents << "d: " << "|" << thePkeyBro.GetPrivateExponent() << "|" << endl;

	LogString(string("PK_Decrypt"),theContents.str());
}

///////////////////////////////////////////////////////////////////////////////
//
// DetouredPK_Decryptor_Decrypt()
//
void *__stdcall DetouredPK_Decryptor_Decrypt(void *decodingResult, //OUT return value
											 void *rng, //IN
											 byte *cipherText, //IN 
											 unsigned int cipherLen, //IN 
											 byte *plainText //OUT
											 )
{
	DWORD thisPtr;
	void *returnValue;

	// This instruction must precede all other statements, with the exception
	// of uninitialized local variable definitions.
	__asm { mov thisPtr, ecx } // Save the 'this' pointer.

	// this pops the parameters
	void *copyDecodingResult = decodingResult;
	void *copyRng = rng;
	byte *copyCipherText = cipherText;
	unsigned int copyCipherLen = cipherLen;
	byte *copyPlainText = plainText;

	byte *preservedCipherText = new byte[copyCipherLen];
	memcpy(preservedCipherText,copyCipherText,copyCipherLen);

	// We have to manually set up the call to this function, because it
	// requires the 'this' pointer in ECX, which could otherwise be destroyed
	// by compiler register usage.
	__asm
	{ 
		//since params are popped, we need to push them back, in reverse order
		mov ecx, copyPlainText
		push ecx
		mov ecx, copyCipherLen
		push ecx
		mov ecx, copyCipherText
		push ecx
		mov ecx, copyRng
		push ecx
		mov ecx, copyDecodingResult
		push ecx
		//the stack should be in the exact same state it was before we detoured

		mov ecx, thisPtr    // Put saved 'this' pointer into the place it should be
		call PK_Decryptor_Decrypt // call the original function
		mov returnValue, eax
	}

	LogPK_Decryptor_Decrypt(thisPtr,(CryptoPP::DecodingResult*)copyDecodingResult,preservedCipherText,copyCipherLen,copyPlainText);
	delete [] preservedCipherText;

	return returnValue;
}

static HANDLE (WINAPI * RealCreateFile)(LPCTSTR lpFileName,
										DWORD dwDesiredAccess,
										DWORD dwShareMode,
										LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										DWORD dwCreationDisposition,
										DWORD dwFlagsAndAttributes,
										HANDLE hTemplateFile) = CreateFile;


HANDLE WINAPI DetouredCreateFile(LPCTSTR lpFileName,
								 DWORD dwDesiredAccess,
								 DWORD dwShareMode,
								 LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
								 DWORD dwCreationDisposition,
								 DWORD dwFlagsAndAttributes,
								 HANDLE hTemplateFile)

{
	string originalFileName = string(lpFileName);

	if (originalFileName.find("matrix.exe") != string::npos || originalFileName.find("matrix2.exe") != string::npos)
	{
		stringstream derp;
		derp << "[" << originalFileName << "]" << "->" << "[" << "launcher.exe" << "]" << endl; 
		LogString(string("CreateFile detour"),derp.str());
		return RealCreateFile(	"launcher.exe",
								dwDesiredAccess,
								dwShareMode,
								lpSecurityAttributes,
								dwCreationDisposition,
								dwFlagsAndAttributes,
								hTemplateFile);
	}
	else
	{
		LogString(string("CreateFile normal"),originalFileName);
		return RealCreateFile(	lpFileName,
								dwDesiredAccess,
								dwShareMode,
								lpSecurityAttributes,
								dwCreationDisposition,
								dwFlagsAndAttributes,
								hTemplateFile);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// DllMain()
//
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

    // Perform DLL initialization.
    if (DLL_PROCESS_ATTACH == fdwReason)
    {
        hModule = hinstDLL;
        hProcessModule = GetModuleHandle (NULL);

        ///////////////////////////////////////////////////////////////////////
        //
        // Attempt to hook the methods.
        //
        
        GetKey = FindGetKey ( reinterpret_cast<DWORD>(hProcessModule) );
        if ( NULL == GetKey )
        {
            MessageBox (NULL, "Failed to locate the GetKey() method!", DLL_NAME, MB_OK);
            return (FALSE);
        }

		CBCDecryption_ProcessBlocks = FindCBCDecryption_ProcessBlocks( reinterpret_cast<DWORD>(hProcessModule) );
		if ( NULL == CBCDecryption_ProcessBlocks )
		{
			MessageBox (NULL, "Failed to locate the CBCDecryption_ProcessBlocks() method!", DLL_NAME, MB_OK);
			return (FALSE);
		}

		CBCEncryption_ProcessBlocks = FindCBCEncryption_ProcessBlocks( reinterpret_cast<DWORD>(hProcessModule) );
		if ( NULL == CBCEncryption_ProcessBlocks )
		{
			MessageBox (NULL, "Failed to locate the CBCEncryption_ProcessBlocks() method!", DLL_NAME, MB_OK);
			return (FALSE);
		}

		VerifyMessage = FindVerifyMessage( reinterpret_cast<DWORD>(hProcessModule) );
		if ( NULL == VerifyMessage )
		{
			MessageBox (NULL, "Failed to locate the VerifyMessage() method!", DLL_NAME,	MB_OK);
			return (FALSE);
		}

		HashUpdate = FindHashUpdate( reinterpret_cast<DWORD>(hProcessModule) );
		if ( NULL == HashUpdate )
		{
			MessageBox (NULL, "Failed to locate the HashUpdate() method!", DLL_NAME, MB_OK);
			return (FALSE);
		}

		GetMaterial = FindGetMaterial( reinterpret_cast<DWORD>(hProcessModule) );
		if ( NULL == GetMaterial )
		{
			MessageBox (NULL, "Failed to locate the GetMaterial() method!", DLL_NAME, MB_OK);
			return (FALSE);
		}

		GetMaterial2 = FindGetMaterial2( reinterpret_cast<DWORD>(hProcessModule) );
		if ( NULL == GetMaterial2 )
		{
			MessageBox (NULL, "Failed to locate the GetMaterial2() method!", DLL_NAME, MB_OK);
			return (FALSE);
		}

		PK_Decryptor_Decrypt = FindPK_Decryptor_Decrypt( reinterpret_cast<DWORD>(hProcessModule) );
		if ( NULL == GetMaterial )
		{
			MessageBox (NULL, "Failed to locate the PK_Decryptor_Decrypt() method!", DLL_NAME, MB_OK);
			return (FALSE);
		}

        DetourTransactionBegin();

        DetourUpdateThread ( GetCurrentThread() );

        if ( NO_ERROR != DetourAttach (reinterpret_cast<PVOID*>(&GetKey), DetouredGetKey) )
        {
            DetourTransactionAbort();

            MessageBox (NULL, "Failed to hook the GetKey() method!", DLL_NAME,
                        MB_OK);
            return (FALSE);
        }

		if ( NO_ERROR != DetourAttach(reinterpret_cast<PVOID*>(&CBCDecryption_ProcessBlocks), DetouredCBCDecryption_ProcessBlocks) )
		{
			DetourTransactionAbort();

			MessageBox (NULL, "Failed to hook the CBCDecryption_ProcessBlocks() method!", DLL_NAME,
				MB_OK);
			return (FALSE);
		}

		if ( NO_ERROR != DetourAttach(reinterpret_cast<PVOID*>(&CBCEncryption_ProcessBlocks), DetouredCBCEncryption_ProcessBlocks) )
		{
			DetourTransactionAbort();

			MessageBox (NULL, "Failed to hook the CBCEncryption_ProcessBlocks() method!", DLL_NAME,
				MB_OK);
			return (FALSE);
		}

		if ( NO_ERROR != DetourAttach(reinterpret_cast<PVOID*>(&VerifyMessage), DetouredVerifyMessage) )
		{
			DetourTransactionAbort();

			MessageBox (NULL, "Failed to hook the VerifyMessage() method!", DLL_NAME,
				MB_OK);
			return (FALSE);
		}

		if ( NO_ERROR != DetourAttach(reinterpret_cast<PVOID*>(&HashUpdate), DetouredHashUpdate) )
		{
			DetourTransactionAbort();

			MessageBox (NULL, "Failed to hook the HashUpdate() method!", DLL_NAME,
				MB_OK);
			return (FALSE);
		}

		if ( NO_ERROR != DetourAttach(reinterpret_cast<PVOID*>(&PK_Decryptor_Decrypt), DetouredPK_Decryptor_Decrypt) )
		{
			DetourTransactionAbort();

			MessageBox (NULL, "Failed to hook the PK_Decryptor_Decrypt() method!", DLL_NAME,
				MB_OK);
			return (FALSE);
		}

		if ( NO_ERROR != DetourAttach(reinterpret_cast<PVOID*>(&RealCreateFile), DetouredCreateFile) )
		{
			DetourTransactionAbort();

			MessageBox (NULL, "Failed to hook the CreateFile() method!", DLL_NAME,
				MB_OK);
			return (FALSE);
		}

        DetourTransactionCommit();

        InitializeCriticalSection (&KeySentCriticalSection);

    }
    else if (DLL_PROCESS_DETACH == fdwReason)
    {
         DeleteCriticalSection (&KeySentCriticalSection);
    }

    // When the system calls the DllMain function with any value other than
    // DLL_PROCESS_ATTACH, the return value is ignored.
    return (TRUE);
}

//
///////////////////////////////////////////////////////////////////////////////