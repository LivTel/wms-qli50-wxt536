/* wms_qli50_command.h
 */
#ifndef WMS_QLI50_COMMAND_H
#define WMS_QLI50_COMMAND_H

/* hash defines */
/**
 * The terminator used to delimit the end of each line read, as a string. &lt;cr&gt; in this case.
 */
#define TERMINATOR_CR         ("\r")
/**
 * The terminator used to delimit the end of each line read, as a string. &lt;cr&gt;&lt;lf&gt; in this case.
 */
#define TERMINATOR_CRLF         ("\r\n")
/**
 * ENQ (Enquiry) control character (Ctrl-E).
 */
#define CHARACTER_ENQ         ('\x05')
/**
 * ETX (End of text) control character (Ctrl-C).
 */
#define CHARACTER_ETX         ('\x03')
/**
 * SOH (Start of header) control character (Ctrl-A).
 */
#define CHARACTER_SOH         ('\x01')
/**
 * STX (Start of text) control character (Ctrl-B).
 */
#define CHARACTER_STX         ('\x02')
/**
 * SYN (Synchronous idle) control character (Ctrl-V).
 */
#define CHARACTER_SYN         ('\x16')

/* enum */
/**
 * An enumeration describing the data value returned. This is one of:
 * <ul>
 * <li>DATA_TYPE_DOUBLE
 * <li>DATA_TYPE_INT
 * <li>DATA_TYPE_ERROR
 * </ul>
 */
enum Wms_Qli50_Data_Type_Enum
{
	DATA_TYPE_DOUBLE,DATA_TYPE_INT,DATA_TYPE_ERROR
};

/* structures */
/**
 * Structure describing one data value returned from a Send Results command.
 * <dl>
 * <dt>Type</dt> <dd>Whether the data value returne was a double, integer, or an error code.</dd>
 * <dt>Value</dt> <dd>A union containing the result data.</dd>
 * </dl>
 * @see #Wms_Qli50_Data_Type_Enum
 */
struct Wms_Qli50_Data_Value
{
	enum Wms_Qli50_Data_Type_Enum Type;
	union
	{
		double DValue;
		int IValue;
		int Error_Code;
	} Value;
};
/**
 * Data structure holding the weather data returned from the QLI50.
 * <dl>
 * <dt>Temperature</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt>Humidity</dt> <dd>A double, the relative humidity as a percentage (0..100).</dd>
 * <dt>Dew_Point</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt>Wind_Speed</dt> <dd>A double in metres/second.</dd>
 * <dt>Wind_Direction</dt> <dd>An integer in degrees (0 = north?).</dd>
 * <dt>Air_Pressure</dt> <dd>A double in hPa/millibars.</dd>
 * <dt>Digital_Surface_Wet</dt> <dd>An integer, digital wetness in Volts (0..5).</dd>
 * <dt>Analogue_Surface_Wet</dt> <dd>An integer, analogue wetness in percent (%) (0..100).</dd>
 * <dt>Light</dt> <dd>An integer in Watts per metre squared.</dd>
 * <dt>Internal_Voltage</dt> <dd>A double in Volts.</dd>
 * <dt>Internal_Current</dt> <dd>A double in milliamps.</dd>
 * <dt>Internal_Temperature</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt>Reference_Temperature</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt></dt> <dd>.</dd>
 * </dl>
 * @see #Wms_Qli50_Data_Value
 */
struct Wms_Qli50_Data_Struct
{
	struct Wms_Qli50_Data_Value Temperature;
	struct Wms_Qli50_Data_Value Humidity;
	struct Wms_Qli50_Data_Value Dew_Point;
	struct Wms_Qli50_Data_Value Wind_Speed;
	struct Wms_Qli50_Data_Value Wind_Direction;
	struct Wms_Qli50_Data_Value Air_Pressure;
	struct Wms_Qli50_Data_Value Digital_Surface_Wet;
	struct Wms_Qli50_Data_Value Analogue_Surface_Wet;
	struct Wms_Qli50_Data_Value Light;
	struct Wms_Qli50_Data_Value Internal_Voltage;
	struct Wms_Qli50_Data_Value Internal_Current;
	struct Wms_Qli50_Data_Value Internal_Temperature;
	struct Wms_Qli50_Data_Value Reference_Temperature;
};


extern int Wms_Qli50_Command(char *class,char *source,char *command_string,
			     char *reply_string,int reply_string_length,char *reply_terminator);
extern int Wms_Qli50_Command_Close(char *class,char *source);
extern int Wms_Qli50_Command_Echo(char *class,char *source,int onoff);
extern int Wms_Qli50_Command_Open(char *class,char *source,char qli_id);
extern int Wms_Qli50_Command_Par(char *class,char *source,char *reply_string,int reply_string_length);
extern int Wms_Qli50_Command_Reset(char *class,char *source);
extern int Wms_Qli50_Command_Sta(char *class,char *source,char *reply_string,int reply_string_length);
extern int Wms_Qli50_Command_Read_Sensors(char *class,char *source,char qli_id,char seq_id);
extern int Wms_Qli50_Command_Send_Results(char *class,char *source,char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data);

#endif
