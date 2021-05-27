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

int serial_init(const char *file, int baudrate) {
  int fd;
  fd = open(file, O_RDWR);
  if (fd == -1) {
    perror("open device error:");
    return -1;
  }
  // 配置串口属性
  struct termios myserial;
  // 初始化myserial结构体
  memset(&myserial, 0, sizeof(myserial));
  // 本地模式并且使能接收
  myserial.c_cflag |= (CLOCAL | CREAD);
  // 无掩码
  myserial.c_cflag &= ~CSIZE;
  // 关闭RTS/CTS硬件流控制
  myserial.c_cflag &= ~CRTSCTS;
  // 八位数据位
  myserial.c_cflag |= CS8;
  // 不使用两个停止位，即一个停止位
  myserial.c_cflag &= ~CSTOPB;
  // 不进行奇偶校验
  myserial.c_cflag &= ~PARENB;
  // 波特率选择
  switch (baudrate) {
    case 9600:
      // 写波特率9600
      cfsetospeed(&myserial, B9600);
      // 读波特率9600
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
  // 刷新输入输出缓冲区
  tcflush(fd, TCIFLUSH);
  // 使设置的终端属性立即生效
  tcsetattr(fd, TCSANOW, &myserial);
  // 返回配置好的串口文件描述符
  return fd;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("./led_control serial baud\n");
    exit(1);
  }
  // 打开串口
  int serial_fd = serial_init(argv[1], atoi(argv[2]));
  // 从串口读到的数据
  char recv_data[80];
  memset(recv_data, 0, sizeof(recv_data));
  // 低频RFID卡号
  unsigned int id = 0;
  while (1) {
    // 往串口读13个数据
    int len = read(serial_fd, recv_data, 13);
    if (13 == len) {
      // 因为硬件定制，RFID卡第三位需要-8
      recv_data[2] = recv_data[2] - 8;
      for (int i = 0; i < 13; ++i) {
        if (i == 11) {
          // 因为硬件定制，RFID卡第第十二位需要+8
          printf("%x ", recv_data[i] + 8);
        } else {
          printf("%x ", recv_data[i]);
        }
        if (i >= 1 && i <= 10) {
          // RFID卡的信息保存在第二位到第十一位
          recv_data[i] = (recv_data[i] > 0x40) ? (recv_data[i] - 0x41 + 10)
                                               : (recv_data[i] - 0x30);
          // 数据拼接
          id = id << 4 | recv_data[i];
        }
      }
      printf("\n");
      printf("get id is: %u\n", id);
    }
  }
  return 0;
}
