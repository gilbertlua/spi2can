#include "CanSocket.h"

CCanSocket::CCanSocket(ChannelType channel, int channelNum)
{
  m_pHandle = (struct Socket_Desc *)malloc(sizeof(struct Socket_Desc));
  m_pHandle->pRx = (struct Can_Socket_Buffer *)malloc(sizeof(struct Can_Socket_Buffer));
  m_pHandle->pTx = (struct Can_Socket_Buffer *)malloc(sizeof(struct Can_Socket_Buffer));
  m_pHandle->PortName = (char *)malloc(IFNAMSIZ * sizeof(char));
  char buffer[5];
  sprintf(buffer, "%d", channelNum);
  char PortName[16];
  switch (channel)
  {
  case ctVcan:
    std::strncpy(PortName, "vcan\0", 5);
    break;

  case ctCan:
    std::strncpy(PortName, "can\0", 4);
    break;

  case ctSlcan:
    std::strncpy(PortName, "slcan\0", 6);
    break;
  }
  std::strncat(PortName, buffer, 5);
  std::strncat(PortName, "\0", 1);
  std::strncpy(m_pHandle->PortName, PortName, IFNAMSIZ);

  initMessageHeader(m_pHandle);
  if (createSocket() < 0)
  {
    printf("error create socket\n");
    // ~CCanSocket();
  }
}
CCanSocket::CCanSocket(const char *PortName)
{
  m_pHandle = (struct Socket_Desc *)malloc(sizeof(struct Socket_Desc));
  m_pHandle->pRx = (struct Can_Socket_Buffer *)malloc(sizeof(struct Can_Socket_Buffer));
  m_pHandle->pTx = (struct Can_Socket_Buffer *)malloc(sizeof(struct Can_Socket_Buffer));
  m_pHandle->PortName = (char *)malloc(IFNAMSIZ * sizeof(char));

  std::strncpy(m_pHandle->PortName, PortName, IFNAMSIZ);

  initMessageHeader(m_pHandle);
  if (createSocket() < 0)
  {
    printf("error create socket\n");
    // ~CCanSocket();
  }
}

CCanSocket::~CCanSocket()
{
  free(m_pHandle->pTx);
  free(m_pHandle->pRx);
  free(m_pHandle->PortName);
  free(m_pHandle);
}

void CCanSocket::resetMessageHeader(Can_Socket_Buffer *pBuffer, sockaddr_can addr)
{
  pBuffer->iov.iov_len = sizeof(pBuffer->frame);
  pBuffer->msg.msg_namelen = sizeof(addr);
  pBuffer->msg.msg_controllen = sizeof(pBuffer->ctrlmsg);
  pBuffer->msg.msg_flags = 0;
}

int CCanSocket::Close()
{
  close(m_pHandle->sockfd);
  return 0;
}

int CCanSocket::SetAcceptanceFilter(unsigned int can_id, unsigned int can_mask)
{
  int rc;
  struct can_filter filter[1];
  filter[0].can_id = can_id;
  filter[0].can_mask = can_mask;
  rc = ::setsockopt(
      m_pHandle->sockfd,
      SOL_CAN_RAW,
      CAN_RAW_FILTER,
      &filter,
      sizeof(filter));
  if (-1 == rc)
  {
    std::perror("setsockopt filter");
    Close();
    return -2;
  }
  return 0;
}

int CCanSocket::createSocket()
{
  int rc;
  struct sockaddr_can addr;
  struct ifreq ifr;
  // Open the CAN network interface
  m_pHandle->sockfd = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (-1 == m_pHandle->sockfd)
  {
    perror("socket");
    return -1;
  }
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 10000;
  rc = setsockopt(m_pHandle->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  if (-1 == rc)
  {
    perror("setsockopt recivetimeout");
    Close();
    return -3;
  }
  // Enable reception of CAN FD frames
  int enable = 0;
  rc = ::setsockopt(m_pHandle->sockfd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable, sizeof(enable));
  if (-1 == rc)
  {
    perror("setsockopt CAN FD");
    Close();
    return -3;
  }
  // Get the index of the network interface
  std::strncpy(ifr.ifr_name, m_pHandle->PortName, IFNAMSIZ);
  //std::cout<<"PortName "<<pHandle->PortName;
  //std::cout<<"fd"<<pHandle->sockfd;
  if (::ioctl(m_pHandle->sockfd, SIOCGIFINDEX, &ifr) == -1)
  {
    perror("ioctl");
    Close();
    return -4;
  }
  // Bind the socket to the network interface
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  rc = ::bind(
      m_pHandle->sockfd,
      reinterpret_cast<struct sockaddr *>(&addr),
      sizeof(addr));
  if (-1 == rc)
  {
    perror("bind");
    Close();
    return -5;
  }
  m_pHandle->addr = addr;
  return 0;
}

bool CCanSocket::Write(CanSocketFrame pFrameIn)
{
  struct can_frame *pFrame = &m_pHandle->pTx->frame;
  memset(pFrame, 0, sizeof(*pFrame));
  pFrame->can_id = pFrameIn.messageId;
  for (int i = 0; i < pFrameIn.dlc; i++)
  {
    pFrame->data[i] = pFrameIn.canData[i];
  }
  pFrame->can_dlc = pFrameIn.dlc;
  resetMessageHeader(m_pHandle->pTx, m_pHandle->addr);
  struct ifreq ifr;
  std::strncpy(ifr.ifr_name, m_pHandle->PortName, IFNAMSIZ);
  ifr.ifr_ifindex = m_pHandle->addr.can_ifindex;
  if (ioctl(m_pHandle->sockfd, SIOCGIFMTU, &ifr) == -1)
  {
    perror("mtu error");
    return false;
  }
  sendmsg(m_pHandle->sockfd, &m_pHandle->pTx->msg, MSG_DONTWAIT /*00*/);
  pFrameIn.status = 0;
  return true;
}

void CCanSocket::Read(CanSocketFrame *pFrameOut)
{
  struct Can_Socket_Buffer *pRx = m_pHandle->pRx;
  struct can_frame *frame = &pRx->frame;
  resetMessageHeader(pRx, m_pHandle->addr);
  int nbytes = recvmsg(m_pHandle->sockfd, &pRx->msg, MSG_WAITALL); //blocking
  pFrameOut->dlc = 0;
  pFrameOut->status = -1;
  if (nbytes >= 0)
  {
    pFrameOut->messageId = pRx->frame.can_id;
    pFrameOut->dlc = frame->can_dlc;
    for (int i = 0; i < pFrameOut->dlc; i++)
    {
      pFrameOut->canData[i] = pRx->frame.data[i];
    }
    pFrameOut->status = 0;
  }
}

void CCanSocket::initMessageBuffer(Can_Socket_Buffer *pBuffer, sockaddr_can *pAddr)
{
  pBuffer->iov.iov_base = &pBuffer->frame;
  pBuffer->msg.msg_name = pAddr;
  pBuffer->msg.msg_iov = &pBuffer->iov;
  pBuffer->msg.msg_iovlen = 1;
  pBuffer->msg.msg_control = &pBuffer->ctrlmsg;
}

void CCanSocket::initMessageHeader(Socket_Desc *pHandle)
{
  initMessageBuffer(pHandle->pTx, &pHandle->addr);
  initMessageBuffer(pHandle->pRx, &pHandle->addr);
}

void CCanSocket::BusOn()
{
}

void CCanSocket::BusOff()
{
}

void CCanSocket::FlushReceiveQueue()
{
}

void CCanSocket::FlushTransmitQueue()
{
}

void CCanSocket::PrintCanFrame(CanSocketFrame frame)
{
  printf("id: %d, ", frame.messageId);
  printf("dlc: %d, data: ", frame.dlc);
  for (int i = 0; i < frame.dlc; i++)
  {
    printf("%d, ", frame.canData[i]);
  }
  putchar(0x0a);
}