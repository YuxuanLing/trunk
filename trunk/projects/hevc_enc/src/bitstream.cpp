/*****************************************************************************
* bitstream.cpp: bitstream writing
*****************************************************************************
* Copyright (C) 2018-2028 hevc encoder project
*
* Authors: Yuxuan  <yuxuanl@gmail.com>
*****************************************************************************/
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include "bitstream.h"


using namespace std;


// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================


ComOutputBitStream::ComOutputBitStream()
{
	clear();
}

ComOutputBitStream::~ComOutputBitStream()
{

}



Void ComOutputBitStream::write(UINT32 uiBits, UINT32 uiNumberOfBits)
{
	assert(uiNumberOfBits <= 32);
	assert(uiNumberOfBits == 32 || (uiBits & (~0 << uiNumberOfBits)) == 0);

	/* any modulo 8 remainder of num_total_bits cannot be written this time,
	* and will be held until next time. */
	UINT32  num_total_bits = uiNumberOfBits + m_num_held_bits;
	UINT32  next_num_held_bits = num_total_bits % 8;

	/* form a byte aligned word (write_bits), by concatenating any held bits
	* with the new bits, discarding the bits that will form the next_held_bits.
	* eg: H = held bits, V = n new bits        /---- next_held_bits
	* len(H)=7, len(V)=1: ... ---- HHHH HHHV . 0000 0000, next_num_held_bits=0
	* len(H)=7, len(V)=2: ... ---- HHHH HHHV . V000 0000, next_num_held_bits=1
	* if total_bits < 8, the value of v_ is not used */
	UINT8  next_held_bits = uiBits << (8 - next_num_held_bits);

	if (!(num_total_bits >> 3))
	{
		/* insufficient bits accumulated to write out, append new_held_bits to
		* current held_bits */
		/* NB, this requires that v only contains 0 in bit positions {31..n} */
		m_held_bits |= next_held_bits;
		m_num_held_bits = next_num_held_bits;
		return;
	}

	/* topword serves to justify held_bits to align with the msb of uiBits */
	UINT32 topword = (uiNumberOfBits - next_num_held_bits) & ~((1 << 3) - 1);      //uiNumberOfBits - next_num_held_bits is the number that original held bits needed to shift left , 
	                                                                               //it is also the number of bits that uiBits need to write to buffer this time
	UINT32 write_bits = (m_held_bits << topword) | (uiBits >> next_num_held_bits); // write_bits = original held bit | (uiBits >> next_num_held_bits higt bits number needed to write out this time) 

	switch (num_total_bits >> 3)
	{
	case 4: m_fifo.push_back(write_bits >> 24);
	case 3: m_fifo.push_back(write_bits >> 16);
	case 2: m_fifo.push_back(write_bits >> 8);
	case 1: m_fifo.push_back(write_bits);
	}

	m_held_bits = next_held_bits;
	m_num_held_bits = next_num_held_bits;
	return;
}



/** insert one bits 1 until the bitstream is byte-aligned */
Void ComOutputBitStream::writeAlignOne()
{
	UINT32 num_bits = getNumBitsUntilByteAligned();
	write((1<<num_bits) - 1, num_bits);
	return;
}

Void ComOutputBitStream::writeAlignZero()
{
	if (0 == m_num_held_bits)
	{
		return;
	}
	m_fifo.push_back(m_held_bits);
	m_held_bits = 0;
	m_num_held_bits = 0;
}

/**
* insert the contents of the bytealigned (and flushed) bitstream src
* into this at byte position pos.
*/
Void ComOutputBitStream::insertAt(const ComOutputBitStream& src, UINT32 pos)
{
	UINT32 src_bits = src.getNumOfWrittenBits();
	assert(0 == src_bits % 8);

	vector<uint8_t>::iterator at = m_fifo.begin() + pos;
	m_fifo.insert(at, src.m_fifo.begin(), src.m_fifo.end());
}


/**
- add substream to the end of the current bitstream
.
\param  pcSubstream  substream to be added
*/
Void   ComOutputBitStream::addSubstream(ComOutputBitStream* pcSubstream)
{
	UINT32 uiNumBits = pcSubstream->getNumOfWrittenBits();

	const vector<UINT8>& rbsp = pcSubstream->getFIFO();
	for (vector<UINT8>::const_iterator it = rbsp.begin(); it != rbsp.end();)
	{
		write(*it++, 8);
	}
	if (uiNumBits & 0x7)
	{
		write(pcSubstream->getHeldBits() >> (8 - (uiNumBits & 0x7)), uiNumBits & 0x7);
	}
}



/**
*  find the emulation code count
*  emulated 00 00 {00,01,02,03}
*/
INT32 ComOutputBitStream::countStartCodeEmulations()
{
	UINT32 cnt = 0;
	vector<uint8_t>& rbsp = getFIFO();
	for (vector<uint8_t>::iterator it = rbsp.begin(); it != rbsp.end();)
	{
		vector<uint8_t>::iterator found = it;
		do
		{
			// find the next emulated 00 00 {00,01,02,03}
			// NB, end()-1, prevents finding a trailing two byte sequence
			found = search_n(found, rbsp.end() - 1, 2, 0);
			found++;
			// if not found, found == end, otherwise found = second zero byte
			if (found == rbsp.end())
			{
				break;
			}
			if (*(++found) <= 3)
			{
				break;
			}
		} while (true);
		it = found;
		if (found != rbsp.end())
		{
			cnt++;
		}
	}
	return cnt;
}


Void ComOutputBitStream::writeByteAlignment()
{
	write(1, 1);
	writeAlignZero();
}


Void ComOutputBitStream::clear()
{
	m_fifo.clear();
	m_held_bits = 0;
	m_num_held_bits = 0;
}


UINT8* ComOutputBitStream::getByteStream() const
{
	return (UINT8*)&m_fifo.front();
}

UINT32 ComOutputBitStream::getByteStreamLength()
{
	return UINT8(m_fifo.size());
}