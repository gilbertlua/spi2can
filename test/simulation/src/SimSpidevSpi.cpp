
#include "SimSpidevSpi.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

SimSpidevSpi::SimSpidevSpi(const char* path, uint32_t speedHz)
{
    counter = 0;
}

SimSpidevSpi::~SimSpidevSpi()
{
}

int SimSpidevSpi::Transfer(char* pBuf, size_t len)
{
    CCanSocket::CanSocketFrame frame;
    frame.messageId = 0x580 + 1;
    frame.dlc = 8;
    memcpy(frame.canData, framedata, 8);
    // NextData(&frame);
    pBuf[0] = 't';
    *(uint16_t *)&pBuf[1] = frame.messageId; // 2 bytes
    pBuf[3] = frame.dlc;
    memcpy(&pBuf[4], frame.canData, pBuf[3]);
    counter++;
    *(uint64_t *)framedata = counter;
	return 0;
}