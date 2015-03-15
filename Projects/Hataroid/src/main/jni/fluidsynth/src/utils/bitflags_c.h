#ifndef BITFLAGS_C_H_
#define BITFLAGS_C_H_

typedef struct
{
	unsigned int	*_flags;
	int				_flagSize32;
} BitFlagsC;

extern BitFlagsC* BitFlagsC_new(int numBits);
extern void BitFlagsC_delete(BitFlagsC *bitFlags);
extern void BitFlagsC_clone(BitFlagsC *this, const BitFlagsC *src);
extern void BitFlagsC_clearAll(BitFlagsC *this);
extern void BitFlagsC_xorAll(BitFlagsC *this, const BitFlagsC *a, const BitFlagsC *b);

extern int BitFlagsC_getBit(BitFlagsC *this, int bit);
extern void BitFlagsC_setBit(BitFlagsC *this, int bit);
extern void BitFlagsC_clearBit(BitFlagsC *this, int bit);

#endif /* BITFLAGS_C_H_ */
