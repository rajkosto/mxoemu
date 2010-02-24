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

#ifndef MXOSIM_LOCATIONVECTOR_H
#define MXOSIM_LOCATIONVECTOR_H

#include "ByteBuffer.h"
#include "Common.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

class LocationVector
{
public:
	LocationVector(double X, double Y, double Z, uint8 O) : x(x), y(Y), z(Z), rot(MxoToDoubleRot(O)) {}
	LocationVector(double X, double Y, double Z) : x(X), y(Y), z(Z), rot(0) {}
	LocationVector() : x(0), y(0), z(0), rot(0) {}
private:
	inline double MxoToDoubleRot(uint8 mxoRot)
	{
		double normalizedRot = (double(mxoRot)/double(256)); //range from 0 to 1
		normalizedRot-=0.5f; //range from -0.5 to 0.5
		normalizedRot*=2*M_PI; //range from -pi to +pi
		return normalizedRot;
	}
	inline uint8 DoubleToMxoRot(double rot)
	{
		//start range in -pi to +pi
		double normalizedRot = rot/(2*M_PI); //range from -0.5 to 0.5
		normalizedRot+=0.5f; //range from 0 to 1
		normalizedRot*=255; //range from 0 to 255
		return uint8(normalizedRot);
	}
public:
	// (dx * dx + dy * dy + dz * dz)
	double DistanceSq(const LocationVector & comp)
	{
		double delta_x = comp.x - x;
		double delta_y = comp.y - y;
		double delta_z = comp.z - z;

		return (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
	}

	double DistanceSq(const double &X, const double &Y, const double &Z)
	{
		double delta_x = X - x;
		double delta_y = Y - y;
		double delta_z = Z - z;

		return (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
	}

	// sqrt(dx * dx + dy * dy + dz * dz)
	double Distance(const LocationVector & comp)
	{
		double delta_x = comp.x - x;
		double delta_y = comp.y - y;
		double delta_z = comp.z - z;

		return sqrt(delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
	}

	double Distance(double &X, const double &Y, const double &Z)
	{
		double delta_x = X - x;
		double delta_y = Y - y;
		double delta_z = Z - z;

		return sqrt(delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
	}

	double Distance2DSq(const LocationVector & comp)
	{
		double delta_x = comp.x - x;
		double delta_y = comp.y - y;
		return (delta_x*delta_x + delta_y*delta_y);
	}

	double Distance2DSq(const double & X, const double & Y)
	{
		double delta_x = X - x;
		double delta_y = Y - y;
		return (delta_x*delta_x + delta_y*delta_y);
	}

	double Distance2D(LocationVector & comp)
	{
		double delta_x = comp.x - x;
		double delta_y = comp.y - y;
		return sqrt(delta_x*delta_x + delta_y*delta_y);
	}

	double Distance2D(const double & X, const double & Y)
	{
		double delta_x = X - x;
		double delta_y = Y - y;
		return sqrt(delta_x*delta_x + delta_y*delta_y);
	}
	// atan2(dx / dy)
	double CalcAngTo(const LocationVector & dest)
	{
		double dx = dest.x - x;
		double dy = dest.y - y;
		if(dy != 0.0f)
			return atan2(dy, dx);
		else 
			return 0.0f;
	}
	inline uint8 CalcAngToMxo(const LocationVector & dest)
	{
		return DoubleToMxoRot(CalcAngTo(dest));
	}
	double CalcAngFrom(const LocationVector & src)
	{
		double dx = x - src.x;
		double dy = y - src.y;
		if(dy != 0.0f)
			return atan2(dy, dx);
		else
			return 0.0f;
	}
	inline uint8 CalcAngFromMxo(const LocationVector & dest)
	{
		return DoubleToMxoRot(CalcAngFrom(dest));
	}
	void ChangeCoords(double X, double Y, double Z)
	{
		x = X;
		y = Y;
		z = Z;
	}
	void ChangeCoords(double X, double Y, double Z, uint8 O)
	{
		x = X;
		y = Y;
		z = Z;
		rot = O;
	}

	// add/subtract/equality vectors
	LocationVector & operator += (const LocationVector & add)
	{
		x += add.x;
		y += add.y;
		z += add.z;
		rot += add.rot;
		return *this;
	}

	LocationVector & operator -= (const LocationVector & sub)
	{
		x -= sub.x;
		y -= sub.y;
		z -= sub.z;
		rot -= sub.rot;
		return *this;
	}

	LocationVector & operator = (const LocationVector & eq)
	{
		x = eq.x;
		y = eq.y;
		z = eq.z;
		rot = eq.rot;
		return *this;
	}

	bool operator == (const LocationVector & eq)
	{
		if(eq.x == x && eq.y == y && eq.z == z)
			return true;
		else
			return false;
	}
	uint8 getMxoRot()
	{
		return DoubleToMxoRot(rot);
	}
	void setMxoRot(uint8 theRot)
	{
		rot=MxoToDoubleRot(theRot);
	}
	bool fromDoubleBuf(ByteBuffer &sourceBuf)
	{
		if (sourceBuf.remaining() < sizeof(double)*3)
			return false;

		sourceBuf >> x;
		sourceBuf >> y;
		sourceBuf >> z;
		return true;
	}
	bool fromFloatBuf(ByteBuffer &sourceBuf)
	{
		if (sourceBuf.remaining() < sizeof(float)*3)
			return false;

		float tempX,tempY,tempZ;
		sourceBuf >> tempX;
		sourceBuf >> tempY;
		sourceBuf >> tempZ;
		x=tempX;
		y=tempY;
		z=tempZ;
		return true;
	}
	bool toDoubleBuf(ByteBuffer &outputBuf)
	{
		outputBuf << double(x) << double(y) << double(z);
		return true;
	}
	bool toDoubleBuf(byte *outputBuf,size_t maxLen)
	{
		ByteBuffer tempByteBuf;
		toDoubleBuf(tempByteBuf);

		if (outputBuf == NULL || maxLen < tempByteBuf.size())
			return false;

		tempByteBuf.read(outputBuf,tempByteBuf.size());
		return true;
	}
	bool toFloatBuf(ByteBuffer &outputBuf) const
	{
		outputBuf << float(x) << float(y) << float(z);
		return true;
	}
	bool toFloatBuf(byte *outputBuf,size_t maxLen) const
	{
		ByteBuffer tempByteBuf;
		toFloatBuf(tempByteBuf);

		if (outputBuf == NULL || maxLen < tempByteBuf.size())
			return false;

		tempByteBuf.read(outputBuf,tempByteBuf.size());
		return true;
	}

	double x;
	double y;
	double z;
	double rot;
};

#endif
