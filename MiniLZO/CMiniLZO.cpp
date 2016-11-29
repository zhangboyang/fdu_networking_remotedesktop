#include <cstdlib>
#include <cstring>
#include <cassert>
#include "minilzo-2.09/minilzo.h"
#include "CMiniLZO.h"

#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);


MiniLZO *MiniLZO::Instance()
{
	static MiniLZO inst;
	return &inst;
}

MiniLZO::MiniLZO() : buf(NULL), buf_len(0)
{
	lzo_init();
}
MiniLZO::~MiniLZO()
{
	free(buf);
}

void MiniLZO::AllocBuffer(size_t len)
{
	if (len > buf_len) {
		free(buf);
		buf = malloc(len);
		buf_len = len;
	}
}

size_t MiniLZO::Compress(void *out, void *in, size_t in_len)
{
	assert(in != out);
	AllocBuffer(in_len + in_len / 16 + 64 + 3);
	lzo_uint out_len;
	int r = lzo1x_1_compress((const unsigned char *) in, in_len, (unsigned char *) buf, &out_len, wrkmem);
	if (r == LZO_E_OK && out_len + sizeof(lzohdr) < in_len) {
		((lzohdr *) out)->orig_size = in_len;
		memcpy((char *) out + sizeof(lzohdr), buf, out_len);
		return out_len + sizeof(lzohdr);
	} else {
		return 0;
	}
}

size_t MiniLZO::GetOrigSize(void *in, size_t in_len)
{
	assert(in_len >= sizeof(lzohdr));
	return ((lzohdr *) in)->orig_size;
}

int MiniLZO::Decompress(void *out, void *in, size_t in_len)
{
	assert(in != out);
	lzo_uint new_len = GetOrigSize(in, in_len);
	int r = lzo1x_decompress_safe((unsigned char *) in + sizeof(lzohdr), in_len - sizeof(lzohdr), (unsigned char *) out, &new_len, NULL);
	return r == LZO_E_OK && new_len == GetOrigSize(in, in_len);
}