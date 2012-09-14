#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "flooder.h"


static char* flooder_version = "802.11 Probe Request Flooder 0.1";
static char* flooder_license = "GPL2";

static flooder_param *global_params = NULL;

static void usage(){
  printf("%s\n%s\n"
	 "usage:\n"
	 " flooder -i<ifname> -c<channel> [-n<times>]\n"
	 "  -i = interface name\n"
	 "  -c = channel number\n"
	 "  -n = flood times, no parameter means forever\n"
	 "example:\n"
	 "  flooder -i wlan0 -c 11 -n 100000000\n", 
	 flooder_version,
	 flooder_license
	 );

}

static int global_init(){
  global_params = (flooder_param *)malloc(sizeof(flooder_param));
  if (global_params == NULL)
    return 0;
  global_params->times = -1;
  return 1;
}

static void global_deinit(){
  if (global_params != NULL)
    free(global_params);
}

static int set_interface(char* opt){
  // TODO
  return 0;
}

static int set_channel(char* opt){
  // TODO
  return 0;
}

static int set_times(char* opt){
  // TODO
  return 0;
}


int main(int argc, char *argv[]){
  
  int c;
  int exitcode = 0;

  if (!global_init())
    goto error;

  if (argc != 4){
    usage();
    goto error;
  }

  while((c = getopt (argc, argv, "i:c:")) != -1){
    switch (c){
    case 'i':
      if(set_interface(optarg) == -1)
	goto error;
      break;
    case 'c':
      if (set_channel(optarg) == -1)
	goto error;
      break;
    case 'n':
      if (set_times(optarg -1))
	goto error;
      break;
    default:
      usage();
      exitcode = 0;
      goto out;
    }
  }
  
  exitcode = probe_req_flood(global_params);
    
 out:
  return exitcode;
 error:
  exitcode = -1;
  global_deinit();
  return exitcode;
    
}
