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
	}
	uint8 ToBytes(byte *rsiDataBytes) const
	{
		byte* dataBytesLoc = rsiDataBytes;
		uint16 swappedFirstPair = swap16(firstPair);
		memcpy(dataBytesLoc,&swappedFirstPair,sizeof(swappedFirstPair));
		dataBytesLoc += sizeof(swappedFirstPair);
		return uint8(dataBytesLoc - rsiDataBytes);
	}
	uint8 getSex() const
	{
#define MASK_SEX 0x8000
#define SHIFT_SEX 15
		return (firstPair & MASK_SEX) >> SHIFT_SEX;
	}
	void setSex(uint8 newSex)
	{
		firstPair |= (newSex << SHIFT_SEX) & MASK_SEX;
	}
	uint8 getBody() const
	{
#define MASK_BODY 0x6000
#define SHIFT_BODY 13
		return (firstPair & MASK_BODY) >> SHIFT_BODY;
	}
	void setBody(uint8 newBody)
	{
		firstPair |= (newBody << SHIFT_BODY) & MASK_BODY;
	}
	uint8 getHat() const
	{
#define MASK_HAT 0x1F80
#define SHIFT_HAT 7
		return (firstPair & MASK_HAT) >> SHIFT_HAT;
	}
	void setHat(uint8 newHat)
	{
		firstPair |= (newHat << SHIFT_HAT) & MASK_HAT;
	}
	uint8 getFace() const
	{
#define MASK_FACE 0x007C
#define SHIFT_FACE 2
		return (firstPair & MASK_FACE) >> SHIFT_FACE;
	}
	void setFace(uint8 newFace)
	{
		firstPair |= (newFace << SHIFT_FACE) & MASK_FACE;
	}
	uint8 getLeggings() const
	{
#define MASK_LEGGINGS 0x0003
#define SHIFT_LEGGINGS 0
		return (firstPair & MASK_LEGGINGS) >> SHIFT_LEGGINGS;
	}
	void setLeggings(uint8 newLeggings)
	{
		firstPair |= (newLeggings << SHIFT_LEGGINGS) & MASK_LEGGINGS;
	}
private:
	uint16 firstPair;
};

#endif