#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <endian.h>
#include <iomanip>
#include <iostream>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#ifndef FMLX_CANSOCKET_HEADER
#define FMLX_CANSOCKET_HEADER

class CCanSocket
{
public:
  enum ChannelType
  {
    ctVcan = 0,
    ctCan = 1,
    ctSlcan = 2
  };
  struct CanSocketFrame
  {
    int32_t flag;
    int32_t messageId;
    int32_t status;
    int32_t dlc;
    int64_t time;
    __u8 canData[8];
  };
  struct Can_Socket_Buffer
  {
    struct msghdr msg;
    struct iovec iov;
    char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
    struct can_frame frame;
  };
  struct Socket_Desc
  {
    char *PortName;
    sockaddr_can addr;
    int sockfd;
    struct Can_Socket_Buffer *pRx;
    struct Can_Socket_Buffer *pTx;
  };

public:
  CCanSocket(ChannelType channel, int channelNum);
  CCanSocket(const char *PortName);
  virtual ~CCanSocket();
  bool Write(CanSocketFrame pFrameIn);
  void Read(CanSocketFrame *pFrameOut);
  int SetAcceptanceFilter(unsigned int can_id, unsigned int can_mask);
  int Close();

  void BusOn();
  void BusOff();
  void FlushReceiveQueue();
  void FlushTransmitQueue();

  void PrintCanFrame(CanSocketFrame frame);
  // extern "C" int Create(void *pHandle);

private:
  void initMessageBuffer(Can_Socket_Buffer *pBuffer, sockaddr_can *pAddr);
  void initMessageHeader(Socket_Desc *pHandle);
  void resetMessageHeader(Can_Socket_Buffer *pBuffer, sockaddr_can addr);
  int createSocket();

private:
  Socket_Desc *m_pHandle;
};
#endif