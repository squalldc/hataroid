#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static void *myallocZ(void *q, unsigned n, unsigned m)
{
    q = Z_NULL;
    return calloc(n, m);
}

static void myfreeZ(void *q, void *p)
{
    q = Z_NULL;
    free(p);
}

unsigned char *uncompressGZ(const unsigned char *src, int srcsize, int *inflateLenOut)
{
	int ret;
	z_stream strm;

	*inflateLenOut = 0;

	strm.zalloc = myallocZ;
	strm.zfree = myfreeZ;
	strm.opaque = Z_NULL;

	strm.next_in = 0;
	strm.avail_in = Z_NULL;
	ret = inflateInit2(&strm, 15 + 16);
	if (ret != Z_OK)
	{
		return NULL;
	}

	int uncompressedLen = (src[srcsize-4]) | (src[srcsize-3] << 8) | (src[srcsize-2] << 16) | (src[srcsize-1] << 24);

	unsigned char *buf = (unsigned char *)malloc(uncompressedLen);
	if (buf == NULL)
	{
		inflateEnd(&strm);
		return NULL;
	}

	strm.next_out = (Bytef *)buf;
	strm.avail_out = uncompressedLen;
	strm.next_in = (Bytef*)src;
	strm.avail_in = srcsize;
	ret = inflate(&strm, Z_NO_FLUSH);
	if (ret == Z_DATA_ERROR)
	{
		inflateEnd(&strm);
		free(buf);
		return NULL;
	}
	if (ret == Z_STREAM_END)
	{
		//inflateReset(&strm);
	}

	inflateEnd(&strm);

	int inflateLen = uncompressedLen - strm.avail_out;
	if (inflateLen != uncompressedLen)
	{
		free(buf);
		return NULL;
	}

	*inflateLenOut = inflateLen;
	return buf;
}
