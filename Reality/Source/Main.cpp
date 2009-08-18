//include a unit test at the very top if you want to run it
//#include "RsiDataTest.h"

#ifndef UNITTEST
#include "Common.h"
#include "Master.h"
#include "Util.h"
#include "Crypto.h"
#include <iostream>
#endif

int main()
{
#ifndef UNITTEST
	Master::getSingleton().Run();
#else
	runTest();
	for(;;){Sleep(10000);}
#endif

	return 0;
}

