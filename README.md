# autd

This repository contains driver softwares of Airborne Ultrasound Tactile Display (AUTD).
[日本語README](README_ja.md)

Please see here for the details of this technology, which we have developed since 2008.
http://www.hapis.k.u-tokyo.ac.jp/?page_id=447&lang=en

## Getting Hardware

### Compatible Device Suppliers

* Shinko Shoji Co.,Ltd. < http://www.shinko-sj.co.jp/ >

## General Rules

### Units

Unless otherwise noted, units of physical values are as follows:

* Length - millimeter[mm]
* Angle  - radian[rad]
* Time   - millisecond[msec]

## Installation for application developers

### Server

* TwinCAT3
* Visual Studio 2013

To run AUTD Server TwinCAT3 Engineering Environment and Visual Studio 2013 is required. Please note that this server program is strictly sensitive to the version of Visual Studio (2013) and doesn't work on any other version.

http://www.beckhoff.com/english.asp?download/tc3-download-xae.htm

After installing TwinCAT3, run `dist/AUTDServer/Install.bat` to install autd device driver.

#### Tested Compatible Network Interface Card

For realtime network and precise clock compensation, EtherCAT requires specific NICs manufactured by Intel. This is the NIC list we tested.

* Intel 82574L
* Intel 82579V
* Intel I218LM

See also official list by Beckhoff but it seems to be outdated.
http://infosys.beckhoff.com/english.php?content=content/1033/tcsystemmanager/reference/ethercat/html/ethercat_supnetworkcontroller.htm

### Client (API)

* Eigen3
* Boost (for compiling source)
* CMake > 3.0.0 (for compiling source)

Tested on:

* Windows 8/8.1
* Windows 10
* MacOS X 10.11.6

## How to use

1. Connect AUTD Devices with a server machine by category 5e or superior RJ45 cable. Make sure to plug CN10 in PC side and CN11 in peripheral side serially.  CN9 (the most internal one) is not for EtherCAT.

2. Supply 24V power to AUTD devices. A device consumes up to 2A.

3. Run AUTD Server startup program `dist/AUTDServer/AUTDServer.exe` to configure the Server. The program scans connected devices and configures them. If you asked an IP address, enter an IPv4 address of a host to run a client software. Either localhost or over-ip remotehost is acceptable. Currently only single remotehost can be configured by the server simultaneously. If you would like to run multiple clients simultaneously, modify the routing table of TwinCAT ADS manually. When connecting from remotehost, configure your firewall to allow incoming 48898-TCP and 48899-UDP at the server host.

4. Run a client program at a local/remote machine which is specified above. See example source codes and `client/README.md` for a guide to develop your application.

## Citing

If you use this SDK in your research please consider to include the following citation in your publications:

* S. Inoue, Y. Makino and H. Shinoda "Scalable Architecture for Airborne Ultrasound Tactile Display", Asia Haptics 2016

## Contributing

We always appreciate your bug reports, suggestions and pull requests. If you are interested in improving firmwares on CPU and FPGA, please let me know and will invite you to our internal repository.

## Links

Lab: 
http://www.hapis.k.u-tokyo.ac.jp

Youtube Channel:
https://www.youtube.com/user/ShinodaLab
