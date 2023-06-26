#include "../../../src/CanSocket.h"
#include "../../../src/Spi2Can.h"
#include "../../../src/SysfsGpio.h"
#include "SimSpidevSpi.h"
#include "SimSpidevSpiTx.h"

#define SPI_FRAME_SIZE 12
#define POWER_MANAGEMENT_ADDRESS 0x577


static void print_usage(char *prg)
{
	fprintf(stderr, "Usage: %s [Options]\n"
		"\n"
		"spi2can simulation, this app doesn't require SPI hardware"
		"\n"
		"Options:\n"
		" -c		cpu id of the receive thread min/default : 0, max : %d. CpuId -1 means skip to pinning a cpu id.\n"
		" -s		the spi frequency, default: 16000000\n"
		" -h		this help\n",
		prg, std::thread::hardware_concurrency());
}


int main(int argc, char *argv[])
{
    int opt;
    int cpu_id = 0;
    int cpu_count = std::thread::hardware_concurrency();
    uint32_t spiSpeed = 16000000;
    while( (opt = getopt( argc, argv, "s:c:h")) != -1 )
    {
        switch(opt)
        {
            case 's':
            {
                spiSpeed = std::atoi(optarg);
                std::cout << "set spi speed to " << spiSpeed << "\n";
                break;
            }
            case 'c':
            {
                cpu_id = std::atoi(optarg);
                if( cpu_id < -1 || cpu_id > cpu_count)
                {
                    fprintf( stderr, "%d is not in cpu id range!\n\n", cpu_id);
                    print_usage(argv[0]);
                    exit(0);
                    break;
                }
                std::cout << "set vcan tx thread cpu id to " << cpu_id << "\n";
                break;
            }
            case 'h':
            {
                print_usage(basename(argv[0]));
                exit(EXIT_SUCCESS);
                break;
            }
        }
    }
    // initialization -------------------------------------------------------------------------
    std::cout << "FMLX SPI2VCAN Daemon Init\n";
    std::cout << "SPI FREQ = " << spiSpeed << "\n";

    CCanSocket canSocket(CCanSocket::ctVcan, 0);

    SimSpidevSpiTx spiTx("simulation", 0);
    SpidevSpi spiRx("/dev/spidev0.1", spiSpeed);
    SysfsGpio gpio(1);

    Spi2Can spi2Can(cpu_id);
    spi2Can.start( canSocket, spiTx, spiRx, gpio );
    // should never get here
    return 0;
}