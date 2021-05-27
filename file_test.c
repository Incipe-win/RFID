#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  int fd;
  ssize_t w_len = 0;
  ssize_t r_len = 0;
  char r_buf[30] = {0};
  char w_buf[100] = "sjfkjsdklfjsdklf";

  //打开文件
  fd = open("./1.txt", O_RDWR);
  if (-1 == fd) {
    perror("open error");
    return -1;
  }
  printf("open success,the fd is %d\n", fd);

  //写入数据到文件
  w_len = write(fd, w_buf, strlen(w_buf));
  if (w_len > 0) {
    printf("wirte success,write size is %ld\n", w_len);
  } else if (0 == w_len) {
    printf("什么也没写进去\n");
  } else if (-1 == w_len) {
    perror("no data to write\n");
    return -1;
  }

  //读取数据
  r_len = read(fd, r_buf, 20);
  if (r_len > 0) {
    printf("read size is %d,data:%s\n", r_len, r_buf);
  } else if (0 == r_len) {
    printf("no data to read\n");
  } else if (-1 == w_len) {
    perror("read error\n");
    return -1;
  }

  //关闭文件
  close(fd);
}
