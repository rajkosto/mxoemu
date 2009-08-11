#ifndef LOGGING_H
#define LOGGING_H

#include "Util.h"

typedef enum
{
	CLIENT_TO_AUTH,
	AUTH_TO_CLIENT,
	CLIENT_TO_MARGIN,
	MARGIN_TO_CLIENT,
	CLIENT_TO_WORLD,
	WORLD_TO_CLIENT
} PacketDirection;

string ClientToAuth(const char* pData,size_t pSize);
string AuthToClient(const char* pData,size_t pSize);

string ClientToMargin(const char* pData,size_t pSize);
string MarginToClient(const char* pData,size_t pSize);

string ClientToWorld(const char* pData,size_t pSize);
string WorldToClient(const char* pData,size_t pSize);

#endif