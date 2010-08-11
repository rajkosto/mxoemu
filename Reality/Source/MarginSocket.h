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

#ifndef MXOSIM_MARGINSOCKET_H
#define MXOSIM_MARGINSOCKET_H

#include "TCPVarLenSocket.h"

#include "Common.h"
#include "Crypto.h"
#include "SymmetricCrypto.h"
#include "EncryptedPacket.h"

class MarginSocket : public TCPVarLenSocket
{
public:
	MarginSocket(ISocketHandler& );
	~MarginSocket();

	void OnDisconnect(short info, int code);

	uint32 GetSessionId() {return sessionId;}
	uint64 GetCharUID() {return charId;};
	uint32 GetWorldCharId() {return worldCharId;}
	vector<byte> GetTwofishKey() 
	{
		vector<byte> tempVect(sizeof(twofishKey));
		memcpy(&tempVect[0],&twofishKey[0],tempVect.size());
		return tempVect;
	}
	void ForceDisconnect() 
	{
		this->SetCloseAndDelete(true);
	}
	bool UdpReady(class GameClient *theClient);
private:
	void ProcessData(const byte *buf,size_t len);
	void SendCrypted(TwofishEncryptedPacket &cryptedPacket);

	void NewCharacterReply()
	{
		numCharacterReplies=0;
	}
	void SendCharacterReply(uint16 shortAfterId,bool lastPacket,uint8 opcode,ByteBuffer theData);

	uint32 m_userId;
	string m_username;

	string m_charName,m_firstName,m_lastName,m_background;

	byte twofishKey[16];

	TwofishCryptEngine m_tfEngine;

	uint32 sessionId;
	uint64 charId;

	byte challenge[16];
	byte weirdSequenceOfBytes[16];
	string soeChatString;
	uint8 numCharacterReplies;

	uint32 worldCharId;

	bool readyForUdp;
};


#endif
