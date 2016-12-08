#include "stdafx.h"
#include "common.h"

RDFileTransfer::RDFileTransfer(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn) : 
	RDService(recvqueue, sendqueue, conn)
{
}

RDFileTransfer::~RDFileTransfer()
{
}

void RDFileTransfer::NotifyClose()
{
	PostMessage(WM_QUIT, 0, 0);
}

bool RDFileTransfer::CheckCancelPacket(MsgPacket *packet)
{
	FileTransHdr *pkt = (FileTransHdr *) packet->LockBuffer();
	bool ret = pkt->type == TRANSFER_CANCEL;
	packet->UnlockBuffer();
	return ret;
}
bool RDFileTransfer::CheckCancelPacketInRecvQueue()
{
	// there is only one consumer here
	// no need to lock
	MsgPacket *packet;
	if (recvqueue->PeekWithoutRemove(&packet) < 0) {
		return false;
	}
	FileTransHdr *pkt = (FileTransHdr *) packet->LockBuffer();
	if (pkt->type == TRANSFER_CANCEL) {
		packet->UnlockBuffer();
		if (recvqueue->Get(&packet) == 0) {
			delete packet;
		} else {
			assert(0);
		}
		plog("file transfer cancelled.\n");
		return true;
	} else {
		packet->UnlockBuffer();
		return false;
	}
}

void RDFileTransfer::SendPacket(FileTransHdr *pkt, size_t len)
{
	MsgPacket *packet = new MsgPacket();
	packet->LoadData((char *) pkt, len, RDSERVICE_FILETRANSFER);
	sendqueue->Append(packet);
}

bool RDFileTransfer::FileDialog(char *buf, size_t buf_size, int type)
{
	// type = 0 for save (buf should contain file name)
	//        1 for open
	assert(buf_size > 0);
	if (type) buf[0] = 0;
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = buf_size;
	ofn.lpstrFilter = "所有文件 (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	if (type) {
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		return !!GetOpenFileName(&ofn);
	} else {
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		return !!GetSaveFileName(&ofn);
	}
}
void RDFileTransfer::RecvFileDoModal(FileTransHdr *initpkt, size_t len)
{
	if (initpkt->len + sizeof(FileTransHdr) != len) {
		plog("invalid length in file info packet.\n");
		RequestExit();
		return;
	}

	size_t filelen = initpkt->value;

	char filename[MAXLINE]; // full path

	// get filename from packet
	strncpy(filename, initpkt->data, initpkt->len);
	filename[min(initpkt->len, sizeof(filename) - 1)] = 0;
	char *pname = strrchr(filename, '\\');
	if (pname) { pname++; memmove(filename, pname, strlen(pname) + 1); }

	// prompt user
	FILE *fp;
	if (!FileDialog(filename, sizeof(filename), 0) || !(fp = fopen(filename, "wb"))) {
		plog("user cancelled file receiving.\n");

		FileTransHdr rejectpkt;
		rejectpkt.type = SEND_RESPONSE;
		rejectpkt.value = 0; // reject
		rejectpkt.len = 0;
		SendPacket(&rejectpkt, sizeof(rejectpkt));
		return;
	}

	// send confirm packet
	FileTransHdr confirmpkt;
	confirmpkt.type = SEND_RESPONSE;
	confirmpkt.value = 1; // confirm
	confirmpkt.len = 0;
	SendPacket(&confirmpkt, sizeof(confirmpkt));

	// start recv file
	plog("filename is %s\n", filename);
	size_t recvlen = 0;
	MsgPacket *packet = NULL;
	while (1) {
		if (recvqueue->Get(&packet) < 0) {
			pmsg("transfer cancelled.\n");
			packet = NULL;
			break;
		}
		if (CheckCancelPacket(packet)) {
			fclose(fp); fp = NULL;
			_unlink(filename);
			break;
		}
		FileTransHdr *pkt = (FileTransHdr *) packet->LockBuffer();
		if (pkt->type != SEND_DATA) {
			plog("invalid packet type %d during file receiving, ignored\n", pkt->type);
		} else if (pkt->len + sizeof(FileTransHdr) != packet->GetBufferSize()) {
			plog("invalid packet length, ignored.\n");
		} else {
			size_t r = fwrite(pkt->data, 1, pkt->len, fp);
			if (r == 0 && pkt->len > 0) {
				pmsg("error during file writing, data might be lost.\n");
			} else {
				recvlen += r;
				pmsg("data received %.2f%% (%d/%d)\n", (double) recvlen / filelen, (int) recvlen, (int) filelen);
			}
		}
		packet->UnlockBuffer();

		delete packet; packet = NULL;

		if (recvlen >= filelen) {
			pmsg("file receive OK!\n");
			break;
		}
	}
	if (packet) delete packet;
	if (fp) fclose(fp);
}

void RDFileTransfer::SendFileDoModal()
{
	char filename[MAXLINE];

	// prompt user
	FILE *fp;
	if (!FileDialog(filename, sizeof(filename), 1) || !(fp = fopen(filename, "rb"))) {
		plog("file select cancelled.\n");
		return;
	}
	char *pname = strrchr(filename, '\\');
	if (pname) pname++; else pname = filename;

	// get file length
	size_t filelen;
	fseek(fp, SEEK_SET, SEEK_END);
	filelen = ftell(fp);
	rewind(fp);

	// send request packet
	size_t reqpktlen = sizeof(FileTransHdr) + strlen(pname);
	FileTransHdr *reqpkt = (FileTransHdr *) malloc(reqpktlen);
	reqpkt->type = SEND_REQUEST;
	reqpkt->value = filelen;
	reqpkt->len = strlen(pname);
	memcpy(reqpkt->data, pname, strlen(pname));
	SendPacket(reqpkt, reqpktlen);
	free(reqpkt); reqpkt = NULL;

	// wait for resonse packet
	MsgPacket *packet = NULL;
	while (1) {
		if (recvqueue->Get(&packet) < 0) { packet = NULL; goto done; }
		FileTransHdr *pkt = (FileTransHdr *) packet->LockBuffer();
		if (pkt->type != SEND_RESPONSE) {
			plog("ignored invalid packet when waiting for response.\n");
		} else {
			if (!pkt->value) {
				pmsg("remote rejected.\n");
				packet->UnlockBuffer();
				goto done;
			} else {
				packet->UnlockBuffer();
				break;
			}
		}
		packet->UnlockBuffer();
		delete packet; packet = NULL;
	}
	if (packet) { delete packet; packet = NULL; }

	size_t sentlen = 0;
	// send file data
	while (1) {
		if (CheckCancelPacketInRecvQueue()) {
			plog("send operation cancelled.\n");
			break;
		}
		char data[FILETRANS_MAXDATA];
		size_t datalen = fread(data, 1, sizeof(data), fp);
		if (datalen == 0) {
			plog("can't read data from file.\n");
			break;
		}
		size_t pktlen = sizeof(FileTransHdr) + datalen;
		FileTransHdr *pkt = (FileTransHdr *) malloc(pktlen);
		pkt->type = SEND_DATA;
		pkt->value = 0;
		pkt->len = datalen;
		memcpy(pkt->data, data, datalen);
		SendPacket(pkt, pktlen);
		free(pkt);
		sentlen += datalen;
		pmsg("data sent %.2f%% (%d/%d)\n", (double) sentlen / filelen, (int) sentlen, (int) filelen);
		if (feof(fp)) {
			assert(sentlen == filelen);
			plog("file send OK!\n");
			break;
		}
	}
done:
	if (packet) delete packet;
	if (fp) fclose(fp);
}

void RDFileTransfer::DispatchPacket(FileTransHdr *pkt, size_t len)
{
	switch (pkt->type) {
		case SEND_REQUEST:
			plog("enter recv mode.\n");
			RecvFileDoModal(pkt, len);
			break;
		case DOWNLOAD_REQUEST:
			plog("enter send mode.\n");
			SendFileDoModal();
			break;
		default:
			plog("invalid first file transfer packet %d, ignored.\n", pkt->type);
			break;
	}
}

void RDFileTransfer::ThreadProc()
{
	while (1) {
		MsgPacket *packet;
		if (recvqueue->Get(&packet) < 0) break;
		FileTransHdr *pkt = (FileTransHdr *) packet->LockBuffer();
		DispatchPacket(pkt, packet->GetBufferSize());
		packet->UnlockBuffer();
		delete packet;
	}
}