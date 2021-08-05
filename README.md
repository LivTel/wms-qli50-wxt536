# wms-wxt536

This is a source code repository for the Liverpool Telescope project. Here we are trying to interface a new weather station (Vaisala WXT536 Compact Weather Station) to our QNX4 Wms process (part of the QNX software stack that feeds weather data into the telescope systems). The Wms currently communicates with our old weather station which uses a Vaisala QLI50 to colect the weather data.

Therefore this software is designed to look like a Vaisala QLI50 at the front end, so our current Wms can talk to it. It then talks to the Vaisala WXT536 hardware at the back end. We decided to write this intermediate piece of software, rather than re-write the Wms, as we don't have a QNX4 development system at the present time.

