#include "SysfsGpio.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char CSysfsGpio::EDGE_NONE[] = "none";
const char CSysfsGpio::EDGE_FALLING[] = "falling";
const char CSysfsGpio::EDGE_RISING[] = "rising";
const char CSysfsGpio::EDGE_BOTH[] = "both";
const char CSysfsGpio::DIRECTION_IN[] = "in";
const char CSysfsGpio::DIRECTION_OUT[] = "out";

CSysfsGpio::CSysfsGpio(unsigned int pin) : m_Pin(pin)
{

  // Check if already exported
  char buf[48];
  snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d", m_Pin);
  if (access(buf, F_OK) != 0)
  {
    exportPin();
  }

  snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", m_Pin);
  while (access(buf, W_OK) == -1)
    ;
  // TODO: need timeout while waiting

  m_ValueFd = open(buf, O_RDWR);

  if (m_ValueFd < 0)
  {
    printf("open error\n");
  }
  lseek(m_ValueFd, 0, SEEK_SET);

  m_Dir = getRealDir();

  m_PollFd.fd = m_ValueFd;
  m_PollFd.events = POLLPRI;
  m_PollFd.revents = 0;
}

CSysfsGpio::~CSysfsGpio()
{
  // StopWatcher();
}

int CSysfsGpio::GetLastErrno()
{
  return m_LastErrno;
}

void CSysfsGpio::Write(bool on)
{
  if (m_Dir == 0)
    return;

  char buf[2];
  buf[0] = on ? '1' : '0';
  buf[1] = '\0';
  lseek(m_ValueFd, 0, SEEK_SET);
  if (write(m_ValueFd, buf, 1) != 1)
  {
    printf("write error\n");
    m_LastErrno = errno;
    return;
  }
}

bool CSysfsGpio::Read()
{
  char buf[2];
  lseek(m_ValueFd, 0, SEEK_SET);
  if (read(m_ValueFd, buf, sizeof(buf)) != sizeof(buf))
  {
    printf("read error\n");
    m_LastErrno = errno;
    return false;
  }

  return (buf[0] != '0');
}

void CSysfsGpio::SetDir(int dir)
{
  if (dir && (getEdge() != edgeNone))
  {
    setEdge(edgeNone);
  }

  char directionStr[48];
  snprintf(directionStr, sizeof(directionStr),
           "/sys/class/gpio/gpio%d/direction", m_Pin);

  int fd = open(directionStr, O_WRONLY);
  if (fd < 0)
  {
    printf("set pin direction open error\n");
    m_LastErrno = errno;
    return;
  }

  if (dir)
  {
    if (write(fd, DIRECTION_OUT, sizeof(DIRECTION_OUT)) != sizeof(DIRECTION_OUT))
    {
      printf("set pin direction error\n");
      m_LastErrno = errno;
      return;
    }
  }
  else
  {
    if (write(fd, DIRECTION_IN, sizeof(DIRECTION_IN)) != sizeof(DIRECTION_IN))
    {
      printf("set pin direction error\n");
      m_LastErrno = errno;
      return;
    }
  }

  m_Dir = dir;
}

int CSysfsGpio::GetDir()
{
  return m_Dir;
}

int CSysfsGpio::getRealDir()
{
  char directionStr[48];
  snprintf(directionStr, sizeof(directionStr),
           "/sys/class/gpio/gpio%d/direction", m_Pin);

  int fd = open(directionStr, O_RDONLY);
  if (fd < 0)
  {
    printf("get real dir open error\n");
    m_LastErrno = errno;
    return -1;
  }

  lseek(fd, 0, SEEK_SET);

  char buf[8] = "";
  if (read(fd, buf, sizeof(buf)) < 0)
  {
    printf("get real dir error\n");
    m_LastErrno = errno;
    return -1;
  }

  if (memcmp(buf, DIRECTION_IN, sizeof(DIRECTION_IN) - 1) == 0)
  {
    return 0;
  }
  else if (memcmp(buf, DIRECTION_OUT, sizeof(DIRECTION_OUT) - 1) == 0)
  {
    return 1;
  }

  return -1;
}

void CSysfsGpio::exportPin()
{
  int fd = open("/sys/class/gpio/export", O_WRONLY);
  if (fd < 0)
  {
    printf("export open error\n");
    m_LastErrno = errno;
    return;
  }

  char pinStr[4];
  int pinStrLen = snprintf(pinStr, sizeof(pinStr), "%d", m_Pin);
  if (write(fd, pinStr, pinStrLen) != pinStrLen)
  {
    printf("export error\n");
    m_LastErrno = errno;
    close(fd);
    return;
  }
  close(fd);
}

void CSysfsGpio::unexportPin()
{
  int fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (fd < 0)
  {
    printf("unexport open error\n");
    m_LastErrno = errno;
    return;
  }

  char pinStr[4];
  int pinStrLen = snprintf(pinStr, sizeof(pinStr), "%d", m_Pin);
  if (write(fd, pinStr, pinStrLen) != pinStrLen)
  {
    printf("unexport error\n");
    m_LastErrno = errno;
    close(fd);
    return;
  }
  close(fd);
}

void CSysfsGpio::setEdge(CSysfsGpio::Edge edge)
{
  if (m_Dir)
    return;

  char edgeStr[48];
  snprintf(edgeStr, sizeof(edgeStr), "/sys/class/gpio/gpio%d/edge", m_Pin);

  int fd = open(edgeStr, O_WRONLY);
  if (fd < 0)
  {
    printf("set edge open error\n");
    m_LastErrno = errno;
    return;
  }

  switch (edge)
  {
  case edgeNone:
    if (write(fd, EDGE_NONE, sizeof(EDGE_NONE)) != sizeof(EDGE_NONE))
    {
      printf("set edgeNone error\n");
      m_LastErrno = errno;
      return;
    }
    break;

  case edgeFalling:
    if (write(fd, EDGE_FALLING, sizeof(EDGE_FALLING)) != sizeof(EDGE_FALLING))
    {
      printf("set edgeFalling error\n");
      m_LastErrno = errno;
      return;
    }
    break;

  case edgeRising:
    if (write(fd, EDGE_RISING, sizeof(EDGE_RISING)) != sizeof(EDGE_RISING))
    {
      printf("set edgeRising error\n");
      m_LastErrno = errno;
      return;
    }
    break;

  case edgeBoth:
    if (write(fd, EDGE_BOTH, sizeof(EDGE_BOTH)) != sizeof(EDGE_BOTH))
    {
      printf("set edgeBoth error\n");
      m_LastErrno = errno;
      return;
    }
    break;
  }
  m_Edge = edge;
}

CSysfsGpio::Edge CSysfsGpio::getEdge()
{
  return m_Edge;
}

CSysfsGpio::Edge CSysfsGpio::getRealEdge()
{
  char edgeStr[48];
  snprintf(edgeStr, sizeof(edgeStr), "/sys/class/gpio/gpio%d/edge", m_Pin);

  int fd = open(edgeStr, O_RDONLY);
  if (fd < 0)
  {
    printf("get edge open error\n");
    m_LastErrno = errno;
    return edgeError;
  }

  lseek(fd, 0, SEEK_SET);

  char buf[8];
  if (read(fd, buf, sizeof(buf)) < 0)
  {
    printf("get edge error\n");
    m_LastErrno = errno;
    return edgeError;
  }

  if (memcmp(buf, EDGE_NONE, sizeof(EDGE_NONE) - 1) == 0)
  {
    return edgeNone;
  }
  else if (memcmp(buf, EDGE_FALLING, sizeof(EDGE_FALLING) - 1) == 0)
  {
    return edgeFalling;
  }
  else if (memcmp(buf, EDGE_RISING, sizeof(EDGE_RISING) - 1) == 0)
  {
    return edgeRising;
  }
  else if (memcmp(buf, EDGE_BOTH, sizeof(EDGE_BOTH) - 1) == 0)
  {
    return edgeBoth;
  }

  return edgeError;
}

int CSysfsGpio::WaitForEdge(CSysfsGpio::Edge edge, int timeoutMs)
{
  if (edge != m_Edge)
    return -1;
  int ret = poll(&m_PollFd, 1, timeoutMs);
  return Read();
}

// void CSysfsGpio::StartWatcher(CSysfsGpio::Edge edge, const CSysfsGpio::TCallback& cb)
// {
// 	m_EdgeCallback = cb;
// 	setEdge(edge);

// 	Read();

// 	printf("Hello\n");

// 	m_IsWatcherRunning = true;
// 	m_WatcherThread = std::thread(&CSysfsGpio::watcherThread, this);
// }

// void CSysfsGpio::StopWatcher()
// {
// 	if (m_IsWatcherRunning)
// 	{
// 		m_IsWatcherRunning = false;
// 		m_WatcherThread.join();
// 	}
// }

// void CSysfsGpio::watcherThread()
// {
// 	while (m_IsWatcherRunning)
// 	{
// 		int ret = poll(&m_PollFd, 1, 500);

// 		if (ret < 0)
// 		{
// 			// Error
// 			continue;
// 		}
// 		else if (ret == 0)
// 		{
// 			// Timed-out
// 			continue;
// 		}
// 		else if ((ret == 1) && (m_PollFd.revents & POLLPRI))
// 		{
// 			if (m_EdgeCallback) m_EdgeCallback(Read());
// 		}
// 	}
// }
