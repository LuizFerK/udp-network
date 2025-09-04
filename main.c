#include <stdio.h>
#include "net/defs.h"
#include "net/helpers.h"
#include "net/setup.h"
#include "net/modules/menu.h"

int main(int argc, char *argv[]) {
  int id = get_cli_opt(argc, argv, "-i");
  Config* config = setup(id);

  menu(config);

  return 0;
}
