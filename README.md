# wms-qli50-wxt536

This is a source code repository for the Liverpool Telescope project. Here we are trying to interface a new weather station (Vaisala WXT536 Compact Weather Station) to our QNX4 Wms process (part of the QNX software stack that feeds weather data into the telescope systems). The Wms currently communicates with our old weather station which uses a Vaisala QLI50 to colect the weather data.

Therefore this software is designed to look like a Vaisala QLI50 at the front end, so our current Wms can talk to it. It then talks to the Vaisala WXT536 hardware at the back end. We decided to write this intermediate piece of software, rather than re-write the Wms, as we don't have a QNX4 development system at the present time.

The serial directory contains a low-level serial library (libwms-qli50-wxt536_serial.so) that is used by the higher level libraries.

The wxt536 directory contains the code and test programs for the library that handles communication with the Vaisala Wxt536 (libwms-qli50-wxt536_wxt536.so).

The qli50 directory contains code for a server that waits for commands on a open serial port, and either returns a fake reply or invokes a callback to create a reply. It also has a client-side implementation of the command set (subset supported by the LT Wms) and test progrems.

The c and include directories contain the software for the main program. This instantiates a Qli50 server on a serial port, and when asked for weather data uses the wxt536 command set to retrieve and return it.

## Dependencies

The repo is configured to sit in an LT standard /home/dev/ environment. It uses the following LT packages:

- log_udp
- commandserver
- estar/config


