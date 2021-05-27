#include "compute_time.h"
int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;

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
