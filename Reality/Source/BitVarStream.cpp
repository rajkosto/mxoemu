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

#include "BitVarStream.h"
#include "BitStream.h"


BitVarStream::BitVarStream()
{
	ClearVariableDefs();
}

void BitVarStream::FromBytes( const byte* dataBuf,const size_t dataLen )
{
	ClearData();

	BitStream bits(const_cast<byte*>(dataBuf),dataLen,false);
	bits.ResetReadPointer();
	for (vectVarDefs::const_iterator i = varDefs.begin();i != varDefs.end();++i)
	{
		if (i->name.length() < 1)
		{
			bits.IgnoreBits(i->numBits);
		}
		else
		{
			uint8 tempVar = 0;

			if (bits.ReadBits(&tempVar,i->numBits) == false)
				throw NotEnoughBits();

			varValues[i->name] = tempVar;
		}			
	}
}

uint8 BitVarStream::ToBytes( byte *rsiDataBytes,const size_t maxLen ) const
{
	BitStream bits;
	bits.ResetWritePointer();
	for (vectVarDefs::const_iterator i = varDefs.begin();i != varDefs.end();++i)
	{
		if (i->name.length() < 1 || varValues.find(i->name) == varValues.end())
		{
			uint8 zeroBits = 0;
			bits.WriteBits(&zeroBits,i->numBits);
		}
		else
		{
			uint8 bitsToWrite = varValues.find(i->name)->second;

			uint8 maxValue = (2 << (i->numBits-1)) - 1; // same as (2 to the power of numBits) - 1 
			if (bitsToWrite > maxValue)
				bitsToWrite = maxValue;

			bits.WriteBits(&bitsToWrite,i->numBits);
		}
	}

	uint32 lenRequired = bits.GetNumberOfBytesUsed();
	if (lenRequired > maxLen)
		throw NotEnoughBits();

	memcpy(rsiDataBytes,bits.GetData(),lenRequired);
	return lenRequired;
}

uint8& BitVarStream::operator[]( const string& nameIndex )
{
	bool foundVal = false;
	for (vectVarDefs::const_iterator i = varDefs.begin();i != varDefs.end();++i)
	{
		if (i->name == nameIndex)
		{
			foundVal = true;
			break;
		}
	}

	if (foundVal == false)
		throw InvalidVar();

	return varValues[nameIndex];
}

void BitVarStream::ClearData()
{
	varValues.clear();
}

void BitVarStream::ClearVariableDefs()
{
	varDefs.clear();
}

void BitVarStream::AddVariableDef( const string& name,uint8 numBits )
{
	varDef newDef;
	newDef.name = name;
	newDef.numBits = numBits;
	varDefs.push_back(newDef);
}

void BitVarStream::AddSkipBitsDef( uint8 numBits )
{
	varDef newDef;
	newDef.name.clear();
	newDef.numBits = numBits;
	varDefs.push_back(newDef);
}