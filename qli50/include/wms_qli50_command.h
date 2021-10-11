/* wms_qli50_command.h
 */
#ifndef WMS_QLI50_COMMAND_H
#define WMS_QLI50_COMMAND_H

extern int Wms_Qli50_Command(char *class,char *source,char *command_string,char *reply_string,int reply_string_length);
extern int Wms_Qli50_Command_Close(char *class,char *source);
extern int Wms_Qli50_Command_Echo(char *class,char *source,int onoff);
extern int Wms_Qli50_Command_Open(char *class,char *source,char qli_id);
extern int Wms_Qli50_Command_Par(char *class,char *source,char *reply_string,int reply_string_length);
extern int Wms_Qli50_Command_Reset(char *class,char *source);
extern int Wms_Qli50_Command_Sta(char *class,char *source,char *reply_string,int reply_string_length);
extern int Wms_Qli50_Command_Read_Sensors(char *class,char *source,char qli_id,char seq_id);
extern int Wms_Qli50_Command_Send_Results(char *class,char *source,char qli_id,char seq_id);

#endif
