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

/* internal data types */
/**
 * Data structure holding local data for the server module.
 * <dl>
 * <dt>Read_Sensor_Callback</dt> <dd>The callback the server invokes when it receives a 'Read Sensor' request.</dd>
 * <dt>Send_Result_Callback</dt> <dd>The callback the server invokes when it receives a 'Send Result' request.</dd>
 * </dl>
 */
struct Server_Struct
{
	Read_Sensor_Callback_T Read_Sensor_Callback;
	Send_Result_Callback_T Send_Result_Callback;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The instance of the server data used by the server.
 * @see #Server_Struct
 */
static struct Server_Struct Server_Data;

/* ==========================================
** external functions 
** ========================================== */
/**
 * Routine to set the callback to be invoked by the server when it receives a 'Read Sensor' request.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param callback A function pointer of type Read_Sensor_Callback_T.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Server_Data
 */
int Wms_Qli50_Server_Set_Read_Sensor_Callback(char *class,char *source,Read_Sensor_Callback_T callback)
{
	Server_Data.Read_Sensor_Callback = callback;
	return TRUE;
}

/**
 * Routine to set the callback to be invoked by the server when it receives a 'Send Result' request.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param callback A function pointer of type Send_Result_Callback_T.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Server_Data
 */
int Wms_Qli50_Server_Set_Send_Result_Callback(char *class,char *source,Send_Result_Callback_T callback)
{
	Server_Data.Send_Result_Callback = callback;
	return TRUE;
}

/**
 * Routine to start the server. In this case, open the connection to the specified serial device.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param device_name A string representing the device name of the serial device to open e.g. '/dev/ttyS0'.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see wms_wxt536_connection.html#Wms_Qli50_Serial_Handle
 * @see wms_wxt536_connection.html#Wms_Qli50_Connection_Open
 * @see wms_wxt536_general.html#Wms_Qli50_Log
 * @see wms_wxt536_general.html#Wms_Qli50_Log_Format
 */
int Wms_Qli50_Server_Start(char *class,char *source,char *device_name)
{
	int retval;
	
	retval = Wms_Qli50_Connection_Open(class,source,device_name);
	if(retval == FALSE)
		return FALSE;
	return TRUE;
}

int Wms_Qli50_Server_Loop(char *class,char *source)
{
	int done;

	
	return TRUE;
}
