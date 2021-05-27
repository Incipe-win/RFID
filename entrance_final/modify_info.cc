#include "get_id.h"
#include "modify_info.h"

int modify_info(int serial_fd) {
  system("clear");
  unsigned int card_id = get_id(serial_fd);
  int fd = open("./info.txt", O_RDWR);
  char buf[65];
  memset(buf, 0, sizeof(buf));
  int ret = read(fd, buf, sizeof(buf));
  char name[20];
  memset(name, 0, sizeof(name));
  unsigned int id;
  sscanf(buf, "%u %s", &id, name);
  if (id == card_id) {
    printf("welecome %s go home\n", name);
    close(fd);
    return 0;
  }
  while (ret) {
    ret = read(fd, buf, sizeof(buf));
    sscanf(buf, "%u %s", &id, name);
    if (id == card_id) {
      printf("welecome %s go home\n", name);
      close(fd);
      return 0;
    }
  }
  printf("You're not from our village\n");
  close(fd);
  return -1;
}
