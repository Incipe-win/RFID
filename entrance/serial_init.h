#ifndef __SERIAL_INIT_H_
#define __SERIAL_INIT_H_

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

int serial_init(const char *file, int baudrate);
#endif  // __SERIAL_INIT_H_
