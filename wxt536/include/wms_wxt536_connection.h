/* wms_wxt536_connection.h
 */
#ifndef WMS_WXT536_CONNECTION_H
#define WMS_WXT536_CONNECTION_H
#include "wms_serial_serial.h"

extern int Wms_Wxt536_Connection_Open(char *class,char *source,char *device_name);
extern int Wms_Wxt536_Connection_Close(char *class,char *source);

/* external variables */
extern Wms_Serial_Handle_T Wms_Wxt536_Serial_Handle;

#endif
