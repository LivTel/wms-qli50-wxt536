/* wms_qli50_server.h
 */
#ifndef WMS_QLI50_SERVER_H
#define WMS_QLI50_SERVER_H
/**
 * Typedef for a function pointer to be invoked when a Read Sensor command is received by the server.
 */
typedef int (*Read_Sensor_Callback_T)(char qli_id,char seq_id);

/**
 * Typedef for a function pointer to be invoked when a Send Results command is received by the server.
 */
typedef int (*Send_Result_Callback_T)(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data);
	
/**
 * Typedef for a function pointer to be invoked when a PAR command is received by the server.
 */
typedef int (*Par_Callback_T)(char *return_parameter_string,int return_parameter_string_length);
/**
 * Typedef for a function pointer to be invoked when a STA (status) command is received by the server.
 */
typedef int (*Sta_Callback_T)(char *return_string,int return_string_length);
	
extern int Wms_Qli50_Server_Set_Read_Sensor_Callback(char *class,char *source,Read_Sensor_Callback_T callback);
extern int Wms_Qli50_Server_Set_Send_Result_Callback(char *class,char *source,Send_Result_Callback_T callback);
extern int Wms_Qli50_Server_Set_Par_Callback(char *class,char *source,Par_Callback_T callback);
extern int Wms_Qli50_Server_Set_Sta_Callback(char *class,char *source,Sta_Callback_T callback);
extern int Wms_Qli50_Server_Start(char *class,char *source,char *device_name);
extern int Wms_Qli50_Server_Loop(char *class,char *source);

#endif
