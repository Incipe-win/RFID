#include <stdio.h>

#include "choose.h"
#include "delete_info.h"
#include "modify_info.h"
#include "register_info.h"
#include "serial_init.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("./a.out /dev/ttySACnumber baudrate\n");
    return -1;
  }
  int serial_fd = serial_init(argv[1], atoi(argv[2]));
  while (1) {
    int chooice = choose();
    if (chooice == 1) {
      int ret = 999;
      ret = register_info(serial_fd);
      if (ret == 0) {
        printf("register success\n");
      } else {
        printf("register error\n");
      }
    } else if (chooice == 2) {
      delete_info(serial_fd);
    } else if (chooice == 3) {
      modify_info(serial_fd);
    }
  }
  close(serial_fd);
  return 0;
}
