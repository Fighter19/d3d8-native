#pragma once

#include <stdio.h>
#include <signal.h>

#define ENABLE_DEBUG_OUTPUT 0

#define WINE_DECLARE_DEBUG_CHANNEL(channel) \
    static const char *__wine_debug_channel_##channel = #channel;

#define WINE_DEFAULT_DEBUG_CHANNEL(channel) \
    static const char *__wine_debug_channel = #channel;

#if ENABLE_DEBUG_OUTPUT
#define WARN_(channel) \
    printf("%s: ", #channel); \
    printf

#define WARN(fmt, ...) \
    do { \
        fprintf(stderr, "%s: ", __wine_debug_channel); \
        fprintf(stderr, "WARNING in %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define ERR(fmt, ...) \
    do { \
        fprintf(stderr, "%s: ", __wine_debug_channel); \
        fprintf(stderr, "ERROR in %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define ERR_(channel) \
    printf("%s: ", #channel); \
    printf

#define TRACE(fmt, ...) \
    do { \
        fprintf(stderr, "%s: ", __wine_debug_channel); \
        fprintf(stderr, "TRACE in %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define TRACE_(channel) \
    printf("%s: ", #channel); \
    printf

#define FIXME(fmt, ...) \
    do { \
        fprintf(stderr, "%s: ", __wine_debug_channel); \
        fprintf(stderr, "FIXME in %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define FIXME_(channel) \
    printf("%s: ", #channel); \
    printf

#else
#define WARN(fmt, ...) do { } while (0)
#define WARN_(channel) WARN
#define ERR(fmt, ...) do { } while (0)
#define ERR_(channel) ERR
#define TRACE(fmt, ...) do { } while (0)
#define TRACE_(channel) TRACE
#define FIXME(fmt, ...) do { } while (0)
#define FIXME_(channel) FIXME
#endif

static const char *debugstr_guid(const void *guid)
{
  static char buffer[39];
  const unsigned char *b = (const unsigned char *)guid;

  snprintf(buffer, sizeof(buffer),
           "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
           *(const unsigned int *)(b),
           *(const unsigned short *)(b + 4),
           *(const unsigned short *)(b + 6),
           b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
  return buffer;
}

static const char *wine_dbgstr_longlong(long long value)
{
  static char buffer[32];
  snprintf(buffer, sizeof(buffer), "%lld", value);
  return buffer;
}

static const char *wine_dbg_sprintf(const char *fmt, ...)
{
  static char buffer[512];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return buffer;
}

#define __WINE_IS_DEBUG_ON(channel_level, channel) (0)
#define FIXME_ON(channel) (0)
#define DEBUG_ON(channel) (0)
#define TRACE_ON(channel) (0)
#define WARN_ON(channel) (0)
#define ERR_ON(channel) (0)

static inline void DebugBreak(void)
{
  raise(SIGTRAP);
}

static inline void __wine_dbg_output(const char *str)
{
  fputs(str, stdout);
}