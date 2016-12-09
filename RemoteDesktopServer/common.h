#pragma once

#define MAXLINEFMT "%" TOSTR(MAXLINE) "s"

extern char rdpsw[];

#include "misc.h"

#include "CMiniLZO.h"

// UtilityKit
#include "ThreadHelper.h"
#include "ProducerConsumerQueue.h"
#include "FrameTimer.h"
#include "PeriodCounter.h"
#include "ScreenCapturer.h"

// RemoteDesktopKit
#include "RDServiceConstants.h"
#include "RDService.h"
#include "RDServiceFactory.h"
#include "RDScreenSender.h"
#include "RDControlReceiver.h"
#include "RDFileTransfer.h"

// TCPKit
#include "MsgPacket.h"
#include "TCPServer.h"
#include "TCPConnection.h"
#include "TCPSender.h"
#include "TCPReceiver.h"