#ifndef SUBPACKETSTEST_H
#define SUBPACKETSTEST_H

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