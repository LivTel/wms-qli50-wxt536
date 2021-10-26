/* qli50_wxt536_general.h
 */
#ifndef QLI50_WXT536_GENERAL_H
#define QLI50_WXT536_GENERAL_H
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
#define QLI50_WXT536_ERROR_LENGTH (1024)
/**
 * One millosecond in nanoseconds (1000000).
 */
#define QLI50_WXT536_ONE_MILLISECOND_NS   (1000000)

/**
 * Macro to check whether the parameter is either TRUE or FALSE.
 */
#define QLI50_WXT536_IS_BOOLEAN(value)	(((value) == TRUE)||((value) == FALSE))

/* external functions */
extern void Qli50_Wxt536_Error(void);
extern void Qli50_Wxt536_Error_To_String(char *error_string);
extern int Qli50_Wxt536_Error_Number_Get(void);
extern void Qli50_Wxt536_Current_Time_String_Get(char *time_string,int string_length);
extern void Qli50_Wxt536_Log_Format(char *class,char *source,int level,char *format,...);
extern void Qli50_Wxt536_Log(char *class,char *source,int level,char *string);
extern void Qli50_Wxt536_Log_Handler_Function_Set(void (*log_fn)(char *class,char *source,int level,char *string));
extern void Qli50_Wxt536_Log_Filter_Function_Set(int (*filter_fn)(char *class,char *source,int level,char *string));
extern void Qli50_Wxt536_Log_Handler_Stdout(char *class,char *source,int level,char *string);
extern void Qli50_Wxt536_Log_Filter_Level_Set(int level);
extern int Qli50_Wxt536_Log_Filter_Level_Absolute(char *class,char *source,int level,char *string);

/* external variables */
extern int Qli50_Wxt536_Error_Number;
extern char Qli50_Wxt536_Error_String[];

#endif
