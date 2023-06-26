#include "Spi2Can.h"

Spi2Can::Spi2Can(int cpu_id)
{
  _cpu_id = cpu_id;
}

void Spi2Can::start(CCanSocket &canSocket, ISpidevSpi &spiTx, ISpidevSpi &spiRx, ISysfsGpio &gpio)
{
  std::thread spiReceiveThread(&Spi2Can::rxSpi, this, std::ref(canSocket), std::ref(spiRx), std::ref(gpio));
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // let it print first
  Spi2Can::txSpi(canSocket, spiTx);
}

void Spi2Can::rxSpi(CCanSocket &canSocket, ISpidevSpi &spiRx, ISysfsGpio &gpio)
{
  if (_cpu_id != -1)
  {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);         //clears the cpuset
    CPU_SET(_cpu_id, &cpuset); //set CPU id on cpuset
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
  }

  syslog(LOG_INFO, "rx thread id: %d", (long int)syscall(224));
  char dataRx[SPI_FRAME_SIZE];
  CCanSocket::CanSocketFrame frame;
  bool isValid = false;
  while (1)
  {
    // wait for has data notification
    if (gpio.WaitForEdge(ISysfsGpio::edgeFalling, 10))
      continue;

    memset(dataRx, 0, SPI_FRAME_SIZE);
    spiRx.Transfer(dataRx, SPI_FRAME_SIZE);
    switch (dataRx[0])
    {
    // if it's can frame data
    case 't':
      frame.messageId = *(uint16_t *)&dataRx[1]; // 2 bytes
      // std::cout << "id:" << frame.messageId << "\n";
      frame.dlc = dataRx[3];
      memcpy(frame.canData, &dataRx[4], frame.dlc);
      isValid = true;
      break;

    // if it's configuration data
    case 'p':
    case 'F': // status flag
    case 'V': // version
    case 'N': // serial number
      frame.messageId = POWER_MANAGEMENT_ADDRESS - 128;
      frame.dlc = 8;
      memcpy(frame.canData, &dataRx[4], frame.dlc);
      isValid = true;
      break;
    }

    if (isValid)
      canSocket.Write(frame);
    isValid = false;
  }
}

void Spi2Can::txSpi(CCanSocket &canSocket, ISpidevSpi &spiTx)
{
  syslog(LOG_INFO, "tx thread id: %d", ::getpid());
  char dataTx[SPI_FRAME_SIZE];
  while (1)
  {
    CCanSocket::CanSocketFrame frame;
    canSocket.Read(&frame);
    if (frame.status == 0)
    {
      dataTx[0] = 't';
      *(uint16_t *)&dataTx[1] = frame.messageId; // 2 bytes
      if (frame.dlc > 8)
        frame.dlc = 8;
      dataTx[3] = frame.dlc;
      memcpy(&dataTx[4], frame.canData, dataTx[3]);
      spiTx.Transfer(dataTx, SPI_FRAME_SIZE);
      // std::cout << "transmitted data : " << dataTx[4] << '\n';

      // if it's also for other config configuration
      if (frame.messageId == POWER_MANAGEMENT_ADDRESS)
      {
        switch (frame.canData[0])
        {
        case 'p': // power management status request
        case 'O': // open CAN Channel
        case 'C': // close CAN channel
        case 's': // NACK
        case 'F': // status flag
        case 'V': // version
        case 'N': // serial number
          break;

        case 'M': // set CAN acceptance mask
        case 'm': // set CAN acceptance id
          *(uint32_t *)&dataTx[1] = *(uint32_t *)&frame.canData[1];
          break;

        case 'S': // setup CAN bit rates
          dataTx[1] = frame.canData[1];
          break;
        }
        dataTx[0] = frame.canData[0];
        spiTx.Transfer(dataTx, SPI_FRAME_SIZE);
      }
    }
  }
}