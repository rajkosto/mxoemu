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

#include "OutPacket.h"
#include "Util.h"

OutgoingPacket::OutgoingPacket(uint8 flagz,uint16 client_sequencez,uint16 server_sequencez)
{
	flags = flagz;
	client_sequence = client_sequencez;
	server_sequence = server_sequencez;
}

OutgoingPacket::~OutgoingPacket()
{}

void OutgoingPacket::PreSave()
{
	contents.clear();
	uint32 storage = htonl((flags << 24) | ((client_sequence & 0xFFF) << 12) | (server_sequence & 0xFFF)); // FL CC CS SS
	contents << storage;
/*	
	std::string nub;
	ConvertBytesintoHex((const byte*)&storage,nub,sizeof(storage));
	std::cout << nub << std::endl;*/
}

void OutgoingPacket::FromString(const std::string &string)
{
	PreSave();
	contents.append(string);
}
