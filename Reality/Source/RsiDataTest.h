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

#ifndef UNITTEST
#define UNITTEST

#include "RsiData.h"
#include "Util.h"

void runTest()
{
	byte rawPointer[sizeof(uint16)+sizeof(uint64)+sizeof(uint32)];
	memset(rawPointer,0,sizeof(rawPointer));
	RsiData theRsiData(rawPointer);
	theRsiData.ToBytes(rawPointer);
	cout << "Before setting values " << Bin2Hex(rawPointer,sizeof(rawPointer)) << endl;
	assert(theRsiData.getSex() == 0);
	assert(theRsiData.getBody() == 0);
	assert(theRsiData.getHat() == 0);
	assert(theRsiData.getFace() == 0);
	assert(theRsiData.getUnknown1() == 0);

	assert(theRsiData.getShirt() == 0);
	assert(theRsiData.getCoat() == 0);
	assert(theRsiData.getPants() == 0);
	assert(theRsiData.getShoes() == 0);
	assert(theRsiData.getGloves() == 0);
	assert(theRsiData.getGlasses() == 0);
	assert(theRsiData.getHair() == 0);
	assert(theRsiData.getFacialDetail() == 0);
	assert(theRsiData.getShirtColor() == 0);
	assert(theRsiData.getPantsColor() == 0);
	assert(theRsiData.getCoatColor() == 0);
	assert(theRsiData.getUnknown2() == 0);

	assert(theRsiData.getHairColor() == 0);
	assert(theRsiData.getSkinTone() == 0);
	assert(theRsiData.getUnknown3() == 0);
	assert(theRsiData.getTattoo() == 0);
	assert(theRsiData.getFacialDetailColor() == 0);

	theRsiData.setSex(1);
	assert(theRsiData.getSex() == 1);
	theRsiData.setBody(3);
	assert(theRsiData.getBody() == 3);
	theRsiData.setHat(63);
	assert(theRsiData.getHat() == 63);
	theRsiData.setFace(31);
	assert(theRsiData.getFace() == 31);
	theRsiData.setUnknown1(3);
	assert(theRsiData.getUnknown1() == 3);

	theRsiData.setShirt(15);
	assert(theRsiData.getShirt() == 15);
	theRsiData.setCoat(63);
	assert(theRsiData.getCoat() == 63);
	theRsiData.setPants(31);
	assert(theRsiData.getPants() == 31);
	theRsiData.setShoes(63);
	assert(theRsiData.getShoes() == 63);
	theRsiData.setGloves(31);
	assert(theRsiData.getGloves() == 31);
	theRsiData.setGlasses(31);
	assert(theRsiData.getGlasses() == 31);
	theRsiData.setHair(31);
	assert(theRsiData.getHair() == 31);
	theRsiData.setFacialDetail(15);
	assert(theRsiData.getFacialDetail() == 15);
	theRsiData.setShirtColor(63);
	assert(theRsiData.getShirtColor() == 63);
	theRsiData.setPantsColor(31);
	assert(theRsiData.getPantsColor() == 31);
	theRsiData.setCoatColor(31);
	assert(theRsiData.getCoatColor() == 31);
	theRsiData.setUnknown2(0xFF);
	assert(theRsiData.getUnknown2() == 0xFF);

	theRsiData.setHairColor(31);
	assert(theRsiData.getHairColor() == 31);
	theRsiData.setSkinTone(31);
	assert(theRsiData.getSkinTone() == 31);
	theRsiData.setUnknown3(3);
	assert(theRsiData.getUnknown3() == 3);
	theRsiData.setTattoo(7);
	assert(theRsiData.getTattoo() == 7);
	theRsiData.setFacialDetailColor(7);
	assert(theRsiData.getFacialDetailColor() == 7);

	theRsiData.ToBytes(rawPointer);
	cout << "After setting values " << Bin2Hex(rawPointer,sizeof(rawPointer)) << endl;

	theRsiData.setSex(0);
	assert(theRsiData.getSex() == 0);
	theRsiData.setBody(0);
	assert(theRsiData.getBody() == 0);
	theRsiData.setHat(0);
	assert(theRsiData.getHat() == 0);
	theRsiData.setFace(0);
	assert(theRsiData.getFace() == 0);
	theRsiData.setUnknown1(0);
	assert(theRsiData.getUnknown1() == 0);

	theRsiData.setShirt(0);
	assert(theRsiData.getShirt() == 0);
	theRsiData.setCoat(0);
	assert(theRsiData.getCoat() == 0);
	theRsiData.setPants(0);
	assert(theRsiData.getPants() == 0);
	theRsiData.setShoes(0);
	assert(theRsiData.getShoes() == 0);
	theRsiData.setGloves(0);
	assert(theRsiData.getGloves() == 0);
	theRsiData.setGlasses(0);
	assert(theRsiData.getGlasses() == 0);
	theRsiData.setHair(0);
	assert(theRsiData.getHair() == 0);
	theRsiData.setFacialDetail(0);
	assert(theRsiData.getFacialDetail() == 0);
	theRsiData.setShirtColor(0);
	assert(theRsiData.getShirtColor() == 0);
	theRsiData.setPantsColor(0);
	assert(theRsiData.getPantsColor() == 0);
	theRsiData.setCoatColor(0);
	assert(theRsiData.getCoatColor() == 0);
	theRsiData.setUnknown2(0);
	assert(theRsiData.getUnknown2() == 0);

	theRsiData.setHairColor(0);
	assert(theRsiData.getHairColor() == 0);
	theRsiData.setSkinTone(0);
	assert(theRsiData.getSkinTone() == 0);
	theRsiData.setUnknown3(0);
	assert(theRsiData.getUnknown3() == 0);
	theRsiData.setTattoo(0);
	assert(theRsiData.getTattoo() == 0);
	theRsiData.setFacialDetailColor(0);
	assert(theRsiData.getFacialDetailColor() == 0);

	theRsiData.ToBytes(rawPointer);
	cout << "After clearing values " << Bin2Hex(rawPointer,sizeof(rawPointer)) << endl;
}

#endif