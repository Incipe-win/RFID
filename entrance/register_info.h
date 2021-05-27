#ifndef __REGISTER_INFO_H_
#define __REGISTER_INFO_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int register_info(int fd);
int write_info(int card_id, const char *name);

#endif  // __REGISTER_INFO_H_
