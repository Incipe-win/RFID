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
#include <time.h>
#include <unistd.h>

int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;

int serial_init(const char* file, int baudrate) {
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

unsigned char CalBcc(unsigned char* buf) {
  unsigned char bcc = 0;
  for (int i = 0; i < buf[0] - 2; ++i) {
    bcc ^= buf[i];
  }
  return (~bcc);
}

int write_info(unsigned int card_id, const char* info) {
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
    unsigned int cardid = 0;
    cardid = (req[4] << 24) | (req[5] << 16) | (req[6] << 8) | req[7];
    return cardid;
  }
  return -1;
}

unsigned int get_id(int fd) {
  if (PiccRequest(fd) != 0) {
    printf("PiccRequest error\n");
    return -1;
  }
  unsigned int card_id = 0;
  card_id = PiccAnticoll(fd);
  if ((int)card_id == -1) {
    printf("PiccAnticoll error\n");
    return -1;
  }
  return card_id;
}

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

void get_price() {
  time_t now_time;
  struct tm* tm = NULL;
  now_time = time(NULL);
  tm = localtime(&now_time);
  ++tm->tm_mon;
  printf("Time out of the garage: \n\t %d-%d-%d %d:%d:%d\n", tm->tm_year + 1900,
         tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  sec = tm->tm_sec - sec;
  if (sec < 0) {
    sec += 60;
    --tm->tm_min;
  }
  min = tm->tm_min - min;
  if (min < 0) {
    min += 60;
    --tm->tm_hour;
  }
  hour = tm->tm_hour - hour;
  if (hour < 0) {
    hour += 24;
    --tm->tm_mday;
  }
  day = tm->tm_mday - day;
  if (day < 0) {
    day += 30;
    --tm->tm_mon;
  }
  mon = tm->tm_mon - mon;
  if (mon < 0) {
    mon += 12;
    --tm->tm_year;
  }
  year = tm->tm_year + 1900 - year;

  printf("The car stoped time:\n\t %d-%d-%d %d:%d:%d\n", year, mon, day, hour,
         min, sec);
  double price = 0.0;
  if (year == 0 && mon == 0 && day == 0 && hour == 0) {
    if (min > 30) {
      price = 5;
    } else {
      price = 0;
    }
  } else {
    if (hour < 12 && min > 30) {
      price = (year * 365 + mon * 30 + day) * 60 + hour * 5 + 5;
    } else if (hour > 12 && min > 30) {
      price = (year * 365 + mon * 30 + day + 1) * 60 + 5;
    } else if (hour < 12 && min < 30) {
      price = (year * 365 + mon * 30 + day) * 60 + hour * 5;
    } else if (hour > 12 && min < 30) {
      price = (year * 365 + mon * 30 + day + 1) * 60;
    }
  }
  printf("you have to pay: %.2lfyuan\n", price);
}

void compute_time(unsigned int card_id) {
  bool flag = false;
  int fd = open("./info.txt", O_RDWR);
  int index = 0;
  int file_size = 0;
  file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  char write_info[file_size / 65 + 1][65];
  memset(write_info, 0, sizeof(write_info));

  char buf[65];
  memset(buf, 0, sizeof(buf));
  int ret = read(fd, buf, sizeof(buf));
  char info[20];
  memset(info, 0, sizeof(info));
  unsigned int id;
  sscanf(buf, "%u %s", &id, info);
  if (id == card_id) {
    flag = true;
    sscanf(info, "%d-%d-%d_%d:%d:%d", &year, &mon, &day, &hour, &min, &sec);
    printf("Entering garage time: \n\t %d-%d-%d %d:%d:%d\n", year, mon, day,
           hour, min, sec);
    get_price();
  } else {
    strcpy(write_info[index++], buf);
  }
  while (ret) {
    ret = read(fd, buf, sizeof(buf));
    sscanf(buf, "%u %s", &id, info);
    if (id == card_id) {
      if (!flag) {
        printf("Entering garage time: \n\t %d-%d-%d %d:%d:%d\n", year, mon, day,
               hour, min, sec);
        sscanf(info, "%d-%d-%d_%d:%d:%d", &year, &mon, &day, &hour, &min, &sec);
        get_price();
      }
    } else {
      strcpy(write_info[index++], buf);
    }
  }
  close(fd);
  fd = open("./info.txt", O_RDWR | O_TRUNC);
  for (int i = 0; i < file_size / 65 - 1; ++i) {
    write(fd, write_info[i], sizeof(write_info[i]) - 1);
    write(fd, "\n", 1);
  }
  close(fd);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("./a.out /dev/ttySACnumber baudrate\n");
    return -1;
  }
  int serial_fd = serial_init(argv[1], atoi(argv[2]));
  while (1) {
    unsigned int card_id = get_id(serial_fd);
    if (judge(card_id)) {
#if 1
      printf("please input time:\n");
      char time[64];
      fgets(time, 64, stdin);
      system(time);
#endif
      // system("date -s \"2021-05-23 12:21:33\"");
      compute_time(card_id);
    } else {
      time_t now_time;
      struct tm* tm = NULL;
      now_time = time(NULL);
      tm = localtime(&now_time);
      char info[20];
      memset(info, 0, sizeof(info));
      sprintf(info, "%d-%d-%d_%d:%d:%d", tm->tm_year + 1900, tm->tm_mon + 1,
              tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
      if (write_info(card_id, info) == 0) {
        printf("write info success\n");
      }
    }
    getchar();
  }
  close(serial_fd);
  return 0;
}
