package com.RetroSoft.Hataroid.Util;

public class BitFlags
{
	public int[]	_flags;
	public int		_flagSize32;
	
	public BitFlags(int numBits)
	{
		_flagSize32 = (numBits + 31) >> 5;
		_flags = new int [_flagSize32];
		for (int i = 0; i < _flagSize32; ++i) { _flags[i] = 0; }
	}

	public void clone(BitFlags src)				{ for(int i = 0; i < _flagSize32; ++i) _flags[i] = src._flags[i]; }
	public void clearAll()						{ for(int i = 0; i < _flagSize32; ++i) _flags[i] = 0; }
	public void xorAll(BitFlags a, BitFlags b)	{ for(int i = 0; i < _flagSize32; ++i) _flags[i] = a._flags[i] ^ b._flags[i]; }

	public boolean getBit(int bit)	{ return (((_flags[bit>>5] >> (bit&31)) & 1) != 0) ? true : false; }
	public void setBit(int bit)		{ _flags[bit>>5] |= (1<<(bit&31)); }
	public void clearBit(int bit)	{ _flags[bit>>5] &= ~(1<<(bit&31)); }
	
	public int findFirstEmptyBit()
	{
		for (int i = 0; i < _flagSize32; ++i)
		{
			if (_flags[i] != (int)0xffffffff)
			{
				int f = _flags[i];
				for (int b = 0; b < 32; ++b)
				{
					if ((f & 1) == 0)
					{
						return (i<<5) + b;
					}
					f = f >> 1;
				}
				
				_flags[i] = 0;
			}
		}
		return -1;
	}
}
