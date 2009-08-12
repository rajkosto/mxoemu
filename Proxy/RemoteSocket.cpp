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

#include "RemoteSocket.h"
#include "LocalSocket.h"
#include "Util.h"
#include <iostream>
#include "SequencedPacket.h"
#include "Logging.h"
#include "EncryptedPacket.h"

extern LocalSocket *worldLocal;
extern u_short GlobalPort;
extern std::string GlobalIP;

RemoteSocket *globalRemoteSocket = NULL;
extern LocalSocket *globalLocalSocket;

extern uint16 WTC_localSeq;
extern uint16 WTC_remoteSeq;
extern uint8 WTC_flags;

RemoteSocket::RemoteSocket(ISocketHandler& h)
:UdpSocket(h)
{
	WTC_localSeq = 0;
	WTC_remoteSeq = 0;
	WTC_flags = 0;

	InitializeCriticalSection(&CriticalSection); 
}

RemoteSocket::~RemoteSocket()
{
	WTC_localSeq = 0;
	WTC_remoteSeq = 0;
	WTC_flags = 0;

	DeleteCriticalSection(&CriticalSection);
}

extern CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptWTC;
extern CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *TFEncryptWTC;

void PerformReplay( const char*p, size_t l, bool server)
{
	if (server == false)
	{
		if (globalRemoteSocket == NULL)
			return;

		globalRemoteSocket->Replay(p,l);
	}
	else
	{
		if (globalLocalSocket == NULL)
			return;

		globalLocalSocket->Replay(p,l);
	}
}

void RemoteSocket::Replay( const char *p,size_t l)
{
	if (p==NULL || l < 1)
		return;

	if (TFEncryptWTC == NULL)
		return;
	
	EnterCriticalSection(&CriticalSection);

	WTC_localSeq++;

	if (WTC_localSeq >= 4096)
		WTC_localSeq=0;

	SequencedPacket replayMePacket(WTC_localSeq,WTC_remoteSeq,WTC_flags,string(p,l));
	ByteBuffer replayMePlain = replayMePacket.getDataWithHeader();
	EncryptedPacket replayMeCryptor;
	replayMeCryptor.append((const byte*)replayMePlain.contents(),replayMePlain.size());
	ByteBuffer SendMe = replayMeCryptor.toCipherText(TFEncryptWTC);

	LeaveCriticalSection(&CriticalSection);

	//output to OnRawData for retesting and decomposition
	OnRawData(SendMe.contents(),SendMe.size(),NULL,97);
}

void RemoteSocket::OnRawData(const char *p,size_t l,struct sockaddr *sa_from,socklen_t sa_len)
{
	if (TFDecryptWTC!=NULL && p[0]==0x01)
	{
		globalRemoteSocket = this;
	}

	EnterCriticalSection(&CriticalSection);

	string outputMe = WorldToClient(p,l);
	worldLocal->SendToBuf(GlobalIP,GlobalPort,outputMe.data(),outputMe.size());

	LeaveCriticalSection(&CriticalSection);
}