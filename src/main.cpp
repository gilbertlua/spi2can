#include "CanSocket.h"
#include "Spi2Can.h"
#include "SpidevSpi.h"
#include "SysfsGpio.h"
#include <libconfig.h++>
#include <syslog.h> // use syslog to

#define APP_NAME "fmlx-spi2can"
#define SPI_FRAME_SIZE 12
#define POWER_MANAGEMENT_ADDRESS 0x577
#define CONFIG_PATH "/etc/opt/fmlx-spi2can/fmlx-spi2can.conf"

using namespace std;
using namespace libconfig;

int main()
{
  int cpu_count = std::thread::hardware_concurrency();

  //init log
  setlogmask(LOG_UPTO(LOG_DEBUG));
  openlog(APP_NAME, LOG_PID | LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "%s started.", APP_NAME);

  //Close standard file descriptors for security reasons
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  //setup configuration
  Configuration config;
  Config cfg;
  try
  {
    syslog(LOG_INFO, "Read configuration file : %s", CONFIG_PATH);
    cfg.readFile(CONFIG_PATH);
  }
  catch (const FileIOException &fioex)
  {
    syslog(LOG_ERR, "I/O error while reading file. : %s", CONFIG_PATH);
    return (EXIT_FAILURE);
  }
  catch (const ParseException &pex)
  {
    syslog(LOG_ERR, "Parse error at %s : %s - %s", pex.getFile(), pex.getLine(), pex.getError());
    return (EXIT_FAILURE);
  }

  try
  {
    cfg.lookupValue("spiSpeed", config.SpiSpeed);
  }
  catch (const SettingNotFoundException &nfex)
  {
    syslog(LOG_EMERG, "'spiSpeed' setting is not found. Use default value %d.", config.SpiSpeed);
  }

  try
  {
    int defaultId = config.CpuId;
    cfg.lookupValue("cpuId", config.CpuId);
    if (config.CpuId < -1 || config.CpuId > cpu_count - 1)
    {
      config.CpuId = defaultId;
      syslog(LOG_EMERG, "'cpuId' not in cpu id range. Use default value %d.", config.CpuId);
    }
  }
  catch (const SettingNotFoundException &nfex)
  {
    syslog(LOG_EMERG, "'cpuId' setting is not found. Use default value %d.", config.CpuId);
  }

  try
  {
    cfg.lookupValue("portName", config.PortName);
  }
  catch (const SettingNotFoundException &nfex)
  {
    syslog(LOG_EMERG, "'portName' setting is not found. Use default value %s.", config.PortName.c_str());
  }

  try
  {
    cfg.lookupValue("gpioPin", config.GpioPin);
  }
  catch (const SettingNotFoundException &nfex)
  {
    syslog(LOG_EMERG, "'gpioPin' setting is not found. Use default value %d.", config.GpioPin);
  }

  try
  {
    cfg.lookupValue("spiTxName", config.SpiTxName);
  }
  catch (const SettingNotFoundException &nfex)
  {
    syslog(LOG_EMERG, "'spiTxName' setting is not found. Use default value %s.", config.SpiTxName.c_str());
  }

  try
  {
    cfg.lookupValue("spiRxName", config.SpiRxName);
  }
  catch (const SettingNotFoundException &nfex)
  {
    syslog(LOG_EMERG, "'spiRxName' setting is not found. Use default value %s.", config.SpiRxName.c_str());
  }

  // initialization -------------------------------------------------------------------------

  CCanSocket canSocket(config.PortName.c_str());
  CSpidevSpi spiTx(config.SpiTxName.c_str(), config.SpiSpeed);
  CSpidevSpi spiRx(config.SpiRxName.c_str(), config.SpiSpeed);
  spiRx.Open(0, false, 8);
  spiTx.Open(0, false, 8);
  CSysfsGpio gpio(config.GpioPin);
  gpio.SetDir(0);
  gpio.setEdge(ISysfsGpio::edgeFalling);

  Spi2Can spi2Can(config.CpuId);
  spi2Can.start(canSocket, spiTx, spiRx, gpio);
  // -----------------------------------------------------------------------------------------
  // should never get here
  return 0;
}