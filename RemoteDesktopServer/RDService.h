#pragma once

class MsgPacket;

class RDService : public ThreadHelper {
public:
	ProducerConsumerQueue<MsgPacket *> *recvqueue, *sendqueue;
	RDService(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue);
	virtual ~RDService();
	virtual void NotifyClose() = 0;
};

enum {
	// the order is define by priority
	// lower value means higher priority
	RDSERVICE_SCREENSENDER,

	RDSERVICE_MAX // EOF
};

#define MAX_MSGPACKETSIZE_HARD 16384 // datasize
#define MAX_MSGPACKETSIZE_SOFT 1300

#define MAX_QUEUED_PACKET 1000