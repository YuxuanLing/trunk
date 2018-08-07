/*****************************************************************************
* bitstream.h: bitstream writing
*****************************************************************************
* Copyright (C) 2018-2028 hevc encoder project
*
* Authors: Yuxuan  <yuxuanl@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
*
* This program is also available under a commercial proprietary license.
*****************************************************************************/
#ifndef HEVC_ENC_BS_H
#define HEVC_ENC_BS_H

#include "commonDef.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000





// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// pure virtual class for basic bit handling
class ComBitIf
{
public:
	virtual Void        writeAlignOne() {};
	virtual Void        writeAlignZero() {};
	virtual Void        write(UINT32 uiBits, UINT32 uiNumberOfBits) = 0;
	virtual Void        resetBits() = 0;
	virtual UINT32        getNumOfWrittenBits() const = 0;
	virtual UINT32        getNumBitsUntilByteAligned() const = 0;
	virtual ~ComBitIf() {}
};



class ComOutputBitStream : public ComBitIf
{
	/**
	* FIFO for storage of bytes.  Use:
	*  - fifo.push_back(x) to append words
	*  - fifo.clear() to empty the FIFO
	*  - &fifo.front() to get a pointer to the data array.
	*    NB, this pointer is only valid until the next push_back()/clear()
	*/
private:

	std::vector<UINT8> m_fifo;
	UINT32  m_num_held_bits;     /// number of bits not flushed to bytestream.
	UINT8   m_held_bits;         ///the bits held and not flushed to bytestream. this value is always msb-aligned, bigendian.

public:
	ComOutputBitStream();
	~ComOutputBitStream();

	// interface for encoding
	/**
	* append uiNumberOfBits least significant bits of uiBits to
	* the current bitstream
	*/
	Void        write(UINT32 uiBits, UINT32 uiNumberOfBits);

	/** insert one bits until the bitstream is byte-aligned */
	Void        writeAlignOne();

	/** insert zero bits until the bitstream is byte-aligned */
	Void        writeAlignZero();

	/** this function should never be called */
	Void resetBits() { assert(0); }

	/**
	* Return a pointer to the start of the byte-stream buffer.
	* Pointer is valid until the next write/flush/reset call.
	* NB, data is arranged such that subsequent bytes in the
	* bytestream are stored in ascending addresses.
	*/
	UINT8* getByteStream() const;

	/**
	* Return the number of valid bytes available from  getByteStream()
	*/
	UINT32 getByteStreamLength();

	/**
	* Reset all internal state.
	*/
	Void clear();

	/**
	* returns the number of bits that need to be written to
	* achieve byte alignment.
	*/
	UINT32 getNumBitsUntilByteAligned() const { return (8 - m_num_held_bits) & 0x7; }

	/**
	* Return the number of bits that have been written since the last clear()
	*/
	UINT32 getNumOfWrittenBits() const { return UINT32(m_fifo.size()) * 8 + m_num_held_bits; }

	/**
	* insert the contents of the bytealigned (and flushed) bitstream src
	* into this at byte position pos.
	*/
	Void insertAt(const ComOutputBitStream & src, UINT32 pos);

	/**
	* Return a reference to the internal fifo
	*/
	std::vector<UINT8>& getFIFO() { return m_fifo; }

	UINT8 getHeldBits() { return m_held_bits; }

	/**
	* add the stream pcSubstream to this stream
	*/
	Void          addSubstream(ComOutputBitStream* pcSubstream);

	Void writeByteAlignment();

	//! returns the number of start code emulations contained in the current buffer
	INT32 countStartCodeEmulations();

};



#endif


