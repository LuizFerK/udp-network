#include <stdio.h>
#include "net/defs.h"
#include "net/helpers.h"
#include "net/setup.h"
#include "net/modules/menu.h"

int main(int argc, char *argv[]) {
  int id = get_cli_opt(argc, argv, "-i", -1);
  int routing_timeout = get_cli_opt(argc, argv, "-t", 10);
  Config* config = setup(id, routing_timeout);

  menu(config);

  return 0;
}
