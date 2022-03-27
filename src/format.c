#include "format.h"

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
    uint8_t *copy = malloc(8);
    if (copy == NULL) return -1;
    memcpy(copy, buf, 8);
    *copy = *copy & ~(1 << 5);//on met TR a 0
    uint32_t crc = htonl((uint32_t) crc32(0, copy, 8));
    memcpy(ptr, &crc, 4);

    //copie du msg
    ptr+=4;
    //Si Data avec TR=0
    if ((*buf & 1 << 6) && !(*buf & 1 << 5)){
        memcpy(ptr, msg, msg_len);
        crc = htonl((uint32_t) crc32(0, ptr, msg_len));//pour le CRC2
        ptr+=msg_len;
        memcpy(ptr, &crc, 4);
    } else if(!(*buf & 1 << 7) && !(*buf & 1 << 6)){//type FEC
        if (!(*buf & 1 << 5)){// Si TR=0
            crc = htonl((uint32_t) crc32(0, ptr, 512));//pour le CRC2
            ptr+=512;
            memcpy(ptr, &crc, 4);
        } else {
            ptr+=512;
        }
    }





    return 0;
}