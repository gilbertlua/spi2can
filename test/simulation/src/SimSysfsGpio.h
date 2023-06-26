#ifndef SIMSYSFSGPIO_H
#define SIMSYSFSGPIO_H

#include "../../../src/ISysfsGpio.h"
#include <sys/poll.h>
#include <cstdint>
#include <vector>
#include <functional>
// #include <atomic>
// #include <thread>
// #include <mutex>

class SimSysfsGpio : public ISysfsGpio
{
public:

	SimSysfsGpio(unsigned int pin);
	~SimSysfsGpio();
	int WaitForEdge( Edge edge, int timeoutMs=-1) override;

};
#endif // SIMSYSFSGPIO_H_