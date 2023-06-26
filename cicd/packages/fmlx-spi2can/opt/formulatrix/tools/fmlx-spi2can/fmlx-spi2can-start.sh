#!/usr/bin/env bash

EXEC_CMD="/opt/formulatrix/tools/fmlx-spi2can/fmlx-spi2can"
CONF_FILE="/etc/opt/fmlx-spi2can/fmlx-spi2can.conf"  
PORTNAME="vcan0"
IFTYPE="vcan"

# Read configuration file
if [ -f ${CONF_FILE} ]; then
  source "${CONF_FILE}"
  if [ "${PortName}" != "" ]; then
    PORTNAME=${PortName}
  fi
  if [ "${ifType}" != "" ]; then
    IFTYPE=${ifType}
  fi
fi

sudo modprobe ${IFTYPE}
sudo ip link add dev ${PORTNAME} type ${IFTYPE}
sudo ip link set up ${PORTNAME}

# Run spi2can daemon
echo "Run command: ${EXEC_CMD}"
${EXEC_CMD}&