#ifndef SIMSPIDEVSPITX_H
#define SIMSPIDEVSPITX_H

#include <string>
#include "../../../src/ISpidevSpi.h"
#include "../../../src/CanSocket.h"
#include <stdio.h>

using namespace std;

class SimSpidevSpiTx : public ISpidevSpi
{
public:
	SimSpidevSpiTx(const char* path, uint32_t speedHz);
	virtual ~SimSpidevSpiTx();
	int Transfer(char* pBuf, size_t len) override;
	
private:
	char framedata[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int m_Fd;
    int counter;
	long currVal;
	long prevVal;
	FILE * logFile;
	
};

#endif //SIMSPIDEVSPITX_H
