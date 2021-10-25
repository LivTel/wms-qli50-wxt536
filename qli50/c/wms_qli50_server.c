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

/* internal functions */
static int Server_Create_Send_Result_String(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct data,
					    char *message_string,int message_string_length);
static int Server_Add_Result_To_String(char *name,struct Wms_Qli50_Data_Value data_value,int add_comma,
					      char *message_string,int message_string_length);

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
	
#if LOGGING > 1
	Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Qli50_Server_Start: starting with device name '%s'.",device_name);
#endif /* LOGGING */
	retval = Wms_Qli50_Connection_Open(class,source,device_name);
	if(retval == FALSE)
		return FALSE;
#if LOGGING > 1
	Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Qli50_Server_Start: opened server port to device  '%s'.",device_name);
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to sit on the open serial port, reading lines of data (commands) terminatred by the terminator,
 * and based on the command returning a fake reply, or invoking a callback, or printing an error to stderr.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Server_Create_Send_Result_String
 * @see wms_wxt536_command.html#TERMINATOR_CR
 * @see wms_wxt536_command.html#Wms_Qli50_Data_Struct
 * @see wms_wxt536_connection.html#Wms_Qli50_Serial_Handle
 * @see wms_wxt536_general.html#Wms_Qli50_Log
 * @see wms_wxt536_general.html#Wms_Qli50_Log_Format
 * @see wms_wxt536_general.html#Wms_Qli50_Error_Number
 * @see wms_wxt536_general.html#Wms_Qli50_Error_String
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Error_Number
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Error
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Read_Line
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Write
 */
int Wms_Qli50_Server_Loop(char *class,char *source)
{
	struct Wms_Qli50_Data_Struct data;
	char command_message_string[256];
	char reply_message_string[256];
	char parameter_string[32];
	char parameter_char,qli_id,seq_id;
	int done,retval,bytes_read;

#if LOGGING > 1
	Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Qli50_Server_Loop: Start.");
#endif /* LOGGING */
	done = FALSE;
	while(done == FALSE)
	{
		retval = Wms_Serial_Read_Line(class,source,Wms_Qli50_Serial_Handle,TERMINATOR_CR,
					      command_message_string,255,&bytes_read);
		if(retval)
		{
#if LOGGING > 9
			Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Read command message string '%s'.",
					     command_message_string);
#endif /* LOGGING */
			if(strstr(command_message_string,"CLOSE") != NULL)
			{
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'CLOSE' command.");
#endif /* LOGGING */
				sprintf(reply_message_string,"LINE CLOSED%s",TERMINATOR_CR);
#if LOGGING > 9
				Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Reply String is '%s'.",
						     reply_message_string);
#endif /* LOGGING */
				retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
							  strlen(reply_message_string));
				if(retval == FALSE)
				{
					Wms_Serial_Error();
				}
			}/* end if command was "CLOSE" */
			else if(strstr(command_message_string,"ECHO") != NULL)
			{
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'ECHO' command.");
#endif /* LOGGING */
				/* parse parameter */
				retval = sscanf(command_message_string,"ECHO %31s",parameter_string);
				if(retval == 1)
				{
					/* construct reply */
					sprintf(reply_message_string,"ECHO %s%s",parameter_string,TERMINATOR_CR);
#if LOGGING > 9
					Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Reply String is '%s'.",
							     reply_message_string);
#endif /* LOGGING */
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
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'OPEN' command.");
#endif /* LOGGING */
				/* parse parameter */
				retval = sscanf(command_message_string,"OPEN %c",&parameter_char);
				if(retval == 1)
				{
					/* construct reply */
					sprintf(reply_message_string,"%c OPENED FOR OPERATOR COMMANDS%s",parameter_char,TERMINATOR_CR);
#if LOGGING > 9
					Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Reply String is '%s'.",
							     reply_message_string);
#endif /* LOGGING */
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
			else if(strstr(command_message_string,"PAR") != NULL)
			{
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'PAR' command.");
#endif /* LOGGING */
				if(Server_Data.Par_Callback != NULL)
					Server_Data.Par_Callback(reply_message_string,254);
				else
					strcpy(reply_message_string,"PARAMETERS");
				strcat(reply_message_string,TERMINATOR_CR);
#if LOGGING > 9
				Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Reply String is '%s'.",
						     reply_message_string);
#endif /* LOGGING */
				retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
							  strlen(reply_message_string));
				if(retval == FALSE)
				{
					Wms_Serial_Error();
				}
			}/* end if command was "PAR" */
			else if(strstr(command_message_string,"RESET") != NULL)
			{
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'RESET' command.");
#endif /* LOGGING */
				sprintf(reply_message_string,"RESET COMMAND%s",TERMINATOR_CR);
#if LOGGING > 9
				Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Reply String is '%s'.",
						     reply_message_string);
#endif /* LOGGING */
				retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
							  strlen(reply_message_string));
				if(retval == FALSE)
				{
					Wms_Serial_Error();
				}
			}/* end if command was "RESET" */
			else if(strstr(command_message_string,"STA") != NULL)
			{
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'STA' command.");
#endif /* LOGGING */
				if(Server_Data.Sta_Callback != NULL)
					Server_Data.Sta_Callback(reply_message_string,254);
				else
					strcpy(reply_message_string,"STATUS");
				strcat(reply_message_string,TERMINATOR_CR);
#if LOGGING > 9
				Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Reply String is '%s'.",
						     reply_message_string);
#endif /* LOGGING */
				retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
							  strlen(reply_message_string));
				if(retval == FALSE)
				{
					Wms_Serial_Error();
				}
			}/* end if command was "STA" */
			else if(command_message_string[0] == CHARACTER_SYN) /* Read Sensors */
			{
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'Read Sensors' command.");
#endif /* LOGGING */
				retval = sscanf(command_message_string,"%c%c%c",&parameter_char,&qli_id,&seq_id);
				if(retval != 3)
				{
					Wms_Qli50_Error_Number = 203;
					sprintf(Wms_Qli50_Error_String,
						"Wms_Qli50_Server_Loop:Failed to parse Read Sensors command '%s' (%d).",
						command_message_string,retval);
					Wms_Qli50_Error();
				}
				if(Server_Data.Read_Sensor_Callback != NULL)
					Server_Data.Read_Sensor_Callback(qli_id,seq_id);
				/* there is no reply to a <syn> / read sensors command */
			}/* end if command was <syn> / read sensors */
			else if(command_message_string[0] == CHARACTER_ENQ) /* Send Results */
			{
#if LOGGING > 9
				Wms_Qli50_Log(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Detected 'Send Results' command.");
#endif /* LOGGING */
				retval = sscanf(command_message_string,"%c%c%c",&parameter_char,&qli_id,&seq_id);
				if(retval != 3)
				{
					Wms_Qli50_Error_Number = 204;
					sprintf(Wms_Qli50_Error_String,
						"Wms_Qli50_Server_Loop:Failed to parse Send Results command '%s' (%d).",
						command_message_string,retval);
					Wms_Qli50_Error();
				}
				if(Server_Data.Send_Result_Callback != NULL)
					Server_Data.Send_Result_Callback(qli_id,seq_id,&data);
				else
				{
					/* create some data to send back - valid but _bad_ weather in case the
					** Send_Result_Callback is not set correctly in a 'real' situation. */
					data.Temperature.Type = DATA_TYPE_DOUBLE;
					data.Temperature.Value.DValue = 0.0;
					data.Humidity.Type = DATA_TYPE_DOUBLE;
					data.Humidity.Value.DValue = 99.0;
					data.Dew_Point.Type = DATA_TYPE_DOUBLE;
					data.Dew_Point.Value.DValue = 1.0;
					data.Wind_Speed.Type = DATA_TYPE_DOUBLE;
					data.Wind_Speed.Value.DValue = 20.0;
					data.Wind_Direction.Type = DATA_TYPE_INT;
					data.Wind_Direction.Value.IValue = 359;
					data.Air_Pressure.Type = DATA_TYPE_DOUBLE;
					data.Air_Pressure.Value.DValue = 768.0;
					data.Digital_Surface_Wet.Type = DATA_TYPE_INT;
					data.Digital_Surface_Wet.Value.IValue = 4;
					data.Analogue_Surface_Wet.Type = DATA_TYPE_INT;
					data.Analogue_Surface_Wet.Value.IValue = 98;
					data.Light.Type = DATA_TYPE_INT;
					data.Light.Value.IValue = 1000;
					data.Internal_Voltage.Type = DATA_TYPE_DOUBLE;
					data.Internal_Voltage.Value.DValue = 0.0;
					data.Internal_Current.Type = DATA_TYPE_DOUBLE;
					data.Internal_Current.Value.DValue = 0.0;
					data.Internal_Temperature.Type = DATA_TYPE_DOUBLE;
					data.Internal_Temperature.Value.DValue = 0.0;
					data.Reference_Temperature.Type = DATA_TYPE_DOUBLE;
					data.Reference_Temperature.Value.DValue = 0.0;
				}
				if(Server_Create_Send_Result_String(qli_id,seq_id,data,reply_message_string,254))
				{
#if LOGGING > 9
					Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Server_Loop: Reply String is '%s'.",
							     reply_message_string);
#endif /* LOGGING */
					retval = Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,reply_message_string,
								  strlen(reply_message_string));
					if(retval == FALSE)
					{
						Wms_Serial_Error();
					}
				}
				else
				{
					Wms_Qli50_Error();
				}
			}/* end if command was <enq> / send results */
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
			/* don't print out read-line timeouts - this just means nothing was received for 10 seconds */
			if(Wms_Serial_Error_Number != 10)
				Wms_Serial_Error();
		}
	}/* end while */
#if LOGGING > 1
	Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Qli50_Server_Loop: Finished.");
#endif /* LOGGING */
	return TRUE;
}

/* ==========================================
** internal functions 
** ========================================== */
/**
 * Internal routine to take the data in an instance of Wms_Qli50_Data_Struct, and turn it into a suitably formatted
 * string to reply to a Send Results message.
 * The general format is as follows:
 * &lt;soh&gt;&lt;qli_id&gt;&lt;seq_id&gt;&lt;stx&gt; &lt;measurement&gt; [, &lt;measurement&gt;]...&lt;etx&gt;&lt;cr&gt;&lt;lf&gt;
 * @param qli_id A character representing the QLI Identifier this result is from, usually 'A'.
 * @param seq_id A character representing the measurement sequence this result is from, usually 'A'.
 * @param data The data to format into the reply string, an instance of Wms_Qli50_Data_Struct.
 * @param message_string A string, on a successful return this is filled with the formatted send result string 
 *        in the correct format.
 * @param message_string_length An integer, the allocated length of message_string.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Server_Add_Result_To_String
 * @see wms_wxt536_command.html#Wms_Qli50_Data_Struct
 * @see wms_wxt536_command.html#CHARACTER_SOH
 * @see wms_wxt536_command.html#CHARACTER_STX
 * @see wms_wxt536_command.html#CHARACTER_ETX
 * @see wms_wxt536_command.html#TERMINATOR_CRLF
 * @see wms_wxt536_general.html#Wms_Qli50_Log
 * @see wms_wxt536_general.html#Wms_Qli50_Log_Format
 * @see wms_wxt536_general.html#Wms_Qli50_Error_Number
 * @see wms_wxt536_general.html#Wms_Qli50_Error_String
 */
static int Server_Create_Send_Result_String(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct data,
					    char *message_string,int message_string_length)
{
	int current_message_string_length;
	
	if(message_string == NULL)
	{
		Wms_Qli50_Error_Number = 205;
		sprintf(Wms_Qli50_Error_String,"Server_Create_Send_Result_String:message_string was NULL.");
		return FALSE;
	}
	strcpy(message_string,"");
	message_string[0] = CHARACTER_SOH;
	message_string[1] = qli_id;
	message_string[2] = seq_id;
	message_string[3] = CHARACTER_STX;
	/* NULL terminate header as Server_Add_Result_To_String concatenates to the end of the string. */
	message_string[4] = '\0';
	if(!Server_Add_Result_To_String("Temperature",data.Temperature,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Humidity",data.Humidity,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Dew_Point",data.Dew_Point,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Wind_Speed",data.Wind_Speed,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Wind_Direction",data.Wind_Direction,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Air_Pressure",data.Air_Pressure,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Digital_Surface_Wet",data.Digital_Surface_Wet,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Analogue_Surface_Wet",data.Analogue_Surface_Wet,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Light",data.Light,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Internal_Voltage",data.Internal_Voltage,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Internal_Current",data.Internal_Current,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Internal_Temperature",data.Internal_Temperature,TRUE,message_string,message_string_length))
		return FALSE;
	if(!Server_Add_Result_To_String("Reference_Temperature",data.Reference_Temperature,FALSE,
					message_string,message_string_length))
		return FALSE;
	current_message_string_length = strlen(message_string);
	if((current_message_string_length+3) >= message_string_length)
	{
		Wms_Qli50_Error_Number = 206;
		sprintf(Wms_Qli50_Error_String,"Server_Create_Send_Result_String:"
			"current message_string length is too long to add terminator characters (%d,%d).",
			(current_message_string_length+3),message_string_length);
		return FALSE;
	}
	message_string[current_message_string_length] = CHARACTER_ETX;
	strcat(message_string,TERMINATOR_CRLF);
	return TRUE;
}

/**
 * Routine to add a double data value (or an error code if the measurement failed) to an under construction 
 * Send Results reply string.
 * @param name The name of the daya  value we are adding (for logging / error message purposes only).
 * @param data_value The data value to add.
 * @param add_comma A boolean, whether we need to add a comma to the end of the value string (to separate it from the next value). 
 *        This should be true for every value added, except the last one.
 * @param message_string A string, on a successful return the value (or errorcode) is appended to the string.
 * @param message_string_length An integer, the allocated length of message_string.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see wms_wxt536_command.html#Wms_Qli50_Data_Value
 * @see wms_wxt536_general.html#Wms_Qli50_Log
 * @see wms_wxt536_general.html#Wms_Qli50_Log_Format
 * @see wms_wxt536_general.html#Wms_Qli50_Error_Number
 * @see wms_wxt536_general.html#Wms_Qli50_Error_String
 */
static int Server_Add_Result_To_String(char *name,struct Wms_Qli50_Data_Value data_value,int add_comma,
					      char *message_string,int message_string_length)
{
	char data_value_string[32];

	if(name == NULL)
	{
		Wms_Qli50_Error_Number = 207;
		sprintf(Wms_Qli50_Error_String,"Server_Add_Result_To_String:name was NULL.");		
		return FALSE;
	}
	if(message_string == NULL)
	{
		Wms_Qli50_Error_Number = 208;
		sprintf(Wms_Qli50_Error_String,"Server_Add_Result_To_String:message_string was NULL.");
		return FALSE;
	}
	if(data_value.Type == DATA_TYPE_DOUBLE)
		sprintf(data_value_string,"%.2lf",data_value.Value.DValue);
	else if(data_value.Type == DATA_TYPE_INT)
		sprintf(data_value_string,"%d",data_value.Value.IValue);
	else if(data_value.Type == DATA_TYPE_ERROR)
		sprintf(data_value_string,"E%04d",data_value.Value.Error_Code);
	if(add_comma)
	{
		strcat(data_value_string,",");
	}
	if(strlen(data_value_string)+strlen(message_string) >= message_string_length)
	{
		Wms_Qli50_Error_Number = 209;
		sprintf(Wms_Qli50_Error_String,
			"Server_Add_Result_To_String:message string too long when adding %s with value %s to %s.",
			name,data_value_string,message_string);		
		return FALSE;
	}
	strcat(message_string,data_value_string);
	return TRUE;
}
