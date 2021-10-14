/* wms_qli50_server.c
** Weather monitoring system (qli50 -> wxt536 conversion), Vaisala Qli50 interface library, 
** Software to start and run a server, which receives Qli50 commands over a serial link from a client, and
** either fakes a reply or invokes a function to handle the command implementation.
*/
/**
 * Software to start and run a server, which receives Qli50 commands over a serial link from a client, and
 * either fakes a reply or invokes a function to handle the command implementation.
 * @author Chris Mottram
 * @version $Revision$
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L
#include <errno.h>   /* Error number definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_udp.h"
#include "wms_serial_serial.h"
#include "wms_qli50_command.h"
#include "wms_qli50_connection.h"
#include "wms_qli50_general.h"
#include "wms_qli50_server.h"

/* ==========================================
** external functions 
** ========================================== */
int Wms_Qli50_Server_Set_Read_Sensor_Callback(Read_Sensor_Callback callback)
{
	return TRUE;
}

int Wms_Qli50_Server_Set_Send_Result_Callback(Send_Result_Callback callback)
{
	return TRUE;
}

int Wms_Qli50_Server_Start(char *device_name)
{
	return TRUE;
}

int Wms_Qli50_Server_Loop(void)
{
	return TRUE;
}
