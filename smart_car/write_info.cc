#include "write_info.h"

int write_info(unsigned int card_id, const char *info) {
  int fd = open("./info.txt", O_RDWR | O_CREAT, 0666);
  lseek(fd, 0, SEEK_END);
  char buf[64];
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "%u %s", card_id, info);
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
  close(fd);
  return 0;
}
