/* qli50_wxt536_server.c
** Weather monitoring system (qli50 -> wxt536 conversion), routines to configure,start and handle Qli50 commands from
** a serial line.
*/
/**
 * Routines to configure, start and handle Qli50 commands from a serial line.
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
#include "qli50_wxt536_config.h"
#include "qli50_wxt536_general.h"
#include "qli50_wxt536_server.h"
#include "qli50_wxt536_wxt536.h"
#include "wms_qli50_connection.h"
#include "wms_qli50_server.h"
#include "log_udp.h"

/* defines */
/**
 * How long the string holding the serial device name is.
 */
#define FILENAME_LENGTH           (256)

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The filename used to store the serial device filename to connect the server code to.
 */
static char Serial_Device_Filename[FILENAME_LENGTH];

/* internal functions */
static void Server_Read_Sensor_Callback(char qli_id,char seq_id);
static void Server_Send_Result_Callback(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data);
static void Server_Par_Callback(char *return_parameter_string,int return_parameter_string_length);
static void Server_Sta_Callback(char *return_string,int return_string_length);

/* =======================================================
** external functions 
** ======================================================= */
/**
 * Read the serial device filename from config, and then setup a connection to it. Also setup the server callbacks.
 * The configuration file must have previously been read, before calling this routine.
 * @return The routine returns TRUE on success and FALSE on failure. If it fails, Qli50_Wxt536_Error_Number and
 *         Qli50_Wxt536_Error_String will be set with a suitable error.
 * @see #FILENAME_LENGTH
 * @see #Serial_Device_Filename
 * @see #Server_Read_Sensor_Callback
 * @see #Server_Send_Result_Callback
 * @see #Server_Par_Callback
 * @see #Server_Sta_Callback
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_String_Get
 * @see ../qli50/cdocs/wms_qli50_server.html#Wms_Qli50_Server_Set_Read_Sensor_Callback
 * @see ../qli50/cdocs/wms_qli50_server.html#Wms_Qli50_Server_Set_Send_Result_Callback
 * @see ../qli50/cdocs/wms_qli50_server.html#Wms_Qli50_Server_Set_Par_Callback
 * @see ../qli50/cdocs/wms_qli50_server.html#Wms_Qli50_Server_Set_Sta_Callback
 * @see ../qli50/cdocs/wms_qli50_server.html#Wms_Qli50_Server_Start
 */
int Qli50_Wxt536_Server_Initialise(void)
{
	int retval;

	Qli50_Wxt536_Error_Number = 0;
	/* get the serial device filename from config */
	if(!Qli50_Wxt536_Config_String_Get("qli50.serial_device.name",Serial_Device_Filename,FILENAME_LENGTH))
		return FALSE;
	/* setup server callbacks */
	if(!Wms_Qli50_Server_Set_Read_Sensor_Callback("Server","qli50_wxt536_server.c",Server_Read_Sensor_Callback))
	{
		Qli50_Wxt536_Error_Number = 100;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Server_Initialise:failed to set read sensor callback.");		
		return FALSE;
	}
	if(!Wms_Qli50_Server_Set_Send_Result_Callback("Server","qli50_wxt536_server.c",Server_Send_Result_Callback))
	{
		Qli50_Wxt536_Error_Number = 101;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Server_Initialise:failed to set send result callback.");		
		return FALSE;
	}
	if(!Wms_Qli50_Server_Set_Par_Callback("Server","qli50_wxt536_server.c",Server_Par_Callback))
	{
		Qli50_Wxt536_Error_Number = 102;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Server_Initialise:failed to set par callback.");		
		return FALSE;
	}
	if(!Wms_Qli50_Server_Set_Sta_Callback("Server","qli50_wxt536_server.c",Server_Sta_Callback))
	{
		Qli50_Wxt536_Error_Number = 103;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Server_Initialise:failed to set sta callback.");		
		return FALSE;
	}
	/* start the Qli50 server. This just opens the serial connection and configures it.
	** It does not start the loop */
	retval = Wms_Qli50_Server_Start("Server","qli50_wxt536_server.c",Serial_Device_Filename);
	if(retval == FALSE)
	{
		Qli50_Wxt536_Error_Number = 104;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Server_Initialise:Wms_Qli50_Server_Start(serial_device='%s') failed.",
			Serial_Device_Filename);		
		return FALSE;
	}
	return TRUE;
}

/**
 * Routine to start the server loop, which reads Qli50 commands and processes them. Normally, this routine
 * does not return.
 * @return The routine nominally returns TRUE on sucess and FALSE on failure. In reality, the routine enters
 *         a loop that should never terminate.
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../qli50/cdocs/wms_qli50_server.html#Wms_Qli50_Server_Loop
 */
int Qli50_Wxt536_Server_Start(void)
{
	int retval;
	
	Qli50_Wxt536_Error_Number = 0;
	retval = Wms_Qli50_Server_Loop("Server","qli50_wxt536_server.c");
	if(retval == FALSE)
	{
		Qli50_Wxt536_Error_Number = 105;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Server_Start:Wms_Qli50_Server_Loop failed.");		
		return FALSE;
	}
	return TRUE;
}

/* =======================================================
** internal functions 
** ======================================================= */
/**
 * This routine should get called by the server loop when a 'Read Sensor' command is read from the server's serial
 * link. We call Qli50_Wxt536_Wxt536_Read_Sensors to tell the Wxt536 weather station to read it's sensors.
 * @param qli_id A single character, representing the QLI Id of the Qli50 that is required to read it's sensors.
 * @param seq_id A single character, representing the QLI50 sequence id of the set of readings 
 *        the QLI50 is meant to take.
 * @see qli50_wxt536_wxt536.html#Qli50_Wxt536_Wxt536_Read_Sensors
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error
 */
static void Server_Read_Sensor_Callback(char qli_id,char seq_id)
{
	Qli50_Wxt536_Error_Number = 0;
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Server","qli50_wxt536_server.c",LOG_VERBOSITY_TERSE,
				"Server_Read_Sensor_Callback invoked with qli_id '%c' and seq_id '%c'.",
				qli_id,seq_id);
#endif /* LOGGING */
	if(!Qli50_Wxt536_Wxt536_Read_Sensors(qli_id,seq_id))
	{
		Qli50_Wxt536_Error();
	}
}

/**
 * This routine should get called by the server loop when a 'Send Result' command is read from the server's serial
 * link.
 * @param qli_id A single character, representing the QLI Id of the Qli50 that is required to send it's results.
 * @param seq_id A single character, representing the QLI50 sequence id of the set of readings 
 *        to return.
 * @param data The address of a pointer to a Wms_Qli50_Data_Struct, on return from this function this should be filled
 *        in with the weather data values we want to return as Qli50 data.
 */
static void Server_Send_Result_Callback(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data)
{
	Qli50_Wxt536_Error_Number = 0;
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Server","qli50_wxt536_server.c",LOG_VERBOSITY_TERSE,
				"Server_Send_Result_Callback invoked with qli_id '%c' and seq_id '%c'.",
				qli_id,seq_id);
#endif /* LOGGING */
	data->Temperature.Type = DATA_TYPE_DOUBLE;
	data->Temperature.Value.DValue = 0.0;
	data->Humidity.Type = DATA_TYPE_DOUBLE;
	data->Humidity.Value.DValue = 99.0;
	data->Dew_Point.Type = DATA_TYPE_DOUBLE;
	data->Dew_Point.Value.DValue = 1.0;
	data->Wind_Speed.Type = DATA_TYPE_DOUBLE;
	data->Wind_Speed.Value.DValue = 20.0;
	data->Wind_Direction.Type = DATA_TYPE_INT;
	data->Wind_Direction.Value.IValue = 359;
	data->Air_Pressure.Type = DATA_TYPE_DOUBLE;
	data->Air_Pressure.Value.DValue = 768.0;
	data->Digital_Surface_Wet.Type = DATA_TYPE_INT;
	data->Digital_Surface_Wet.Value.IValue = 4;
	data->Analogue_Surface_Wet.Type = DATA_TYPE_INT;
	data->Analogue_Surface_Wet.Value.IValue = 98;
	data->Light.Type = DATA_TYPE_INT;
	data->Light.Value.IValue = 1000;
	data->Internal_Voltage.Type = DATA_TYPE_DOUBLE;
	data->Internal_Voltage.Value.DValue = 0.0;
	data->Internal_Current.Type = DATA_TYPE_DOUBLE;
	data->Internal_Current.Value.DValue = 0.0;
	data->Internal_Temperature.Type = DATA_TYPE_DOUBLE;
	data->Internal_Temperature.Value.DValue = 0.0;
	data->Reference_Temperature.Type = DATA_TYPE_DOUBLE;
	data->Reference_Temperature.Value.DValue = 0.0;
}

/**
 * This routine should get called by the server loop when a 'PAR' command is read from the server's serial
 * link.
 * @param return_parameter_string A pointer to a character string, on return this should be filled with the data
 *        string we wish the Qli50 to return as a result of the PAR command.
 * @param return_parameter_string_length An integer containing the maximum number of characters we can put into
 *        return_parameter_string before the buffer overflows.
 */
static void Server_Par_Callback(char *return_parameter_string,int return_parameter_string_length)
{
	Qli50_Wxt536_Error_Number = 0;
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Server","qli50_wxt536_server.c",LOG_VERBOSITY_TERSE,"Server_Par_Callback invoked.");
#endif /* LOGGING */
	strcpy(return_parameter_string,"PARAMETERS");
}

/**
 * This routine should get called by the server loop when a 'STA' command is read from the server's serial
 * link.
 * @param return_string A pointer to a character string, on return this should be filled with the data
 *        string we wish the Qli50 to return as a result of the STA command.
 * @param return_string_length An integer containing the maximum number of characters we can put into
 *        return_string before the buffer overflows.
 */
static void Server_Sta_Callback(char *return_string,int return_string_length)
{
	Qli50_Wxt536_Error_Number = 0;
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Server","qli50_wxt536_server.c",LOG_VERBOSITY_TERSE,"Server_Sta_Callback invoked.");
#endif /* LOGGING */
	strcpy(return_string,"STATUS");
}
