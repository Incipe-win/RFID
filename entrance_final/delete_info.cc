#include "delete_info.h"
#include "get_id.h"

void delete_info(int serial_fd) {
  system("clear");
  int fd = open("./info.txt", O_RDWR);
  int index = 0;
  unsigned int card_id = get_id(serial_fd);
  int file_size = 0;
  file_size = lseek(fd, 0, SEEK_END);
  printf("file_size is %d\n", file_size);
  lseek(fd, 0, SEEK_SET);

  char info[file_size / 65 + 1][65];
  memset(info, 0, sizeof(info));

  char buf[65];
  memset(buf, 0, sizeof(buf));
  int ret = read(fd, buf, sizeof(buf));
  char name[20];
  memset(name, 0, sizeof(name));
  unsigned int id;
  sscanf(buf, "%u %s", &id, name);
  printf("id is %u, name is %s\n", id, name);
  printf("card_id is %u\n", card_id);
  if (id != card_id) {
    strcpy(info[index++], buf);
    printf("info %s\n", info[0]);
  }
  while (ret) {
    ret = read(fd, buf, sizeof(buf));
    sscanf(buf, "%u %s", &id, name);
    if (id != card_id) {
      strcpy(info[index++], buf);
    }
  }
  close(fd);
  fd = open("./info.txt", O_RDWR | O_TRUNC);
  for (int i = 0; i < file_size / 65 - 1; ++i) {
    write(fd, info[i], sizeof(info[i]) - 1);
    write(fd, "\n", 1);
  }
  close(fd);
}
