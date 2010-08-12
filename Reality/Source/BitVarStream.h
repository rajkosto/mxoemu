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

#ifndef MXOSIM_BITVARSTREAM_H
#define MXOSIM_BITVARSTREAM_H

#include "Common.h"

class BitVarStream
{
public:
	class NotEnoughBits : public exception {};
	class DataOverflow : public exception {};
	class InvalidVar : public exception {};

	BitVarStream();
	~BitVarStream() {}
	void FromBytes(const byte* dataBuf,const size_t dataLen);
	uint8 ToBytes(byte *rsiDataBytes,const size_t maxLen) const;
	uint8& operator[] (const string& nameIndex);
	void ClearData();
protected:
	void ClearVariableDefs();
	void AddVariableDef(const string& name,uint8 numBits);
	void AddSkipBitsDef(uint8 numBits);
private:
	typedef struct  
	{
		string name;
		uint8 numBits;
	} varDef;
	typedef vector<varDef> vectVarDefs;
	vectVarDefs varDefs;
	typedef map<string,uint8> varData;
	varData varValues;
};

#endif