#pragma once

enum {
	// the order is define by priority
	// lower value means higher priority
	RDSERVICE_SCREENSENDER,
	RDSERVICE_CONTROLRECEIVER,
	RDSERVICE_FILETRANSFER,

	RDSERVICE_MAX, // EOF

	RDSERVICE_PASSWORD = -1, // the is a special packet type
};


#define MAX_MSGPACKETSIZE_HARD 16384 // datasize
#define MAX_MSGPACKETSIZE_SOFT 4000

#define MAX_QUEUED_PACKET 10



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
#define SCRFRAME_FPS 30
//#define SCRFRAME_LOWLIMIT (1024 * 1024 * 1 / SCRFRAME_FPS)
#define SCRFRAME_LOWLIMIT 0




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
	int state;
	int len;
	char data[0];
};





enum {
	SEND_REQUEST,
	SEND_RESPONSE,
	SEND_DATA,
	TRANSFER_CANCEL,
	DOWNLOAD_REQUEST,
};
struct FileTransHdr {
	int type;
	unsigned value;
	int len;
	char data[0];
};
#define FILETRANS_MAXDATA (MAX_MSGPACKETSIZE_SOFT - sizeof(FileTransHdr))
