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

#include "WorldPacket.h"

#pragma pack(1)

typedef struct  
{
	
} worldPacketHeader_t;

WorldPacket::WorldPacket( ByteBuffer &withHeader )
{
	uint32 packedSeqs; //FL CC CS SS

	withHeader.rpos(0);
	withHeader >> packedSeqs;

	uint32 packed = swap32(packedSeqs);
	serverSequence = packed&0xFFF;
	clientSequence = (packed>>12)&0xFFF;
	playerSetupState = (packed>>24)&0xFF;

	vector<byte> restOfPacket;
	restOfPacket.resize(withHeader.size() - withHeader.rpos());
	withHeader.read(&restOfPacket[0],restOfPacket.size());

	this->clear();
	this->append(&restOfPacket[0],restOfPacket.size());

	withHeader.rpos(0);
}

WorldPacket::WorldPacket( string &withHeader )
{
	ByteBuffer simulation;
	simulation.append((const byte*)withHeader.data(),withHeader.size());

	WorldPacket::WorldPacket(simulation);
}

ByteBuffer WorldPacket::getDataWithHeader()
{
	uint32 packedSeqs = swap32((playerSetupState << 24) | ((clientSequence & 0xFFF) << 12) | (serverSequence & 0xFFF)); // FL CC CS SS

	ByteBuffer returnMe;
	returnMe << packedSeqs;
	returnMe.append(this->contents(),this->size());

	return returnMe;
}