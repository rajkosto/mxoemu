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

#ifndef MXOSIM_MARGINSOCKET_H
#define MXOSIM_MARGINSOCKET_H

#include "TCPVarLenSocket.h"

#include "Common.h"
#include "Crypto.h"

class MarginSocket : public TCPVarLenSocket
{
public:
	MarginSocket(ISocketHandler& );
	~MarginSocket();

	uint32 GetSessionId() {return sessionId;}
	uint64 GetCharUID() {return charId;};
	uint32 GetWorldCharId() {return worldCharId;}
	vector<byte> GetTwofishKey() 
	{
		vector<byte> tempVect(sizeof(twofishKey));
		memcpy(&tempVect[0],&twofishKey[0],tempVect.size());
		return tempVect;
	}
	void ForceDisconnect() {this->SetCloseAndDelete(true);}
	bool UdpReady();
private:
	void ProcessData(const byte *buf,size_t len);
	void SendCrypted(class EncryptedPacket &cryptedPacket);

	void NewCharacterReply()
	{
		numCharacterReplies=0;
	}
	void SendCharacterReply(uint16 shortAfterId,bool lastPacket,uint8 opcode,ByteBuffer &theData);

	uint32 m_userId;
	string m_username;

	string m_charName,m_firstName,m_lastName,m_background;

	byte twofishKey[16];

	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption Decryptor;
	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption Encryptor;

	auto_ptr<Decryptor> TFDecrypt;
	auto_ptr<Encryptor> TFEncrypt;

	uint32 sessionId;
	uint64 charId;

	static const byte blankIV[16];
	byte challenge[16];
	byte weirdSequenceOfBytes[16];
	string soeChatString;
	uint8 numCharacterReplies;

	uint32 worldCharId;

	bool readyForUdp;
};


#endif
