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
		setAckBits(0);
		this->clear();
	}
	SequencedPacket(uint16 _localSeq,uint16 _remoteSeq,uint8 _ackBits)
	{
		setSequences(_localSeq,_remoteSeq);
		setAckBits(_ackBits);
		this->clear();
	}
	SequencedPacket(uint16 _localSeq,uint16 _remoteSeq,uint8 _ackBits,const string &headerless)
	{
		setSequences(_localSeq,_remoteSeq);
		setAckBits(_ackBits);
		this->clear();
		this->append((const byte*)headerless.data(),headerless.size());
	}
	SequencedPacket(uint16 _localSeq,uint16 _remoteSeq,uint8 _ackBits,const ByteBuffer &headerless)
	{
		setSequences(_localSeq,_remoteSeq);
		setAckBits(_ackBits);
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
	void setAckBits(byte _ackBits)
	{
		ackBits = _ackBits;
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

	unsigned short getRemoteSeq() const
	{
		return remoteSeq;
	}
	unsigned short getLocalSeq() const
	{
		return localSeq;
	}
	byte getAckBits() const
	{
		return ackBits;
	}
	ByteBuffer getData() const
	{
		ByteBuffer zeReturn;
		zeReturn.append((const byte*)this->contents(),this->count());
		zeReturn.rpos(0);
		return zeReturn;
	}
	ByteBuffer getDataWithHeader() const;
private:
	void Construct(ByteBuffer withHeader);
	uint16 localSeq;
	uint16 remoteSeq;
	uint8 ackBits;
};

#endif
