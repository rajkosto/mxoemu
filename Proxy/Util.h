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

#ifndef UTIL_H
#define UTIL_H

#include <sstream>
#include <string>
#include <fstream>
#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <list>
#include <iostream>

typedef unsigned char byte;
using namespace std;

void ConvertBytesintoHex(const byte *data,string &string,unsigned int count);

#include <CryptoPP/cryptlib.h>
#include <CryptoPP/crc.h>
#include <CryptoPP/modes.h>
#include <CryptoPP/twofish.h>
#include <CryptoPP/filters.h>
#include <CryptoPP/osrng.h>
#include <CryptoPP/rsa.h>
#include <CryptoPP/asn.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <CryptoPP/md5.h>

#define swap16 _byteswap_ushort
#define swap32 _byteswap_ulong

typedef long        int32;
typedef short       int16;
typedef char        int8;

typedef unsigned long        uint32;
typedef unsigned short       uint16;
typedef unsigned char        uint8;
typedef unsigned char		 byte;


#include "ByteBuffer.h"

#endif