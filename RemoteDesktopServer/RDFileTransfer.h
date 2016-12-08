#pragma once

class RDFileTransfer : public RDService {
	void SendPacket(FileTransHdr *pkt, size_t len);
	bool FileDialog(char *buf, size_t buf_size, int type);
	bool CheckCancelPacketInRecvQueue();
	bool CheckCancelPacket(MsgPacket *packet);
	void RecvFileDoModal(FileTransHdr *pkt, size_t len);
	void SendFileDoModal();
	void DispatchPacket(FileTransHdr *pkt, size_t len);
public:
	RDFileTransfer(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn);
	virtual void ThreadProc();
	virtual void NotifyClose();
	virtual ~RDFileTransfer();
};