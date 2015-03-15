#include <stdio.h>
#include "fluid_sys.h"
#include "bitflags_c.h"

BitFlagsC* BitFlagsC_new(int numBits)
{
	int i;
	BitFlagsC *bitFlags = FLUID_NEW(BitFlagsC);
	bitFlags->_flagSize32 = (numBits + 31) >> 5;
	bitFlags->_flags = FLUID_ARRAY(unsigned int, bitFlags->_flagSize32);
	for (i = 0; i < bitFlags->_flagSize32; ++i) bitFlags->_flags[i] = 0;

	return bitFlags;
}

void BitFlagsC_delete(BitFlagsC *bitFlags)
{
	FLUID_FREE(bitFlags->_flags);
	FLUID_FREE(bitFlags);
}

void BitFlagsC_clone(BitFlagsC *this, const BitFlagsC *src)						{ int i; for(i = 0; i < this->_flagSize32; ++i) this->_flags[i] = src->_flags[i]; }
void BitFlagsC_clearAll(BitFlagsC *this)										{ int i; for(i = 0; i < this->_flagSize32; ++i) this->_flags[i] = 0; }
void BitFlagsC_xorAll(BitFlagsC *this, const BitFlagsC *a, const BitFlagsC *b)	{ int i; for(i = 0; i < this->_flagSize32; ++i) this->_flags[i] = a->_flags[i] ^ b->_flags[i]; }

int BitFlagsC_getBit(BitFlagsC *this, int bit)		{ return ((this->_flags[bit>>5] >> (bit&31)) & 1) ? 1 : 0; }
void BitFlagsC_setBit(BitFlagsC *this, int bit)		{ this->_flags[bit>>5] |= (1<<(bit&31)); }
void BitFlagsC_clearBit(BitFlagsC *this, int bit)	{ this->_flags[bit>>5] &= ~(1<<(bit&31)); }
