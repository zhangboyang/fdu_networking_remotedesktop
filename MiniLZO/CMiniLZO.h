#pragma once

class MiniLZO {
public:
	static MiniLZO *Instance();
	size_t Compress(void *out, void *in, size_t in_len);

	size_t GetOrigSize(void *in, size_t in_len);
	int Decompress(void *out, void *in, size_t in_len);
private:
	void AllocBuffer(size_t len);
	void *buf;
	size_t buf_len;

	struct lzohdr {
		size_t orig_size;
	};
	MiniLZO();
	~MiniLZO();
	MiniLZO(MiniLZO const &);
	MiniLZO& operator= (MiniLZO const &);
};