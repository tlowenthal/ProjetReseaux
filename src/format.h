#ifndef __FORMAT_H_
#define __FORMAT_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <zlib.h>
#define MAXLEN 640

//buf doit etre initie a 0 et avoir une longueur de MAXLEN
//msg is freed a the end, so it must have been allocated for a DATA type
int format(uint8_t *buf, uint8_t window, uint8_t *msg, uint16_t msg_len, uint8_t seq_num);

#endif