/* wms_qli50_connection.h
 */
#ifndef WMS_QLI50_CONNECTION_H
#define WMS_QLI50_CONNECTION_H
#include "wms_serial_serial.h"

extern int Wms_Qli50_Connection_Open(char *class,char *source,char *device_name);
extern int Wms_Qli50_Connection_Close(char *class,char *source);

/* external variables */
extern Wms_Serial_Handle_T Wms_Qli50_Serial_Handle;

#endif
