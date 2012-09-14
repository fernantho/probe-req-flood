#ifndef _FLOODER_H_
#define _FLOODER_H_

typedef struct{
  char* iface;
  int channel;
  int times;
}flooder_param;

int probe_req_flood(flooder_param *params);

#endif
