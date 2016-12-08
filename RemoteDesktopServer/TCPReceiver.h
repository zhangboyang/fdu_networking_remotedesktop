#pragma once

class TCPReceiver : public ThreadHelper {
	friend class TCPConnection;
private:
	TCPConnection *conn;
	ProducerConsumerQueueMux<MsgPacket *> recvqueue;
	virtual void ThreadProc();
	void NotifyClose();
	TCPReceiver(TCPConnection *conn);
	ProducerConsumerQueue<MsgPacket *> *GetServiceRecvQueue(int id);
	bool DoAuth();
	MsgPacket *RecvPacket();
};