#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "version.h"
#include "help.h"

#define TARGET "Frame-Selector"

int main(int argc, char *argv[])
{
  const char *name;
  const char *pcap = NULL;

  int exit_status = EXIT_FAILURE;

  name = (const char *)strrchr(argv[0], '/');
  name = name ? (name + 1) : argv[0];

  enum opt {
    OPT_COMMIT = 0x100
  };

  struct opt_help helps[] = {
    { 'h', "help", "Show this help message" },
    { 'V', "version", "Print version information" },
#ifdef COMMIT
    { 0, "commit", "Display commit information" },
#endif /* COMMIT */
    { 0, NULL, NULL }
  };

  struct option opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
#ifdef COMMIT
    { "commit", no_argument, NULL, OPT_COMMIT },
#endif /* COMMIT */
    { NULL, 0, NULL, 0 }
  };

  while(1) {
    int c = getopt_long(argc, argv, "hV", opts, NULL);

    if(c == -1)
      break;

    switch(c) {
#ifdef COMMIT
    case(OPT_COMMIT):
      commit();
      exit_status = EXIT_SUCCESS;
      goto EXIT;
#endif /* COMMIT */
    case('V'):
      version(TARGET);
      exit_status = EXIT_SUCCESS;
      goto EXIT;
    case('h'):
      exit_status = EXIT_SUCCESS;
    default:
      help(name, "[OPTIONS] ... [PCAP FILE]", helps);
      goto EXIT;
    }
  }

  if((argc - optind) == 1) {
    int ret;

    pcap = argv[optind];

    ret = access(pcap, R_OK);

    switch(ret) {
    case 0:
      //pcap_open(pcap);
      break;
    case ENOENT:
      break;
    default:
      err(EXIT_FAILURE, "cannot access %s", pcap);
    }
  }

EXIT:
  return exit_status;
}

