#include "stdafx.h"
#include "common.h"

MsgPacket::MsgPacket() : raw_length(0), raw_data(NULL), locked(0)
{
}
MsgPacket::~MsgPacket()
{
	assert(!locked);
	FreeBuffer();
}
void MsgPacket::Dump()
{
	if (raw_length == 0 || raw_data == NULL) {
		plog("no buffer allocated.\n");
	} else {
		plog("raw length: %d\n", (int) raw_length);
		char str[16 * 3 + 1];
		char *ptr = raw_data;
		while ((unsigned) (ptr - raw_data) < raw_length) {
			for (int i = 0; i < 16; i++) {
				if (unsigned (ptr + i - raw_data) >= raw_length) break;
				sprintf(str + i * 3, "%02X ", (unsigned)(unsigned char)ptr[i]);
			}
			ptr += 16;
			plog(" %s\n", str);
		}
	}
}
void MsgPacket::GetRawData(const char **pbuf, size_t *len)
{
	assert(!locked);
	*pbuf = raw_data;
	*len = raw_length;
}

void MsgPacket::FreeBuffer()
{
	assert(!locked);
	if (raw_data) {
		free(raw_data);
		raw_length = 0;
		raw_data = NULL;
	}
}
void MsgPacket::AllocBuffer(size_t len, int type)
{
	assert(!locked);
	raw_length = len + sizeof(raw_header);
	raw_data = (char *) realloc(raw_data, raw_length);
	raw_header hdr = { len, type };
	*(raw_header *)raw_data = hdr;
}
char *MsgPacket::LockBuffer()
{
	assert(!locked);
	locked = 1;
	return raw_data + sizeof(raw_header);
}
void MsgPacket::UnlockBuffer()
{
	assert(locked);
	locked = 0;
}
void MsgPacket::TruncateBuffer(size_t new_size)
{
	size_t new_raw_length = new_size + sizeof(raw_header);
	assert(new_raw_length <= raw_length);
	raw_length = new_raw_length;
	((raw_header *)raw_data)->data_length = new_size;
}

int MsgPacket::LoadHeader(const raw_header *phdr)
{
	if (phdr->data_length > MAX_MSGPACKETSIZE_HARD) return -1;
	AllocBuffer(phdr->data_length, phdr->type);
	return 0;
}

size_t MsgPacket::GetBufferSize()
{
	assert(raw_length >= sizeof(raw_header));
	return raw_length - sizeof(raw_header);
}

int MsgPacket::GetType()
{
	return ((raw_header *)raw_data)->type;
}

void MsgPacket::LoadData(const char *data, size_t len, int type)
{
	AllocBuffer(len, type);
	char *pbuf = LockBuffer();
	memcpy(pbuf, data, len);
	UnlockBuffer();
}