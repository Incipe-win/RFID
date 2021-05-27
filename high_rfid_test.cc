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

unsigned char CalBcc(unsigned char *buf) {
  unsigned char bcc = 0;
  // 计算bbc位, 第一位是帧大小, 故按照手册对相应的位进行异或
  for (int i = 0; i < buf[0] - 2; ++i) {
    bcc ^= buf[i];
  }
  // 最后取反得到bbc
  return (~bcc);
}

// 请求
int PiccRequest(int fd) {
  // 请求数据大小
  unsigned char cmd[7];
  int ret = 0;
  memset(cmd, 0, sizeof(cmd));
  unsigned char req[8];
  memset(req, 0, sizeof(req));
  // 帧大小
  cmd[0] = 0x07;
  // 命令类型
  cmd[1] = 0x02;
  // 命令A表示请求
  cmd[2] = 0x41;
  // 数据长度, 就是info的长度
  cmd[3] = 0x01;
  // 获得周围所有卡的请求
  cmd[4] = 0x52;
  // bbc位
  cmd[5] = CalBcc(cmd);
  // 结束位
  cmd[6] = 0x03;
  // 往RFID高频卡发送七个字节的请求
  ret = write(fd, cmd, 7);
  if (ret == -1) {
    perror("write request cmd error");
    return -1;
  } else if (ret == 0) {
    printf("request nothing write\n");
    return -1;
  }
  usleep(500000);
  // 让文件偏移量移动到文件头
  lseek(fd, 0, SEEK_SET);
  // 往RFID高频卡读取八个字节
  ret = read(fd, req, 8);
  if (ret == -1) {
    perror("read request req error");
    return -1;
  } else if (ret == 0) {
    printf("request nothing read\n");
    return -1;
  }
  printf("request cmd is success\n");
  // 输出读取到的信息
  for (int i = 0; i < 8; ++i) {
    printf("%x ", req[i]);
  }
  printf("\n");
  // 如果第三位不是0x00, 就说明请求失败
  if (req[2] != 0x00) {
    printf("request error, errno is %d\n", req[2]);
    return -1;
  }
  // 判断一下RFID卡的类型
  if (req[4] == 0x04 && req[5] == 0x00) {
    printf("cardid size 4 byte, card type: Mifarel S50 or SHC1101 or 11RF32\n");
  }
  return 0;
}

// 放碰撞
int PiccAnticoll(int fd) {
  // 饭碰撞往RFID高频卡发送八个字节
  unsigned char cmd[8];
  memset(cmd, 0, sizeof(cmd));
  // 数据长度
  cmd[0] = 0x08;
  // 命令类型
  cmd[1] = 0x02;
  // 命令B表示饭碰撞
  cmd[2] = 0x42;
  // 数据长度, 是info的长度
  cmd[3] = 0x02;
  // 一级饭碰撞
  cmd[4] = 0x93;
  cmd[5] = 0x00;
  // bbc位
  cmd[6] = CalBcc(cmd);
  // 结束位
  cmd[7] = 0x03;
  int ret = 0;
  // 往RFID高频卡发送八个字节的请求
  ret = write(fd, cmd, 8);
  if (ret == -1) {
    perror("write anticoll cmd error");
    return -1;
  } else if (ret == 0) {
    printf("anticoll nothing write\n");
    return -1;
  }
  usleep(500000);
  // 让文件偏移量移动到文件头
  lseek(fd, 0, SEEK_SET);
  unsigned char req[10];
  memset(req, 0, sizeof(req));
  // 往RFID高频卡读取十个字节
  ret = read(fd, req, 10);
  if (ret == -1) {
    perror("read anticoll req error");
    return -1;
  } else if (ret == 0) {
    printf("anticoll nothing read\n");
    return -1;
  }
  printf("anticoll cmd success\n");
  for (int i = 0; i < 10; ++i) {
    printf("%x ", req[i]);
  }
  printf("\n");
  /* *
   *       0000 0000 0000 0000 0000 0000 0001 0000
   *   |   0000 0000 0000 0000 1000 0110
   *   |   0000 0000 0110 1110
   *   |   1000 1110
   *   --------------------------------------------
   *       1000 1110 0110 1110 1000 0110 0001 0000
   * */
  if (req[2] == 0x00) {
    cardid = (req[4] << 24) | (req[5] << 16) | (req[6] << 8) | req[7];
    return 0;
  }
  return -1;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("./led_control serial baud\n");
    exit(1);
  }
  int serial_fd = serial_init(argv[1], atoi(argv[2]));
  while (1) {
    if (PiccRequest(serial_fd)) {
      printf("The request failed!\n");
      return -1;
    }
    if (PiccAnticoll(serial_fd)) {
      printf("Couldn't get card-id!\n");
      return -1;
    }
    // 打印RFID高平卡卡号
    printf("card ID = %x\n", cardid);
  }
  close(serial_fd);
  return 0;
}
