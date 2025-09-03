#include <stdio.h>
#include "net/defs.h"
#include "net/helpers.h"
#include "net/setup.h"

int main(int argc, char *argv[]) {
  int id = get_cli_opt(argc, argv, "-i");
  Router router_config = setup(id);

  return 0;
}
