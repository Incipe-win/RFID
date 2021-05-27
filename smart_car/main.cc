#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compute_time.h"
#include "get_id.h"
#include "judge.h"
#include "serial_init.h"
#include "write_info.h"

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
