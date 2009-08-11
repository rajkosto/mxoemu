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

#include "InPacket.h"

IncomingPacket::IncomingPacket(std::string &source)
{
	uint32 packed;
	memcpy(&packed,source.data(),sizeof(packed));
	packed= ntohl(packed);
	client_sequence = uint16(packed&0xFFF);
	server_sequence = uint16((packed>>12)&0xFFF);
	flags = uint8((packed>>24)&0xFF);
	contents << std::string(source.data()+sizeof(packed),source.size()-sizeof(packed));
}

IncomingPacket::~IncomingPacket()
{
}