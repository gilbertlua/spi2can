#include "SimSysfsGpio.h"
#include <thread> 
#include <chrono>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


SimSysfsGpio::SimSysfsGpio(unsigned int pin)
{
	
}

SimSysfsGpio::~SimSysfsGpio()
{
	// StopWatcher();
}

int SimSysfsGpio::WaitForEdge(Edge edge, int timeoutMs)
{
	// if(edge != m_Edge)
	// return -1;
	// int ret = poll(&m_PollFd, 1, timeoutMs);
	// return Read();

    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return 0;
}