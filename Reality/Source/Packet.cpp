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

#include "Packet.h"

Packet::Packet()
{
}

Packet::~Packet()
{
}

void Packet::Clear()
{
	contents.clear();
}

uint16 Packet::Size()
{
	return contents.size();
}

void Packet::FromString(std::string &string)
{
	contents.clear();
	contents.append(string);
}

std::string Packet::ToString()
{
	uint8 *data=new uint8[contents.size()];
	contents.rpos(0);
	contents.read(data,contents.size());
	std::string hax = std::string((const char *)data,contents.size());
	delete[] data;
	return hax;
}

void Packet::Append(std::string &string)
{
	contents.append(string);
}