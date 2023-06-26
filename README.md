# Formulatrix Spi2can Daemon (fmlx-spi2can)

## How to build

fmlx-spi2can daemon requires libconfig is installed on the system in compile time.
to install libconfig, execute this following command:
```sh
sudo apt-get install libconfig++-dev
```

if libconfig is already installed, execute this following commands:

```sh
mkdir build && cd build
cmake ..
make
```

## How to Install

fmlx-spi2can also deployed on Formulatrix apt repository. Please follow the instructions on [This Wiki](https://openproject.formulatrix.com/projects/general-r-d/wiki/debian-packages) if you have not add Formulatrix apt repository on your Raspberry Pi.

then execute this following comands.
```sh
sudo apt-get update
sudo apt-get install fmlx-spi2can
```

fmlx-spi2can daemon requires the application is installed as systemd service to make sure the fmlx-spi2can is running before the application service started.

then add _After=fmlx-spi2can.service_ and _Requires=fmlx-spi2can.service_ on your application .service file.
```sh
[Unit]
...
After=fmlx-spi2can.service
Requires=fmlx-spi2can.service
...
```

then restart your service, for example your service name is application.service.
```sh
sudo systemctl restart application.service
```


## Configuration

| Config Key | Default Value | Meaning 
| ------ | ------ | ------ |
| spiSpeed | 16000000 | SPI speed (int).
| cpuId | -1 | Rx CPU ID (int, -1 means that Rx thread will not be pinned to one CPU).
| portName | vcan0 | Interface Port Name.
| gpioPin | 1 | Gpio pin.
| spiTxName | /dev/spidev0.0 | Spi TX path.
| spiRxName | /dev/spidev0.1 | Spi RX path.

How to change the configuration:

Edit the configuration file
```sh
sudo nano /etc/opt/fmlx-spi2can/fmlx-spi2can.conf
```

Restart the fmlx-spi2can
```sh
sudo systemctl restart fmlx-spi2can
```
Done
