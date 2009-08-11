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

#include "LocalSocket.h"
#include "RemoteSocket.h"
#include <Utility.h>
#include "Util.h"
#include <iostream>
#include "EncryptedPacket.h"
#include "WorldPacket.h"
#include "Logging.h"

extern RemoteSocket *worldRemote;
extern u_short GlobalPort;
extern std::string GlobalIP;

LocalSocket *globalLocalSocket = NULL;

extern CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptCTW;
extern CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *TFEncryptCTW;

extern uint16 CTW_client_sequence;
extern uint16 CTW_server_sequence;
extern uint8 CTW_flags;

LocalSocket::LocalSocket(ISocketHandler& h)
:UdpSocket(h)
{
	CTW_client_sequence = 0;
	CTW_server_sequence = 0;
	CTW_flags = 0;

	InitializeCriticalSection(&CriticalSection); 
}

LocalSocket::~LocalSocket()
{
	CTW_client_sequence = 0;
	CTW_server_sequence = 0;
	CTW_flags = 0;

	DeleteCriticalSection(&CriticalSection);
}

void LocalSocket::Replay( const char *p,size_t l)
{
	if (p==NULL || l < 1)
		return;

	if (TFEncryptCTW == NULL)
		return;

	EnterCriticalSection(&CriticalSection);

	CTW_client_sequence++;

	if (CTW_client_sequence >= 4096)
		CTW_client_sequence=0;

	WorldPacket replayMePacket(CTW_server_sequence,CTW_client_sequence,CTW_flags,string(p,l));
	ByteBuffer replayMePlain = replayMePacket.getDataWithHeader();
	EncryptedPacket replayMeCryptor;
	replayMeCryptor.append((const byte*)replayMePlain.contents(),replayMePlain.size());
	ByteBuffer SendMe = replayMeCryptor.toCipherText(TFEncryptCTW);

	LeaveCriticalSection(&CriticalSection);

	//output to OnRawData for retesting and decomposition
	OnRawData(SendMe.contents(),SendMe.size(),NULL,97);
}

void LocalSocket::OnRawData(const char *p,size_t l,struct sockaddr *sa_from,socklen_t sa_len)
{
	if (sa_from != NULL && sa_len != 97)
	{
		struct sockaddr_in sa;
		memcpy(&sa,sa_from,sa_len);
		ipaddr_t a;
		memcpy(&a,&sa.sin_addr,4);
		Utility::l2ip(a,GlobalIP); 
		GlobalPort = htons(sa.sin_port);
	}

	if (TFDecryptCTW!=NULL && p[0]==0x01)
	{
		globalLocalSocket = this;
	}

	EnterCriticalSection(&CriticalSection);

	string outputMe = ClientToWorld(p,l);
	worldRemote->SendBuf(outputMe.data(),outputMe.size());

	LeaveCriticalSection(&CriticalSection);
}