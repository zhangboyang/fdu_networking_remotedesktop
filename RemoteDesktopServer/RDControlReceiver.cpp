#include "stdafx.h"
#include "common.h"

RDControlReceiver::RDControlReceiver(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn) :
	RDService(recvqueue, sendqueue, conn)
{
}
RDControlReceiver::~RDControlReceiver()
{
}


// should verify if data is valid
void RDControlReceiver::HandleMousePacket(struct CtrlPktHdr *pkt)
{
	int *pint;
	MOUSEINPUT mi;
	memset(&mi, 0, sizeof(mi));
	switch (pkt->type) {
		case CTRLPKT_MOUSE_MOVE:
			mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_WHEEL;
			if (pkt->len != 2 * sizeof(int)) {
				RequestExit();
				return;
			}
			pint = (int *)pkt->data;
			mi.dx = pint[0];
			mi.dy = pint[1];
			break;
		case CTRLPKT_MOUSE_LEFT: mi.dwFlags = pkt->state ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP; goto mousekey;
		case CTRLPKT_MOUSE_RIGHT: mi.dwFlags = pkt->state ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP; goto mousekey;
		case CTRLPKT_MOUSE_MID: mi.dwFlags = pkt->state ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP; goto mousekey;
		mousekey:
			plog("mousekey: type=%d state=%d\n", pkt->type, pkt->state);
			break;
		default: assert(0);
	}
	INPUT e;
	e.type = INPUT_MOUSE;
	e.mi = mi;
	SendInput(1, &e, sizeof(INPUT));
}

void RDControlReceiver::HandleKeyboardPacket(struct CtrlPktHdr *pkt)
{
	std::vector<INPUT> elist;
	for (int r = 0; r <= 1; r++) {
		DWORD flag = !r ? 0 : KEYEVENTF_KEYUP;
		for (int i = 0; i < pkt->len; i++) {
			UINT v = pkt->data[i];
			KEYBDINPUT ki;
			memset(&ki, 0, sizeof(ki));
			ki.wVk = v;
			ki.wScan = MapVirtualKey(v, MAPVK_VK_TO_VSC);
			ki.dwFlags = flag;
			plog("VK=%d SCAN=%d\n", (int) ki.wVk, (int) ki.wScan);
			INPUT e;
			e.type = INPUT_KEYBOARD;
			e.ki = ki;
			elist.push_back(e);
		}
	}
	plog("kbd input: count=%d\n", (int)elist.size());
	SendInput(elist.size(), &elist[0], sizeof(INPUT));
}
void RDControlReceiver::DispatchPacket(struct CtrlPktHdr *pkt, size_t len)
{
	if (len < sizeof(CtrlPktHdr) || len != sizeof(CtrlPktHdr) + pkt->len) {
		plog("invalid ctrl pkt len %d\n", (int) len);
		RequestExit();
		return;
	}
	switch (pkt->type) {
		case CTRLPKT_PADDING:
			break;
		case CTRLPKT_MOUSE_MOVE:
		case CTRLPKT_MOUSE_LEFT:
		case CTRLPKT_MOUSE_MID:
		case CTRLPKT_MOUSE_RIGHT:
			HandleMousePacket(pkt);
			break;
		case CTRLPKT_KEYBOARD:
			HandleKeyboardPacket(pkt);
			break;
		default:
			plog("invalid ctrl pkt type %d\n", pkt->type);
			RequestExit();
			return;
	}
}

void RDControlReceiver::ThreadProc()
{
	while (1) {
		MsgPacket *packet;
		if (recvqueue->Get(&packet) < 0) break;
		CtrlPktHdr *pkt = (CtrlPktHdr *) packet->LockBuffer();
		DispatchPacket(pkt, packet->GetBufferSize());
		packet->UnlockBuffer();
		delete packet;
	}
}

void RDControlReceiver::NotifyClose()
{
	recvqueue->NotifyClose();
}