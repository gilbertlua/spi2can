
#include "SimSpidevSpiTx.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <ctime>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

SimSpidevSpiTx::SimSpidevSpiTx(const char* path, uint32_t speedHz)
{
    counter = 0;
    prevVal = -1;
    currVal = -1;
    logFile = fopen ("swapped-log.txt","w");
    time_t now = time(0);
    fprintf( logFile, "%s Start Spi2can simulation log... \n\n", ctime(&now) );
    fclose(logFile);
}

SimSpidevSpiTx::~SimSpidevSpiTx()
{
    fclose(logFile);
}

int SimSpidevSpiTx::Transfer(char* pBuf, size_t len)
{
    CCanSocket::CanSocketFrame frame;

    frame.messageId = *(uint16_t *)&pBuf[1]; // 2 bytes
    // std::cout << "id:" << frame.messageId << "\n";
    frame.dlc = pBuf[3];
    memcpy(frame.canData, &pBuf[4], frame.dlc);
    // int i = 0;
    // printf("(%d) 0x%03X [%d]", counter, frame.messageId, frame.dlc );
    // for (i = 0; i < frame.dlc; i++)
    //   printf("%02X ",frame.canData[i]);
    // printf("\r\n");

    prevVal = currVal;
    currVal = (uint64_t)(*(uint64_t*)&frame.canData[0]);
    counter++;
    if( prevVal > currVal ){
        logFile = fopen ("swapped-log.txt","a");
        time_t now = time(0);
        fprintf( logFile, "%s Frame Swapped!!\n", ctime(&now) );
        fprintf( logFile, "%s prevVal %d, currVal %d !!\n\n", ctime(&now), prevVal, currVal );
        fclose(logFile);
    }

	return 0;
}