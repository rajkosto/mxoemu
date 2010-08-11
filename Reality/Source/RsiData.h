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

#ifndef MXOSIM_RSIDATA_H
#define MXOSIM_RSIDATA_H

#include "BitVarStream.h"

//values that are the same for both genders
class RsiData : public BitVarStream
{
public:
	RsiData()
	{
		AddVariableDef("Sex",1);
		AddVariableDef("Body",2);
		AddVariableDef("Hat",6);
		AddVariableDef("Face",5);
		AddSkipBitsDef(1); //unknown, doesnt seem to change anything, definately not part of FACE
		AddVariableDef("Shirt",5);
	}
	~RsiData() {}
};

class RsiDataMale : public RsiData
{
public:
	RsiDataMale()
	{
		AddVariableDef("Coat",6);
		AddVariableDef("Pants",5);
		AddVariableDef("Shoes",6);
		AddVariableDef("Gloves",5);

		AddVariableDef("Glasses",5);
		AddVariableDef("Hair",5);
		AddVariableDef("FacialDetail",4);
		AddVariableDef("ShirtColor",6);
		AddVariableDef("PantsColor",5);
		AddVariableDef("CoatColor",5);
		AddVariableDef("ShoeColor",4);
		AddVariableDef("GlassesColor",4);
		AddVariableDef("HairColor",5);
		AddVariableDef("SkinTone",5);
		AddSkipBitsDef(2); //these are supposed to go with tattoo, but they only defined textures for the first 8
		AddVariableDef("Tattoo",3);
		AddVariableDef("FacialDetailColor",3);
	}
	~RsiDataMale() {}
};

class RsiDataFemale : public RsiData
{
public:
	RsiDataFemale()
	{
		AddVariableDef("Coat",5);
		AddVariableDef("Pants",5);
		AddVariableDef("Shoes",5);
		AddVariableDef("Gloves",6);


		AddVariableDef("Glasses",5);
		AddVariableDef("Hair",5);
		AddVariableDef("Leggings",4);
		AddVariableDef("FacialDetail",4);
		AddVariableDef("ShirtColor",6);
		AddVariableDef("PantsColor",5);
		AddVariableDef("CoatColor",5);
		AddVariableDef("ShoeColor",4);
		AddVariableDef("GlassesColor",4);
		AddVariableDef("HairColor",5);
		AddVariableDef("SkinTone",5);
		AddSkipBitsDef(2); //these are supposed to go with tattoo, but they only defined textures for the first 8
		AddVariableDef("Tattoo",3);
		AddVariableDef("FacialDetailColor",3);
	}
	~RsiDataFemale() {}
};

#endif