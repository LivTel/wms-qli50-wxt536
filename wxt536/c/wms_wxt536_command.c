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
#include "wms_wxt536_command.h"
#include "wms_wxt536_connection.h"
#include "wms_wxt536_general.h"

/* hash defines */
/**
 * The terminator used to delimit the end of each line read. <cr><lf> in this case.
 */
#define TERMINATOR_CRLF         ("\r\n")
/**
 * Length of the Keyword String in Wxt536_Parameter_Value_Struct.
 * @see #Wxt536_Parameter_Value_Struct
 */
#define KEYWORD_LENGTH          (32)
/**
 * Length of the Value String in Wxt536_Parameter_Value_Struct.
 * @see #Wxt536_Parameter_Value_Struct
 */
#define VALUE_LENGTH            (128)
/* internal structures */
/**
 * Structure to hold one keyword/value pair in the list of parameter values returned by a command.
 * <dl>
 * <dt>Keyword</dt> <dd>The keyword we are holding the value for, of length KEYWORD_LENGTH.</dd>
 * <dt>Value_String</dt> <dd>The value of the keyword, of length VALUE_LENGTH.</dd>
 * </dl>
 * @see #KEYWORD_LENGTH
 * @see #VALUE_LENGTH
 */
struct Wxt536_Parameter_Value_Struct
{
	char Keyword[KEYWORD_LENGTH];
	char Value_String[VALUE_LENGTH];
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
static int Wxt536_Parse_Parameter(char *class,char *source,char *keyword,char *format,
				  struct Wxt536_Parameter_Value_Struct *parameter_value_list,
				  int parameter_value_count,void *data_ptr);

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
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Wxt536_Error_Number and Wms_Wxt536_Error_String are filled in on failure).
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
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Wxt536_Error_Number and Wms_Wxt536_Error_String are filled in on failure).
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
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Wxt536_Error_Number and Wms_Wxt536_Error_String are filled in on failure).
 * @see #Wms_Wxt536_Command
 * @see #Wxt536_Command_Comms_Settings_Struct
 * @see #Wxt536_Parameter_Value_Struct
 * @see #Wxt536_Parse_CSV_Reply
 * @see #Wxt536_Parse_Parameter
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 */
int Wms_Wxt536_Command_Comms_Settings_Get(char *class,char *source,int device_number,
						  struct Wxt536_Command_Comms_Settings_Struct *comms_settings)
{
	struct Wxt536_Parameter_Value_Struct *parameter_value_list = NULL;
	char command_string[256];
	char reply_string[256];
	int parameter_value_count;

	Wms_Wxt536_Error_Number = 0;
	if(comms_settings == NULL)
	{
		Wms_Wxt536_Error_Number = 118;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Comms_Settings_Get:comms_settings was NULL.");
		return FALSE;		
	}
	sprintf(command_string,"%dXU",device_number);
	/* send the command and get the reply string */
	if(!Wms_Wxt536_Command(class,source,command_string,reply_string,255))
		return FALSE;
	/* parse the reply string into keyword/value pairs */
	if(!Wxt536_Parse_CSV_Reply(class,source,reply_string,&parameter_value_list,&parameter_value_count))
		return FALSE;
	/* Extract the relvant parameters from the parameter_value_list, parse them and store them in the
	** return data structure */
	if(!Wxt536_Parse_Parameter(class,source,"A","%c",parameter_value_list,parameter_value_count,
				   &(comms_settings->Address)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"M","%c",parameter_value_list,parameter_value_count,
				   &(comms_settings->Protocol)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"C","%c",parameter_value_list,parameter_value_count,
				   &(comms_settings->Serial_Interface)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"I","%d",parameter_value_list,parameter_value_count,
				   &(comms_settings->Composite_Repeat_Interval)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"B","%d",parameter_value_list,parameter_value_count,
				   &(comms_settings->Baud_Rate)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"D","%d",parameter_value_list,parameter_value_count,
				   &(comms_settings->Data_Bits)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"P","%c",parameter_value_list,parameter_value_count,
				   &(comms_settings->Parity)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"S","%d",parameter_value_list,parameter_value_count,
				   &(comms_settings->Stop_Bits)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"L","%d",parameter_value_list,parameter_value_count,
				   &(comms_settings->RS485_Line_Delay)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"N","%256s",parameter_value_list,parameter_value_count,
				   &(comms_settings->Device_Name)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"V","%256s",parameter_value_list,parameter_value_count,
				   &(comms_settings->Software_Version)))
		return FALSE;
	if(!Wxt536_Parse_Parameter(class,source,"H","%d",parameter_value_list,parameter_value_count,
				   &(comms_settings->Parameter_Locking)))
		return FALSE;
       	return TRUE;
}

/**
 * Routine to set the Wxt536 communication protocol.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param device_number The device number of the Wxt536 (can be retrieved using Wms_Wxt536_Command_Device_Address_Get).
 * @param protocol A character describing the protocol to set the Wxt536 to use. One of: 
 *        WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC, WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC_CRC, 
 *        WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED, WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED_CRC.
 * @see #WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC
 * @see #WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC_CRC
 * @see #WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED
 * @see #WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED_CRC
 * @see #Wms_Wxt536_Command
 * @see #Wxt536_Parameter_Value_Struct
 * @see #Wxt536_Parse_CSV_Reply
 * @see #Wxt536_Parse_Parameter
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 */
int Wms_Wxt536_Command_Comms_Settings_Protocol_Set(char *class,char *source,int device_number,char protocol)
{
	struct Wxt536_Parameter_Value_Struct *parameter_value_list = NULL;
	char command_string[256];
	char reply_string[256];
	int parameter_value_count;

	Wms_Wxt536_Error_Number = 0;
	if((protocol != WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC)&&
	   (protocol != WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_AUTOMATIC_CRC)&&
	   (protocol != WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED)&&
	   (protocol != WXT536_COMMAND_COMMS_SETTINGS_PROTOCOL_POLLED_CRC))
	{
		Wms_Wxt536_Error_Number = 119;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Comms_Settings_Protocol_Set:Illegal protocol '%c'.",
			protocol);
		return FALSE;		
	}
	sprintf(command_string,"%dXU,M=%c",device_number,protocol);
	/* send the command and get the reply string */
	if(!Wms_Wxt536_Command(class,source,command_string,reply_string,255))
		return FALSE;
	/* parse the reply string into keyword/value pairs */
	if(!Wxt536_Parse_CSV_Reply(class,source,reply_string,&parameter_value_list,&parameter_value_count))
		return FALSE;
	/* there should be one reply parameter, M=protocol */
	if(parameter_value_count != 1)
	{
		Wms_Wxt536_Error_Number = 120;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Comms_Settings_Protocol_Set:"
			"Wrong number of reply parameters in reply (%d vd 1).",parameter_value_count);
		return FALSE;		
	}
	if(strcmp(parameter_value_list[0].Keyword,"M") != 0)
	{
		Wms_Wxt536_Error_Number = 121;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Comms_Settings_Protocol_Set:"
			"Wrong reply parameter keyword ('%s' vd 'M').",parameter_value_list[0].Keyword);
		return FALSE;		
	}
	if(parameter_value_list[0].Value_String[0] != protocol)
	{
		Wms_Wxt536_Error_Number = 122;
		sprintf(Wms_Wxt536_Error_String,"Wms_Wxt536_Command_Comms_Settings_Protocol_Set:"
			"Wrong reply parameter value ('%c' vd '%c').",parameter_value_list[0].Value_String[0],protocol);
		return FALSE;		
	}
	return TRUE;
}

/* ----------------------------------------------------------------------
** internal functions
** ---------------------------------------------------------------------- */
/**
 * Internal routine to parse a reply into a series of keyword/value pairs. Many Wxt536 commands return replies of the
 * form:
 * "0XU,A=0,M=A,T=1,C=3,I=0,B=9600,D=8,P=N,S=1,L=25, N=WXT530,V=1.00<cr><lf>"
 * i.e. the command, followed by a series of comma separated parameters, of the form "keyword=value".
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param reply_string The character string containing the reply to be parsed. The string contents are edited
 *        as part of the parsing.
 * @param parameter_value_list The address of a list pointer of Wxt536_Parameter_Value_Struct structs. On return
 *        of the function this list will be allocated and filled with parsed parameter keyword/values.
 * @param parameter_value_count The address of an integer, on a successful return this contains the number of elements
 *        in parameter_value_list.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Wxt536_Error_Number and Wms_Wxt536_Error_String are filled in on failure).
 * @see #Wxt536_Parameter_Value_Struct
 * @see #TERMINATOR_CRLF
 * @see #KEYWORD_LENGTH
 * @see #VALUE_LENGTH
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 */
static int Wxt536_Parse_CSV_Reply(char *class,char *source,char *reply_string,
				  struct Wxt536_Parameter_Value_Struct **parameter_value_list,
				  int *parameter_value_count)
{
	char *comma_ptr = NULL;
	char *end_parameter_comma_ptr = NULL;
	char *equals_ptr = NULL;
	char *crlf_ptr = NULL;
	
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
	/* remove the terminator at the end */
	crlf_ptr = strstr(reply_string,TERMINATOR_CRLF);
	if(crlf_ptr != NULL)
		(*crlf_ptr) = '\0';
	/* find the end of the command string */
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
			(*end_parameter_comma_ptr) = '\0';
		/* the current parameter now starts at comma_ptr+1, ends at end_parameter_comma_ptr,
		**  and is NULL terminated */
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
		/* parse the keyword=value pair and put the results in the newly alloacted list position */
		equals_ptr = strstr(comma_ptr+1,"=");
		if(equals_ptr == NULL)
		{
			Wms_Wxt536_Error_Number = 113;
			sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_CSV_Reply:"
				"Failed to parse keyword/value string '%s' at parameter %d: No equals sign found.",
				comma_ptr+1,(*parameter_value_count));
			return FALSE;
		}
		(*equals_ptr) = '\0';
		/* keyword now null terminated at comma_ptr+1 */
		if(strlen(comma_ptr+1) > KEYWORD_LENGTH)
		{
			Wms_Wxt536_Error_Number = 114;
			sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_CSV_Reply:"
			  "Failed to parse keyword string '%s' at parameter %d:Keyword was too long (%lu vs %d).",
				comma_ptr+1,(*parameter_value_count),strlen(comma_ptr+1),KEYWORD_LENGTH);
			return FALSE;
		}
		strcpy((*parameter_value_list)[(*parameter_value_count)].Keyword,comma_ptr+1);
		/* value now null terminated at equals_ptr+1 */
		if(strlen(equals_ptr+1) > VALUE_LENGTH)
		{
			Wms_Wxt536_Error_Number = 115;
			sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_CSV_Reply:"
			  "Failed to parse value string '%s' at parameter %d:Value was too long (%lu vs %d).",
				equals_ptr+1,(*parameter_value_count),strlen(equals_ptr+1),VALUE_LENGTH);
			return FALSE;
		}
		strcpy((*parameter_value_list)[(*parameter_value_count)].Value_String,equals_ptr+1);
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

/**
 * This routine finds the specified keyword in the parameter_value_list, uses the format to parse
 * the value, and stores the parsed value in the variable pointed to data address data_ptr.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param keyword The keyword of the parameter we want to parse.
 * @param format The format of the paramaters value we want to parse, specified as a sscanf format string.
 * @param parameter_value_list The list of keyword/value pairs parsed from a Wxt536 command.
 * @param parameter_value_count The number of elements in parameter_value_list.
 * @param data_ptr The address of a variable (hopefully of a type matching for format) to store the parsed 
 *        parameter value in.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Wxt536_Error_Number and Wms_Wxt536_Error_String are filled in on failure). 
 * @see #Wxt536_Parameter_Value_Struct
 * @see #Wxt536_Parameter_Parsing_Struct
 * @see wms_wxt536_general.html#Wms_Wxt536_Log
 * @see wms_wxt536_general.html#Wms_Wxt536_Log_Format
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_Number
 * @see wms_wxt536_general.html#Wms_Wxt536_Error_String
 */
static int Wxt536_Parse_Parameter(char *class,char *source,char *keyword,char *format,
				  struct Wxt536_Parameter_Value_Struct *parameter_value_list,
				  int parameter_value_count,void *data_ptr)
{
	int parameter_value_list_index,retval;

	/* find keyword/value pair in parameter_value_list with the specified keyword */
	parameter_value_list_index = 0;
	while((parameter_value_list_index < parameter_value_count)&&
	      strcmp(parameter_value_list[parameter_value_list_index].Keyword,keyword))
	{
		parameter_value_list_index++;
	}
	if(parameter_value_list_index == parameter_value_count)
	{
		Wms_Wxt536_Error_Number = 116;
		sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_Parameter:"
			"Failed to find keyword string '%s' in parameter value list.",keyword);
		return FALSE;
	}
	retval = sscanf(parameter_value_list[parameter_value_list_index].Value_String,format,data_ptr);
	if(retval != 1)
	{
		Wms_Wxt536_Error_Number = 117;
		sprintf(Wms_Wxt536_Error_String,"Wxt536_Parse_Parameter:"
			"Failed to parse keyword '%s' value '%s' with format '%s' in parameter value list.",
			keyword,parameter_value_list[parameter_value_list_index].Value_String,format);
		return FALSE;
	}
	return TRUE;
}
