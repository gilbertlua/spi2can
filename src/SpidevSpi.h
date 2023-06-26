#ifndef SPIDEV_SPI_H__
#define SPIDEV_SPI_H__

#include "ISpidevSpi.h"
#include <string>

class CSpidevSpi : public ISpidevSpi
{
public:
  CSpidevSpi(const char *path, uint32_t speedHz);
  ~CSpidevSpi();

  bool Open(uint8_t mode, bool lsbFirst, uint8_t bitsPerWord);
  bool Close();
  int Transfer(char *pBuf, size_t len) override;

private:
  const std::string m_Path;
  const uint32_t m_SpeedHz;

  int m_Fd;
};

#endif // SPIDEV_SPI_H__
