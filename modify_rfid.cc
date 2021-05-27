#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

// 高频RFID卡卡号
unsigned int cardid;
char cardBuff[4];
unsigned char passwd[6] = {0x68, 0x65, 0x0A, 0x00, 0x00, 0x00};
char loft[20];

int serial_init(const char *file, int baudrate) {
  int fd;
  fd = open(file, O_RDWR);
  if (fd == -1) {
    perror("open device error:");
    return -1;
  }
  struct termios myserial;
  memset(&myserial, 0, sizeof(myserial));
  myserial.c_cflag |= (CLOCAL | CREAD);
  myserial.c_cflag &= ~CSIZE;
  myserial.c_cflag &= ~CRTSCTS;
  myserial.c_cflag |= CS8;
  myserial.c_cflag &= ~CSTOPB;
  myserial.c_cflag &= ~PARENB;
  switch (baudrate) {
    case 9600:
      cfsetospeed(&myserial, B9600);
      cfsetispeed(&myserial, B9600);
      break;
    case 115200:
      cfsetospeed(&myserial, B115200);
      cfsetispeed(&myserial, B115200);
      break;
    case 19200:
      cfsetospeed(&myserial, B19200);
      cfsetispeed(&myserial, B19200);
      break;
  }
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &myserial);
  return fd;
}

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
    printf("request nothing to write\n");
    return -1;
  }
  usleep(500000);
  ret = read(fd, req, 8);
  if (ret == -1) {
    perror("read request req error");
    return -1;
  } else if (ret == 0) {
    printf("request nothing to read\n");
    return -1;
  }
  printf("request cmd is success\n");
  for (int i = 0; i < 8; ++i) {
    printf("%x ", req[i]);
  }
  printf("\n");
  if (req[2] != 0x00) {
    printf("request error, errno is %d\n", req[2]);
    return -1;
  }
#if 0
  // 判断一下RFID卡的类型
  if (req[4] == 0x04 && req[5] == 0x00) {
    printf("cardid size 4 byte, card type: Mifarel S50 or SHC1101 or 11RF32\n");
  }
#endif
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
    printf("anticoll nothing to write\n");
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
    printf("anticoll nothing to read\n");
    return -1;
  }
  printf("anticoll cmd success\n");
  printf("Anticoll data:\n\t");
  for (int i = 0; i < 10; ++i) {
    printf("%x ", req[i]);
  }
  printf("\n");
  if (req[2] == 0x00) {
    cardBuff[0] = req[4];
    cardBuff[1] = req[5];
    cardBuff[2] = req[6];
    cardBuff[3] = req[7];
    cardid = (req[7] << 24) | (req[6] << 16) | (req[5] << 8) | req[4];
    return 0;
  }
  return -1;
}

int PiccSelect(int fd) {
  unsigned char WBuf[0x0B], RBuf[0x07];
  int ret = 0;
  memset(WBuf, 0, 0x0B);
  memset(RBuf, 0, 0x07);
  WBuf[0] = 0x0B;
  WBuf[1] = 0x02;
  WBuf[2] = 0x43;
  WBuf[3] = 0x05;
  WBuf[4] = 0x93;
  WBuf[5] = cardBuff[0];
  WBuf[6] = cardBuff[1];
  WBuf[7] = cardBuff[2];
  WBuf[8] = cardBuff[3];
  WBuf[9] = CalBcc(WBuf);
  WBuf[10] = 0x03;

  ret = write(fd, WBuf, 0x0B);
  if (ret < 0) {
    perror("write select error");
    return -1;
  } else if (ret == 0) {
    printf("write select to nothing\n");
    return -1;
  }
  usleep(100000);
  ret = read(fd, RBuf, 0x07);
  printf("Select Data is: \n\t");
  for (int i = 0; i < 7; ++i) {
    printf("%x ", RBuf[i]);
  }
  printf("\n");
  if (ret < 0) {
    perror("read select error");
    return -1;
  } else if (ret == 0) {
    printf("read select to nothing\n");
    return -1;
  }
  if (RBuf[2] == 0x00) {
    return 0;
  }
  return -1;
}

void getcard(int fd) {
  if (PiccRequest(fd)) {
    printf("The request failed!\n");
  }
  if (PiccAnticoll(fd)) {
    printf("Couldn't get card-id!\n");
  }
  if (PiccSelect(fd)) {
    printf("Select error\n");
  }
}

int PassWD(int fd, char AorB, char *uid, unsigned char *passwd,
           char block_num) {
  unsigned char cmd[0x12];
  memset(cmd, 0, sizeof(cmd));

  cmd[0] = 0x12;
  cmd[1] = 0x02;
  cmd[2] = 0x46;
  cmd[3] = 0x0C;
  cmd[4] = (AorB == 'A') ? 0x60 : 0x61;
  cmd[5] = uid[0];
  cmd[6] = uid[1];
  cmd[7] = uid[2];
  cmd[8] = uid[3];
  cmd[9] = passwd[0];   // 0xFF
  cmd[10] = passwd[1];  // 0xFF
  cmd[11] = passwd[2];  // 0xFF
  cmd[12] = passwd[3];  // 0xFF
  cmd[13] = passwd[4];  // 0xFF
  cmd[14] = passwd[5];  // 0xFF
  cmd[15] = block_num;
  cmd[16] = CalBcc(cmd);
  cmd[17] = 0x03;
  printf("PassWD cmd is: \n\t");
  for (int i = 0; i < 18; ++i) {
    printf("%x ", cmd[i]);
  }
  printf("\n");
  int ret = 0;

  ret = write(fd, cmd, 0x12);
  if (ret == -1) {
    perror("write passwd error");
  } else if (ret == 0) {
    printf("write passwd to nothing\n");
  }
  usleep(500000);
  unsigned char req[6];
  memset(req, 0, sizeof(req));
  ret = read(fd, req, 6);
  printf("passwd is: \n\t");
  for (int i = 0; i < 6; ++i) {
    printf("%x ", req[i]);
  }
  printf("\n");
  if (ret < 0) {
    perror("read passwd error");
    return -1;
  } else if (ret == 0) {
    printf("read passwd to nothing\n");
    return -1;
  }
  if (req[2] == 0x00) {
    printf("check passwd success\n");
    return 0;
  }
  return -1;
}

void ReadData(int fd, char num_block, char *loft) {
  unsigned char cmd[7];
  memset(cmd, 0, sizeof(cmd));
  cmd[0] = 0x07;
  cmd[1] = 0x02;
  cmd[2] = 0x47;
  cmd[3] = 0x01;
  cmd[4] = num_block;
  cmd[5] = CalBcc(cmd);
  cmd[6] = 0x03;
  int ret = 0;
  ret = write(fd, cmd, 7);
  if (ret == -1) {
    perror("ReadData write error");
  } else if (ret == 0) {
    printf("ReadData nothing write\n");
  }
  usleep(500000);
  unsigned char req[0x16];
  memset(req, 0, sizeof(req));
  ret = read(fd, req, 0x16);
  printf("ReadData is: \n\t");
  for (int i = 0; i < 0x16; ++i) {
    printf("%x ", req[i]);
  }
  printf("\n");
  if (ret == -1) {
    perror("ReadData error");
  } else if (ret == 0) {
    printf("ReadData to nothing\n");
  }
  if (req[2] != 0x00) {
    printf("ReadData fault\n");
    return;
  }
  for (int i = 0; i < 16; ++i) {
    loft[i] = req[i + 4];
  }
}

void WriteData(int fd, char num_block, unsigned char *message) {
  unsigned char cmd[0x17];
  memset(cmd, 0, sizeof(cmd));
  cmd[0] = 0x17;
  cmd[1] = 0x02;
  cmd[2] = 0x48;
  cmd[3] = 0x11;
  cmd[4] = num_block;
  for (int i = 5; i < 11; ++i) {
    cmd[i] = message[i - 5];
  }
  cmd[11] = 0xFF;
  cmd[12] = 0x07;
  cmd[13] = 0x80;
  cmd[14] = 0x69;
  for (int i = 15; i < 21; ++i) {
    cmd[i] = message[i - 15];
  }
  cmd[21] = CalBcc(cmd);
  cmd[22] = 0x03;
  int ret = 0;
  ret = write(fd, cmd, 0x17);
  if (ret == -1) {
    perror("WriteData write error");
  } else if (ret == 0) {
    printf("WriteData nothing write\n");
  }
  usleep(500000);
  unsigned char req[0x06];
  memset(req, 0, sizeof(req));
  ret = read(fd, req, 0x06);
  if (ret == -1) {
    perror("WriteData error");
  } else if (ret == 0) {
    printf("WriteData to nothing\n");
  }
  if (req[2] != 0x00) {
    printf("WriteData fault\n");
    return;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("./led_control serial baud\n");
    exit(1);
  }
  int serial_fd = serial_init(argv[1], atoi(argv[2]));
  while (1) {
    getcard(serial_fd);
    printf("card id is %x\n", cardid);
    int ret = 0;
    ret = PassWD(serial_fd, argv[4][0], cardBuff, passwd, atoi(argv[3]));
    if (!ret) {
      unsigned char newpasswd[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      WriteData(serial_fd, atoi(argv[3]), newpasswd);
      sleep(1);
      ReadData(serial_fd, atoi(argv[3]), loft);
      printf("read data is: %s\n", loft);
    }
    getchar();
  }
  close(serial_fd);
  return 0;
}
