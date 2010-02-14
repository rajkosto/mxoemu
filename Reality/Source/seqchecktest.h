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

#ifndef MXOEMU_SUBPACKETSTEST_H
#define MXOEMU_SUBPACKETSTEST_H

#define UNITTEST

#include "Common.h"
#include "ByteBuffer.h"
#include "Crypto.h"
#include "Util.h"

inline bool isSequenceMoreRecent( uint16 biggerSequence, uint16 smallerSequence, uint32 max_sequence=4096 )
{
	return	( (biggerSequence > smallerSequence) && (biggerSequence-smallerSequence <= max_sequence/2) )
		|| ( (smallerSequence > biggerSequence) && (smallerSequence-biggerSequence > max_sequence/2) );
}

void runTest()
{
	cout << isSequenceMoreRecent(4000,4000);
	cout << isSequenceMoreRecent(3000,2000);
	cout << isSequenceMoreRecent(2000,3000);
	cout << isSequenceMoreRecent(50,4090);
	cout << endl;
	cout << isSequenceMoreRecent(9000,10000,65536);
	cout << isSequenceMoreRecent(60000,50000,65536);
	cout << isSequenceMoreRecent(50,65300,65536);
}

#endif