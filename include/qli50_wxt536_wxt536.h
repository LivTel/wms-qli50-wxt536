/* qli50_wxt536_wxt536.h
 */
#ifndef QLI50_WXT536_WXT536_H
#define QLI50_WXT536_WXT536_H

#include "wms_qli50_command.h" /* for declaration of Wms_Qli50_Data_Struct */

extern int Qli50_Wxt536_Wxt536_Initialise(void);
extern int Qli50_Wxt536_Wxt536_Close(void);
extern int Qli50_Wxt536_Wxt536_Read_Sensors(char qli_id,char seq_id);
extern int Qli50_Wxt536_Wxt536_Send_Results(char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data);

#endif
