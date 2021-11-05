/* qli50_wxt536_general.c
** Weather monitoring system (qli50 -> wxt536 conversion), general routines.
*/
/**
 * Error and Log handlers.
 * @author Chris Mottram
 * @version $Revision$
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <errno.h>   /* Error number definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include "qli50_wxt536_general.h"
#include "wms_qli50_general.h"
#include "wms_wxt536_general.h"
#include "wms_serial_general.h"

/* defines */
/**
 * How long some buffers are when generating logging messages.
 */
#define LOG_BUFF_LENGTH           (1024)

/* data types */
/**
 * Data type holding local data to qli50_wxt536_general. This consists of the following:
 * <dl>
 * <dt>Log_Handler</dt> <dd>Function pointer to the routine that will log messages passed to it.</dd>
 * <dt>Log_Filter</dt> <dd>Function pointer to the routine that will filter log messages passed to it.
 * 		The funtion will return TRUE if the message should be logged, and FALSE if it shouldn't.</dd>
 * <dt>Log_Filter_Level</dt> <dd>A globally maintained log filter level. 
 * 		This is set using Qli50_Wxt536_Log_Filter_Level_Set.
 * 		Qli50_Wxt536_Log_Filter_Level_Absolute and Qli50_Wxt536_Log_Filter_Level_Bitwise 
 *              test it against message levels to determine whether to log messages.</dd>
 * </dl>
 * @see #Qli50_Wxt536_Log
 * @see #Qli50_Wxt536_Log_Filter_Level_Set
 * @see #Qli50_Wxt536_Log_Filter_Level_Absolute
 */
struct General_Struct
{
	void (*Log_Handler)(char *class,char *source,int level,char *string);
	int (*Log_Filter)(char *class,char *source,int level,char *string);
	int Log_Filter_Level;
};

/* external variables */
/**
 * The error number.
 */
int Qli50_Wxt536_Error_Number = 0;
/**
 * The error string.
 * @see #QLI50_WXT536_ERROR_LENGTH
 */
char Qli50_Wxt536_Error_String[QLI50_WXT536_ERROR_LENGTH];

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/**
 * The instance of General_Struct that contains local data for this module.
 * This is statically initialised to the following:
 * <dl>
 * <dt>Log_Handler</dt> <dd>NULL</dd>
 * <dt>Log_Filter</dt> <dd>NULL</dd>
 * <dt>Log_Filter_Level</dt> <dd>0</dd>
 * </dl>
 * @see #General_Struct
 */
static struct General_Struct General_Data = 
{
	NULL,NULL,0,
};

/* internal functions */

/* =======================================================
** external functions 
** ======================================================= */
/**
 * Report errors generated in the main program, serial library, Qli50 and Wxt536 library, to stderr.
 * @see #Qli50_Wxt536_Error_Number
 * @see #Qli50_Wxt536_Error_String
 * @see #Qli50_Wxt536_Current_Time_String_Get
 * @see ../../qli50/cdocs/wms_qli60_general.html#Wms_Qli50_Get_Error_Number
 * @see ../../qli50/cdocs/wms_qli60_general.html#Wms_Qli50_Error
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Get_Error_Number
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Error
 * @see ../../wxt536/cdocs/wms_wxt536_general.html#Wms_Wxt536_Get_Error_Number
 * @see ../../wxt536/cdocs/wms_wxt536_general.html#Wms_Wxt536_Error
 */
void Qli50_Wxt536_Error(void)
{
	char time_string[32];

	Qli50_Wxt536_Current_Time_String_Get(time_string,32);
	if(Qli50_Wxt536_Error_Number == 0)
		sprintf(Qli50_Wxt536_Error_String,"%s Qli50 Wxt536:An unknown error has occured.",time_string);
	fprintf(stderr,"%s Qli50 Wxt536:Error(%d) : %s\n",time_string,Qli50_Wxt536_Error_Number,
		Qli50_Wxt536_Error_String);
	if(Wms_Qli50_Get_Error_Number() > 0)
	{
		Wms_Qli50_Error();
	}
	if(Wms_Wxt536_Get_Error_Number() > 0)
	{
		Wms_Wxt536_Error();
	}
	if(Wms_Serial_Get_Error_Number() > 0)
	{
		Wms_Serial_Error();
	}
}

/**
 * Basic error reporting routine, to the specified string.
 * @param error_string Pointer to an already allocated area of memory, to store the generated error string. 
 *        This should be at least 256 bytes long.
 * @see #Qli50_Wxt536_Error_Number
 * @see #Qli50_Wxt536_Error_String
 * @see #Qli50_Wxt536_Current_Time_String_Get
 * @see ../../qli50/cdocs/wms_qli60_general.html#Wms_Qli50_Get_Error_Number
 * @see ../../qli50/cdocs/wms_qli60_general.html#Wms_Qli50_Error_To_String
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Get_Error_Number
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Error_To_String
 * @see ../../wxt536/cdocs/wms_wxt536_general.html#Wms_Wxt536_Get_Error_Number
 * @see ../../wxt536/cdocs/wms_wxt536_general.html#Wms_Wxt536_Error_To_String
 */
void Qli50_Wxt536_Error_To_String(char *error_string)
{
	char time_string[32];

	strcpy(error_string,"");
	Qli50_Wxt536_Current_Time_String_Get(time_string,32);
	if(Qli50_Wxt536_Error_Number != 0)
	{
		sprintf(error_string+strlen(error_string),"%s Qli50 Wxt536:Error(%d) : %s\n",time_string,
			Qli50_Wxt536_Error_Number,Qli50_Wxt536_Error_String);
	}
	if(Wms_Qli50_Get_Error_Number() > 0)
	{
		Wms_Qli50_Error_To_String(error_string+strlen(error_string));
	}
	if(Wms_Wxt536_Get_Error_Number() > 0)
	{
		Wms_Wxt536_Error_To_String(error_string+strlen(error_string));
	}
	if(Wms_Serial_Get_Error_Number() > 0)
	{
		Wms_Serial_Error_To_String(error_string+strlen(error_string));
	}
	if(strlen(error_string) == 0)
	{
		sprintf(error_string,"%s Error:Qli50 Wxt536:Error not found\n",time_string);
	}
	
}

/**
 * Routine to return the current value of the error number.
 * @return The value of Qli50_Wxt536_Error_Number.
 * @see #Qli50_Wxt536_Error_Number
 */
int Qli50_Wxt536_Error_Number_Get(void)
{
	return Qli50_Wxt536_Error_Number;
}

/**
 * Routine to get the current time in a string. The string is returned in the format 'YYYY-MM-DDTHH:MM:SS.sss'.
 * The time is in UTC.
 * @param time_string The string to fill with the current time.
 * @param string_length The length of the buffer passed in. It is recommended the length is at least 24 characters.
 * @see #QLI50_WXT536_ONE_MILLISECOND_NS
 */
void Qli50_Wxt536_Current_Time_String_Get(char *time_string,int string_length)
{
	struct timespec current_time;
	struct tm *utc_time = NULL;
	char ms_buff[16];
	int ms;

	if(time_string == NULL)
		return;
	if(string_length < 24)
	{
		strcpy(time_string,"");
		return;
	}
	clock_gettime(CLOCK_REALTIME,&current_time);
	utc_time = gmtime(&(current_time.tv_sec));
	strftime(time_string,string_length,"%Y-%m-%dT%H:%M:%S",utc_time);
	/*  add milliseconds to this string */
	ms = (current_time.tv_nsec/QLI50_WXT536_ONE_MILLISECOND_NS);
	sprintf(ms_buff,".%03d",ms);
	strcat(time_string,ms_buff);
}

/**
 * Routine to log a message to a defined logging mechanism. This routine has an arbitary number of arguments,
 * and uses vsprintf to format them i.e. like fprintf. 
 * Qli50_Wxt536_Log is then called to handle the log message.
 * @param class The class that produced this log message.
 * @param source The source that produced this log message.
 * @param level An integer, used to decide whether this particular message has been selected for
 * 	logging or not.
 * @param format A string, with formatting statements the same as fprintf would use to determine the type
 * 	of the following arguments.
 * @see #Qli50_Wxt536_Log
 * @see #LOG_BUFF_LENGTH
 */
void Qli50_Wxt536_Log_Format(char *class,char *source,int level,char *format,...)
{
	char buff[LOG_BUFF_LENGTH];
	va_list ap;

/* format the arguments */
	va_start(ap,format);
	vsprintf(buff,format,ap);
	va_end(ap);
/* call the log routine to log the results */
	Qli50_Wxt536_Log(class,source,level,buff);
}

/**
 * Routine to log a message to a defined logging mechanism. If the string or General_Data.Log_Handler are NULL
 * the routine does not log the message. If the General_Data.Log_Filter function pointer is non-NULL, the
 * message is passed to it to determoine whether to log the message. We call Wms_Qli50_Log_Fix_Control_Chars to replace
 * any control characters in the string with a textual equivalent before logging the result.
 * @param class The class that produced this log message.
 * @param source The source that produced this log message.
 * @param level An integer, used to decide whether this particular message has been selected for
 * 	logging or not.
 * @param string The message to log.
 * @see #General_Data
 * @see ../../qli50/cdocs/wms_qli60_general.html#Wms_Qli50_Log_Fix_Control_Chars
 */
void Qli50_Wxt536_Log(char *class,char *source,int level,char *string)
{
	char buff[LOG_BUFF_LENGTH];
	
/* If the string is NULL, don't log. */
	if(string == NULL)
	{
		fprintf(stdout,"Qli50_Wxt536_Log:String was NULL.\n");
		return;
	}
/* If there is no log handler, return */
	if(General_Data.Log_Handler == NULL)
	{
		fprintf(stdout,"Qli50_Wxt536_Log:Log_Handler was NULL when handling '%s'.\n",string);
		return;
	}
/* If there's a log filter, check it returns TRUE for this message */
	if(General_Data.Log_Filter != NULL)
	{
		if(General_Data.Log_Filter(class,source,level,string) == FALSE)
		{
			/*
			fprintf(stdout,"Qli50_Wxt536_Log:Filter was false when handling '%s' with level %d.\n",
				string,level);
			*/
			return;
		}
	}
	/* copy string to buff, and replace any control characters with a string representation in buff */
	Wms_Qli50_Log_Fix_Control_Chars(string,buff);
/* We can log the message */
	(*General_Data.Log_Handler)(class,source,level,buff);
}

/**
 * Routine to set the General_Data.Log_Handler used by Qli50_Wxt536_Log.
 * @param log_fn A function pointer to a suitable handler.
 * @see #General_Data
 * @see #Wms_Serial_Log
 */
void Qli50_Wxt536_Log_Handler_Function_Set(void (*log_fn)(char *class,char *source,int level,char *string))
{
	General_Data.Log_Handler = log_fn;
}

/**
 * Routine to set the General_Data.Log_Filter used by Qli50_Wxt536_Log.
 * @param log_fn A function pointer to a suitable filter function.
 * @see #General_Data
 * @see #Qli50_Wxt536_Log
 */
void Qli50_Wxt536_Log_Filter_Function_Set(int (*filter_fn)(char *class,char *source,int level,char *string))
{
	General_Data.Log_Filter = filter_fn;
}

/**
 * A log handler to be used for the General_Data.Log_Handler function.
 * This creates a timestamp using Qli50_Wxt536_Current_Time_String_Get, and then prints a string of the form:
 * "<timestamp> : <class> : <source> : <string>\n"
 * @param class The class that produced this log message.
 * @param source The source that produced this log message.
 * @param level The log level for this message.
 * @param string The log message to be logged. 
 * @see #Qli50_Wxt536_Current_Time_String_Get
 */
void Qli50_Wxt536_Log_Handler_Stdout(char *class,char *source,int level,char *string)
{
	char timestring[32];
	
	/* put timestamp in buff */
	Qli50_Wxt536_Current_Time_String_Get(timestring,31);
	if(string == NULL)
		return;
	fprintf(stdout,"%s : %s : %s : %s\n",timestring,class,source,string);
}

/**
 * Routine to set the General_Data.Log_Filter_Level.
 * @see #General_Data
 */
void Qli50_Wxt536_Log_Filter_Level_Set(int level)
{
	General_Data.Log_Filter_Level = level;
	/*fprintf(stdout,"Qli50_Wxt536_Set_Log_Filter_Level:Log level set to %d.\n",level);*/
}

/**
 * A log message filter routine, to be used for the General_Data.Log_Filter function pointer.
 * @param class The class that produced this log message.
 * @param source The source that produced this log message.
 * @param level The log level of the message to be tested.
 * @param string The log message to be logged, not used in this filter. 
 * @return The routine returns TRUE if the level is less than or equal to the General_Data.Log_Filter_Level,
 * 	otherwise it returns FALSE.
 * @see #General_Data
 */
int Qli50_Wxt536_Log_Filter_Level_Absolute(char *class,char *source,int level,char *string)
{
	return (level <= General_Data.Log_Filter_Level);
}

