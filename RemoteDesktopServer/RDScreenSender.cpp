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

	timer.Reset(1000 / SCRFRAME_FPS);
	int x = 0;
	int infoflag = 0;
	PeriodCounter fps;
	while (!exitflag) {
		timer.WaitNextFrame();
		SwapBuffer();
		scrcap.CaptureFrame(info, bits);
		if (!infoflag) {
			MsgPacket *pkt = new MsgPacket;
			pkt->AllocBuffer(sizeof(ScrPktHdr) + infosize, RDSERVICE_SCREENSENDER);
			ScrPktHdr *spkt = (ScrPktHdr *) pkt->LockBuffer();
			spkt->id = 0;
			spkt->type = SCRPKT_BITMAPINFO;
			memcpy(spkt->data, info, infosize);
			pkt->UnlockBuffer();
			sendqueue->Append(pkt);
			infoflag = 1;
		}
		int pktcnt = -1;
		size_t totbytes = 0;
		for (size_t p = 0; p < bitssize; p += SCRPKT_MAXDATA) {
			pktcnt++;
			int datasize = min(bitssize - p, SCRPKT_MAXDATA);
			if (memcmp((char *) old_bits + p, (char *) bits + p, datasize) == 0) continue;
			int pktsize = sizeof(ScrPktHdr) + datasize;
			MsgPacket *pkt = new MsgPacket;
			pkt->AllocBuffer(pktsize, RDSERVICE_SCREENSENDER);
			ScrPktHdr *spkt = (ScrPktHdr *) pkt->LockBuffer();
			spkt->id = pktcnt;
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
			totbytes += newdatasize;
		}
		while (totbytes < SCRFRAME_LOWLIMIT) {
			int datasize = min(SCRFRAME_LOWLIMIT - totbytes, SCRPKT_MAXDATA);
			int pktsize = sizeof(ScrPktHdr) + datasize;
			MsgPacket *pkt = new MsgPacket;
			pkt->AllocBuffer(pktsize, RDSERVICE_SCREENSENDER);
			ScrPktHdr *spkt = (ScrPktHdr *) pkt->LockBuffer();
			memset(spkt, 0, pktsize);
			spkt->id = 0;
			spkt->type = SCRPKT_PADDING;
			pkt->UnlockBuffer();
			sendqueue->Append(pkt);
			totbytes += datasize;
		}
		if (fps.IncreaseCount()) {
			plog("screen fps: %.1f\n", fps.GetCountsPerSecond());
		}
	}
	plog("screensender thread exited.\n");
}
void RDScreenSender::NotifyClose()
{
	exitflag = 1;
}