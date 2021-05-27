#include "get_id.h"
#include "register_info.h"

int register_info(int fd) {
  system("clear");
  printf("please input your name:\n");
  char name[20];
  memset(name, 0, sizeof(name));
  scanf("%s", name);
  unsigned int card_id = get_id(fd);
  if ((int)card_id == -1) {
    printf("get_id error\n");
    return -1;
  }
  if (write_info(card_id, name) != 0) {
    return -1;
  }
  return 0;
}

int write_info(unsigned int card_id, const char *name) {
  int fd = open("./info.txt", O_RDWR | O_CREAT, 0666);
  lseek(fd, 0, SEEK_END);
  char buf[64];
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "%u %s", card_id, name);
  int ret = 0;
  ret = write(fd, buf, sizeof(buf));
  write(fd, "\n", 1);
  if (ret < 0) {
    perror("write info error");
    close(fd);
    return -1;
  } else if (ret == 0) {
    printf("write info to nothing\n");
    close(fd);
    return -1;
  }
  printf("register success\n");
  close(fd);
  return 0;
}
