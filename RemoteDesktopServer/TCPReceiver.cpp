#include "stdafx.h"
#include "common.h"

TCPReceiver::TCPReceiver(TCPConnection *conn) : conn(conn), recvqueue(RDSERVICE_MAX, MAX_QUEUED_PACKET)
{
}

bool TCPReceiver::DoAuth()
{
	plog("wait for password ...\n");
	MsgPacket *pkt = RecvPacket();
	if (!pkt) return false;
	char *clipsw = pkt->LockBuffer();
	bool result = pkt->GetBufferSize() == strlen(rdpsw) && strncmp(clipsw, rdpsw, strlen(rdpsw)) == 0;
	pkt->UnlockBuffer();
	delete pkt;
	if (!result) {
		plog("wrong password, closing.\n");
	}
	return result;
}

MsgPacket *TCPReceiver::RecvPacket()
{
	MsgPacket *pkt = NULL, *ret = NULL;
	MsgPacket::raw_header pkthdr;

	int r = conn->RecvAll((char *)&pkthdr, sizeof(MsgPacket::raw_header));
	if (r < 0) goto done;

	// receive data
	pkt = new MsgPacket;
	if (pkt->LoadHeader(&pkthdr) < 0) {
		plog("invalid packet header.\n");
		goto done;
	}

	char *buf = pkt->LockBuffer();
	r = conn->RecvAll(buf, pkt->GetBufferSize());
	pkt->UnlockBuffer();
	
	if (r < 0) goto done;
	ret = pkt;

done:
	if (ret) return ret;
	if (pkt) delete pkt;
	return NULL;
}

void TCPReceiver::ThreadProc()
{
	plog("receiver thread created.\n");
	MsgPacket *pkt = NULL;

	while (1) {
		pkt = RecvPacket();
		if (!pkt) break;

		int type = pkt->GetType();

		// dispatch packet
		if (type < 0 || type >= RDSERVICE_MAX) {
			plog("invalid packet type %d.\n", type);
			break;
		}
		recvqueue.GetIndividualQueue(type)->Append(pkt);

		// set pkt to NULL, since we dispatched the packet
		pkt = NULL;
	}

	if (pkt) delete pkt;
	conn->Close();
	plog("receiver thread exited.\n");
}

void TCPReceiver::NotifyClose()
{
	recvqueue.NotifyClose();
}

ProducerConsumerQueue<MsgPacket *> *TCPReceiver::GetServiceRecvQueue(int id)
{
	return recvqueue.GetIndividualQueue(id);
}