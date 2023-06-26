#!/usr/bin/env bash

CONF_FILE="/etc/opt/fmlx-spi2can/fmlx-spi2can.conf"
IF_NAME="vcan0"

if [ -f ${CONF_FILE} ]; then
  source "${CONF_FILE}"
  if [ "${PortName}" != "" ]; then
    IF_NAME=${PortName}
  fi
fi

isInterfaceUp=`sudo ip link | grep ${IF_NAME}`
if [ -n "${isInterfaceUp}" ]; then
  sudo ip link set down ${IF_NAME}
  sudo ip link delete ${IF_NAME}
fi