#pragma once

#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)

#define MAXLINE 4096
#define MAXLINEFMT "%" TOSTR(MAXLINE) "s"


#define plog(fmt, ...) printf("LOG: " fmt, ## __VA_ARGS__)
#define pmsg(fmt, ...) printf("MSG: " fmt, ## __VA_ARGS__)


#define fail(fmt, ...) __fail(strrchr("\\" __FILE__, '\\') + 1, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
static inline void __fail(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[MAXLINE];
    int len;
    _snprintf(buf, sizeof(buf), "FILE: %s\nLINE: %d\nFUNC: %s\n\n", file, line, func);
    len = strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
    MessageBoxA(NULL, buf, NULL, MB_ICONERROR);
    TerminateProcess(GetCurrentProcess(), 1);
    va_end(ap);
}

#include "RDServiceConstants.h"
#include "PeriodCounter.h"
#include "FrameTimer.h"
#include "CMiniLZO.h"
