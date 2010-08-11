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

#ifndef MXOSIM_BYTEBUFFER_H
#define MXOSIM_BYTEBUFFER_H

#include <string>
#include <vector>
#include <map>
#include <list>
#include <cstring>
#include <stdexcept>

class ByteBuffer
{
public:
	class out_of_range : public std::out_of_range
	{
	public:
		out_of_range() : std::out_of_range("ByteBuffer") {}
		out_of_range(const string& reason) : std::out_of_range(reason) {}
	};

	ByteBuffer() :
		_rpos(0), _wpos(0)
	{
		_storage.reserve(DEFAULT_SIZE);
	}
	ByteBuffer(size_t res) :
		_rpos(0), _wpos(0)
	{
		_storage.reserve(res);
	}
	ByteBuffer(const ByteBuffer &buf) :
		_rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage)
	{
	}

	ByteBuffer(const string &input) :
		_rpos(0), _wpos(0)
	{
		_storage.reserve(input.size());
		append(input);
		_rpos = _wpos = 0;
	}
	ByteBuffer(const vector<byte> &input) :
		_rpos(0), _wpos(0)
	{
		_storage.reserve(input.size());
		append(input);
		_rpos = _wpos = 0;
	}
	ByteBuffer(const byte* data, size_t len) :
		_rpos(0), _wpos(0)
	{
		if (len > 0 && data != NULL)
		{
			_storage.reserve(len);
			append(data, len);
			_rpos = _wpos = 0;
		}
	}
	ByteBuffer(const char* data, size_t len) :
		_rpos(0), _wpos(0)
	{
		if (len > 0 && data != NULL)
		{
			_storage.reserve(len);
			append(data, len);
			_rpos = _wpos = 0;
		}
	}

	void clear()
	{
		_storage.clear();
		_rpos = _wpos = 0;
	}

	//template <typename T> void insert(size_t pos, T value) {
	//  insert(pos, (uint8 *)&value, sizeof(value));
	//}
	template<typename T> void append(T value)
	{
		append((uint8 *) &value, sizeof(value));
	}
	template<typename T> void put(size_t pos, T value)
	{
		put(pos, (uint8 *) &value, sizeof(value));
	}

	// stream like operators for storing data
	ByteBuffer &operator<<(bool value)
	{
		append<char> ((char) value);
		return *this;
	}
	ByteBuffer &operator<<(uint8 value)
	{
		append<uint8> (value);
		return *this;
	}
	ByteBuffer &operator<<(char value)
	{
		append<char> (value);
		return *this;
	}

	ByteBuffer &operator<<(uint16 value)
	{
		append<uint16> (value);
		return *this;
	}

	ByteBuffer &operator<<(short value)
	{
		append<short> (value);
		return *this;
	}

	ByteBuffer &operator<<(uint32 value)
	{
		append<uint32> (value);
		return *this;
	}
	ByteBuffer &operator<<(uint64 value)
	{
		append<uint64> (value);
		return *this;
	}
	ByteBuffer &operator<<(int value)
	{
		append<int> (value);
		return *this;
	}
	/* 
	 ByteBuffer &operator<<(unicode value) {
	 append<unicode>(value);
	 return *this;
	 }
	 */
	ByteBuffer &operator<<(float value)
	{
		append<float> (value);
		return *this;
	}
	ByteBuffer &operator<<(double value)
	{
		append<double> (value);
		return *this;
	}
	ByteBuffer &operator<<(const std::string &value)
	{
		append((uint8 *) value.data(), value.length());
		//append((uint8)0);
		return *this;
	}
	ByteBuffer &operator<<(const char *str)
	{
		append((uint8 *) str, strlen(str));
		//append((uint8)0);
		return *this;
	}

	// stream like operators for reading data
	ByteBuffer &operator>>(bool &value)
	{
		value = read<char> () > 0 ? true : false;
		return *this;
	}
	ByteBuffer &operator>>(uint8 &value)
	{
		value = read<uint8> ();
		return *this;
	}
	ByteBuffer &operator>>(uint16 &value)
	{
		value = read<uint16> ();
		return *this;
	}
	ByteBuffer &operator>>(uint32 &value)
	{
		value = read<uint32> ();
		return *this;
	}
	ByteBuffer &operator>>(uint64 &value)
	{
		value = read<uint64> ();
		return *this;
	}
	ByteBuffer &operator>>(float &value)
	{
		value = read<float> ();
		return *this;
	}
	ByteBuffer &operator>>(double &value)
	{
		value = read<double> ();
		return *this;
	}
	ByteBuffer &operator>>(std::string& value)
	{
		value.clear();
		while (true)
		{
			char c = read<char> ();
			if (c == 0)
				break;
			value += c;
		}
		return *this;
	}

	uint8 operator[](size_t pos)
	{
		return read<uint8> (pos);
	}

	size_t rpos() const
	{
		return _rpos;
	}

	size_t rpos(size_t rpos)
	{
		if (rpos > count())
			throw out_of_range();

		_rpos = rpos;
		return _rpos;
	}

	size_t wpos() const
	{
		return _wpos;
	}

	size_t wpos(size_t wpos)
	{
		_wpos = wpos;
		return _wpos;
	}

	template<typename T> T read()
	{
		if (_rpos + sizeof(T) > count())
			throw out_of_range();

		T r = read<T> (_rpos);
		_rpos += sizeof(T);
		return r;
	}

	template<typename T> T read(size_t pos) const
	{
		if (pos + sizeof(T) > count())
			throw out_of_range();

		return *((T*) &_storage[pos]);
	}

	template<typename T> void read(T* dest, size_t cnt)
	{
		size_t outSize = cnt*sizeof(T);
		if (_rpos + outSize > count())
			throw out_of_range();

		if (outSize > 0)
		{
			memcpy(dest, &_storage[_rpos], outSize );
			_rpos += outSize;
		}
	}

	template<typename T> void read(vector<T> &dest)
	{
		this->read(&dest[0],dest.size());
	}

	uint8 *read(size_t len)
	{
		if (_rpos + len > count())
			throw out_of_range();

		uint8 *Temp = new uint8[len];
		memcpy(Temp, &_storage[_rpos], len);
		_rpos += len;
		return Temp;
	}

	inline char *contents() const
	{
		return (char*)&_storage[0];
	}

	inline size_t count() const
	{
		return (uint16)_storage.size();
	}

	inline uint16 size() const
	{
		return (uint16)count();
	}

	inline size_t remaining() const
	{
		return this->count() - this->rpos();
	}

	// one should never use resize probably
	void resize(size_t newsize)
	{
		_storage.resize(newsize);
		_rpos = 0;
		_wpos = count();
	}

	void reserve(size_t ressize)
	{
		if (ressize > count())
			_storage.reserve(ressize);
	}

	// appending to the end of buffer
	void append(const string& str)
	{
		append(str.data(),str.size());
	}
	void append(const vector<byte> &vect)
	{
		append(&vect[0], vect.size());
	}
	void append(const char *src, size_t cnt)
	{
		append((const uint8 *) src, cnt);
	}
	void append(const uint8 *src, size_t cnt)
	{
		if (!cnt)
			return;

		// noone should even need uint8buffer longer than 10mb
		// if you DO need, think about it
		// then think some more
		// then use something else
		// -- qz
		assert(count() < 10000000);

		if (count() < _wpos + cnt)
			_storage.resize(_wpos + cnt);

		memcpy(&_storage[_wpos], src, cnt);
		_wpos += cnt;
	}
	void append(const ByteBuffer& buffer)
	{
		if (buffer.count() > 0)
			append(buffer.contents(), buffer.count());
	}

	void put(size_t pos, const uint8 *src, size_t cnt)
	{
		if (pos + cnt > count())
			throw out_of_range();

		memcpy(&_storage[pos], src, cnt);
	}

	//MXO specific data formats
	string readString()
	{
		uint16 strLen=0;
		(*this) >> strLen;
		if(this->remaining() < strLen)
			throw out_of_range();

		vector<char> strBuf(strLen);
		this->read(strBuf);
		if (strBuf.back() == 0)
			strBuf.pop_back();

		return string(&strBuf[0],strBuf.size());
	}
	void writeString(const string& str)
	{
		(*this) << uint16(str.length()+1);
		this->append(str.c_str(),str.length()+1);
	}
	void writeString(const uint8* data, size_t dataLen)
	{
		writeString(string((const char*)data,dataLen));
	}
	void writeString(const char* str)
	{
		writeString(string(str));
	}

protected:
	// read and write positions
	size_t _rpos, _wpos;
	std::vector<uint8> _storage;
private:
	const static size_t DEFAULT_SIZE = 0x1000;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T> ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
	b << (uint32) v.size();
	for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++)
	{
		b << *i;
	}
	return b;
}

template<typename T> ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
	uint32 vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template<typename T> ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
	b << (uint32) v.size();
	for (typename std::list<T>::iterator i = v.begin(); i != v.end(); i++)
	{
		b << *i;
	}
	return b;
}

template<typename T> ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
	uint32 vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template<typename K, typename V> ByteBuffer &operator<<(ByteBuffer &b,
		std::map<K, V> &m)
{
	b << (uint32) m.size();
	for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); i++)
	{
		b << i->first << i->second;
	}
	return b;
}

template<typename K, typename V> ByteBuffer &operator>>(ByteBuffer &b,
		std::map<K, V> &m)
{
	uint32 msize;
	b >> msize;
	m.clear();
	while (msize--)
	{
		K k;
		V v;
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}

#endif
