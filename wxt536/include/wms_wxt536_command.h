/* wms_wxt536_command.h
 */
#ifndef WMS_WXT536_COMMAND_H
#define WMS_WXT536_COMMAND_H

/**
 * Protocol value to use to set the Wxt536 communication protocol. 'A' means ASCII automatic.
 * @see #Wms_Wxt536_Command_Comms_Settings_Protocol_Set
 */
#define WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC       ('A')
/**
 * Protocol value to use to set the Wxt536 communication protocol. 'a' means ASCII automatic with CRC.
 * @see #Wms_Wxt536_Command_Comms_Settings_Protocol_Set
 */
#define WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC_CRC   ('a')
/**
 * Protocol value to use to set the Wxt536 communication protocol. 'P' means ASCII polled.
 * @see #Wms_Wxt536_Command_Comms_Settings_Protocol_Set
 */
#define WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED          ('P')
/**
 * Protocol value to use to set the Wxt536 communication protocol. 'p' means ASCII polled with CRC.
 * @see #Wms_Wxt536_Command_Comms_Settings_Protocol_Set
 */
#define WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED_CRC      ('p')

/**
 * Data structure containing data parsed from a reply to a Request Current Communication Settings Message (aXU).
 * See the WXT530-Users-Guide-M211840EN.pdf, Section 6.4/P72/P74.
 * <dl>
 * <dt>Address</dt> <dd>A character, 0..9,A..Z,a..z, the device address.</dd>
 * <dt>Protocol</dt> <dd>A character, the communication protocol:
 *                       <ul>
 *                       <li><b>A</b> - ASCII, automatic
 *                       <li><b>a</b> - ASCII, automatic with CRC
 *                       <li><b>P</b> - ASCII, polled
 *                       <li><b>p</b> - ASCII, polled, with CRC
 *                       <li><b>N</b> - NMEA 0183 v3.0, automatic
 *                       <li><b>Q</b> - NMEA 0183 v3.0, query (= polled)
 *                       <li><b>S</b> - SDI-12 v1.3
 *                       <li><b>R</b> - SDI-12 v1.3 continuous measurement
 *                       </ul>
 *                   </dd>
 * <dt>Serial_Interface</dt> <dd>A character, the type of serial interface, one of:
 *                           <ul>
 *                           <li><b>1</b> - SDI-12
 *                           <li><b>2</b> - RS-232
 *                           <li><b>3</b> - RS-485
 *                           <li><b>4</b> - RS-422
 *                           </ul>
 *                           </dd>
 * <dt>Composite_Repeat_Interval</dt> <dd>An integer, Automatic repeat interval for Composite data message: 
 *                                        1 ... 3600 s, 0 = no automatic repeat</dd>
 * <dt>Baud_Rate</dt> <dd>An integer, Baud rate: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200</dd>
 * <dt>Data_Bits</dt> <dd>An integer, Data bits: 7/8</dd>
 * <dt>Parity</dt> <dd>A character, Parity:
 *                     <ul>
 *                     <li><b>O</b> - Odd.
 *                     <li><b>E</b> - Even.
 *                     <li><b>N</b> - None.
 *                     </ul>
 *                 </dd>
 * <dt>Stop_Bits</dt> <dd>An integer, Stop bits: 1/2.</dd>
 * <dt>RS485_Line_Delay</dt> <dd>An integer, RS-485 line delay: 0 ... 10000 ms.</dd>
 * <dt>Device_Name</dt> <dd></dd>
 * <dt>Software_Version</dt> <dd></dd>
 * <dt>Parameter_Locking</dt> <dd></dd>
 * </dl>
 */
struct Wxt536_Command_Comms_Settings_Struct
{
	char Address;
	char Protocol;
	char Serial_Interface;
	int Composite_Repeat_Interval;
	int Baud_Rate;
	int Data_Bits;
	char Parity;
	int Stop_Bits;
	int RS485_Line_Delay;
	char Device_Name[256];
	char Software_Version[256];
	int Parameter_Locking;
};

/**
 * Data structure containing data parsed from a reply to a Wind Data Message (aR1).
 * <dl>
 * <dt>Wind_Direction_Minimum</dt> <dd>A double representing the wind direction minimum in degrees.</dd>
 * <dt>Wind_Direction_Average</dt> <dd>A double representing the wind direction average in degrees.</dd>
 * <dt>Wind_Direction_Maximum</dt> <dd>A double representing the wind direction maximum in degrees.</dd>
 * <dt>Wind_Speed_Minimum</dt> <dd>A double representing the wind speed minimum in metres/s.</dd>
 * <dt>Wind_Speed_Average</dt> <dd>A double representing the wind speed average in metres/s.</dd>
 * <dt>Wind_Speed_Maximum</dt> <dd>A double representing the wind speed maximum in metres/s.</dd>
 * </dl>
 */
struct Wxt536_Command_Wind_Data_Struct
{
	double Wind_Direction_Minimum;
	double Wind_Direction_Average;
	double Wind_Direction_Maximum;
	double Wind_Speed_Minimum;
	double Wind_Speed_Average;
	double Wind_Speed_Maximum;
};

/**
 * Data structure containing data parsed from a reply to a Pressure/Temperature/Humidity Data Message (aR2).
 * <dl>
 * <dt>Air_Temperature</dt> <dd>A double representing the air temperature in degrees Centigrade.</dd>
 * <dt>Relative_Humidity</dt> <dd>A double representing the relative humidity in percent RH.</dd>
 * <dt>Air_Pressure</dt> <dd>A double representing the air pressure in hPA (Hectopascals = mbar).</dd>
 * </dl>
 */
struct Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct
{
	double Air_Temperature;
	double Relative_Humidity;
	double Air_Pressure;
};

/**
 * Data structure containing data parsed from a reply to a Precipitation Data Message (aR3).
 * <dl>
 * <dt>Rain_Accumulation</dt> <dd>A double representing the rain accumulation in mm.</dd>
 * <dt>Rain_Duration</dt> <dd>A double representing the rain duration in seconds.</dd>
 * <dt>Rain_Intensity</dt> <dd>A double representing the rain intensity in mm/h.</dd>
 * <dt>Hail_Accumulation</dt> <dd>A double representing the hail accumulation in hits/cm^2.</dd>
 * <dt>Hail_Duration</dt> <dd>A double representing the hail duration in seconds.</dd>
 * <dt>Hail_Intensity</dt> <dd>A double representing the hail intensity in hits/cm^2h.</dd>
 * <dt>Rain_Peak_Intensity</dt> <dd>A double representing the rain peak intensity in mm/h. 
 *                              Not returned by our weather station.</dd>
 * <dt>Hail_Peak_Intensity</dt> <dd>A double representing the hail peak intensity in hits/cm^2h. 
 *                              Not returned by our weather station.</dd>
 * </dl>
 */
struct Wxt536_Command_Precipitation_Data_Struct
{
	double Rain_Accumulation;
	double Rain_Duration;
	double Rain_Intensity;
	double Hail_Accumulation;
	double Hail_Duration;
	double Hail_Intensity;
	double Rain_Peak_Intensity;
	double Hail_Peak_Intensity;
};

/**
 * Data structure containing data parsed from a reply to a Precipitation Data Message (aR3).
 * <dl>
 * <dt>Heating_Temperaure</dt> <dd>A double representing the heating temperature in degrees Centigrade.</dd>
 * <dt>Heating_Voltage</dt> <dd>A double representing the heating voltage in Volts?.</dd>
 * <dt>Supply_Voltage</dt> <dd>A double representing the supply voltage in Volts.</dd>
 * <dt>Reference_Voltage</dt> <dd>A double representing the reference voltage in Volts. The reference voltage is
 *     nominally 3.5V.</dd>
 * <dt>Information</dt> <dd>A string containing the returned information field. 
 *                      This field does not seem to be returned by our unit.</dd>
 * </dl>
 */
struct Wxt536_Command_Supervisor_Data_Struct
{
	double Heating_Temperaure;
	double Heating_Voltage;
	double Supply_Voltage;
	double Reference_Voltage;
	char Information[256];
};

/**
 * Data structure containing data parsed from a reply to a Analog Data Message (aR4).
 * <dl>
 * <dt>PT1000_Temperaure</dt> <dd>A double representing the PT1000 temperature in degrees Centigrade. 
 *                            We do not have this sensor connected to our unit.</dd>
 * <dt>Aux_Rain_Accumulation</dt> <dd>A double representing auxiliary rain accumulation in mm. 
 *                                We do not have this sensor connected to our unit.</dd>
 * <dt>Ultrasonic_Level_Voltage</dt> <dd>A double representing the ultrasonic level input voltage in Volts
 *                                   (multiplied by the gain). We have a DRD11A rain sensor attached to this input.</dd>
 * <dt>Solar_Radiation_Voltage</dt> <dd>A double representing the voltage returned by the pyranometer 
 *                                  (in Volts multiplied by the gain)</dd>
 * </dl>
 */
struct Wxt536_Command_Analogue_Data_Struct
{
	double PT1000_Temperaure;
	double Aux_Rain_Accumulation;
	double Ultrasonic_Level_Voltage;
	double Solar_Radiation_Voltage;
};

extern int Wms_Wxt536_Command(char *class,char *source,char *command_string,char *reply_string,int reply_string_length);
extern int Wms_Wxt536_Command_Device_Address_Get(char *class,char *source,char *device_address);
extern int Wms_Wxt536_Command_Ack_Active(char *class,char *source,char device_address);
extern int Wms_Wxt536_Command_Comms_Settings_Get(char *class,char *source,char device_address,
						 struct Wxt536_Command_Comms_Settings_Struct *comms_settings);
extern int Wms_Wxt536_Command_Comms_Settings_Protocol_Set(char *class,char *source,char device_address,char protocol);
extern int Wms_Wxt536_Command_Reset(char *class,char *source,char device_address);
extern int Wms_Wxt536_Command_Wind_Data_Get(char *class,char *source,char device_address,
					    struct Wxt536_Command_Wind_Data_Struct *data);
extern int Wms_Wxt536_Command_Pressure_Temperature_Humidity_Data_Get(char *class,char *source,char device_address,
					    struct Wxt536_Command_Pressure_Temperature_Humidity_Data_Struct *data);
extern int Wms_Wxt536_Command_Precipitation_Data_Get(char *class,char *source,char device_address,
						     struct Wxt536_Command_Precipitation_Data_Struct *data);
extern int Wms_Wxt536_Command_Supervisor_Data_Get(char *class,char *source,char device_address,
						  struct Wxt536_Command_Supervisor_Data_Struct *data);
extern int Wms_Wxt536_Command_Analogue_Data_Get(char *class,char *source,char device_address,
						struct Wxt536_Command_Analogue_Data_Struct *data);
#endif
