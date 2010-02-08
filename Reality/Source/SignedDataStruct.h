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