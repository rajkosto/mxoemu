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

void ConvertBytesintoHex(const byte *data,string *string,unsigned int count)
{
	if (data != NULL)
	{
		ostringstream myStream;
		
		int i = 0;
		for (unsigned int j = 0;j < count;j++)
		{
			i++;
			if (i == 33)
			{
				myStream << endl;
				i = 1;
			}
			byte n = data[j];
			if (n <= 15)
				myStream << "0";
			myStream << hex << (int)n;
			myStream << " ";
		}
		myStream.flush();
		*string = myStream.str();
	}
}