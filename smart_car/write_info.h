#ifndef __WRITE_INFO_H_
#define __WRITE_INFO_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int write_info(unsigned int card_id, const char *info);

#endif  // __WRITE_INFO_H_
