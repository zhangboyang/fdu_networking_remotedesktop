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
	void SwapBuffer();
public:
	RDScreenSender(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn);
	virtual void ThreadProc();
	virtual void NotifyClose();
	virtual ~RDScreenSender();
};
