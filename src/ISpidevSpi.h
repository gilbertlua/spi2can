#ifndef ISPIDEVSPI_H
#define ISPIDEVSPI_H
#include <string>

class ISpidevSpi
{
public:
  virtual int Transfer(char *pBuf, size_t len) = 0;
  virtual ~ISpidevSpi(){};
};

#endif // ISPIDEVSPI_H