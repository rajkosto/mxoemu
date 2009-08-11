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
private:
	void ProcessData(const byte *buf,size_t len);
	void SendCrypted(class EncryptedPacket &cryptedPacket);
	string zeUsername;

	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption Decryptor;
	typedef CryptoPP::CBC_Mode<CryptoPP::Twofish>::Encryption Encryptor;

	auto_ptr<Decryptor> TFDecrypt;
	auto_ptr<Encryptor> TFEncrypt;

	static const byte blankIV[16];
	byte challenge[16];
};


#endif
