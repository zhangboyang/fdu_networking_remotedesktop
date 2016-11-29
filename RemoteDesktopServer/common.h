#pragma once

#define MAXLINEFMT "%" TOSTR(MAXLINE) "s"

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

// TCPKit
#include "MsgPacket.h"
#include "TCPServer.h"
#include "TCPConnection.h"
#include "TCPSender.h"
#include "TCPReceiver.h"