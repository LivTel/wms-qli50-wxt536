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
#include "wms_serial_general.h"
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
 * <dt>Par_Callback</dt> <dd>The callback the server invokes when it receives a 'PAR' request.</dd>
 * <dt>Sta_Callback</dt> <dd>The callback the server invokes when it receives a 'STA' request.</dd>
 * </dl>
 */
struct Server_Struct
{
	Read_Sensor_Callback_T Read_Sensor_Callback;
	Send_Result_Callback_T Send_Result_Callback;
	Par_Callback_T Par_Callback;
	Sta_Callback_T Sta_Callback;
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
 * Routine to set the callback to be invoked by the server when it receives a 'STA' request.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param callback A function pointer of type Sta_Callback_T.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Server_Data
 */
int Wms_Qli50_Server_Set_Sta_Callback(char *class,char *source,Sta_Callback_T callback)
{
	Server_Data.Sta_Callback = callback;
	return TRUE;
}

/**
 * Routine to set the callback to be invoked by the server when it receives a 'PAR' request.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param callback A function pointer of type Par_Callback_T.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Server_Data
 */
int Wms_Qli50_Server_Set_Par_Callback(char *class,char *source,Par_Callback_T callback)
{
	Server_Data.Par_Callback = callback;
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

/**
 * Routine to sit on the open serial port, reading lines of data (commands) terminatred by the terminator,
 * and based on the command returning a fake reply, or invoking a callback, or printing an error to stderr.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see wms_wxt536_command.html#TERMINATOR_CR
 * @see wms_wxt536_connection.html#Wms_Qli50_Serial_Handle
 * @see wms_wxt536_general.html#Wms_Qli50_Log
 * @see wms_wxt536_general.html#Wms_Qli50_Log_Format
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Read_Line
 */
int Wms_Qli50_Server_Loop(char *class,char *source)
{
	char command_message_string[256];
	char reply_message_string[256];
	char parameter_string[32];
	char parameter_char;
	int done,retval,bytes_read;

	done = FALSE;
	while(done == FALSE)
	{
		retval = Wms_Serial_Read_Line(class,source,Wms_Qli50_Serial_Handle,TERMINATOR_CR,
					      command_message_string,255,&bytes_read);
		if(retval)
		{
			if(strstr(command_message_string,"CLOSE") != NULL)
			{
				sprintf(reply_message_string,"LINE CLOSED%s",TERMINATOR_CR);
				retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
							  strlen(reply_message_string));
				if(retval == FALSE)
				{
					Wms_Serial_Error();
				}
			}/* end if command was "CLOSE" */
			else if(strstr(command_message_string,"ECHO") != NULL)
			{
				/* parse parameter */
				retval = sscanf(command_message_string,"ECHO %31s",parameter_string);
				if(retval == 1)
				{
					/* construct reply */
					sprintf(reply_message_string,"ECHO %s%s",parameter_string,TERMINATOR_CR);
					retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
								  strlen(reply_message_string));
					if(retval == FALSE)
					{
						Wms_Serial_Error();
					}
				}
				else
				{
					Wms_Qli50_Error_Number = 200;
					sprintf(Wms_Qli50_Error_String,
						"Wms_Qli50_Server_Loop:Failed to parse ECHO command '%s' (%d).",
						command_message_string,retval);
					Wms_Qli50_Error();
				}
			}/* end if command was "ECHO" */
			else if(strstr(command_message_string,"OPEN") != NULL)
			{
				/* parse parameter */
				retval = sscanf(command_message_string,"OPEN %c",&parameter_char);
				if(retval == 1)
				{
					/* construct reply */
					sprintf(reply_message_string,"OPEN %c%s",parameter_char,TERMINATOR_CR);
					retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
								  strlen(reply_message_string));
					if(retval == FALSE)
					{
						Wms_Serial_Error();
					}
				}
				else
				{
					Wms_Qli50_Error_Number = 201;
					sprintf(Wms_Qli50_Error_String,
						"Wms_Qli50_Server_Loop:Failed to parse OPEN command '%s' (%d).",
						command_message_string,retval);
					Wms_Qli50_Error();
				}
			}/* end if command was "OPEN" */
			if(strstr(command_message_string,"PAR") != NULL)
			{
				if(Server_Data.Par_Callback != NULL)
					Server_Data.Par_Callback(reply_message_string,254);
				else
					strcpy(reply_message_string,"PARAMETERS");
				strcat(reply_message_string,TERMINATOR_CR);
				retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
							  strlen(reply_message_string));
				if(retval == FALSE)
				{
					Wms_Serial_Error();
				}
			}/* end if command was "CLOSE" */
			
			else /* we don't know what this command is */
			{
				Wms_Qli50_Error_Number = 202;
				sprintf(Wms_Qli50_Error_String,
					"Wms_Qli50_Server_Loop:Failed to parse unknown command '%s'.",command_message_string);
				Wms_Qli50_Error();
			}
		}/* end if command was read successfully */
		else
		{
			Wms_Serial_Error();
		}
	}/* end while */
	return TRUE;
}
