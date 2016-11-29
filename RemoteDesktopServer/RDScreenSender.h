#pragma once

class RDScreenSender : public RDService {
private:
	FrameTimer timer;
	int exitflag;
	ScreenCapturer scrcap;
	size_t infosize, bitssize;
	BITMAPINFO *info;
	void *bits;
	void *old_bits;
	void ResetDepth(int depth);
	struct ScrPktHdr {
		int type;
		int id;
		char data[0];
	};
	enum {
		SCRPKT_BITMAPINFO,
		SCRPKT_BITMAPDATA,
		SCRPKT_BITMAPDATA_COMPRESSED,
	};
	static const int SCRPKT_MAXDATA = (MAX_MSGPACKETSIZE_SOFT - sizeof(ScrPktHdr));
	void SwapBuffer();
public:
	RDScreenSender(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue);
	virtual void ThreadProc();
	virtual void NotifyClose();
	virtual ~RDScreenSender();
};
