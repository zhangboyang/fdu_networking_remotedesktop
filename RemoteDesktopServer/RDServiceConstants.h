#pragma once

enum {
	// the order is define by priority
	// lower value means higher priority
	RDSERVICE_SCREENSENDER,

	RDSERVICE_MAX // EOF
};


#define MAX_MSGPACKETSIZE_HARD 16384 // datasize
#define MAX_MSGPACKETSIZE_SOFT 1300

#define MAX_QUEUED_PACKET 1000



enum {
	SCRPKT_BITMAPINFO,
	SCRPKT_BITMAPDATA,
	SCRPKT_BITMAPDATA_COMPRESSED,
	SCRPKT_PADDING,
};
struct ScrPktHdr {
	int type;
	int id;
	char data[0];
};
#define SCRPKT_MAXDATA (MAX_MSGPACKETSIZE_SOFT - sizeof(ScrPktHdr))
#define SCRFRAME_FPS 60
#define SCRFRAME_LOWLIMIT (1024 * 1024 * 1 / SCRFRAME_FPS)
