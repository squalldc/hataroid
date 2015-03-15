#ifndef BITFLAGS_H_
#define BITFLAGS_H_

class BitFlags
{
	public:
		uint32_t	*_flags;
		int			_flagSize32;

	public:
		BitFlags(int numBits)
		{
			_flagSize32 = (numBits + 31) >> 5;
			_flags = new uint32_t [_flagSize32];
			for (int i = 0; i < _flagSize32; ++i) _flags[i] = 0;
		}
		~BitFlags()				{ delete [] _flags; }

		void clone(const BitFlags *src)						{ for(int i = 0; i < _flagSize32; ++i) _flags[i] = src->_flags[i]; }
		void clearAll()										{ for(int i = 0; i < _flagSize32; ++i) _flags[i] = 0; }
		void xorAll(const BitFlags *a, const BitFlags *b)	{ for(int i = 0; i < _flagSize32; ++i) _flags[i] = a->_flags[i] ^ b->_flags[i]; }

		bool getBit(int bit)	{ return ((_flags[bit>>5] >> (bit&31)) & 1) ? true : false; }
		void setBit(int bit)	{ _flags[bit>>5] |= (1<<(bit&31)); }
		void clearBit(int bit)	{ _flags[bit>>5] &= ~(1<<(bit&31)); }
};

#endif /* BITFLAGS_H_ */
