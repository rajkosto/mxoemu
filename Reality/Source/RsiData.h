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

#ifndef RSIDATA_H
#define RSIDATA_H

#include "Common.h"

//females are slightly different, hair is shifted left by 1 and so
class RsiData
{
public:
	RsiData() : firstPair(0), secondPair(0), thirdPair(0) {}
	RsiData(const byte* rsiDataBytes)
	{
		FromBytes(rsiDataBytes);
	}
	~RsiData() {}
	void FromBytes(const byte* rsiDataBytes)
	{
		const byte* dataBytesLoc = rsiDataBytes;

		memcpy(&firstPair,dataBytesLoc,sizeof(firstPair));
		firstPair = swap16(firstPair);
		dataBytesLoc += sizeof(firstPair);

		memcpy(&secondPair,dataBytesLoc,sizeof(secondPair));
		secondPair = swap64(secondPair);
		dataBytesLoc += sizeof(secondPair);

		
		memcpy(&thirdPair,dataBytesLoc,sizeof(thirdPair));
		thirdPair = swap32(thirdPair);
		dataBytesLoc += sizeof(thirdPair);
	}
	uint8 ToBytes(byte *rsiDataBytes) const
	{
		byte* dataBytesLoc = rsiDataBytes;

		uint16 swappedFirstPair = swap16(firstPair);
		memcpy(dataBytesLoc,&swappedFirstPair,sizeof(swappedFirstPair));
		dataBytesLoc += sizeof(swappedFirstPair);

		uint64 swappedSecondPair = swap64(secondPair);
		memcpy(dataBytesLoc,&swappedSecondPair,sizeof(swappedSecondPair));
		dataBytesLoc += sizeof(swappedSecondPair);

		uint32 swappedThirdPair = swap32(thirdPair);
		memcpy(dataBytesLoc,&swappedThirdPair,sizeof(swappedThirdPair));
		dataBytesLoc += sizeof(swappedThirdPair);

		return uint8(dataBytesLoc - rsiDataBytes);
	}

//----MACRO MAGIC----
#define EXPAND_PARAMETER(paramName,pairVal,maskVal,shiftVal) \
uint8 get ## paramName () const { return (pairVal & maskVal) >> shiftVal; } \
void set ## paramName (const uint8 newVal) { pairVal &= ~(maskVal); pairVal |= (uint64(newVal) << shiftVal) & maskVal; }
//----MACRO MAGIC----

	EXPAND_PARAMETER(Sex,firstPair,		0x8000,15)
	EXPAND_PARAMETER(Body,firstPair,	0x6000,13)
	EXPAND_PARAMETER(Hat,firstPair,		0x1F80,7)
	EXPAND_PARAMETER(Face,firstPair,	0x007C,2)
	EXPAND_PARAMETER(Unknown1,firstPair,0x0003,0)

	EXPAND_PARAMETER(Shirt,secondPair,			0xF000000000000000ULL,60)
	EXPAND_PARAMETER(Coat,secondPair,			0x0FC0000000000000ULL,54)
	EXPAND_PARAMETER(Pants,secondPair,			0x003E000000000000ULL,49)
	EXPAND_PARAMETER(Shoes,secondPair,			0x0001F80000000000ULL,43)
	EXPAND_PARAMETER(Gloves,secondPair,			0x000007C000000000ULL,38)
	EXPAND_PARAMETER(Glasses,secondPair,		0x0000003E00000000ULL,33)
	EXPAND_PARAMETER(Hair,secondPair,			0x00000001F0000000ULL,28)
	EXPAND_PARAMETER(FacialDetail,secondPair,	0x000000000F000000ULL,24)
	EXPAND_PARAMETER(ShirtColor,secondPair,		0x0000000000FC0000ULL,18)
	EXPAND_PARAMETER(PantsColor,secondPair,		0x000000000003E000ULL,13)
	EXPAND_PARAMETER(CoatColor,secondPair,		0x0000000000001F00ULL,8)
	EXPAND_PARAMETER(Unknown2,secondPair,		0x00000000000000FFULL,0)

	EXPAND_PARAMETER(HairColor,thirdPair,			0xF8000000,27)
	EXPAND_PARAMETER(SkinTone,thirdPair,			0x07C00000,22)
	EXPAND_PARAMETER(Unknown3,thirdPair,			0x00300000,20)
	EXPAND_PARAMETER(Tattoo,thirdPair,				0x000E0000,17)
	EXPAND_PARAMETER(FacialDetailColor,thirdPair,	0x0001C000,14)

private:
	uint16 firstPair;
	uint64 secondPair;
	uint32 thirdPair;
};

#endif