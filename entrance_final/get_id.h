#ifndef __GET_ID_H_
#define __GET_ID_H_

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

unsigned char CalBcc(unsigned char *buf);
int PiccAnticoll(int fd);
int PiccRequest(int fd);
unsigned int get_id(int fd);

#endif  // __GET_ID_H_
