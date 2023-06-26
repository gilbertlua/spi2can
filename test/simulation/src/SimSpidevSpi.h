#ifndef SIMSPIDEVSPI_H
#define SIMSPIDEVSPI_H

#include <string>
#include "../../../src/ISpidevSpi.h"
#include "../../../src/CanSocket.h"

class SimSpidevSpi : public ISpidevSpi
{
public:
	SimSpidevSpi(const char* path, uint32_t speedHz);
	virtual ~SimSpidevSpi();
	int Transfer(char* pBuf, size_t len) override;
	
private:
	char framedata[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int m_Fd;
    uint64_t counter;
};

#endif //SIMSPIDEVSPI_H
