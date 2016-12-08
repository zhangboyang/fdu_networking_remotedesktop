#pragma once

class MsgPacket {
private:
	
	size_t raw_length;
	char *raw_data;
	int locked;
public:
	MsgPacket();
	~MsgPacket();
	struct raw_header {
		size_t data_length;
		int type;
	};
	void Dump();
	int LoadHeader(const raw_header *phdr);
	void LoadData(const char *data, size_t len, int type);
	void GetRawData(const char **pbuf, size_t *len);
	void FreeBuffer();
	void AllocBuffer(size_t len, int type);
	size_t GetBufferSize();
	char *LockBuffer();
	void UnlockBuffer();
	void TruncateBuffer(size_t new_len);
	int GetType();
};