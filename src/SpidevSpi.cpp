#include "SpidevSpi.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>

CSpidevSpi::CSpidevSpi(const char *path, uint32_t speedHz) : m_Path(path), m_SpeedHz(speedHz)
{
  m_Fd = -1;
}

CSpidevSpi::~CSpidevSpi()
{
}

bool CSpidevSpi::Open(uint8_t mode, bool lsbFirst, uint8_t bitsPerWord)
{
  int ret = -1;

  // printf("try to open,path:%s ,speed:%u\n", m_Path.c_str(),m_SpeedHz); // %s is format specifier

  int fd = open(m_Path.c_str(), O_RDWR);
  if (fd < 0)
  {
    // Open failed
    // TODO: handle
    perror("Spi Open Fail:");
    return false;
  }

  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_SpeedHz);
  if (ret < 0)
  {
    // Set max speed failed
    // TODO: properly handle
    printf("Set max speed failed: %d\n", ret);
    close(fd);
    return false;
  }

  uint8_t modeFlags = 0;
  // printf("Try to set mode: %d,  %d\n",mode,lsbFirst);
  switch (mode)
  {
  case 0:
    modeFlags = SPI_MODE_0;
    break;
  case 1:
    modeFlags = SPI_MODE_1;
    break;
  case 2:
    modeFlags = SPI_MODE_2;
    break;
  case 3:
    modeFlags = SPI_MODE_3;
    break;
  default:
    modeFlags = SPI_MODE_0;
  }
  modeFlags |= (lsbFirst ? SPI_LSB_FIRST : 0);

  ret = ioctl(fd, SPI_IOC_WR_MODE, &modeFlags);
  if (ret < 0)
  {
    // Set mode failed
    // TODO: properly handle
    printf("Set mode failed: %d\n", ret);
    close(fd);
    return false;
  }

  m_Fd = fd;
  // printf("open\n");
  return true;
}

bool CSpidevSpi::Close()
{
  printf("closed\n");
  close(m_Fd);
}

int CSpidevSpi::Transfer(char *pBuf, size_t len)
{
  struct spi_ioc_transfer tr;
  memset(&tr, 0, sizeof(tr));
  tr.tx_buf = (uint64_t)pBuf;
  tr.rx_buf = (uint64_t)pBuf;
  tr.len = len;

  int ret = ioctl(m_Fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 0)
  {
    printf("transfer fail!\n");
  }
  return ret;
}
