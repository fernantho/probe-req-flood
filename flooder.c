/*
 *
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netlink/genl/genl.h>

#include "flooder.h"

int debug_level;

static uint32_t port_bitmap[32] = {0};

static void handle_destroy(struct nl_handle *handle){
  uint32_t port = nl_socket_get_local_port(handle);
  
  port >>= 22;
  port_bitmap[port / 32] &= ~(1 << (port % 32));
  
  nl_handle_destroy(handle);

}

static struct nl_handle *gen_handle(struct nl_cb* cb){
  struct nl_handle *handle;
  uint32_t pid = getpid() & 0x3FFFFF;
  int i;
  
  handle = nl_handle_alloc_cb(cb);
  
  for (i = 0; i < 1024; i++) {
    if (port_bitmap[i / 32] & (1 << (i % 32)))
      continue;
    port_bitmap[i / 32] |= 1 << (i % 32);
    pid += i << 22;
    break;
  }
  
  nl_socket_set_local_port(handle, pid);
  
  if (!handle)
    return handle;
  
  if(genl_connect(handle))
    handle_destroy(handle);
  
  handle = NULL;

  return handle;
}

static struct nl_cb *gen_cb(){
  struct nl_cb *ret = nl_cb_alloc(NL_CB_DEFAULT);
  return ret;
}

static struct nl_msg *gen_msg(flooder_param *params){
  struct nl_msg *msg, *ssids, *freqs, *rates;
  
  msg  = nlmsg_alloc();
  
  return msg;
}

static send_and_recv(){


  
}

static void send_one_probe_request(struct nl_handle* handle, struct nl_msg* msg, struct nl_cb* cb){
  printf("send one\n");
  
  
  
}

int probe_req_flood(flooder_param *params){
  struct nl_cb *cb = gen_cb();
  struct nl_handle *handle = gen_handle(cb);
  struct nl_msg *msg = gen_msg(params);
  if (msg == NULL || cb == NULL || handle == NULL)
    return -1;

  
  
  int i = 0;
  while(params->times == -1 || i < params->times){
    send_one_probe_request(handle, msg, cb);
    i++;
  }

  destroy_handle(handle);
  
  
  return 0;
}

void flooder_log(int level, const char*fmt, ...){
  static char* level_map[] = {
    "[DUMP]    ",
    "[DEBUG]   ",
    "[INFO]    ",
    "[WARN]    ",
    "[ERROR]   "
  };

  va_list ap;
  va_start(ap, fmt);
  printf(level_map[level]);
  if (level >= debug_level)
    vprintf(fmt, ap);
  va_end(ap);
}
