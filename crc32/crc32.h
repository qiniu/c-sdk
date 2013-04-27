#ifndef CRC32_H_
#define CRC32_H_

#include <stdlib.h>

unsigned long Crc32_ComputeBuf(unsigned long inCrc32, const void *buf, size_t bufLen);

#endif

