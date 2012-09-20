#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include "flooder.h"


static char* flooder_version = "802.11 Probe Request Flooder 0.1";
static char* flooder_license = "GPL2";

static flooder_param *global_params = NULL;

static void usage(){
  printf("%s\n%s\n"
	 "usage:\n"
	 " flooder -i<ifname> -c<channel> [-b<dst_mac>] [-t<times>] [-d]\n"
	 "  -i = interface name\n"
	 "  -c = channel number\n"
	 "  -b = target MAC address\n"
	 "  -t = flood times, no parameter means forever\n"
	 "  -d = display debug messages\n"
	 "example:\n"
	 "  flooder -i wlan0 -c 11 -t 100000000\n", 
	 flooder_version,
	 flooder_license
	 );

}

static int global_init(){
  global_params = (flooder_param *)malloc(sizeof(flooder_param));

  if (global_params == NULL)
    return 0;

  global_params->times = -1;

  int i;

  for(i = 0; i < ETH_ALEN; ++i)
    global_params->addr[i] = 0xff;

  return 1;
}

static void global_deinit(){
  if (global_params != NULL)
    free(global_params);
}

static int set_interface(char* opt){
  global_params->iface = if_nametoindex(opt);
  if (global_params->iface == 0)
    return -1;
  return 0;
}

static int set_channel(char* opt){
  global_params->channel = atoi(opt);
  if (global_params->channel > 11 || global_params->channel < 1){
    flooder_log(FLOODER_ERROR, "Invalid Channel");
    return -1;
  }
  return 0;
}

static int set_times(char* opt){
  global_params->times = atoi(opt);
  return 0;
}

static int set_debug_level(int level){
  debug_level = level;
  return 0;
}

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int hwaddr_aton(const char *txt, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num(*txt++);
		if (a < 0)
			return -1;
		b = hex2num(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
		if (i < 5 && *txt++ != ':')
			return -1;
	}

	return 0;
}

static int set_dst_addr(char* opt){
  hwaddr_aton(opt, global_params->addr);
  return 0;
}

int main(int argc, char *argv[]){
#define FLAGS_REQUIRED 2
  int c;
  int required_count = 0;
  int exitcode = 0;

  if (!global_init())
    goto error;

  set_debug_level(FLOODER_INFO);

  while((c = getopt (argc, argv, "b:i:c:t:d")) != -1){
    switch (c){
    case 'i':
      if(set_interface(optarg) == -1)
	goto error;
      required_count++;
      break;
    case 'c':
      if (set_channel(optarg) == -1)
	goto error;
      required_count++;
      break;
    case 't':
      if (set_times(optarg) == -1)
	goto error;
      break;
    case 'd':
      if (set_debug_level(FLOODER_DEBUG) == -1)
	goto error;
      break;
    case 'b':
      if (set_dst_addr(optarg) == -1)
	goto error;
      break;
    case '?':
      if (optopt == 'i' || optopt == 'c')
	printf ("Option -%c requires an argument.\n", optopt);
      else
	printf("Unknown option -%c.\n", optopt);
      break;
    default:
      usage();
      exitcode = 0;
      goto out;
    }
  }
  
  if (required_count != FLAGS_REQUIRED){
    usage();
    goto out;
  }

#undef FLAGS_REQUIRED
  if(probe_req_flood(global_params))
    goto error;
  
 out:
  return exitcode;
  
 error:
  exitcode = -1;
  global_deinit();
  flooder_log(FLOODER_ERROR, "internal error!\n");
  return exitcode;
}
