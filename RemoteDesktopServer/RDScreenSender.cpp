#include "stdafx.h"
#include "common.h"

RDScreenSender::RDScreenSender(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue) : 
	RDService(recvqueue, sendqueue),
	exitflag(0),
	info(NULL),
	bits(NULL),
	old_bits(NULL),
	infosize(0),
	bitssize(0)
{
}
RDScreenSender::~RDScreenSender()
{
	if (info) free(info);
	if (bits) free(bits);
	if (old_bits) free(old_bits);
}
void RDScreenSender::ResetDepth(int depth)
{
	scrcap.SetDepth(depth);
	scrcap.GetBufferSize(&infosize, &bitssize);
	info = (BITMAPINFO *) realloc(info, infosize);
	bits = realloc(bits, bitssize);
	old_bits = realloc(old_bits, bitssize);
}
void RDScreenSender::SwapBuffer()
{
	std::swap(bits, old_bits);
}
void RDScreenSender::ThreadProc()
{
	plog("screensender thread started.\n");

	recvqueue->SetAppendDisable(true);
	ResetDepth(16);

	timer.Reset(33);
	int x = 0;
	PeriodCounter fps;
	while (!exitflag) {
		timer.WaitNextFrame();
		SwapBuffer();
		scrcap.CaptureFrame(info, bits);
		int pktcnt = 0;
		for (size_t p = 0; p < bitssize; p += SCRPKT_MAXDATA) {
			int datasize = min(bitssize - p, SCRPKT_MAXDATA);
			if (memcmp((char *) old_bits + p, (char *) bits + p, datasize) == 0) continue;
			int pktsize = sizeof(ScrPktHdr) + datasize;
			MsgPacket *pkt = new MsgPacket;
			pkt->AllocBuffer(pktsize, 0);
			ScrPktHdr *spkt = (ScrPktHdr *) pkt->LockBuffer();
			spkt->id = pktcnt++;
			int newdatasize = MiniLZO::Instance()->Compress(spkt->data, (char *) bits + p, datasize);
			if (newdatasize) {
				spkt->type = SCRPKT_BITMAPDATA_COMPRESSED;
			} else {
				spkt->type = SCRPKT_BITMAPDATA;
				memcpy(spkt->data, (char *) bits + p, datasize);
				newdatasize = datasize;
			}
			pkt->UnlockBuffer();
			pkt->TruncateBuffer(sizeof(ScrPktHdr) + newdatasize);
			sendqueue->Append(pkt);
		}
		if (fps.IncreseCount()) {
			plog("screen fps: %.1f\n", fps.GetCountsPerSecond());
		}
	}
	plog("screensender thread exited.\n");
}
void RDScreenSender::NotifyClose()
{
	exitflag = 1;
}