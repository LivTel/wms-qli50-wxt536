/* wms_wxt536_command.h
 */
#ifndef WMS_WXT536_COMMAND_H
#define WMS_WXT536_COMMAND_H

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
 * 
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
extern int Wms_Wxt536_Command_Current_Comms_Settings_Get(char *class,char *source,char *comms_settings);
extern int Wms_Wxt536_Command_Reset(char *class,char *source);
extern int Wms_Wxt536_Command_Wind_Data_Get(char *class,char *source,struct Wxt536_Command_Wind_Data_Struct *data);

#endif
