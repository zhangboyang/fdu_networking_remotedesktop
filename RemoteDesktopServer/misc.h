#pragma once

#define MAXLINE 4096
#define fail(fmt, ...) __fail(strrchr("\\" __FILE__, '\\') + 1, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define plog(fmt, ...) printf("LOG: " fmt, ## __VA_ARGS__)
#define pmsg(fmt, ...) printf("MSG: " fmt, ## __VA_ARGS__)

__declspec(noreturn) extern void __fail(const char *file, int line, const char *func, const char *fmt, ...);
