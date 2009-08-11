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

#ifndef MXOSIM_GAMECLIENT_H
#define MXOSIM_GAMECLIENT_H

#include "OutPacket.h"
#include "Crypto.h"

class GameClient
{
	private:
		// Valid Client + Anti-Flood bool
		bool Valid_Client;
		bool Handled_Session;

		// Master Sock handle, client's address structure, last recieved packet
		SOCKET *_sock;
		struct sockaddr_in _address;
		uint32 _last_activity;

		//Number of packets received
		uint32 numPackets;

		// Sequences
		uint8 PlayerSetupState;
		uint16 server_sequence;
		uint16 client_sequence;

		// Client tick count
		uint16 tick;

		//Player name lol

		std::string name;

		CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecrypt;
		CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption *TFEncrypt;

	public:
		
		GameClient(sockaddr_in, SOCKET*);
		~GameClient();

		inline uint32 LastActive() { return _last_activity; }
		inline bool IsValid() { return Valid_Client; }
		char *Address() { return inet_ntoa(_address.sin_addr); }

		void HandlePacket(char *pData, uint16 Length);
		bool Decrypt(char *pData, uint16 nLength,std::string &output);
		void Send(const std::string &contents);
};

#endif
