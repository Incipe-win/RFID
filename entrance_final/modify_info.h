#ifndef __MODIFY_INFO_
#define __MODIFY_INFO_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int modify_info(int serial_fd);

#endif  // __MODIFY_INFO_
