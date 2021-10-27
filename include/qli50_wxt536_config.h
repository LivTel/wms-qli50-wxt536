/* qli50_wxt536_config.h
 */
#ifndef QLI50_WXT536_CONFIG_H
#define QLI50_WXT536_CONFIG_H

extern int Qli50_Wxt536_Config_Filename_Set(char *filename);
extern int Qli50_Wxt536_Config_Load(void);
extern int Qli50_Wxt536_Config_String_Get(char *keyword,char *value,int value_length);
extern int Qli50_Wxt536_Config_Int_Get(char *keyword,int *value);
extern int Qli50_Wxt536_Config_Double_Get(char *keyword,double *value);
extern int Qli50_Wxt536_Config_Boolean_Get(char *keyword,int *value);

#endif
