#include "get_id.h"

unsigned char CalBcc(unsigned char *buf) {
  unsigned char bcc = 0;
  for (int i = 0; i < buf[0] - 2; ++i) {
    bcc ^= buf[i];
  }
  return (~bcc);
}

int PiccRequest(int fd) {
  unsigned char cmd[7];
  int ret = 0;
  memset(cmd, 0, sizeof(cmd));
  unsigned char req[8];
  memset(req, 0, sizeof(req));
  cmd[0] = 0x07;
  cmd[1] = 0x02;
  cmd[2] = 0x41;
  cmd[3] = 0x01;
  cmd[4] = 0x52;
  cmd[5] = CalBcc(cmd);
  cmd[6] = 0x03;
  ret = write(fd, cmd, 7);
  if (ret == -1) {
    perror("write request cmd error");
    return -1;
  } else if (ret == 0) {
    printf("request nothing write\n");
    return -1;
  }
  usleep(500000);
  ret = read(fd, req, 8);
  if (ret == -1) {
    perror("read request req error");
    return -1;
  } else if (ret == 0) {
    printf("request nothing read\n");
    return -1;
  }
  printf("request cmd is success\n");
  if (req[2] != 0x00) {
    printf("request error, errno is %d\n", req[2]);
    return -1;
  }
  return 0;
}

int PiccAnticoll(int fd) {
  unsigned char cmd[8];
  memset(cmd, 0, sizeof(cmd));
  cmd[0] = 0x08;
  cmd[1] = 0x02;
  cmd[2] = 0x42;
  cmd[3] = 0x02;
  cmd[4] = 0x93;
  cmd[5] = 0x00;
  cmd[6] = CalBcc(cmd);
  cmd[7] = 0x03;
  int ret = 0;
  ret = write(fd, cmd, 8);
  if (ret == -1) {
    perror("write anticoll cmd error");
    return -1;
  } else if (ret == 0) {
    printf("anticoll nothing write\n");
    return -1;
  }
  usleep(500000);
  unsigned char req[10];
  memset(req, 0, sizeof(req));
  ret = read(fd, req, 10);
  if (ret == -1) {
    perror("read anticoll req error");
    return -1;
  } else if (ret == 0) {
    printf("anticoll nothing read\n");
    return -1;
  }
  printf("anticoll cmd success\n");
  if (req[2] == 0x00) {
    int cardid = 0;
    cardid = (req[4] << 24) | (req[5] << 16) | (req[6] << 8) | req[7];
    return cardid;
  }
  return -1;
}

int get_id(int fd) {
  if (PiccRequest(fd) != 0) {
    printf("PiccRequest error\n");
    return -1;
  }
  int card_id = 0;
  card_id = PiccAnticoll(fd);
  if (card_id == -1) {
    printf("PiccAnticoll error\n");
    return -1;
  }
  return card_id;
}
