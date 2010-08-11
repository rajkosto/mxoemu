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

#ifndef MXOSIM_BITSTREAM_H
#define MXOSIM_BITSTREAM_H

#include "Common.h"
#include "Util.h"
#include <math.h>
#include <float.h>

#ifdef _MSC_VER
#pragma warning( push )
#endif

/// Threshold at which to do a malloc / free rather than pushing data onto a fixed stack for the bitstream class
/// Arbitrary size, just picking something likely to be larger than most packets
#define BITSTREAM_STACK_ALLOCATION_SIZE 256

/// This class allows you to write and read native types as a string of bits.
class BitStream
{

public:
	/// Default Constructor
	BitStream();

	/// \brief Create the bitstream, with some number of bytes to immediately allocate.
	/// \details There is no benefit to calling this, unless you know exactly how many bytes you need and it is greater than BITSTREAM_STACK_ALLOCATION_SIZE.
	/// In that case all it does is save you one or more realloc calls.
	/// \param[in] initialBytesToAllocate the number of bytes to pre-allocate.
	BitStream( const unsigned int initialBytesToAllocate );

	/// \brief Initialize the BitStream, immediately setting the data it contains to a predefined pointer.
	/// \details Set \a _copyData to true if you want to make an internal copy of the data you are passing. Set it to false to just save a pointer to the data.
	/// You shouldn't call Write functions with \a _copyData as false, as this will write to unallocated memory
	/// 99% of the time you will use this function to cast Packet::data to a bitstream for reading, in which case you should write something as follows:
	/// \code
	/// BitStream bs(packet->data, packet->length, false);
	/// \endcode
	/// \param[in] _data An array of bytes.
	/// \param[in] lengthInBytes Size of the \a _data.
	/// \param[in] _copyData true or false to make a copy of \a _data or not.
	BitStream( unsigned char* _data, const unsigned int lengthInBytes, bool _copyData );

	// Destructor
	~BitStream();

	/// Resets the bitstream for reuse.
	void Reset( void );

	/// \brief Bidirectional serialize/deserialize any integral type to/from a bitstream.  
	/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
	/// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
	/// \param[in] var The value to write
	/// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
	template <class templateType>
	bool Serialize(bool writeToBitstream, templateType &var);

	/// \brief Bidirectional serialize/deserialize any integral type to/from a bitstream. 
	/// \details If the current value is different from the last value
	/// the current value will be written.  Otherwise, a single bit will be written
	/// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
	/// \param[in] currentValue The current value to write
	/// \param[in] lastValue The last value to compare against.  Only used if \a writeToBitstream is true.
	/// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
	template <class templateType>
	bool SerializeDelta(bool writeToBitstream, templateType &currentValue, templateType lastValue);

	/// \brief Bidirectional version of SerializeDelta when you don't know what the last value is, or there is no last value.
	/// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
	/// \param[in] currentValue The current value to write
	/// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
	template <class templateType>
	bool SerializeDelta(bool writeToBitstream, templateType &currentValue);

	/// \brief Bidirectional serialize/deserialize any integral type to/from a bitstream.
	/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
	/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
	/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
	/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
	/// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
	/// \param[in] var The value to write
	/// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
	template <class templateType>
	bool SerializeCompressed(bool writeToBitstream, templateType &var);

	/// \brief Bidirectional serialize/deserialize any integral type to/from a bitstream.  
	/// \details If the current value is different from the last value
	/// the current value will be written.  Otherwise, a single bit will be written
	/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
	/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
	/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
	/// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
	/// \param[in] currentValue The current value to write
	/// \param[in] lastValue The last value to compare against.  Only used if \a writeToBitstream is true.
	/// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
	template <class templateType>
	bool SerializeCompressedDelta(bool writeToBitstream, templateType &currentValue, templateType lastValue);

	/// \brief Save as SerializeCompressedDelta(templateType &currentValue, templateType lastValue) when we have an unknown second parameter
	/// \return true on data read. False on insufficient data in bitstream
	template <class templateType>
	bool SerializeCompressedDelta(bool writeToBitstream, templateType &currentValue);

	/// \brief Bidirectional serialize/deserialize an array or casted stream or raw data.  This does NOT do endian swapping.
	/// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
	/// \param[in] input a byte buffer
	/// \param[in] numberOfBytes the size of \a input in bytes
	/// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
	bool Serialize(bool writeToBitstream,  char* input, const unsigned int numberOfBytes );

	/// \brief Bidirectional serialize/deserialize numberToSerialize bits to/from the input. 
	/// \details Right aligned data means in the case of a partial byte, the bits are aligned
	/// from the right (bit 0) rather than the left (as in the normal
	/// internal representation) You would set this to true when
	/// writing user data, and false when copying bitstream data, such
	/// as writing one bitstream to another
	/// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
	/// \param[in] input The data
	/// \param[in] numberOfBitsToSerialize The number of bits to write
	/// \param[in] rightAlignedBits if true data will be right aligned
	/// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
	bool SerializeBits(bool writeToBitstream, unsigned char* input, const uint32 numberOfBitsToSerialize, const bool rightAlignedBits = true );

	/// \brief Write any integral type to a bitstream.  
	/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
	/// \param[in] var The value to write
	template <class templateType>
	void Write(templateType var);

	/// \brief Write the dereferenced pointer to any integral type to a bitstream.  
	/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
	/// \param[in] var The value to write
	template <class templateType>
	void WritePtr(templateType *var);

	/// \brief Write any integral type to a bitstream.  
	/// \details If the current value is different from the last value
	/// the current value will be written.  Otherwise, a single bit will be written
	/// \param[in] currentValue The current value to write
	/// \param[in] lastValue The last value to compare against
	template <class templateType>
	void WriteDelta(templateType currentValue, templateType lastValue);

	/// \brief WriteDelta when you don't know what the last value is, or there is no last value.
	/// \param[in] currentValue The current value to write
	template <class templateType>
	void WriteDelta(templateType currentValue);

	/// \brief Write any integral type to a bitstream.  
	/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
	/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
	/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
	/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
	/// \param[in] var The value to write
	template <class templateType>
	void WriteCompressed(templateType var);

	/// \brief Write any integral type to a bitstream.  
	/// \details If the current value is different from the last value
	/// the current value will be written.  Otherwise, a single bit will be written
	/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
	/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
	/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
	/// \param[in] currentValue The current value to write
	/// \param[in] lastValue The last value to compare against
	template <class templateType>
	void WriteCompressedDelta(templateType currentValue, templateType lastValue);

	/// \brief Save as WriteCompressedDelta(templateType currentValue, templateType lastValue) when we have an unknown second parameter
	template <class templateType>
	void WriteCompressedDelta(templateType currentValue);

	/// \brief Read any integral type from a bitstream.  
	/// \details Define __BITSTREAM_NATIVE_END if you need endian swapping.
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <class templateType>
	bool Read(templateType &var);

	/// \brief Read into a pointer to any integral type from a bitstream.  
	/// \details Define __BITSTREAM_NATIVE_END if you need endian swapping.
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <class templateType>
	bool ReadPtr(templateType *var);

	/// \brief Read any integral type from a bitstream.  
	/// \details If the written value differed from the value compared against in the write function,
	/// var will be updated.  Otherwise it will retain the current value.
	/// ReadDelta is only valid from a previous call to WriteDelta
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <class templateType>
	bool ReadDelta(templateType &var);

	/// \brief Read any integral type from a bitstream.  
	/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
	/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
	/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
	/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <class templateType>
	bool ReadCompressed(templateType &var);

	/// \brief Read any integral type from a bitstream.  
	/// \details If the written value differed from the value compared against in the write function,
	/// var will be updated.  Otherwise it will retain the current value.
	/// the current value will be updated.
	/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
	/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
	/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
	/// ReadCompressedDelta is only valid from a previous call to WriteDelta
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <class templateType>
	bool ReadCompressedDelta(templateType &var);

	/// \brief Read one bitstream to another.
	/// \param[in] numberOfBits bits to read
	/// \param bitStream the bitstream to read into from
	/// \return true on success, false on failure.
	bool Read( BitStream *bitStream, uint32 numberOfBits );
	bool Read( BitStream *bitStream );
	bool Read( BitStream &bitStream, uint32 numberOfBits );
	bool Read( BitStream &bitStream );

	/// \brief Write an array or casted stream or raw data.  This does NOT do endian swapping.
	/// \param[in] input a byte buffer
	/// \param[in] numberOfBytes the size of \a input in bytes
	void Write( const char* input, const unsigned int numberOfBytes );

	/// \brief Write one bitstream to another.
	/// \param[in] numberOfBits bits to write
	/// \param bitStream the bitstream to copy from
	void Write( BitStream *bitStream, uint32 numberOfBits );
	void Write( BitStream *bitStream );
	void Write( BitStream &bitStream, uint32 numberOfBits );
	void Write( BitStream &bitStream );

	/// \brief Read an array or casted stream of byte.
	/// \details The array is raw data. There is no automatic endian conversion with this function
	/// \param[in] output The result byte array. It should be larger than @em numberOfBytes.
	/// \param[in] numberOfBytes The number of byte to read
	/// \return true on success false if there is some missing bytes.
	bool Read( char* output, const unsigned int numberOfBytes );

	/// \brief Sets the read pointer back to the beginning of your data.
	void ResetReadPointer( void );

	/// \brief Sets the write pointer back to the beginning of your data.
	void ResetWritePointer( void );

	/// \brief This is good to call when you are done with the stream to make
	/// sure you didn't leave any data left over void
	void AssertStreamEmpty( void );

	/// \brief printf the bits in the stream.  Great for debugging.
	void PrintBits( char *out ) const;
	void PrintBits( void ) const;
	void PrintHex( char *out ) const;
	void PrintHex( void ) const;

	/// \brief Ignore data we don't intend to read
	/// \param[in] numberOfBits The number of bits to ignore
	void IgnoreBits( const uint32 numberOfBits );

	/// \brief Ignore data we don't intend to read
	/// \param[in] numberOfBits The number of bytes to ignore
	void IgnoreBytes( const unsigned int numberOfBytes );

	/// \brief Move the write pointer to a position on the array.
	/// \param[in] offset the offset from the start of the array.
	/// \attention
	/// \details Dangerous if you don't know what you are doing!
	/// For efficiency reasons you can only write mid-stream if your data is byte aligned.
	void SetWriteOffset( const uint32 offset );

	/// \brief Returns the length in bits of the stream
	inline uint32 GetNumberOfBitsUsed( void ) const {return GetWriteOffset();}
	inline uint32 GetWriteOffset( void ) const {return numberOfBitsUsed;}

	/// \brief Returns the length in bytes of the stream
	inline uint32 GetNumberOfBytesUsed( void ) const {return BITS_TO_BYTES( numberOfBitsUsed );}

	/// \brief Returns the number of bits into the stream that we have read
	inline uint32 GetReadOffset( void ) const {return readOffset;}

	/// \brief Sets the read bit index
	void SetReadOffset( const uint32 newReadOffset ) {readOffset=newReadOffset;}

	/// \brief Returns the number of bits left in the stream that haven't been read
	inline uint32 GetNumberOfUnreadBits( void ) const {return numberOfBitsUsed - readOffset;}

	/// \brief Makes a copy of the internal data for you \a _data will point to
	/// the stream. Partial bytes are left aligned.
	/// \param[out] _data The allocated copy of GetData()
	/// \return The length in bits of the stream.
	uint32 CopyData( unsigned char** _data ) const;

	/// \internal
	/// Set the stream to some initial data.
	void SetData( unsigned char *input );

	/// Gets the data that BitStream is writing to / reading from.
	/// Partial bytes are left aligned.
	/// \return A pointer to the internal state
	inline unsigned char* GetData( void ) const {return data;}

	/// \brief Write numberToWrite bits from the input source.
	/// \details Right aligned data means in the case of a partial byte, the bits are aligned
	/// from the right (bit 0) rather than the left (as in the normal
	/// internal representation) You would set this to true when
	/// writing user data, and false when copying bitstream data, such
	/// as writing one bitstream to another.
	/// \param[in] input The data
	/// \param[in] numberOfBitsToWrite The number of bits to write
	/// \param[in] rightAlignedBits if true data will be right aligned
	void WriteBits( const unsigned char* input, uint32 numberOfBitsToWrite, const bool rightAlignedBits = true );

	/// \brief Align the bitstream to the byte boundary and then write the
	/// specified number of bits.  
	/// \details This is faster than WriteBits but
	/// wastes the bits to do the alignment and requires you to call
	/// ReadAlignedBits at the corresponding read position.
	/// \param[in] input The data
	/// \param[in] numberOfBytesToWrite The size of input.
	void WriteAlignedBytes( const unsigned char *input, const unsigned int numberOfBytesToWrite );

	/// \brief Aligns the bitstream, writes inputLength, and writes input. Won't write beyond maxBytesToWrite
	/// \param[in] input The data
	/// \param[in] inputLength The size of input.
	/// \param[in] maxBytesToWrite Max bytes to write
	void WriteAlignedBytesSafe( const char *input, const unsigned int inputLength, const unsigned int maxBytesToWrite );

	/// \brief Read bits, starting at the next aligned bits. 
	/// \details Note that the modulus 8 starting offset of the sequence must be the same as
	/// was used with WriteBits. This will be a problem with packet
	/// coalescence unless you byte align the coalesced packets.
	/// \param[in] output The byte array larger than @em numberOfBytesToRead
	/// \param[in] numberOfBytesToRead The number of byte to read from the internal state
	/// \return true if there is enough byte.
	bool ReadAlignedBytes( unsigned char *output, const unsigned int numberOfBytesToRead );

	/// \brief Reads what was written by WriteAlignedBytesSafe.
	/// \param[in] input The data
	/// \param[in] maxBytesToRead Maximum number of bytes to read
	/// \return true on success, false on failure.
	bool ReadAlignedBytesSafe( char *input, int &inputLength, const int maxBytesToRead );
	bool ReadAlignedBytesSafe( char *input, unsigned int &inputLength, const unsigned int maxBytesToRead );

	/// \brief Same as ReadAlignedBytesSafe() but allocates the memory for you using new, rather than assuming it is safe to write to
	/// \param[in] input input will be deleted if it is not a pointer to 0
	/// \return true on success, false on failure.
	bool ReadAlignedBytesSafeAlloc( char **input, int &inputLength, const unsigned int maxBytesToRead );
	bool ReadAlignedBytesSafeAlloc( char **input, unsigned int &inputLength, const unsigned int maxBytesToRead );

	/// \brief Align the next write and/or read to a byte boundary.  
	/// \details This can be used to 'waste' bits to byte align for efficiency reasons It
	/// can also be used to force coalesced bitstreams to start on byte
	/// boundaries so so WriteAlignedBits and ReadAlignedBits both
	/// calculate the same offset when aligning.
	void AlignWriteToByteBoundary( void );

	/// \brief Align the next write and/or read to a byte boundary.  
	/// \details This can be used to 'waste' bits to byte align for efficiency reasons It
	/// can also be used to force coalesced bitstreams to start on byte
	/// boundaries so so WriteAlignedBits and ReadAlignedBits both
	/// calculate the same offset when aligning.
	void AlignReadToByteBoundary( void );

	/// \brief Read \a numberOfBitsToRead bits to the output source.
	/// \details alignBitsToRight should be set to true to convert internal
	/// bitstream data to userdata. It should be false if you used
	/// WriteBits with rightAlignedBits false
	/// \param[in] output The resulting bits array
	/// \param[in] numberOfBitsToRead The number of bits to read
	/// \param[in] alignBitsToRight if true bits will be right aligned.
	/// \return true if there is enough bits to read
	bool ReadBits( unsigned char *output, uint32 numberOfBitsToRead, const bool alignBitsToRight = true );

	/// \brief Write a 0
	void Write0( void );

	/// \brief Write a 1
	void Write1( void );

	/// \brief Reads 1 bit and returns true if that bit is 1 and false if it is 0.
	bool ReadBit( void );

	/// \brief If we used the constructor version with copy data off, this
	/// *makes sure it is set to on and the data pointed to is copied.
	void AssertCopyData( void );

	/// \brief Use this if you pass a pointer copy to the constructor
	/// *(_copyData==false) and want to overallocate to prevent
	/// reallocation.
	void SetNumberOfBitsAllocated( const uint32 lengthInBits );

	/// \brief Reallocates (if necessary) in preparation of writing numberOfBitsToWrite
	void AddBitsAndReallocate( const uint32 numberOfBitsToWrite );

	/// \internal
	/// \return How many bits have been allocated internally
	uint32 GetNumberOfBitsAllocated(void) const;

	// \brief Read strings, non reference.
	bool Read(char *var);
	bool Read(unsigned char *var);

	/// ---- Member function template specialization declarations ----
	// Used for VC7
#if defined(_MSC_VER) && _MSC_VER == 1300
	/// Write a bool to a bitstream.
	/// \param[in] var The value to write
	template <>
	void Write(bool var);

	/// \brief Write a bool delta.  
	/// \details Same thing as just calling Write
	/// \param[in] currentValue The current value to write
	/// \param[in] lastValue The last value to compare against
	template <>
	void WriteDelta(bool currentValue, bool lastValue);

	template <>
	void WriteCompressed(bool var);

	/// For values between -1 and 1
	template <>
	void WriteCompressed(float var);

	/// For values between -1 and 1
	template <>
	void WriteCompressed(double var);

	/// \brief Write a bool delta.  
	/// \details Same thing as just calling Write
	/// \param[in] currentValue The current value to write
	/// \param[in] lastValue The last value to compare against
	template <>
	void WriteCompressedDelta(bool currentValue, bool lastValue);

	/// \brief Save as WriteCompressedDelta(bool currentValue, templateType lastValue) 
	/// when we have an unknown second bool
	template <>
	void WriteCompressedDelta(bool currentValue);

	/// \brief Read a bool from a bitstream.
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <>
	bool Read(bool &var);

	/// \brief Read a bool from a bitstream.
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <>
	bool ReadDelta(bool &var);

	template <>
	bool ReadCompressed(bool &var);

	template <>
	bool ReadCompressed(float &var);

	/// For values between -1 and 1
	/// \return true on success, false on failure.
	template <>
	bool ReadCompressed(double &var);

	template <>
	bool ReadCompressed(char* &var);
	template <>
	bool ReadCompressed(unsigned char *&var);

	/// \brief Read a bool from a bitstream.
	/// \param[in] var The value to read
	/// \return true on success, false on failure.
	template <>
	bool ReadCompressedDelta(bool &var);
#endif

	static bool DoEndianSwap(void);
	static bool IsBigEndian(void);
	static bool IsNetworkOrder(void);
	static void ReverseBytes(unsigned char *input, unsigned char *output, const unsigned int length);
	static void ReverseBytesInPlace(unsigned char *data,const unsigned int length);

private:

	BitStream( const BitStream &invalid) 
	{
		(void) invalid;
		assert(0);
	}

	/// \brief Assume the input source points to a native type, compress and write it.
	void WriteCompressed( const unsigned char* input, const unsigned int size, const bool unsignedData );

	/// \brief Assume the input source points to a compressed native type. Decompress and read it.
	bool ReadCompressed( unsigned char* output,	const unsigned int size, const bool unsignedData );


	uint32 numberOfBitsUsed;

	uint32 numberOfBitsAllocated;

	uint32 readOffset;

	unsigned char *data;

	/// true if the internal buffer is copy of the data passed to the constructor
	bool copyData;

	/// BitStreams that use less than BITSTREAM_STACK_ALLOCATION_SIZE use the stack, rather than the heap to store data.  It switches over if BITSTREAM_STACK_ALLOCATION_SIZE is exceeded
	unsigned char stackData[BITSTREAM_STACK_ALLOCATION_SIZE];
};

template <class templateType>
inline bool BitStream::Serialize(bool writeToBitstream, templateType &var)
{
	if (writeToBitstream)
		Write(var);
	else
		return Read(var);
	return true;
}

template <class templateType>
inline bool BitStream::SerializeDelta(bool writeToBitstream, templateType &currentValue, templateType lastValue)
{
	if (writeToBitstream)
		WriteDelta(currentValue, lastValue);
	else
		return ReadDelta(currentValue);
	return true;
}

template <class templateType>
inline bool BitStream::SerializeDelta(bool writeToBitstream, templateType &currentValue)
{
	if (writeToBitstream)
		WriteDelta(currentValue);
	else
		return ReadDelta(currentValue);
	return true;
}

template <class templateType>
inline bool BitStream::SerializeCompressed(bool writeToBitstream, templateType &var)
{
	if (writeToBitstream)
		WriteCompressed(var);
	else
		return ReadCompressed(var);
	return true;
}

template <class templateType>
inline bool BitStream::SerializeCompressedDelta(bool writeToBitstream, templateType &currentValue, templateType lastValue)
{
	if (writeToBitstream)
		WriteCompressedDelta(currentValue,lastValue);
	else
		return ReadCompressedDelta(currentValue);
	return true;
}

template <class templateType>
inline bool BitStream::SerializeCompressedDelta(bool writeToBitstream, templateType &currentValue)
{
	if (writeToBitstream)
		WriteCompressedDelta(currentValue);
	else
		return ReadCompressedDelta(currentValue);
	return true;
}

inline bool BitStream::Serialize(bool writeToBitstream, char* input, const unsigned int numberOfBytes )
{
	if (writeToBitstream)
		Write(input, numberOfBytes);
	else
		return Read(input, numberOfBytes);
	return true;
}

inline bool BitStream::SerializeBits(bool writeToBitstream, unsigned char* input, const uint32 numberOfBitsToSerialize, const bool rightAlignedBits )
{
	if (writeToBitstream)
		WriteBits(input,numberOfBitsToSerialize,rightAlignedBits);
	else
		return ReadBits(input,numberOfBitsToSerialize,rightAlignedBits);
	return true;
}

template <class templateType>
inline void BitStream::Write(templateType var)
{
	WriteBits( ( unsigned char* ) & var, sizeof( templateType ) * 8, true );
}

template <class templateType>
inline void BitStream::WritePtr(templateType *var)
{
	WriteBits( ( unsigned char* ) var, sizeof( templateType ) * 8, true );
}

/// \brief Write a bool to a bitstream.
/// \param[in] var The value to write
template <>
inline void BitStream::Write(bool var)
{
	if ( var )
		Write1();
	else
		Write0();
}

/// \brief Write any integral type to a bitstream.  
/// \details If the current value is different from the last value
/// the current value will be written.  Otherwise, a single bit will be written
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <class templateType>
inline void BitStream::WriteDelta(templateType currentValue, templateType lastValue)
{
	if (currentValue==lastValue)
	{
		Write(false);
	}
	else
	{
		Write(true);
		Write(currentValue);
	}
}

/// \brief Write a bool delta. Same thing as just calling Write
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <>
inline void BitStream::WriteDelta(bool currentValue, bool lastValue)
{
	(void) lastValue;

	Write(currentValue);
}

/// \brief WriteDelta when you don't know what the last value is, or there is no last value.
/// \param[in] currentValue The current value to write
template <class templateType>
inline void BitStream::WriteDelta(templateType currentValue)
{
	Write(true);
	Write(currentValue);
}

/// \brief Write any integral type to a bitstream.  
/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// \param[in] var The value to write
template <class templateType>
inline void BitStream::WriteCompressed(templateType var)
{
	WriteCompressed( ( unsigned char* ) & var, sizeof( templateType ) * 8, true );
}

template <>
inline void BitStream::WriteCompressed(bool var)
{
	Write(var);
}

/// For values between -1 and 1
template <>
inline void BitStream::WriteCompressed(float var)
{
	assert(var > -1.01f && var < 1.01f);
	if (var < -1.0f)
		var=-1.0f;
	if (var > 1.0f)
		var=1.0f;
	Write((unsigned short)((var+1.0f)*32767.5f));
}

/// For values between -1 and 1
template <>
inline void BitStream::WriteCompressed(double var)
{
	assert(var > -1.01 && var < 1.01);
	if (var < -1.0f)
		var=-1.0f;
	if (var > 1.0f)
		var=1.0f;
#ifdef _DEBUG
	assert(sizeof(unsigned long)==4);
#endif
	Write((unsigned long)((var+1.0)*2147483648.0));
}

/// \brief Write any integral type to a bitstream.  
/// \details If the current value is different from the last value
/// the current value will be written.  Otherwise, a single bit will be written
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <class templateType>
inline void BitStream::WriteCompressedDelta(templateType currentValue, templateType lastValue)
{
	if (currentValue==lastValue)
	{
		Write(false);
	}
	else
	{
		Write(true);
		WriteCompressed(currentValue);
	}
}

/// \brief Write a bool delta.  Same thing as just calling Write
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <>
inline void BitStream::WriteCompressedDelta(bool currentValue, bool lastValue)
{
	(void) lastValue;

	Write(currentValue);
}

/// \brief Save as WriteCompressedDelta(templateType currentValue, templateType lastValue) 
/// when we have an unknown second parameter
template <class templateType>
inline void BitStream::WriteCompressedDelta(templateType currentValue)
{
	Write(true);
	WriteCompressed(currentValue);
}

/// \brief Save as WriteCompressedDelta(bool currentValue, templateType lastValue) 
/// when we have an unknown second bool
template <>
inline void BitStream::WriteCompressedDelta(bool currentValue)
{
	Write(currentValue);
}

/// \brief Read any integral type from a bitstream.  Define __BITSTREAM_NATIVE_END if you need endian swapping.
/// \param[in] var The value to read
template <class templateType>
inline bool BitStream::Read(templateType &var)
{
	return ReadBits( ( unsigned char* ) &var, sizeof(templateType) * 8, true );
}

template <class templateType>
inline bool BitStream::ReadPtr(templateType *var)
{
	return ReadBits( ( unsigned char* ) var, sizeof(templateType) * 8, true );
}

/// \brief Read a bool from a bitstream.
/// \param[in] var The value to read
template <>
inline bool BitStream::Read(bool &var)
{
	if ( readOffset + 1 > numberOfBitsUsed )
		return false;

	if ( data[ readOffset >> 3 ] & ( 0x80 >> ( readOffset & 7 ) ) )   // Is it faster to just write it out here?
		var = true;
	else
		var = false;

	// Has to be on a different line for Mac
	readOffset++;

	return true;
}

/// \brief Read any integral type from a bitstream.  
/// \details If the written value differed from the value compared against in the write function,
/// var will be updated.  Otherwise it will retain the current value.
/// ReadDelta is only valid from a previous call to WriteDelta
/// \param[in] var The value to read
template <class templateType>
inline bool BitStream::ReadDelta(templateType &var)
{
	bool dataWritten;
	bool success;
	success=Read(dataWritten);
	if (dataWritten)
		success=Read(var);
	return success;
}

/// \brief Read a bool from a bitstream.
/// \param[in] var The value to read
template <>
inline bool BitStream::ReadDelta(bool &var)
{
	return Read(var);
}

/// \brief Read any integral type from a bitstream.  
/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// \param[in] var The value to read
template <class templateType>
inline bool BitStream::ReadCompressed(templateType &var)
{
	return ReadCompressed( ( unsigned char* ) &var, sizeof(templateType) * 8, true );
}

template <>
inline bool BitStream::ReadCompressed(bool &var)
{
	return Read(var);
}

/// For values between -1 and 1
template <>
inline bool BitStream::ReadCompressed(float &var)
{
	unsigned short compressedFloat;
	if (Read(compressedFloat))
	{
		var = ((float)compressedFloat / 32767.5f - 1.0f);
		return true;
	}
	return false;
}

/// For values between -1 and 1
template <>
inline bool BitStream::ReadCompressed(double &var)
{
	unsigned long compressedFloat;
	if (Read(compressedFloat))
	{
		var = ((double)compressedFloat / 2147483648.0 - 1.0);
		return true;
	}
	return false;
}

/// \brief Read any integral type from a bitstream.  
/// \details If the written value differed from the value compared against in the write function,
/// var will be updated.  Otherwise it will retain the current value.
/// the current value will be updated.
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// ReadCompressedDelta is only valid from a previous call to WriteDelta
/// \param[in] var The value to read
template <class templateType>
inline bool BitStream::ReadCompressedDelta(templateType &var)
{
	bool dataWritten;
	bool success;
	success=Read(dataWritten);
	if (dataWritten)
		success=ReadCompressed(var);
	return success;
}

/// \brief Read a bool from a bitstream.
/// \param[in] var The value to read
template <>
inline bool BitStream::ReadCompressedDelta(bool &var)
{
	return Read(var);
}

template <class templateType>
BitStream& operator<<(BitStream& out, templateType& c)
{
	out.Write(c);
	return out;
}
template <class templateType>
BitStream& operator>>(BitStream& in, templateType& c)
{
	bool success = in.Read(c);
	assert(success);
	return in;
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
