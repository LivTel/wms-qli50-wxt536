/* wms_wxt536_command.c
** Weather monitoring system (qli50 -> wxt536 conversion), Vaisala Wxt536 interface library, 
** command routines.
*/
/**
 * Routines to send commands (and parse their replies) to/from the Vaisala Wxt536.
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
#include "log_udp.h"
#include "wms_serial_serial.h"
#include "wms_wxt536_connection.h"
#include "wms_wxt536_general.h"

/* hash defines */
/**
 * The terminator used to delimit the end of each line read. <cr><lf> in this case.
 */
#define TERMINATOR_CRLF         ("\r\n")

/* internal structures */
/**
 * Structure to hold one keyword/value pair in the list of parameter values returned by a command.
 * <dl>
 * <dt>Keyword</dt> <dd>The keyword we are holding the value for.</dd>
 * <dt>Value_String</dt> <dd>The value of the keyword.</dd>
 * </dl>
 */
struct Wxt536_Parameter_Value_Struct
{
	char Keyword[32];
	char Value_String[128];
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* internal function declarations */
static int Wxt536_Parse_CSV_Reply(char *class,char *source,char *reply_string,
				  struct Wxt536_Parameter_Value_Struct **parameter_value_list,
				  int *parameter_value_count);

/* external functions */
/**
 * Basic routine to send a string command to the Vaisala Wxt536 over a previously opened connection, 
 * and wait for a reply string.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param The command to send to the Vaisala Wxt536, as a NULL terminated string. The standard CRLF terminator
 *        will be added to this string before onward transmission to the Wxt536.
 * @param reply_string An empty string, on return this is filled with any reply received from the Wxt536.
 * @param reply_string_length The allocated length of the reply_string buffer.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Wxt536_Error_Number and Wms_Wxt536_Error_String are filled in on failure).
 * @see #TERMINATOR_CRLF
 * @see wms_wxt536_connection.html#Wms_Wxt536_Serial_Handle
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Write
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Read_Line
 */
int Wms_Wxt536_Command(char *class,char *source,char *command_string,char *reply_string,int reply_string_length)
{
	char message[256];
	int bytes_read;
	
	Wms_Wxt536_Error_Number = 0;
	if(command_string == NULL)
	{
		Wms_Wxt536_Error_Number = 100;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Command String was NULL.");
		return FALSE;
		
	}
	if(strlen(command_string) >= 256)
	{
		Wms_Wxt536_Error_Number = 101;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Command String was too long (%lu vs 255).",
			strlen(command_string));
		return FALSE;
		
	}
#if LOGGING > 9
	Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Wxt536_Command(%s) started.",command_string);
#endif /* LOGGING */
	strcpy(message,command_string);
	strcat(message,TERMINATOR_CRLF);
	if(!Wms_Serial_Write(class,source,Wms_Wxt536_Serial_Handle,message,strlen(message)))
	{
		Wms_Wxt536_Error_Number = 102;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Failed to write command string '%s'.",
			command_string);
		return FALSE;
	}
	/* read any reply */
	if(reply_string != NULL)
	{
		if(!Wms_Serial_Read_Line(class,source,Wms_Wxt536_Serial_Handle,TERMINATOR_CRLF,message,255,&bytes_read))
		{
			Wms_Wxt536_Error_Number = 103;
			sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command:Failed to read reply line.");
			return FALSE;
		}
		message[bytes_read] = '\0';
		if(strlen(message) >= reply_string_length)
		{
			Wms_Wxt536_Error_Number = 104;
			sprintf(Wms_Wxt536_Error_String,
				"Wms_Wxt536_Command:Reply message was too long to fit into reply string (%lu vs %d).",
				strlen(message),reply_string_length);
			return FALSE;
		}
		strcpy(reply_string,message);
#if LOGGING > 9
		Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Wxt536_Command(%s) returned reply '%s'.",
				      command_string,reply_string);
#endif /* LOGGING */
	}/* end if reply string was not NULL */
#if LOGGING > 9
	Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Wxt536_Command(%s) finished.",command_string);
#endif /* LOGGING */
	return TRUE;
}

/**
 * Command to retrieve the Device Address of a Wxt536 on the connected serial port.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param device_number The address of an integer. If the procedure is successful, on return this will contain
 *        the device_number of the Wxt536 on the connected serial port.
 * @see #Wms_Wxt536_Command
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 */
int Wms_Wxt536_Command_Device_Address_Get(char *class,char *source,int *device_number)
{
	char reply_string[256];
	int retval;

	Wms_Wxt536_Error_Number = 0;
	if(device_number == NULL)
	{
		Wms_Wxt536_Error_Number = 105;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Device_Address_Get:device_number was NULL.");
		return FALSE;
	}
	if(!Wms_Wxt536_Command(class,source,"?",reply_string,255))
		return FALSE;
	retval = sscanf(reply_string,"%d",device_number);
	if(retval != 1)
	{
		Wms_Wxt536_Error_Number = 106;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Device_Address_Get:"
			"Failed to parse device number from reply string '%s'.",reply_string);
		return FALSE;		
	}
	return TRUE;
}

/**
 * Command to check the  Wxt536 with the specified device_number is active on the connected serial port.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param device_number The device number of the Wxt536 (can be retrieved using Wms_Wxt536_Command_Device_Address_Get).
 * @see #Wms_Wxt536_Command
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 */
int Wms_Wxt536_Command_Ack_Active(char *class,char *source,int device_number)
{
	char command_string[256];
	char reply_string[256];
	int retval,reply_device_number;

	Wms_Wxt536_Error_Number = 0;
	sprintf(command_string,"%d",device_number);
	if(!Wms_Wxt536_Command(class,source,command_string,reply_string,255))
		return FALSE;
	retval = sscanf(reply_string,"%d",&reply_device_number);
	if(retval != 1)
	{
		Wms_Wxt536_Error_Number = 107;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Ack_Active:"
			"Failed to parse device number from reply string '%s'.",reply_string);
		return FALSE;		
	}
	if(device_number != reply_device_number)
	{
		Wms_Wxt536_Error_Number = 108;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Ack_Active:"
			"Returned device number differed from command: %d vs %d  ('%s').",
			device_number,reply_device_number,reply_string);
		return FALSE;		
	}
	return TRUE;	
}

/**
 * Send the Wxt536 with the specified device_number a command to retrieve it's current communication settings, 
 * and parse it's reply.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param device_number The device number of the Wxt536 (can be retrieved using Wms_Wxt536_Command_Device_Address_Get).
 * @param comms_settings The address of a Wxt536_Command_Comms_Settings_Struct structure to fill in the parsed reply.
 * @see #Wms_Wxt536_Command
 * @see #Wxt536_Command_Comms_Settings_Struct
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 */
int Wms_Wxt536_Command_Current_Comms_Settings_Get(char *class,char *source,int device_number,
						  struct Wxt536_Command_Comms_Settings_Struct *comms_settings)
{
	struct Wxt536_Parameter_Value_Struct *parameter_value_list = NULL;
	char command_string[256];
	char reply_string[256];
	int parameter_value_count;
	
	Wms_Wxt536_Error_Number = 0;
	sprintf(command_string,"%dXU",device_number);
	if(!Wms_Wxt536_Command(class,source,command_string,reply_string,255))
		return FALSE;
	if(!Wxt536_Parse_CSV_Reply(class,source,reply_string,&parameter_value_list,&parameter_value_count))
		return FALSE;
	/* iterate over parameter_value_list and extract values into comms_settings structure */
	return TRUE;
}

/* internal functions */

static int Wxt536_Parse_CSV_Reply(char *class,char *source,char *reply_string,
				  struct Wxt536_Parameter_Value_Struct **parameter_value_list,
				  int *parameter_value_count)
{
	char *comma_ptr = NULL;
	char *end_parameter_comma_ptr = NULL;
	int retval;
	
	if(reply_string == NULL)
	{
		Wms_Wxt536_Error_Number = 109;
		sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_CSV_Reply:reply_string was NULL.");
		return FALSE;		
	}
	if(parameter_value_list == NULL)
	{
		Wms_Wxt536_Error_Number = 110;
		sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_CSV_Reply:parameter_value_list was NULL.");
		return FALSE;		
	}
	if(parameter_value_count == NULL)
	{
		Wms_Wxt536_Error_Number = 111;
		sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_CSV_Reply:parameter_value_count was NULL.");
		return FALSE;		
	}
#if LOGGING > 9
	Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wxt536_Parse_CSV_Reply: Parsing'%s'.",reply_string);
#endif /* LOGGING */
	(*parameter_value_count) = 0;
	(*parameter_value_list) = NULL;
	comma_ptr = strstr(reply_string,",");
	if(comma_ptr != NULL)
	{
		/* first part of the reply is the command */
		(*comma_ptr) = '\0';
#if LOGGING > 9
		Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
				      "Wxt536_Parse_CSV_Reply: Parsed command string '%s'.",reply_string);
#endif /* LOGGING */		
	}
	/* while there are more parameters */
	while(comma_ptr != NULL)
	{
		/* find the end of this parameter and null terminate it (if it is NOT the last parameter) */
		end_parameter_comma_ptr = strstr(comma_ptr+1,",");
		if(end_parameter_comma_ptr != NULL)
			end_parameter_comma_ptr = '\0';
#if LOGGING > 9
		Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
				      "Wxt536_Parse_CSV_Reply: Parsing keyword value string '%s'.",comma_ptr+1);
#endif /* LOGGING */
		/* reallocate the parameter list */
		if((*parameter_value_list) == NULL)
		{
			(*parameter_value_list) = (struct Wxt536_Parameter_Value_Struct *)malloc(sizeof(struct Wxt536_Parameter_Value_Struct));
		}
		else
		{
			(*parameter_value_list) = (struct Wxt536_Parameter_Value_Struct *)realloc((*parameter_value_list),((*parameter_value_count)+1)*sizeof(struct Wxt536_Parameter_Value_Struct));
		}
		if((*parameter_value_list) == NULL)
		{
			Wms_Wxt536_Error_Number = 112;
			sprintf(Wms_Wxt536_Error_String,
				"Wxt536_Parse_CSV_Reply:Failed to realloc parameter_value_list (%d).",
				(*parameter_value_count));
			return FALSE;		
		}
		retval = sscanf(comma_ptr+1,"%32s=%128s",(*parameter_value_list)[(*parameter_value_count)].Keyword,
				(*parameter_value_list)[(*parameter_value_count)].Value_String);
		if(retval != 2)
		{
			Wms_Wxt536_Error_Number = 113;
			sprintf(Wms_Wxt536_Error_String,
				"Wxt536_Parse_CSV_Reply:Failed to parse keyword/value string '%s' at parameter %d.",
				comma_ptr+1,(*parameter_value_count));
			return FALSE;		
			
		}
#if LOGGING > 9
		Wms_Wxt536_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
				      "Wxt536_Parse_CSV_Reply: Parsed keyword '%s with value '%s'.",
				      (*parameter_value_list)[(*parameter_value_count)].Keyword,
				      (*parameter_value_list)[(*parameter_value_count)].Value_String);
#endif /* LOGGING */
		(*parameter_value_count)++;
		comma_ptr = end_parameter_comma_ptr;
	}/* end while */
	return TRUE;
}
