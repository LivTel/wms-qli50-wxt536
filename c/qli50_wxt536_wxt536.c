/* qli50_wxt536_wxt536.c
** Weather monitoring system (qli50 -> wxt536 conversion), routines to talk to the Vaisala Wxt536 weather station.
*/
/**
 * Routines to talk to the Vaisala Wxt536 weather station.
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
#include <time.h>
#include "qli50_wxt536_config.h"
#include "qli50_wxt536_general.h"
#include "qli50_wxt536_wxt536.h"
#include "wms_wxt536_command.h"
#include "wms_wxt536_connection.h"
#include "log_udp.h"

/* defines */
/**
 * How long the string holding the serial device name is.
 */
#define FILENAME_LENGTH           (256)

/* internal structures */
/**
 * Structure containing weather and system data read by the Wxt536, along with a timestamp describing when the 
 * values were last updated.
 * <dl>
 * <dt>Wind_Data</dt> <dd>An instance of Wxt536_Command_Wind_Data_Struct containg the read wind data.</dd>
 * <dt>Wind_Timestamp</dt> <dd>A timestamp indicating when the last successful wind data was read.</dd>
 * <dt>Pressure_Temp_Humidity_Data</dt> <dd>An instance of Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct 
 *                                containg pressure, temperature and humidity data from the Wxt536.</dd>
 * <dt>Pressure_Temp_Humidity_Timestamp</dt> <dd>A timestamp indicating when the last successful 
 *                                pressure/temperature/humidity data was read.</dd>
 * <dt>Rain_Data</dt> <dd>An instance of Wxt536_Command_Precipitation_Data_Struct containing the read rain data 
 *                        from the Wxt536.</dd>
 * <dt>Rain_Timestamp</dt> <dd>A timestamp indicating when the last successful rain data was read.</dd>
 * <dt>Supervisor_Data</dt> <dd>An instance of Wxt536_Command_Supervisor_Data_Struct containing internal data 
 *                to the Wxt536 weather station, heater temperature/voltage, supply and reference voltages etc...</dd>
 * <dt>Supervisor_Timestamp</dt> <dd>A timestamp indicating when the last successful supervisor data was read.</dd>
 * <dt>Analogue_Data</dt> <dd>An instance of Wxt536_Command_Analogue_Data_Struct containing analogue data 
 *     from sensors externally connected to the Wxt536 i.e. the external rain sensor, the solar radiation sensor.</dd>
 * <dt>Analogue_Timestamp</dt> <dd>A timestamp indicating when the last successful analogue data was read.</dd>
 * <dt></dt> <dd></dd>
 * <dt></dt> <dd></dd>
 * </dl>
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wxt536_Command_Wind_Data_Struct
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wxt536_Command_Precipitation_Data_Struct
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wxt536_Command_Supervisor_Data_Struct
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wxt536_Command_Analogue_Data_Struct
 */
struct Wxt536_Data_Struct
{
	struct Wxt536_Command_Wind_Data_Struct Wind_Data;
	struct timespec Wind_Timestamp;
	struct Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct Pressure_Temp_Humidity_Data;
	struct timespec Pressure_Temp_Humidity_Timestamp;
	struct Wxt536_Command_Precipitation_Data_Struct Rain_Data;
	struct timespec Rain_Timestamp;
	struct Wxt536_Command_Supervisor_Data_Struct Supervisor_Data;
	struct timespec Supervisor_Timestamp;
	struct Wxt536_Command_Analogue_Data_Struct Analogue_Data;
	struct timespec Analogue_Timestamp;
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The filename used to store the serial device filename of the Wxt536.
 */
static char Serial_Device_Filename[FILENAME_LENGTH];
/**
 * A character representing the device address of the Wxt536.
 */
static char Wxt536_Device_Address;
/**
 * An instance of Wxt536_Data_Struct containing the last set of data read from the Wxt536.
 * @see #Wxt536_Data_Struct
 */
struct Wxt536_Data_Struct Wxt536_Data;

/* =======================================================
** external functions 
** ======================================================= */
/**
 * Initialise the connection to the Vaisala Wxt536 weather station.
 * <ul>
 * <li>We get the serial device filename from the config file (keyword "wxt536.serial_device.name").
 *     This is stored in Serial_Device_Filename.
 * <li>We open connection to the Wxt536 by calling Wms_Wxt536_Connection_Open with the previously discovered
 *     serial device filename.
 * <li>We call Wms_Wxt536_Command_Device_Address_Get to get the Wxt536 device address for this device, and store it in #
 *     Wxt536_Device_Address.
 * <li>We retrieve the Wxt536 protocol to use from the config file (keyword "wxt536.protocol").
 * <li>We call Wms_Wxt536_Command_Comms_Settings_Protocol_Set to set the protocol to use with the Wxt536.
 * </ul>
 * @return The routine returns TRUE on success and FALSE on failure. If it fails, Qli50_Wxt536_Error_Number and
 *         Qli50_Wxt536_Error_String will be set with a suitable error.
 * @see #FILENAME_LENGTH
 * @see #Serial_Device_Filename
 * @see #Wxt536_Device_Address
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_String_Get
 * @see ../wxt536/cdocs/wms_wxt536_connection.html#Wms_Wxt536_Connection_Open
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Device_Address_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Comms_Settings_Protocol_Set
 */
int Qli50_Wxt536_Wxt536_Initialise(void)
{
	char protocol_string[32];
	char protocol;
	
	Qli50_Wxt536_Error_Number = 0;
	/* get the serial device filename from config */
	if(!Qli50_Wxt536_Config_String_Get("wxt536.serial_device.name",Serial_Device_Filename,FILENAME_LENGTH))
		return FALSE;
	if(!Wms_Wxt536_Connection_Open("Wxt536","qli50_wxt536_wxt536.c",Serial_Device_Filename))
	{
		Qli50_Wxt536_Error_Number = 200;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Wxt536_Initialise:Wms_Wxt536_Connection_Open(%s) failed.",
			Serial_Device_Filename);		
		return FALSE;
	}
	/* get the device address of the connected Wxt536, and store for later use. */
	if(!Wms_Wxt536_Command_Device_Address_Get("Wxt536","qli50_wxt536_wxt536.c",&Wxt536_Device_Address))
	{
		Qli50_Wxt536_Error_Number = 201;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Wxt536_Initialise: Failed to retrieve Wxt536 device address.");		
		return FALSE;
	}
	/* ensure the protocol is setup correctly. */
	if(!Qli50_Wxt536_Config_String_Get("wxt536.protocol",protocol_string,31))
		return FALSE;
	if(strlen(protocol_string) > 1)
	{
		Qli50_Wxt536_Error_Number = 203;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Wxt536_Initialise: Protocol string '%s' was too long.",protocol_string);		
		return FALSE;
	}
	protocol = protocol_string[0];
	if(!Wms_Wxt536_Command_Comms_Settings_Protocol_Set("Wxt536","qli50_wxt536_wxt536.c",
							   Wxt536_Device_Address,protocol))
	{
		Qli50_Wxt536_Error_Number = 204;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Initialise: "
			"Failed to set the communication protocol to '%c' "
			"for Wxt536 device address '%c'.",protocol,Wxt536_Device_Address);		
		return FALSE;
	}	
	return TRUE;
}

/**
 * Close the previously opened connection to the Vaisala Wxt536 weather station.
 * @return The routine returns TRUE on success and FALSE on failure. If it fails, Qli50_Wxt536_Error_Number and
 *         Qli50_Wxt536_Error_String will be set with a suitable error.
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../wxt536/cdocs/wms_wxt536_connection.html#Wms_Wxt536_Connection_Close
 */
int Qli50_Wxt536_Wxt536_Close(void)
{
	Qli50_Wxt536_Error_Number = 0;
	if(!Wms_Wxt536_Connection_Close("Wxt536","qli50_wxt536_wxt536.c"))
	{
		Qli50_Wxt536_Error_Number = 202;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Close:Wms_Wxt536_Connection_Close() failed.");
		return FALSE;
	}	
	return TRUE;
}

/**
 * Process a 'Read Sensors' command received by the Qli50 server. This involves reading all the weather station sensors
 * that have values required by the 'Send Results' command.
 * @param qli_id A single character, representing the QLI Id of the Qli50 that is required to read it's sensors.
 * @param seq_id A single character, representing the QLI50 sequence id of the set of readings 
 *        the QLI50 is meant to take.
 * @return The routine returns TRUE on success and FALSE on failure. If it fails, Qli50_Wxt536_Error_Number and
 *         Qli50_Wxt536_Error_String will be set with a suitable error.
 * @see #Wxt536_Device_Address
 * @see #Wxt536_Data
 * @see #Wxt536_Data_Struct
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Wind_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Pressure_Temperature_Humidity_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Precipitation_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Supervisor_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Analogue_Data_Get
 */
int Qli50_Wxt536_Wxt536_Read_Sensors(char qli_id,char seq_id)
{
	struct Wxt536_Command_Wind_Data_Struct wind_data;
	struct Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct pressure_temp_humidity_data;
	struct Wxt536_Command_Precipitation_Data_Struct rain_data;
	struct Wxt536_Command_Supervisor_Data_Struct supervisor_data;
	struct Wxt536_Command_Analogue_Data_Struct analogue_data;
	int retval;

	retval = TRUE;
	Qli50_Wxt536_Error_Number = 0;
	/* read wind data */
	if(Wms_Wxt536_Command_Wind_Data_Get("Wxt536","qli50_wxt536_wxt536.c",Wxt536_Device_Address,&wind_data))
	{
		Wxt536_Data.Wind_Data = wind_data;
		clock_gettime(CLOCK_REALTIME,&(Wxt536_Data.Wind_Timestamp));
	}
	else
	{
		Qli50_Wxt536_Error_Number = 205;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Read_Sensors:Reading Wind data failed.");
		retval = FALSE;
	}
	/* read pressure/temperature/humidity data */
	if(Wms_Wxt536_Command_Pressure_Temperature_Humidity_Data_Get("Wxt536","qli50_wxt536_wxt536.c",
								     Wxt536_Device_Address,&pressure_temp_humidity_data))
	{
		Wxt536_Data.Pressure_Temp_Humidity_Data = pressure_temp_humidity_data;
		clock_gettime(CLOCK_REALTIME,&(Wxt536_Data.Pressure_Temp_Humidity_Timestamp));
	}
	else
	{
		Qli50_Wxt536_Error_Number = 206;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Wxt536_Read_Sensors:Reading Pressure/Temperature/Humidity data failed.");
		retval = FALSE;
	}
	/* read rain data */
	if(Wms_Wxt536_Command_Precipitation_Data_Get("Wxt536","qli50_wxt536_wxt536.c",Wxt536_Device_Address,&rain_data))
	{
		Wxt536_Data.Rain_Data = rain_data;
		clock_gettime(CLOCK_REALTIME,&(Wxt536_Data.Rain_Timestamp));
	}
	else
	{
		Qli50_Wxt536_Error_Number = 207;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Read_Sensors:Reading Rain data failed.");
		retval = FALSE;
	}
	/* read supervisor data (internal temperatures/voltages) */
	if(Wms_Wxt536_Command_Supervisor_Data_Get("Wxt536","qli50_wxt536_wxt536.c",Wxt536_Device_Address,&supervisor_data))
	{
		Wxt536_Data.Supervisor_Data = supervisor_data;
		clock_gettime(CLOCK_REALTIME,&(Wxt536_Data.Supervisor_Timestamp));
	}
	else
	{
		Qli50_Wxt536_Error_Number = 208;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Read_Sensors:Reading Supervisor data failed.");
		retval = FALSE;
	}
	/* read external analogue data (external rain sensor/pyranometer) */
	if(Wms_Wxt536_Command_Analogue_Data_Get("Wxt536","qli50_wxt536_wxt536.c",Wxt536_Device_Address,&analogue_data))
	{
		Wxt536_Data.Analogue_Data = analogue_data;
		clock_gettime(CLOCK_REALTIME,&(Wxt536_Data.Analogue_Timestamp));
	}
	else
	{
		Qli50_Wxt536_Error_Number = 209;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Read_Sensors:Reading Analogue data failed.");
		retval = FALSE;
	}
	return retval;
}

/* =======================================================
** internal functions 
** ======================================================= */
