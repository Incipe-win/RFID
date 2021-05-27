#include "judge.h"

int judge(unsigned int card_id) {
  int fd = open("./info.txt", O_RDWR | O_CREAT, 0666);
  char buf[65];
  memset(buf, 0, sizeof(buf));
  char info[20];
  memset(info, 0, sizeof(info));
  int ret = read(fd, buf, sizeof(buf));
  unsigned int id = 0;
  sscanf(buf, "%d %s", &id, info);
  if (id == card_id) {
    close(fd);
    return 1;
  }
  while (ret) {
    ret = read(fd, buf, sizeof(buf));
    sscanf(buf, "%u %s", &id, info);
    if (id == card_id) {
      close(fd);
      return 1;
    }
  }
  close(fd);
  return 0;
}
