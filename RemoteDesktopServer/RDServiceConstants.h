#pragma once

enum {
	// the order is define by priority
	// lower value means higher priority
	RDSERVICE_SCREENSENDER,
	RDSERVICE_CONTROLRECEIVER,

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




enum {
	CTRLPKT_PADDING,
	CTRLPKT_MOUSE_MOVE,
	CTRLPKT_MOUSE_LEFT,
	CTRLPKT_MOUSE_MID,
	CTRLPKT_MOUSE_RIGHT,
	CTRLPKT_KEYBOARD,
};
struct CtrlPktHdr {
	int type;
	union {
		struct { // mouse
			int keystate;
			int len; // data bytes
			int	data[0];
		} mouse;
		struct { // keyboard
			int ignored;
			int len;
			char data[0];
		} kbd;
	};
};
