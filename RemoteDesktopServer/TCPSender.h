#pragma once

class TCPSender : public ThreadHelper {
	friend class TCPConnection;
private:
	TCPConnection *conn;
	ProducerConsumerQueueMux<MsgPacket *> sendqueue;
	virtual void ThreadProc();
	void NotifyClose();
	TCPSender(TCPConnection *conn);
	ProducerConsumerQueue<MsgPacket *> *GetServiceSendQueue(int id);
};