#pragma once

class RDControlReceiver : public RDService {
	void HandleMousePacket(struct CtrlPktHdr *pkt);
	void HandleKeyboardPacket(struct CtrlPktHdr *pkt);
	void DispatchPacket(struct CtrlPktHdr *pkt, size_t len);
public:
	RDControlReceiver(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn);
	virtual void ThreadProc();
	virtual void NotifyClose();
	virtual ~RDControlReceiver();
};