#ifndef SPI2CAN_H__
#define SPI2CAN_H__

#include "CanSocket.h"
#include "ISpidevSpi.h"
#include "ISysfsGpio.h"
#include <chrono>
#include <cstring>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <syslog.h> // use syslog to
#include <thread>   // std::thread
#include <unistd.h>

#define SPI_FRAME_SIZE 12
#define POWER_MANAGEMENT_ADDRESS 0x577
using namespace std;

struct Configuration
{
  uint32_t SpiSpeed = 16000000;
  int CpuId = 0;
  string PortName = "vcan0";
  int GpioPin = 1;
  string SpiTxName = "/dev/spidev0.0";
  string SpiRxName = "/dev/spidev0.1";
};

class Spi2Can
{
public:
  Spi2Can(int cpu_id);
  void start(CCanSocket &canSocket, ISpidevSpi &spiTx, ISpidevSpi &spiRx, ISysfsGpio &gpio);

private:
  void test(int i);
  void rxSpi(CCanSocket &canSocket, ISpidevSpi &spiRx, ISysfsGpio &gpio);
  void txSpi(CCanSocket &canSocket, ISpidevSpi &spiTx);
  int _cpu_id;
};

#endif // SPI2CAN_H__
