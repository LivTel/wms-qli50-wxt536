/* wms_qli50_server.h
 */
#ifndef WMS_QLI50_SERVER_H
#define WMS_QLI50_SERVER_H
/**
 * Typedef for a fucntion pointer to be invoked when a Read Sensor command is received by the server.
 */
typedef int (*Read_Sensor_Callback)(char qli_id,char seq_id);

/**
 * Typedef for a fucntion pointer to be invoked when a Send Results command is received by the server.
 */
typedef int (*Send_Result_Callback)(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data);
	
extern int Wms_Qli50_Server_Set_Read_Sensor_Callback(Read_Sensor_Callback callback);
extern int Wms_Qli50_Server_Set_Send_Result_Callback(Send_Result_Callback callback);
extern int Wms_Qli50_Server_Start(char *device_name);
extern int Wms_Qli50_Server_Loop(void);

#endif
