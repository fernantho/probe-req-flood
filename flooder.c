#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <netlink/genl/genl.h>


#include "flooder.h"

static struct nl_handle *gen_handle(flooder_param *params){


}

static struct nl_cb *gen_cb(){


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
  struct nl_handle *handle = gen_handle(params);
  struct nl_cb *cb = gen_cb();
  struct nl_msg *msg = gen_msg(params);
  if (msg == NULL || cb == NULL || handle == NULL)
    return -1;
  
  int i = 0;
  while(params->times == -1 || i < params->times){
    send_one_probe_request(handle, msg, cb);
    i++;
  }
  
  return 0;
}
