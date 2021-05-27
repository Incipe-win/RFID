#include "serial_init.h"

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
