/* wms_serial_general.h
 */
#ifndef WMS_SERIAL_GENERAL_H
#define WMS_SERIAL_GENERAL_H
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
#define WMS_SERIAL_ERROR_LENGTH (1024)
/**
 * One millosecond in nanoseconds (1000000).
 */
#define WMS_SERIAL_ONE_MILLISECOND_NS   (1000000)

/**
 * Macro to check whether the parameter is either TRUE or FALSE.
 */
#define WMS_SERIAL_IS_BOOLEAN(value)	(((value) == TRUE)||((value) == FALSE))
/**
 * Macro to check whether the parameter is a sign character, i.e. either '+' or '-'.
 */
#define WMS_SERIAL_IS_SIGN(value)	(((value) == '+')||((value) == '-'))

/* external functions */
extern void Wms_Serial_Error(void);
extern void Wms_Serial_Error_To_String(char *error_string);
extern int Wms_Serial_Get_Error_Number(void);
extern void Wms_Serial_Get_Current_Time_String(char *time_string,int string_length);
extern void Wms_Serial_Log_Format(char *class,char *source,int level,char *format,...);
extern void Wms_Serial_Log(char *class,char *source,int level,char *string);
extern void Wms_Serial_Set_Log_Handler_Function(void (*log_fn)(char *class,char *source,int level,char *string));
extern void Wms_Serial_Set_Log_Filter_Function(int (*filter_fn)(char *class,char *source,int level,char *string));
extern void Wms_Serial_Log_Handler_Stdout(char *class,char *source,int level,char *string);
extern void Wms_Serial_Set_Log_Filter_Level(int level);
extern int Wms_Serial_Log_Filter_Level_Absolute(char *class,char *source,int level,char *string);
extern int Wms_Serial_Log_Filter_Level_Bitwise(char *class,char *source,int level,char *string);

/* external variables */
extern int Wms_Serial_Error_Number;
extern char Wms_Serial_Error_String[];

#endif
