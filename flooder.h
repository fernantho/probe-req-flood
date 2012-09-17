#ifndef _FLOODER_H_
#define _FLOODER_H_

#ifdef __GNUC__
#define PRINTF_FORMAT(a,b) __attribute__ ((format (printf, (a), (b))))
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define PRINTF_FORMAT(a,b)
#define STRUCT_PACKED
#endif

typedef struct{
  char* iface;
  int channel;
  int times;
}flooder_param;

enum{
  FLOODER_DUMP, FLOODER_DEBUG, FLOODER_INFO, FLOODER_WARNING, FLOODER_ERROR
};

extern int debug_level;

int probe_req_flood(flooder_param *params);

void flooder_log(int level, const char* format, ...)
PRINTF_FORMAT(2, 3);

#endif
