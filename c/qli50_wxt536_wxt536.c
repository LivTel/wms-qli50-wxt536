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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "qli50_wxt536_config.h"
#include "qli50_wxt536_general.h"
#include "qli50_wxt536_wxt536.h"
#include "wms_qli50_command.h"
#include "wms_wxt536_command.h"
#include "wms_wxt536_connection.h"
#include "log_udp.h"

/* defines */
/**
 * How long the string holding the serial device name is.
 */
#define FILENAME_LENGTH           (256)

/* enums */
/**
 * Which sensor to use for measurement of some elements. One of:
 * <ul>
 * <li>SENSOR_TYPE_NONE
 * <li>SENSOR_TYPE_WXT536
 * <li>SENSOR_TYPE_DRD11A
 * </ul>
 */
enum Sensor_Type_Enum
{
	SENSOR_TYPE_NONE, SENSOR_TYPE_WXT536, SENSOR_TYPE_DRD11A
};

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
static struct Wxt536_Data_Struct Wxt536_Data;
/**
 * The maximum age of a datum read from the Wxt536 before it is deemed stale data, in decimal seconds.
 */
static double Max_Datum_Age;
/**
 * Configure how often the Wxt536 updates it's analogue input values, in decimal seconds. 
 * It defaults to updating these every minute,
 * as we have the DRD11A attached to one of these inputs we want to react to wetness quicker than that.
 * The update interval must be longer than the averaging time.
 */
static double Wxt536_Analogue_Input_Update_Interval;
/**
 * Configure the length of time the Wxt536 averages the solar radiation and ultrasonic level 
 * (used by us for the DRD11A analogue input) data values, in decimal seconds. 
 * The default of 3s is reasonable for our usage, but if we want to decrease the update interval we may want to decrease 
 * this value as well, as the averaging time must be less than the update interval.
 */
static double Wxt536_Analogue_Input_Averaging_Time;
/**
 * The gain applied to the pyranometer voltage returned by the Wxt536. The factory default is 100000,
 * and the currently set gain can be read by the "0IB,G" command.
 */
static double Wxt536_Pyranometer_Gain = 100000.0;
/**
 * The CMP3 pyranometer sensitivity, in uV/W/m^2. This is written on the sensor, and is 30.95 uV/W/m^2
 * for our current sensor.
 */
static double CMP3_Pyranometer_Sensitivity = 30.95;
/**
 * Which sensor to use for measuring digital surface wet.
 * @see #Sensor_Type_Enum
 */
static enum Sensor_Type_Enum Digital_Surface_Wet_Sensor;
/**
 * Which sensor to use for measuring digital surface wet.
 * @see #Sensor_Type_Enum
 */
static enum Sensor_Type_Enum Analogue_Surface_Wet_Sensor;
/**
 * The DRD11A sensor reports 1v wet, 3v dry. What is the threshold, below which, we report 'wet' for the digital surface wet
 * value?
 */
static double Digital_Surface_Wet_Drd11a_Threshold = 2.8;
/**
 * The DRD11A is attached to the ultrasonic level auxilary input on the wxt536.
 * The DRD11A analogue voltage goes from 1v fully wet to 3v fully dry.
 * Therefore the Analogue surface wet value (0..100) = 100-((Ultrasonic_Level_Voltage-(drd11a.wetpoint))*drd11a.scale)
 * This value is the drd11a.wetpoint.
 */
static double Analogue_Surface_Wet_Drd11a_Wet_Point = 1.0;
/**
 * The DRD11A is attached to the ultrasonic level auxilary input on the wxt536.
 * The DRD11A analogue voltage goes from 1v fully wet to 3v fully dry.
 * Therefore the Analogue surface wet value (0..100) = 100-((Ultrasonic_Level_Voltage-(drd11a.wetpoint))*drd11a.scale)
 * This value is the drd11a.scale.
 */
static double Analogue_Surface_Wet_Drd11a_Scale = 50.0;
/**
 * We use the Wxt536 rain intensity (in mm/h) when computing the analogue surface wetness in wxt536 mode, 
 * and scale it (with range checking).
 * A scale value of 100.0 means a rain intensity of 1mm/h converts to an analaogue wetness of 100%
 */
static double Analogue_Surface_Wet_Wxt536_Scale = 100.0;

/* internal functions */
static int Wxt536_Config_Sensor_Get(char *keyword,enum Sensor_Type_Enum *sensor);
static double Wxt536_Calculate_Dew_Point(struct Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct pth_data);
static int Wxt536_Pyranometer_Volts_To_Watts_M2(double voltage);
static void Wxt536_Digital_Surface_Wet_Set(struct timespec current_time,
					   struct Wms_Qli50_Data_Value *digital_surface_wet_value);
static void Wxt536_Analogue_Surface_Wet_Set(struct timespec current_time,
					    struct Wms_Qli50_Data_Value *analogue_surface_wet_value);

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
 * <li>We call Wms_Wxt536_Command_Device_Address_Get to get the Wxt536 device address for this device, and store it in 
 *     Wxt536_Device_Address.
 * <li>We retrieve the Wxt536 protocol to use from the config file (keyword "wxt536.protocol").
 * <li>We call Wms_Wxt536_Command_Comms_Settings_Protocol_Set to set the protocol to use with the Wxt536.
 * <li>We retrieve the Max_Datum_Age from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We retrieve the Wxt536_Analogue_Input_Update_Interval from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We retrieve the Wxt536_Analogue_Input_Averaging_Time from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We configure the Wxt536 to use the analogue input settings by calling Wms_Wxt536_Command_Analogue_Input_Settings_Set.
 * <li>We retrieve the Wxt536_Pyranometer_Gain from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We configure the Wxt536 to use the pyranometer gain by calling Wms_Wxt536_Command_Solar_Radiation_Gain_Set.
 * <li>We retrieve the CMP3_Pyranometer_Sensitivity from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We retrieve the Digital_Surface_Wet_Sensor from the config file using Wxt536_Config_Sensor_Get.
 * <li>We retrieve the Analogue_Surface_Wet_Sensor from the config file using Wxt536_Config_Sensor_Get.
 * <li>We retrieve the Digital_Surface_Wet_Drd11a_Threshold from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We retrieve the Analogue_Surface_Wet_Drd11a_Wet_Point from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We retrieve the Analogue_Surface_Wet_Drd11a_Scale from the config file using Qli50_Wxt536_Config_Double_Get.
 * <li>We retrieve the Analogue_Surface_Wet_Wxt536_Scale from the config file using Qli50_Wxt536_Config_Double_Get.
 * </ul>
 * @return The routine returns TRUE on success and FALSE on failure. If it fails, Qli50_Wxt536_Error_Number and
 *         Qli50_Wxt536_Error_String will be set with a suitable error.
 * @see #FILENAME_LENGTH
 * @see #Serial_Device_Filename
 * @see #Wxt536_Device_Address
 * @see #Max_Datum_Age
 * @see #Wxt536_Analogue_Input_Update_Interval
 * @see #Wxt536_Analogue_Input_Averaging_Time
 * @see #Wxt536_Pyranometer_Gain
 * @see #CMP3_Pyranometer_Sensitivity
 * @see #Sensor_Type_Enum
 * @see #Digital_Surface_Wet_Sensor
 * @see #Analogue_Surface_Wet_Sensor
 * @see #Digital_Surface_Wet_Drd11a_Threshold
 * @see #Analogue_Surface_Wet_Drd11a_Wet_Point
 * @see #Analogue_Surface_Wet_Drd11a_Scale
 * @see #Analogue_Surface_Wet_Wxt536_Scale
 * @see #Wxt536_Config_Sensor_Get
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_String_Get
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_Double_Get
 * @see ../wxt536/cdocs/wms_wxt536_connection.html#Wms_Wxt536_Connection_Open
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Device_Address_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Comms_Settings_Protocol_Set
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Solar_Radiation_Gain_Set
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Analogue_Input_Settings_Set
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
	/* get the maximum datum age in seconds */
	if(!Qli50_Wxt536_Config_Double_Get("wxt536.max_datum_age",&Max_Datum_Age))
		return FALSE;
	/* get the Wxt536 analogue input settings */
	if(!Qli50_Wxt536_Config_Double_Get("wxt536.analogue_input.update_interval",&Wxt536_Analogue_Input_Update_Interval))
		return FALSE;
	if(!Qli50_Wxt536_Config_Double_Get("wxt536.analogue_input.averaging_time",&Wxt536_Analogue_Input_Averaging_Time))
		return FALSE;
	/* set the wxt536 to use the configured analogue input settings */
	if(!Wms_Wxt536_Command_Analogue_Input_Settings_Set("Wxt536","qli50_wxt536_wxt536.c",Wxt536_Device_Address,
							   Wxt536_Analogue_Input_Update_Interval,
							   Wxt536_Analogue_Input_Averaging_Time))
	{
		Qli50_Wxt536_Error_Number = 214;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Initialise: "
			"Failed to set the anagloue input settings to update interval %.3f s, "
			"averaging time %.3f s for Wxt536 device address '%c'.",
			Wxt536_Analogue_Input_Update_Interval,Wxt536_Analogue_Input_Averaging_Time,Wxt536_Device_Address);
		return FALSE;
	}
	/* get the Wxt536 pyranometer gain */
	if(!Qli50_Wxt536_Config_Double_Get("wxt536.pyranometer.gain",&Wxt536_Pyranometer_Gain))
		return FALSE;
	/* set the wxt536 to use the configured gain */
	if(!Wms_Wxt536_Command_Solar_Radiation_Gain_Set("Wxt536","qli50_wxt536_wxt536.c",Wxt536_Device_Address,
							Wxt536_Pyranometer_Gain))
	{
		Qli50_Wxt536_Error_Number = 213;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Wxt536_Initialise: "
			"Failed to set the pyranometer gain to '%.3f' for Wxt536 device address '%c'.",
			Wxt536_Pyranometer_Gain,Wxt536_Device_Address);		
		return FALSE;
	}
	/* get the CMP3 pyranometer sensitivity in uV/W/m^2*/
	if(!Qli50_Wxt536_Config_Double_Get("cmp3.pyranometer.sensitivity",&CMP3_Pyranometer_Sensitivity))
		return FALSE;
	/* get which sensor to use for the digital surface wet determination */
	if(!Wxt536_Config_Sensor_Get("digital.surface.wet.sensor",&Digital_Surface_Wet_Sensor))
		return FALSE;
	/* get which sensor to use for the analogue surface wet determination */
	if(!Wxt536_Config_Sensor_Get("analogue.surface.wet.sensor",&Analogue_Surface_Wet_Sensor))
		return FALSE;
	/* get the digital surface wet drd11a threashold in volts */
	if(!Qli50_Wxt536_Config_Double_Get("digital.surface.wet.drd11a.threshold",&Digital_Surface_Wet_Drd11a_Threshold))
		return FALSE;
	/* get the analogue surface wet drd11a wet-point voltage */
	if(!Qli50_Wxt536_Config_Double_Get("analogue.surface.wet.drd11a.wetpoint",&Analogue_Surface_Wet_Drd11a_Wet_Point))
		return FALSE;
	/* get the analogue surface wet drd11a scaling factor */
	if(!Qli50_Wxt536_Config_Double_Get("analogue.surface.wet.drd11a.scale",&Analogue_Surface_Wet_Drd11a_Scale))
		return FALSE;
	/* get the analogue surface wet wxt536 scaling factor */
	if(!Qli50_Wxt536_Config_Double_Get("analogue.surface.wet.wxt536.scale",&Analogue_Surface_Wet_Wxt536_Scale))
		return FALSE;
	
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
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_INTERMEDIATE,
				"Qli50_Wxt536_Wxt536_Read_Sensors invoked with qli_id '%c' and seq_id '%c'.",
				qli_id,seq_id);
#endif /* LOGGING */
	/* read wind data */
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,"Reading Wxt536 wind data.");
#endif /* LOGGING */
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
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,
				"Reading Wxt536 pressure/temperature/humidity data.");
#endif /* LOGGING */
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
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,"Reading Wxt536 rain data.");
#endif /* LOGGING */
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
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,"Reading Wxt536 supervisor data.");
#endif /* LOGGING */
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
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,"Reading Wxt536 analogue data.");
#endif /* LOGGING */
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
#if LOGGING > 1
	Qli50_Wxt536_Log("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_INTERMEDIATE,
				"Qli50_Wxt536_Wxt536_Read_Sensors finished.");
#endif /* LOGGING */
	return retval;
}

/**
 * Process a 'Send Results' command received by the Qli50 server. This fills in the supplied instance
 * of Wms_Qli50_Data_Struct with data obtained fromm a previous 'Read Sensor' command, converting any
 * units as necessary and noting any out of date datums.
 * @param qli_id A single character, representing the QLI Id of the Qli50 that is required to read it's sensors.
 * @param seq_id A single character, representing the QLI50 sequence id of the set of readings 
 *        the QLI50 is meant to take.
 * @param data The address of a Wms_Qli50_Data_Struct to fill in with converted Wxt536 data, to be sent as 
 *       a reply to the Qli50 'Send Results' command.
 * @return The routine returns TRUE on success and FALSE on failure. If it fails, Qli50_Wxt536_Error_Number and
 *         Qli50_Wxt536_Error_String will be set with a suitable error.
 * @see #Wxt536_Data
 * @see #Wxt536_Data_Struct
 * @see #Max_Datum_Age
 * @see #Wxt536_Calculate_Dew_Point
 * @see #Wxt536_Pyranometer_Volts_to_Watts_M2
 * @see #Wxt536_Digital_Surface_Wet_Set
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../qli50/cdocs/wms_qli50_command.html#QLI50_ERROR_NO_MEASUREMENT
 * @see ../qli50/cdocs/wms_qli50_command.html#Wms_Qli50_Data_Struct
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Wind_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Pressure_Temperature_Humidity_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Precipitation_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Supervisor_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Analogue_Data_Get
 */
int Qli50_Wxt536_Wxt536_Send_Results(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data)
{
	struct timespec current_time;

#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_INTERMEDIATE,
				"Qli50_Wxt536_Wxt536_Send_Results invoked with qli_id '%c' and seq_id '%c'.",
				qli_id,seq_id);
#endif /* LOGGING */
	/* get the current time */
	clock_gettime(CLOCK_REALTIME,&current_time);
	/* pressure/temperature/humidity */
	if(fdifftime(current_time,Wxt536_Data.Pressure_Temp_Humidity_Timestamp) < Max_Datum_Age)
	{
		/* air temperature in degrees centigrade. */
		data->Temperature.Type = DATA_TYPE_DOUBLE;
		data->Temperature.Value.DValue = Wxt536_Data.Pressure_Temp_Humidity_Data.Air_Temperature;
		/* relative humidity in % */
		data->Humidity.Type = DATA_TYPE_DOUBLE;
		data->Humidity.Value.DValue = Wxt536_Data.Pressure_Temp_Humidity_Data.Relative_Humidity;
		/* air pressure in hPa/mbar */
		data->Air_Pressure.Type = DATA_TYPE_DOUBLE;
		data->Air_Pressure.Value.DValue = Wxt536_Data.Pressure_Temp_Humidity_Data.Air_Pressure;
		/* Calculate the Dew point from the temperature and relative humidity */
		data->Dew_Point.Type = DATA_TYPE_DOUBLE;
		data->Dew_Point.Value.DValue = Wxt536_Calculate_Dew_Point(Wxt536_Data.Pressure_Temp_Humidity_Data);
#if LOGGING > 1
		Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,
	   "Qli50_Wxt536_Wxt536_Send_Results: Dew point %.2f C calculated from Air temperature %.2f C and Humidity %.2f %.",
					data->Dew_Point.Value.DValue,
					Wxt536_Data.Pressure_Temp_Humidity_Data.Air_Temperature,
					Wxt536_Data.Pressure_Temp_Humidity_Data.Relative_Humidity);
#endif /* LOGGING */
	}
	else
	{
#if LOGGING > 1
		Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,
			      "Qli50_Wxt536_Wxt536_Send_Results: Pressure/temperature/humidity data out of date (%.2f s).",
			      fdifftime(current_time,Wxt536_Data.Pressure_Temp_Humidity_Timestamp));
#endif /* LOGGING */
		data->Temperature.Type = DATA_TYPE_ERROR;
		data->Temperature.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		data->Humidity.Type = DATA_TYPE_ERROR;
		data->Humidity.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		data->Air_Pressure.Type = DATA_TYPE_ERROR;
		data->Air_Pressure.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		data->Dew_Point.Type = DATA_TYPE_ERROR;
		data->Dew_Point.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	}
	/* wind speed / direction */
	if(fdifftime(current_time,Wxt536_Data.Wind_Timestamp) < Max_Datum_Age)
	{
		/* wind speed in m/s, currently using wxt536's average value */
		data->Wind_Speed.Type = DATA_TYPE_DOUBLE;
		data->Wind_Speed.Value.DValue = Wxt536_Data.Wind_Data.Wind_Speed_Average;
		/* wind direction in degrees, currently using wxt536's average value */
		data->Wind_Direction.Type = DATA_TYPE_INT;
		data->Wind_Direction.Value.IValue = Wxt536_Data.Wind_Data.Wind_Direction_Average;
	}
	else
	{
#if LOGGING > 1
		Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,
			      "Qli50_Wxt536_Wxt536_Send_Results: Wind data out of date (%.2f s).",
			      fdifftime(current_time,Wxt536_Data.Wind_Timestamp));
#endif /* LOGGING */
		data->Wind_Speed.Type = DATA_TYPE_ERROR;
		data->Wind_Speed.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		data->Wind_Direction.Type = DATA_TYPE_ERROR;
		data->Wind_Direction.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	}
	/* diddly todo */
	/* QLI50 digital surface wetness, valid range 2..5. Output is an open collector, active low signal responds to rain. 
	** Rain is held on for 2 minutes. 
	** Basically, should be 0v when wet, and 5v when dry. */
	Wxt536_Digital_Surface_Wet_Set(current_time,&(data->Digital_Surface_Wet));
	/* analogue surface wetness, valid range 0..10. Actual DRD11A is 1v fully wet, 3v fully dry
	** I think it's actually in percent, therefore 0..10% count as dry, above that it's wet */
	Wxt536_Analogue_Surface_Wet_Set(current_time,&(data->Analogue_Surface_Wet));
	/* pyranometer */
	if(fdifftime(current_time,Wxt536_Data.Analogue_Timestamp) < Max_Datum_Age)
	{
		/* the Wxt536 pyranometer is connected to the analogue input. The Solar_Radiation_Voltage is in
		** volts (multiplied by the gain).
		** The Qli50 supplies Light as an integer, in Watts per metre squared. */
		data->Light.Type = DATA_TYPE_INT;
		data->Light.Value.IValue = Wxt536_Pyranometer_Volts_To_Watts_M2(Wxt536_Data.Analogue_Data.Solar_Radiation_Voltage);
#if LOGGING > 5
		Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERY_VERBOSE,
			      "Qli50_Wxt536_Wxt536_Send_Results: Pyranometer %d W/m^2 from %.5f v.",
					data->Light.Value.IValue,Wxt536_Data.Analogue_Data.Solar_Radiation_Voltage);
#endif /* LOGGING */
	}
	else
	{
#if LOGGING > 1
		Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,
			      "Qli50_Wxt536_Wxt536_Send_Results: Pyranometer data out of date (%.2f s).",
			      fdifftime(current_time,Wxt536_Data.Analogue_Timestamp));
#endif /* LOGGING */
		data->Light.Type = DATA_TYPE_ERROR;
		data->Light.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	}
	if(fdifftime(current_time,Wxt536_Data.Supervisor_Timestamp) < Max_Datum_Age)
	{
		/* QLI50 internal voltage is the primary power voltage - which is the Wxt536 supply voltage */
		data->Internal_Voltage.Type = DATA_TYPE_DOUBLE;
		data->Internal_Voltage.Value.DValue = Wxt536_Data.Supervisor_Data.Supply_Voltage;
	}
	else
	{
#if LOGGING > 1
		Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERBOSE,
			      "Qli50_Wxt536_Wxt536_Send_Results: Internal data out of date (%.2f s).",
			      fdifftime(current_time,Wxt536_Data.Supervisor_Timestamp));
#endif /* LOGGING */
		data->Internal_Voltage.Type = DATA_TYPE_ERROR;
		data->Internal_Voltage.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	}
	/* The Wxt536 does not supply current data? */
	/*
	data->Internal_Current.Type = DATA_TYPE_ERROR;
	data->Internal_Current.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	*/
	/* fake it */
	data->Internal_Current.Type = DATA_TYPE_DOUBLE;
	data->Internal_Current.Value.DValue = 1.2;
	/* The Wxt536 does not supply it's internal temperature. It does supply a heating temperature when the unit
	** is heated, but this can be off. */
	/*
	data->Internal_Temperature.Type = DATA_TYPE_ERROR;
	data->Internal_Temperature.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	*/
	/* fake it */
	data->Internal_Temperature.Type = DATA_TYPE_INT;
	data->Internal_Temperature.Value.IValue = 20;
	/* The Wxt536 does not supply a reference temperature, though it does have a reference voltage! 
	** N.B. Reference temperature is critical in Wms.cfg (-40...80) N.B. */
	/*
	data->Reference_Temperature.Type = DATA_TYPE_ERROR;
	data->Reference_Temperature.Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	*/
	/* fake it */
	data->Reference_Temperature.Type = DATA_TYPE_INT;
	data->Reference_Temperature.Value.Error_Code = 20;
#if LOGGING > 1
	Qli50_Wxt536_Log("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_INTERMEDIATE,
				"Qli50_Wxt536_Wxt536_Send_Results finished.");
#endif /* LOGGING */
	return TRUE;
}

/* =======================================================
** internal functions 
** ======================================================= */
/**
 * Routine to retrieve a sensor type from the config file.
 * @param keyword The keyword with a sensor type value.
 * @param sensor The address of an instance of a Sensor_Type_Enum enum, to hold the parsed sensor.
 * @return The routine returns TRUE on success and FALSE on failure. If it fails, Qli50_Wxt536_Error_Number and
 *         Qli50_Wxt536_Error_String will be set with a suitable error.
 * @see #Sensor_Type_Enum
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_String_Get
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 */
static int Wxt536_Config_Sensor_Get(char *keyword,enum Sensor_Type_Enum *sensor)
{
	char sensor_string[32];
	
	if(keyword == NULL)
	{
		Qli50_Wxt536_Error_Number = 210;
		sprintf(Qli50_Wxt536_Error_String,"Wxt536_Config_Sensor_Get:keyword is NULL.");
		return FALSE;
	}
	if(sensor == NULL)
	{
		Qli50_Wxt536_Error_Number = 211;
		sprintf(Qli50_Wxt536_Error_String,"Wxt536_Config_Sensor_Get:sensor is NULL.");
		return FALSE;
	}
	if(!Qli50_Wxt536_Config_String_Get(keyword,sensor_string,31))
		return FALSE;
	if(strcmp(sensor_string,"wxt536") == 0)
	{
		(*sensor) = SENSOR_TYPE_WXT536;
	}
	else if(strcmp(sensor_string,"drd11a") == 0)
	{
		(*sensor) = SENSOR_TYPE_DRD11A;
	}
	else
	{
		Qli50_Wxt536_Error_Number = 212;
		sprintf(Qli50_Wxt536_Error_String,
			"Wxt536_Config_Sensor_Get:keyword '%s' contained illegal sensor value '%s'.",keyword,sensor_string);
		return FALSE;
	}
	return TRUE;
}

/**
 * Routine to calculate the dew point based on the temperature and relative humidity.
 * This is based on the QLI50 formula, documented in the QLI50 manual, P62 'TDEW Calculation Channel'.
 * @param pth_data An instance of Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct containing
 *        the pressure and humidity readings.
 * @return The routine returns the dew point as a double, in degrees centigrade.
 */
static double Wxt536_Calculate_Dew_Point(struct Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct pth_data)
{
	double tdew,a,b,c,lower;

	a = log(100.0/pth_data.Relative_Humidity);
	b = (15.0 * a)- (2.1 * pth_data.Air_Temperature) + 2711.5;
	c = pth_data.Air_Temperature + 273.16;
	lower = (c * (a/2.0)) + b;
       	tdew = ((c * b)/lower) - 273.16;
	return tdew;
}

/**
 * Internal function to convert the voltage returned by the pyranometer to a light value in watts/m^2.
 * The calculation involves the calibration of the CMP3, and the gain applied by the Wxt536.
 * @param voltage The voltage returned by the Wxt536 for the pyranometer (in Volts multiplied by the gain).
 * @return The amount of light falling on the sensor, as an integer, in Watts/ M^2.
 * @see Wxt536_Pyranometer_Gain
 * @see CMP3_Pyranometer_Sensitivity
 */
static int Wxt536_Pyranometer_Volts_To_Watts_M2(double voltage)
{
	double a,b;
	int light;
	
	/* The CMP3 Pyranometer Sensitivity is in uV/W/m^2 */
	a = voltage/Wxt536_Pyranometer_Gain;
	b =  CMP3_Pyranometer_Sensitivity/1000000.0;
	light = (int)(a/b);
#if LOGGING > 1
	Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERY_VERBOSE,
				"Wxt536_Pyranometer_Volts_To_Watts_M2:"
				"Pyranometer voltage %.3f v, gain %.3f, sensitivity %.3f uV/W/m^2, "
				"a = %.6f, b = %.6f, light = %d W / M^2.",voltage,Wxt536_Pyranometer_Gain,
				CMP3_Pyranometer_Sensitivity,a,b,light);
#endif /* LOGGING */
	return light;
}

/**
 * Routine to set the digital surface wet value based on the Wxt536 precipitation data and Wxt536 analogue data (with DRD11A
 * rain sensor attached).
 * We think the QLI50 digital surface wet value should be 0v when wet, and 5v when dry.
 * We will replace that (when using the Wxt536 piezzo sensor) with the Rain intensity in mm/h.
 * @param current_time An instance of struct timespec representing the current time, we use this to compare
 *        with the data timestamps to ensure the data has not gone out of date.
 * @param digital_surface_wet_value The instance of Wms_Qli50_Data_Value to fill in with the QLI50 digital surface wet
 *        value to return.
 * @see #Wxt536_Data
 * @see #Wxt536_Data_Struct
 * @see #Max_Datum_Age
 * @see #Digital_Surface_Wet_Sensor
 * @see #Digital_Surface_Wet_Drd11a_Threshold
 * @see ../qli50/cdocs/wms_qli50_command.html#QLI50_ERROR_NO_MEASUREMENT
 * @see ../qli50/cdocs/wms_qli50_command.html#Wms_Qli50_Data_Value
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Precipitation_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Analogue_Data_Get
 */
static void Wxt536_Digital_Surface_Wet_Set(struct timespec current_time,
					   struct Wms_Qli50_Data_Value *digital_surface_wet_value)
{
	/* do we want to use the wxt536 piezo sensor to determine this? */
	if(Digital_Surface_Wet_Sensor == SENSOR_TYPE_WXT536)
	{
		/* if the precipitation timestamp is new enough use precipitation */
		if(fdifftime(current_time,Wxt536_Data.Rain_Timestamp) < Max_Datum_Age)
		{
#if LOGGING > 1
			Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERY_VERBOSE,
						"Wxt536_Digital_Surface_Wet_Set:"
						"Using Wxt536 Piezo rain sensor with rain intensity %.2f mm/h "
						"and hail intensity %.2f hits/cm^2h.",
						Wxt536_Data.Rain_Data.Rain_Intensity,Wxt536_Data.Rain_Data.Hail_Intensity);
#endif /* LOGGING */
			/* digital surface wetness type was INT, can I arbitarily change it to DOUBLE? */
			digital_surface_wet_value->Type = DATA_TYPE_DOUBLE;
			digital_surface_wet_value->Value.DValue = Wxt536_Data.Rain_Data.Rain_Intensity;
			/*
			if((Wxt536_Data.Rain_Data.Rain_Intensity > 0.0)||(Wxt536_Data.Rain_Data.Hail_Intensity > 0.0))
			{
				digital_surface_wet_value->Type = DATA_TYPE_INT;
				digital_surface_wet_value->Value.IValue = 0;*/ /* wet */
			/*}
			else
			{
				digital_surface_wet_value->Type = DATA_TYPE_INT;
				digital_surface_wet_value->Value.IValue = 4;*/ /* dry */
			/*}*/
		}
		else
		{
			/* we have no up to date measurement */
			digital_surface_wet_value->Type = DATA_TYPE_ERROR;
			digital_surface_wet_value->Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		}
	}
	/* do we want to use the drd11a sensor to determine this? */
	else if(Digital_Surface_Wet_Sensor == SENSOR_TYPE_DRD11A)
	{
		/* if the analogue timestamp is new enough use the DRD11A */
		if(fdifftime(current_time,Wxt536_Data.Analogue_Timestamp) < Max_Datum_Age)
		{
#if LOGGING > 1
			Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERY_VERBOSE,
						"Wxt536_Digital_Surface_Wet_Set:"
						"Using DRD11A rain sensor with voltage %.3f v (3v dry, 1v wet).",
						Wxt536_Data.Analogue_Data.Ultrasonic_Level_Voltage);
#endif /* LOGGING */
			/* The DRD11A is connected to the Ultrasonic Level analogue input.
			** This should read 3v fully dry, 1v fully wet. */
			if(Wxt536_Data.Analogue_Data.Ultrasonic_Level_Voltage < Digital_Surface_Wet_Drd11a_Threshold)
			{
				digital_surface_wet_value->Type = DATA_TYPE_INT;
				digital_surface_wet_value->Value.IValue = 0; /* wet */
			}
			else
			{
				digital_surface_wet_value->Type = DATA_TYPE_INT;
				digital_surface_wet_value->Value.IValue = 4; /* dry */
			}
		}
		else
		{
			/* we have no up to date measurement */
			digital_surface_wet_value->Type = DATA_TYPE_ERROR;
			digital_surface_wet_value->Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		}
	}
	else
	{
		/* we don't know what sensor to use for this datum */
		digital_surface_wet_value->Type = DATA_TYPE_ERROR;
		digital_surface_wet_value->Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	}
}

/**
 * Routine to set the analogue surface wet value based on the Wxt536 precipitation data and Wxt536 analogue data (with DRD11A
 * rain sensor attached).
 * We think the QLI50 analogue surface wet value is meant to be a percentage 0..100, 
 * the Wms counts 0..10 as dry and above that as wet (i.e. the Wms goes into suspend above 10%).
 * @param current_time An instance of struct timespec representing the current time, we use this to compare
 *        with the data timestamps to ensure the data has not gone out of date.
 * @param analogue_surface_wet_value The instance of Wms_Qli50_Data_Value to fill in with the QLI50 analogue surface wet
 *        value to return.
 * @see #Wxt536_Data
 * @see #Wxt536_Data_Struct
 * @see #Max_Datum_Age
 * @see #Analogue_Surface_Wet_Sensor
 * @see #Analogue_Surface_Wet_Drd11a_Wet_Point
 * @see #Analogue_Surface_Wet_Drd11a_Scale
 * @see #Analogue_Surface_Wet_Wxt536_Scale
 * @see ../qli50/cdocs/wms_qli50_command.html#QLI50_ERROR_NO_MEASUREMENT
 * @see ../qli50/cdocs/wms_qli50_command.html#Wms_Qli50_Data_Value
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Precipitation_Data_Get
 * @see ../wxt536/cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Analogue_Data_Get
 */
static void Wxt536_Analogue_Surface_Wet_Set(struct timespec current_time,
					    struct Wms_Qli50_Data_Value *analogue_surface_wet_value)
{
	/* do we want to use the drd11a sensor to determine this? */
	if(Analogue_Surface_Wet_Sensor == SENSOR_TYPE_DRD11A)
	{
		/* if the analogue timestamp is new enough use the DRD11A */
		if(fdifftime(current_time,Wxt536_Data.Analogue_Timestamp) < Max_Datum_Age)
		{
			/* The DRD11A is connected to the Ultrasonic Level analogue input.
			** This should read 3v fully dry, 1v fully wet. */
			analogue_surface_wet_value->Type = DATA_TYPE_INT;
			analogue_surface_wet_value->Value.IValue = 100-(int)((Wxt536_Data.Analogue_Data.Ultrasonic_Level_Voltage-Analogue_Surface_Wet_Drd11a_Wet_Point)*Analogue_Surface_Wet_Drd11a_Scale);
			if(analogue_surface_wet_value->Value.IValue < 0)
				analogue_surface_wet_value->Value.IValue = 0;
			if(analogue_surface_wet_value->Value.IValue > 100)
				analogue_surface_wet_value->Value.IValue = 100;
#if LOGGING > 1
			Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERY_VERBOSE,
						"Wxt536_Analogue_Surface_Wet_Set:"
						"Using DRD11A rain sensor with voltage %.3f v (3v dry, 1v wet), "
						"using wet point %.3f v and scale %.2, giving value %d %%.",
						Wxt536_Data.Analogue_Data.Ultrasonic_Level_Voltage,
						Analogue_Surface_Wet_Drd11a_Wet_Point,Analogue_Surface_Wet_Drd11a_Scale,
						analogue_surface_wet_value->Value.IValue);
#endif /* LOGGING */
		}
		else
		{
			/* we have no up to date measurement */
			analogue_surface_wet_value->Type = DATA_TYPE_ERROR;
			analogue_surface_wet_value->Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		}
	}
	/* do we want to use the wxt536 piezo sensor to determine this? */
	else if(Analogue_Surface_Wet_Sensor == SENSOR_TYPE_WXT536)
	{
		/* if the precipitation timestamp is new enough use precipitation */
		if(fdifftime(current_time,Wxt536_Data.Rain_Timestamp) < Max_Datum_Age)
		{
			analogue_surface_wet_value->Type = DATA_TYPE_INT;
			/* rain intensity is measured in mm/h. We and scale it (with range checking) with 
			** Analogue_Surface_Wet_Wxt536_Scale. A Analogue_Surface_Wet_Wxt536_Scale value of 100.0 
			** means a rain intensity of 1mm/h converts to an analaogue wetness of 100%  */
			analogue_surface_wet_value->Value.IValue = (int)(Wxt536_Data.Rain_Data.Rain_Intensity*
									 Analogue_Surface_Wet_Wxt536_Scale);
			if(analogue_surface_wet_value->Value.IValue < 0)
				analogue_surface_wet_value->Value.IValue = 0;
			if(analogue_surface_wet_value->Value.IValue > 100)
				analogue_surface_wet_value->Value.IValue = 100;
#if LOGGING > 1
			Qli50_Wxt536_Log_Format("Wxt536","qli50_wxt536_wxt536.c",LOG_VERBOSITY_VERY_VERBOSE,
						"Wxt536_Analogue_Surface_Wet_Set:"
						"Using Wxt536 Piezo rain sensor with rain intensity %.3f mm/h giving value %d %%.",
						Wxt536_Data.Rain_Data.Rain_Intensity,
						analogue_surface_wet_value->Value.IValue);
#endif /* LOGGING */
		}
		else
		{
			/* we have no up to date measurement */
			analogue_surface_wet_value->Type = DATA_TYPE_ERROR;
			analogue_surface_wet_value->Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
		}
	}
	else
	{
		/* we don't know what sensor to use for this datum */
		analogue_surface_wet_value->Type = DATA_TYPE_ERROR;
		analogue_surface_wet_value->Value.Error_Code = QLI50_ERROR_NO_MEASUREMENT;
	}
}
