#ifndef _FLOODER_H_
#define _FLOODER_H_

#ifdef __GNUC__
#define PRINTF_FORMAT(a,b) __attribute__ ((format (printf, (a), (b))))
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define PRINTF_FORMAT(a,b)
#define STRUCT_PACKED
#endif

#define ETH_ALEN 6

typedef unsigned char u8;

typedef struct{
  unsigned iface;
  int channel;
  int times;
  u8 addr[ETH_ALEN];
}flooder_param;

enum{
  FLOODER_DUMP, FLOODER_DEBUG, FLOODER_INFO, FLOODER_WARNING, FLOODER_ERROR
};

extern int debug_level;

int probe_req_flood(flooder_param *params);

void flooder_log(int level, const char* format, ...)
PRINTF_FORMAT(2, 3);

#endif
