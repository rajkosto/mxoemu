#ifndef RSIDATA_H
#define RSIDATA_H

#include "Common.h"

class RsiData
{
public:
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
uint8 get ## paramName () const { return uint64(uint64(pairVal & maskVal) >> shiftVal); } \
void set ## paramName (uint8 newVal) { pairVal |= uint64((uint64(newVal) << shiftVal)) & maskVal; }
//----MACRO MAGIC----

	EXPAND_PARAMETER(Sex,firstPair,0x8000,15)
	EXPAND_PARAMETER(Body,firstPair,0x6000,13)
	EXPAND_PARAMETER(Hat,firstPair,0x1F80,7)
	EXPAND_PARAMETER(Face,firstPair,0x007C,2)
	EXPAND_PARAMETER(Unknown1,firstPair,0x0003,0)

	EXPAND_PARAMETER(Shirt,secondPair,0xF000000000000000,60)
	EXPAND_PARAMETER(Coat,secondPair,0xFC0000000000000,54)
	EXPAND_PARAMETER(Pants,secondPair,0x3E000000000000,49)
	EXPAND_PARAMETER(Shoes,secondPair,0x1F80000000000,43)
	EXPAND_PARAMETER(Gloves,secondPair,0x7C000000000,38)
	EXPAND_PARAMETER(Glasses,secondPair,0x3E00000000,33)
	EXPAND_PARAMETER(Hair,secondPair,0x1F0000000,28)
	EXPAND_PARAMETER(FacialDetail,secondPair,0xF000000,24)
	EXPAND_PARAMETER(ShirtColor,secondPair,0xFC0000,18)
	EXPAND_PARAMETER(PantsColor,secondPair,0x3E000,13)
	EXPAND_PARAMETER(CoatColor,secondPair,0x1F00,8)
	EXPAND_PARAMETER(Unknown2,secondPair,0xFF,0)

	EXPAND_PARAMETER(HairColor,thirdPair,0xF8000000,27)
	EXPAND_PARAMETER(SkinTone,thirdPair,0x7C00000,22)
	EXPAND_PARAMETER(Unknown3,thirdPair,0x300000,20)
	EXPAND_PARAMETER(Tattoo,thirdPair,0xE0000,17)
	EXPAND_PARAMETER(FacialDetailColor,thirdPair,0x1C000,14)

private:
	uint16 firstPair;
	uint64 secondPair;
	uint32 thirdPair;
};

#endif