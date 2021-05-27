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

unsigned char recv_buf[64];

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

int rfid_read_lable(int fd, unsigned char *recvbuf, int n) {
  read(fd, recvbuf, n);
  printf("\trecvbuf is:\n\t");
  for (int i = 0; i < 13; ++i) {
    printf("%x ", recvbuf[i]);
  }
  printf("\n");
  if ((recvbuf[0] != 0x55) || (recvbuf[1] != 0xAA)) {
    printf("the frame format error!\n");
    return -1;
  }
  printf("read_lable====>\n");
  printf("\trecvbuf:");
  for (int i = 0; i < recvbuf[3] + 6; ++i) {
    printf("%x ", recvbuf[i]);
  }
  putchar('\n');
  printf("\tCMD: %d\n", recvbuf[2]);
  printf("\tID: %d\n",
         recvbuf[4] << 24 | recvbuf[5] << 16 | recvbuf[6] << 8 | recvbuf[7]);
  printf("\tExternDataLen: %d\n", recvbuf[8] >> 5);
  printf("\tSport_flag: %d\n", !!(recvbuf[8] & 1 << 4));
  printf("\tMessage: ");
  for (int i = 0; i < recvbuf[3]; ++i) {
    printf("%x ", recvbuf[i + 4]);
  }
  putchar('\n');
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("./led_control serial baud\n");
    exit(1);
  }
  int serial_fd = serial_init(argv[1], atoi(argv[2]));
  while (1) {
    printf("begen...\n");
    rfid_read_lable(serial_fd, recv_buf, 13);
    getchar();
  }
  return 0;
}
