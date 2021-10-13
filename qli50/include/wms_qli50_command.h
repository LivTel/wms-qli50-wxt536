/* wms_qli50_command.h
 */
#ifndef WMS_QLI50_COMMAND_H
#define WMS_QLI50_COMMAND_H

/**
 * Data structure holding the weather data returned from the QLI50.
 * <dl>
 * <dt>Temperature</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt>Humidity</dt> <dd>A double, the relative humidity as a percentage (0..100).</dd>
 * <dt>Dew_Point</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt>Wind_Speed</dt> <dd>A double in metres/second.</dd>
 * <dt>Wind_Direction</dt> <dd>A double(?) in degrees (0 = north?).</dd>
 * <dt>Air_Pressure</dt> <dd>A double in millibars.</dd>
 * <dt>Digital_Surface_Wet</dt> <dd>An integer.</dd>
 * <dt>Analogue_Surface_Wet</dt> <dd>An integer.</dd>
 * <dt>Light</dt> <dd>An integer in Watts per metre squared.</dd>
 * <dt>Internal_Voltage</dt> <dd>A double in Volts.</dd>
 * <dt>Internal_Current</dt> <dd>A double in Amps.</dd>
 * <dt>Internal_Temperature</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt>Reference_Temperature</dt> <dd>A double in degrees Centigrade.</dd>
 * <dt></dt> <dd>.</dd>
 * </dl>
 */
struct Wms_Qli50_Data_Struct
{
	double Temperature;
	double Humidity;
	double Dew_Point;
	double Wind_Speed;
	double Wind_Direction;
	double Air_Pressure;
	int Digital_Surface_Wet;
	int Analogue_Surface_Wet;
	int Light;
	double Internal_Voltage;
	double Internal_Current;
	double Internal_Temperature;
	double Reference_Temperature;
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
