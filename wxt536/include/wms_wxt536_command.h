/* wms_wxt536_command.h
 */
#ifndef WMS_WXT536_COMMAND_H
#define WMS_WXT536_COMMAND_H

#define WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC       ('A')
#define WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC_CRC   ('a')
#define WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED          ('P')
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

extern int Wms_Wxt536_Command(char *class,char *source,char *command_string,char *reply_string,int reply_string_length);
extern int Wms_Wxt536_Command_Device_Address_Get(char *class,char *source,int *device_number);
extern int Wms_Wxt536_Command_Ack_Active(char *class,char *source,int device_number);
extern int Wms_Wxt536_Command_Comms_Settings_Get(char *class,char *source,int device_number,
						 struct Wxt536_Command_Comms_Settings_Struct *comms_settings);
extern int Wms_Wxt536_Command_Comms_Settings_Protocol_Set(char *class,char *source,int device_number,char protocol);
extern int Wms_Wxt536_Command_Reset(char *class,char *source);
extern int Wms_Wxt536_Command_Wind_Data_Get(char *class,char *source,struct Wxt536_Command_Wind_Data_Struct *data);

#endif
