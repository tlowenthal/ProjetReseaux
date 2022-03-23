#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <zlib.h>
#define MAXLEN 640

void printBits(uint8_t a){
    for (int i = 0; i < 8; i++) {
      printf("%d", !!((a << i) & 0x80));
    }
    printf("\n");
}

//buf doit etre initie a 0 et avoir une longueur de MAXLEN
int format(uint8_t *buf, uint8_t window, uint8_t *msg, uint16_t msg_len, uint8_t seq_num){

    uint8_t *ptr = buf;

    //on remplit le champ window
    *ptr = window;

    //pour l'instant, on ne fait que des types DATA avec TR a 0
    *ptr = *ptr | 1 << 6;

    //champ LENGTH
    ptr++;
    memcpy(ptr, (uint8_t *) &msg_len, 2);

    //champ SEQNUM
    ptr+=2;
    *ptr = seq_num;

    ptr++;
    //Ici on met le timestamp

    ptr+=4;
    memcpy(ptr, (uint8_t *) crc32(0, buf, 4), 4);


    return 0;
}


int main(int argc, char **argv) {

    uint8_t *buf = malloc(MAXLEN);
    uint8_t window = 1;
    char *msg = "Bonjour, je m'appelle Tobias";
    uint8_t seq_num = 2;

    format(buf, window, (uint8_t *) msg, (uint16_t) 29, seq_num);

    //printBits(*buf);
}