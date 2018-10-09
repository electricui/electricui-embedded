#include "eui_crc.h"

void
crc16(uint8_t data, uint16_t *crc)
{
    *crc  = (uint8_t)(*crc >> 8) | (*crc << 8);
    *crc ^= data;
    *crc ^= (uint8_t)(*crc & 0xff) >> 4;
    *crc ^= (*crc << 8) << 4;
    *crc ^= ((*crc & 0xff) << 4) << 1;
}
