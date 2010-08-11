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

#ifndef MXOEMU_SEQUENCEDPACKET_H
#define MXOEMU_SEQUENCEDPACKET_H

#include "Util.h"
#include "ByteBuffer.h"

class SequencedPacket : public ByteBuffer
{
public:
	SequencedPacket()
	{
		setSequences(0,0);
		setFlags(0);
		this->clear();
	}
	SequencedPacket(uint16 _localSeq,uint16 _remoteSeq,uint8 _flags)
	{
		setSequences(_localSeq,_remoteSeq);
		setFlags(_flags);
		this->clear();
	}
	SequencedPacket(uint16 _localSeq,uint16 _remoteSeq,uint8 _flags,const string &headerless)
	{
		setSequences(_localSeq,_remoteSeq);
		setFlags(_flags);
		this->clear();
		this->append((const byte*)headerless.data(),headerless.size());
	}
	SequencedPacket(uint16 _localSeq,uint16 _remoteSeq,uint8 _flags,const ByteBuffer &headerless)
	{
		setSequences(_localSeq,_remoteSeq);
		setFlags(_flags);
		this->clear();
		this->append((const byte*)headerless.contents(),headerless.size());
	}
	SequencedPacket(ByteBuffer withHeader);
	SequencedPacket(const string &withHeader);
	~SequencedPacket()
	{

	}
	void setSequences(uint16 _localSeq,uint16 _remoteSeq)
	{
		this->localSeq = _localSeq;
		this->remoteSeq = _remoteSeq;
	}
	void setFlags(byte _flags)
	{
		flags = _flags;
	}
	void setData(const string &headerless)
	{
		this->clear();
		this->append((const byte*)headerless.data(),headerless.size());
	}
	void setData(const ByteBuffer &headerless)
	{
		this->clear();
		this->append((const byte*)headerless.contents(),headerless.size());
	}

	unsigned short getRemoteSeq()
	{
		return remoteSeq;
	}
	unsigned short getLocalSeq()
	{
		return localSeq;
	}
	byte getFlags()
	{
		return flags;
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
	void Construct(ByteBuffer withHeader);
	uint16 localSeq;
	uint16 remoteSeq;
	uint8 flags;
};

#endif
