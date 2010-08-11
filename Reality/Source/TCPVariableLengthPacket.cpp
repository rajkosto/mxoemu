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

#include "Common.h"
#include "TCPVariableLengthPacket.h"


TCPVariableLengthPacket::TCPVariableLengthPacket() : ByteBuffer()
{
}

TCPVariableLengthPacket::TCPVariableLengthPacket(const ByteBuffer &srcBuf) : ByteBuffer(srcBuf)
{
}

TCPVariableLengthPacket::~TCPVariableLengthPacket()
{
}

ByteBuffer TCPVariableLengthPacket::GetProcessed() const
{
	uint16 packetSize = this->size();
	if (packetSize > 0x7f)
	{
		packetSize = htons(packetSize | 0x8000);
		ByteBuffer packetThing;
		packetThing << packetSize;
		packetThing.append((const byte*)this->contents(),this->size());

		return packetThing;
	}
	else
	{
		ByteBuffer packetThing;
		packetThing << byte(packetSize);
		packetThing.append((const byte*)this->contents(),this->size());
		return packetThing;
	}
}
