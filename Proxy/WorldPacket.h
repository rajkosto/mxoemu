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

#include "Util.h"
#include "ByteBuffer.h"

class WorldPacket : public ByteBuffer
{
public:
	WorldPacket()
	{
		setSequences(0,0);
		setPlayerSetupState(0);
		this->clear();
	}
	WorldPacket(uint16 sSeq,uint16 cSeq,uint8 pss)
	{
		setSequences(sSeq,cSeq);
		setPlayerSetupState(pss);
		this->clear();
	}
	WorldPacket(uint16 sSeq,uint16 cSeq,uint8 pss,string &headerless)
	{
		setSequences(sSeq,cSeq);
		setPlayerSetupState(pss);
		this->clear();
		this->append((const byte*)headerless.data(),headerless.size());
	}
	WorldPacket(uint16 sSeq,uint16 cSeq,uint8 pss,ByteBuffer &headerless)
	{
		setSequences(sSeq,cSeq);
		setPlayerSetupState(pss);
		this->clear();
		this->append((const byte*)headerless.contents(),headerless.size());
	}
	WorldPacket(ByteBuffer &withHeader);
	WorldPacket(string &withHeader);
	~WorldPacket()
	{

	}
	void setSequences(unsigned short sSeq,unsigned short cSeq)
	{
		serverSequence = sSeq;
		clientSequence = cSeq;
	}
	void setPlayerSetupState(byte pss)
	{
		playerSetupState = pss;
	}
	void setData(string &headerless)
	{
		this->clear();
		this->append((const byte*)headerless.data(),headerless.size());
	}

	unsigned short getClientSeq()
	{
		return clientSequence;
	}
	unsigned short getServerSeq()
	{
		return serverSequence;
	}
	byte getPSS()
	{
		return playerSetupState;
	}
	ByteBuffer getData()
	{
		ByteBuffer zeReturn;
		zeReturn.append((const byte*)this->contents(),this->size());
		zeReturn.rpos(0);
		return zeReturn;
	}
	ByteBuffer getDataWithHeader();
private:
	uint16 serverSequence;
	uint16 clientSequence;
	uint8 playerSetupState;
};