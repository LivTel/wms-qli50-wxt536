/* wms_wxt536_general.h
 */
#ifndef WMS_WXT536_GENERAL_H
#define WMS_WXT536_GENERAL_H
/* hash defines */
/**
 * TRUE is the value usually returned from routines to indicate success.
 */
#ifndef TRUE
#define TRUE 1
#endif
/**
 * FALSE is the value usually returned from routines to indicate failure.
 */
#ifndef FALSE
#define FALSE 0
#endif

/**
 * How long the error string is.
 */
#define WMS_WXT536_ERROR_LENGTH (1024)
/**
 * One millosecond in nanoseconds (1000000).
 */
#define WMS_WXT536_ONE_MILLISECOND_NS   (1000000)

/**
 * Macro to check whether the parameter is either TRUE or FALSE.
 */
#define WMS_WXT536_IS_BOOLEAN(value)	(((value) == TRUE)||((value) == FALSE))
/**
 * Macro to check whether the parameter is a sign character, i.e. either '+' or '-'.
 */
#define WMS_WXT536_IS_SIGN(value)	(((value) == '+')||((value) == '-'))

/* external functions */
extern void Wms_Wxt536_Error(void);
extern void Wms_Wxt536_Error_To_String(char *error_string);
extern int Wms_Wxt536_Get_Error_Number(void);
extern void Wms_Wxt536_Get_Current_Time_String(char *time_string,int string_length);
extern void Wms_Wxt536_Log_Format(char *class,char *source,int level,char *format,...);
extern void Wms_Wxt536_Log(char *class,char *source,int level,char *string);
extern void Wms_Wxt536_Set_Log_Handler_Function(void (*log_fn)(char *class,char *source,int level,char *string));
extern void Wms_Wxt536_Set_Log_Filter_Function(int (*filter_fn)(char *class,char *source,int level,char *string));
extern void Wms_Wxt536_Log_Handler_Stdout(char *class,char *source,int level,char *string);
extern void Wms_Wxt536_Set_Log_Filter_Level(int level);
extern int Wms_Wxt536_Log_Filter_Level_Absolute(char *class,char *source,int level,char *string);
extern int Wms_Wxt536_Log_Filter_Level_Bitwise(char *class,char *source,int level,char *string);

/* external variables */
extern int Wms_Wxt536_Error_Number;
extern char Wms_Wxt536_Error_String[];

#endif
