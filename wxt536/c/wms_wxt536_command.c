/* wms_wxt536_command.c
** Weather monitoring system (qli50 -> wxt536 conversion), Vaisala Wxt536 interface library, 
** command routines.
*/
/**
 * Routines to send commands (and parse their replies) to/from the Vaisala Wxt536.
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
#include "wms_wxt536_connection.h"
#include "wms_wxt536_general.h"

/* hash defines */
/**
 * The terminator used to delimit the end of each line read. <cr><lf> in this case.
 */
#define TERMINATOR_CRLF         ("\r\n")

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* external functions */
/**
 * Basic routine to send a string command to the Vaisala Wxt536 over a previously opened connection, 
 * and wait for a reply string.
 * @see #TERMINATOR_CRLF
 * @see wms_wxt536_connection.html#Wms_Wxt536_Serial_Handle
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Write
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Read_Line
 */
int Wms_Wxt536_Command(char *class,char *source,char *command_string,char *reply_string,int reply_string_length)
{
	char message[256];
	int bytes_read;
	
	if(command_string == NULL)
	{
		Wms_Wxt536_Error_Number = 100;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Command String was NULL.");
		return FALSE;
		
	}
	if(strlen(command_string) >= 256)
	{
		Wms_Wxt536_Error_Number = 101;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Command String was too long (%lu vs 255).",
			strlen(command_string));
		return FALSE;
		
	}
#if LOGGING > 9
	Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Wxt536_Command(%s) started.",command_string);
#endif /* LOGGING */
	strcpy(message,command_string);
	strcat(message,TERMINATOR_CRLF);
	if(!Wms_Serial_Write(class,source,Wms_Wxt536_Serial_Handle,message,strlen(message)))
	{
		Wms_Wxt536_Error_Number = 102;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Failed to write command string '%s'.",
			command_string);
		return FALSE;
	}
	/* read any reply */
	if(reply_string != NULL)
	{
		if(!Wms_Serial_Read_Line(class,source,Wms_Wxt536_Serial_Handle,TERMINATOR_CRLF,message,255,&bytes_read))
		{
			Wms_Wxt536_Error_Number = 103;
			sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Failed to read reply line.");
			return FALSE;
		}
		message[bytes_read] = '\0';
		if(strlen(message) >= reply_string_length)
		{
			Wms_Wxt536_Error_Number = 104;
			sprintf(Wms_Wxt536_Error_String,
				"Wms_Wxt536_Command:Reply message was too long to fit into reply string (%lu vs %d).",
				strlen(message),reply_string_length);
			return FALSE;
		}
		strcpy(reply_string,message);
#if LOGGING > 9
		Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Wxt536_Command(%s) returned reply '%s'.",
				      command_string,reply_string);
#endif /* LOGGING */
	}/* end if reply string was not NULL */
#if LOGGING > 9
	Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Wxt536_Command(%s) finished.",command_string);
#endif /* LOGGING */
	return TRUE;
}
