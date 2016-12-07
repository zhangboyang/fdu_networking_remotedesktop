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
		if (!packet) break; // a null pointer means processor thread request quit
		const char *buf;
		size_t len;
		packet->GetRawData(&buf, &len);
		assert(*(size_t*)buf + sizeof(MsgPacket::raw_header) == len);
		conn->SendAll(buf, len);
		delete packet;
		if (pktcnt.IncreaseCount()) {
			plog("sender: packet rate at %f.\n", pktcnt.GetCountsPerSecond());
		}
		if (bytecnt.IncreaseCount(len)) {
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