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

#ifndef MXOSIM_SIGNEDDATASTRUCT_H
#define MXOSIM_SIGNEDDATASTRUCT_H

#include "Common.h"

#pragma pack(push,1)
typedef struct  
{
	uint8 unknownByte; //always 01
	uint32 userId1; //4 bytes, last byte is 00 on real server, unique per user
	char userName[33]; //32 for actual text, 1 for null terminator (if max len)
	uint16 unknownShort; //always 256
	uint32 padding1; //4 bytes of 0 padding
	uint32 expiryTime; //10 minutes ahead of current time
	byte padding2[32]; //32 bytes of 0 padding
	uint16 publicExponent; //17 for cryptopp, but big endian format here
	byte modulus[96]; //768bit public modulus of user RSA key
	uint32 timeCreated; //4 bytes, unique per user, users registered later have this number higher
} signedDataStruct;
#pragma pack(pop)

#endif