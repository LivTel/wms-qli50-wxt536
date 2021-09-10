/* wms_wxt536_connection.c
** Weather monitoring system (qli50 -> wxt536 conversion), Vaisala Wxt536 interface library, 
** connection (open/close) routines.
*/
/**
 * Routines to open and close the connection to the Vaisala Wxt536.
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
#include <termios.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include "log_udp.h"
#include "wms_serial_serial.h"
#include "wms_wxt536_general.h"
#include "wms_wxt536_connection.h"

/* external variables */
/**
 * The handle we use (via the Wms Serial library) to open and close the serial port connection.
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Handle_T
 */
Wms_Serial_Handle_T Wms_Wxt536_Serial_Handle;

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* external functions */
/**
 * Open the connection to the specified serial port using the Wms serial library.
 * We assume the device has the default connection settings, 19200 baud, 8N1.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param device_name The device filename (i.e. /dev/ttyS0) that the Wxt536 is connected to.
 * @return We return TRUE if the connection is opened successfully, FALSE otherwise.
 * @see #Wms_Wxt536_Serial_Handle
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see ../../serial/cdocs/wms_serial_serial.html#WMS_SERIAL_DEVICE_NAME_STRING_LENGTH
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Open
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Baud_Rate_Set
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Input_Flags_Set
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Output_Flags_Set
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Control_Flags_Set
 */
int Wms_Wxt536_Connection_Open(char *class,char *source,char *device_name)
{
#if LOGGING > 0
	Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Wxt536_Connection_Open(%s).",device_name);
#endif /* LOGGING */
	/* setup device_name */
	if(device_name == NULL)
	{
		Wms_Wxt536_Error_Number = 1;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Connection_Open:device_name was NULL.");
		return FALSE;
	}
	if(strlen(device_name) >= WMS_SERIAL_DEVICE_NAME_STRING_LENGTH)
	{
		Wms_Wxt536_Error_Number = 2;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Connection_Open:device_name was too long (%lu vs %d).",
			strlen(device_name),WMS_SERIAL_DEVICE_NAME_STRING_LENGTH);
		return FALSE;
	}
	strcpy(Wms_Wxt536_Serial_Handle.Device_Name,device_name);
	/* set baud rate to B19200 */
	if(!Wms_Serial_Baud_Rate_Set(B19200))
	{
		Wms_Wxt536_Error_Number = 3;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Connection_Open:Failed to configure baud rate to B19200.");
		return FALSE;
	}
	/* Configure input flags to ignore parity bits */
	if(!Wms_Serial_Input_Flags_Set(IGNPAR))
	{
		Wms_Wxt536_Error_Number = 4;
		sprintf(Wms_Wxt536_Error_String,
			"Wms_Wxt536_Connection_Open:Failed to configure input flags to IGNPAR.");
		return FALSE;
	}
	if(!Wms_Serial_Output_Flags_Set(0))
	{
		Wms_Wxt536_Error_Number = 5;
		sprintf(Wms_Wxt536_Error_String,
			"Wms_Wxt536_Connection_Open:Failed to configure output flags to 0.");
		return FALSE;
	}
	if(!Wms_Serial_Control_Flags_Set(CS8|CLOCAL|CREAD))
	{
		Wms_Wxt536_Error_Number = 6;
		sprintf(Wms_Wxt536_Error_String,
			"Wms_Wxt536_Connection_Open:Failed to configure control flags to CS8|CLOCAL|CREAD (8N1).");
		return FALSE;
	}
	if(!Wms_Serial_Open(class,source,&Wms_Wxt536_Serial_Handle))
	{
		Wms_Wxt536_Error_Number = 7;
		sprintf(Wms_Wxt536_Error_String,
			"Wms_Wxt536_Connection_Open:Wms_Serial_Open failed to open connection on '%s'.",device_name);
		return FALSE;
	}
#if LOGGING > 0
	Wms_Wxt536_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Wxt536_Connection_Open:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to close the connection previously opened by Wms_Wxt536_Connection_Open.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @return We return TRUE if the connection is opened successfully, FALSE otherwise.
 * @see #Wms_Wxt536_Serial_Handle
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Close
 */
int Wms_Wxt536_Connection_Close(char *class,char *source)
{
#if LOGGING > 0
	Wms_Wxt536_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Wxt536_Connection_Close:Started.");
#endif /* LOGGING */
	if(!Wms_Serial_Close(class,source,&Wms_Wxt536_Serial_Handle))
	{
		Wms_Wxt536_Error_Number = 8;
		sprintf(Wms_Wxt536_Error_String,
			"Wms_Wxt536_Connection_Close:Wms_Serial_Close failed to close connection.");
		return FALSE;
	}
#if LOGGING > 0
	Wms_Wxt536_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Wxt536_Connection_Close:Finished.");
#endif /* LOGGING */
	return TRUE;
}
