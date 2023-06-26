#ifndef ISYSFSGPIO_H
#define ISYSFSGPIO_H

class ISysfsGpio
{
public:
  enum Edge
  {
    edgeError = -1,
    edgeNone = 0,
    edgeFalling,
    edgeRising,
    edgeBoth
  };

  virtual int WaitForEdge(Edge edge, int timeoutMs = -1) = 0;
  virtual ~ISysfsGpio(){};
};
#endif // ISYSFSGPIO_H