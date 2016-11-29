#include "stdafx.h"
#include "common.h"

TCPSender::TCPSender(TCPConnection *conn) : conn(conn), sendqueue(RDSERVICE_MAX, MAX_QUEUED_PACKET)
{
}

void TCPSender::ThreadProc()
{
	plog("sender thread created.\n");
	PeriodCounter pktcnt, bytecnt;
	while (1) {
		MsgPacket *packet;
		if (sendqueue.Get(&packet) < 0) break;
		const char *buf;
		size_t len;
		packet->GetRawData(&buf, &len);
		conn->SendAll(buf, len);
		delete packet;
		if (pktcnt.IncreseCount()) {
			plog("sender: packet rate at %f.\n", pktcnt.GetCountsPerSecond());
		}
		if (bytecnt.IncreseCount(len)) {
			plog("sender: data rate %f MB/s.\n", bytecnt.GetCountsPerSecond() / 1048576.0);
		}
	}
	conn->Close();
	plog("sender thread exited.\n");
}

void TCPSender::NotifyClose()
{
	sendqueue.NotifyClose();
}

ProducerConsumerQueue<MsgPacket *> *TCPSender::GetServiceSendQueue(int id)
{
	return sendqueue.GetIndividualQueue(id);
}