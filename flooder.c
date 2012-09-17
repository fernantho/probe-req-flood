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
#include <net/if.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

#include "nl80211_copy.h"
#include "flooder.h"

int debug_level;

static uint32_t port_bitmap[32] = {0};
static int handle_id;


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
  
  if (!handle){
    flooder_log(FLOODER_DEBUG, "Failed to allocate a handle");
    return handle;
  }
  
  if(genl_connect(handle)){
    flooder_log(FLOODER_DEBUG, "Cannot connect to handle");
    handle_destroy(handle);
    return NULL;
  }  

  if ((handle_id = genl_ctrl_resolve(handle, "nl80211")) < 0){
    flooder_log(FLOODER_DEBUG, "Cannot resolve nl80211");
    handle_destroy(handle);
    return NULL;
  }

  return handle;
}

static struct nl_cb *gen_cb(){
  struct nl_cb *ret = nl_cb_alloc(NL_CB_DEFAULT);
  if(!ret)
    flooder_log(FLOODER_DEBUG, "Failed to allocate a callback");
  return ret;
}

static unsigned getFreq(int channel){
  return 2407 + channel * 5;
}

static struct nl_msg *gen_msg(flooder_param *params){
  struct nl_msg *msg, *freqs;
  
  msg  = nlmsg_alloc();
  freqs = nlmsg_alloc();

  if (!msg || !freqs){
    flooder_log(FLOODER_DEBUG, "Failed to allocate a netlink message");
    if(msg)
      nlmsg_free(msg);
    if(freqs)
      nlmsg_free(freqs);
    return NULL;
  }

  genlmsg_put(msg, 0, 0, handle_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);
  NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, params->iface);

  NLA_PUT_U32(freqs, 1, getFreq(params->channel));
  nla_put_nested(msg, NL80211_ATTR_SCAN_FREQUENCIES, freqs);

  return msg;

 nla_put_failure:
  nlmsg_free(msg);
  nlmsg_free(freqs);
  return NULL;
}

static int send_and_recv(struct nl_handle* handle, struct nl_msg* msg, struct nl_cb* cb){
  int err = -1;
  struct nl_cb *tmp_cb;
  tmp_cb = nl_cb_clone(cb);
  if (!cb)
    goto out;
  
  err = nl_send_auto_complete(handle, msg);

 out:
  return err;
}

static void send_one_probe_request(struct nl_handle* handle, struct nl_msg* msg, struct nl_cb* cb){
  flooder_log(FLOODER_DEBUG, "Send One Probe Request");
  int ret = send_and_recv(handle, msg, cb);
  if (ret)
    flooder_log(FLOODER_DEBUG, "Sending Failed because %d", ret);
  else
    flooder_log(FLOODER_DEBUG, "Sending Finished");
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

  handle_destroy(handle);  
  
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
  if (level >= debug_level){
    printf(level_map[level]);
    vprintf(fmt, ap);
    printf("\n");
  }
  va_end(ap);
}
