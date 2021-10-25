/* wms_qli50_command.c
** Weather monitoring system (qli50 -> wxt536 conversion), Vaisala Qli50 interface library, 
** command routines.
*/
/**
 * Routines to send commands (and parse their replies) to/from the Vaisala Qli50.
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
#include "wms_qli50_command.h"
#include "wms_qli50_connection.h"
#include "wms_qli50_general.h"


/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* internal functions */
static int Parse_Reply_Data_Value(char *class,char *source,char *ch_ptr,char *name,enum Wms_Qli50_Data_Type_Enum data_type,
				  struct Wms_Qli50_Data_Value *data_value,char **next_element_ptr);

/* ==========================================
** external functions 
** ========================================== */

/**
 * Basic routine to send a string command to the Vaisala Qli50 over a previously opened connection, 
 * and wait for a reply string.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param The command to send to the Vaisala Qli50 , as a NULL terminated string. The standard CR terminator
 *        will be added to this string before onward transmission to the Qli50.
 * @param reply_string An empty string, on return this is filled with any reply received from the Qli50.
 * @param reply_string_length The allocated length of the reply_string buffer.
 * @param reply_terminator The terminator to search for when reading the reply string. This is normally TERMINATOR_CR
 *        for most Qli50 commands, but results are terminated with TERMINATOR_CRLF.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #TERMINATOR_CR
 * @see #TERMINATOR_CRLF
 * @see wms_qli50_connection.html#Wms_Qli50_Serial_Handle
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Write
 * @see ../../serial/cdocs/wms_serial_serial.html#Wms_Serial_Read_Line
 */
int Wms_Qli50_Command(char *class,char *source,char *command_string,
		      char *reply_string,int reply_string_length,char *reply_terminator)
{
	char message[256];
	int bytes_read;
	
	Wms_Qli50_Error_Number = 0;
	if(command_string == NULL)
	{
		Wms_Qli50_Error_Number = 100;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command:Command String was NULL.");
		return FALSE;
	}
	if(strlen(command_string) >= 256)
	{
		Wms_Qli50_Error_Number = 101;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command:Command String was too long (%lu vs 255).",
			strlen(command_string));
		return FALSE;
	}
#if LOGGING > 9
	Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Command(%s) started.",command_string);
#endif /* LOGGING */
	strcpy(message,command_string);
	strcat(message,TERMINATOR_CR);
#if LOGGING > 9
	Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Qli50_Command(%s): Writing '%s' to serial handle.",
			     command_string,message);
#endif /* LOGGING */
	if(!Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,message,strlen(message)))
	{
		Wms_Qli50_Error_Number = 102;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command:Failed to write command string '%s'.",
			command_string);
		return FALSE;
	}
	/* read any reply */
	if(reply_string != NULL)
	{
		if(reply_terminator == NULL)
		{
			Wms_Qli50_Error_Number = 116;
			sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command:reply terminator string is NULL.");
			return FALSE;
		}
		if(!Wms_Serial_Read_Line(class,source,Wms_Qli50_Serial_Handle,reply_terminator,message,255,&bytes_read))
		{
			Wms_Qli50_Error_Number = 103;
			sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command:Failed to read reply line.");
			return FALSE;
		}
		message[bytes_read] = '\0';
		if(strlen(message) >= reply_string_length)
		{
			Wms_Qli50_Error_Number = 104;
			sprintf(Wms_Qli50_Error_String,
				"Wms_Qli50_Command:Reply message was too long to fit into reply string (%lu vs %d).",
				strlen(message),reply_string_length);
			return FALSE;
		}
		strcpy(reply_string,message);
#if LOGGING > 9
		Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Command(%s) returned reply '%s'.",
				      command_string,reply_string);
#endif /* LOGGING */
	}/* end if reply string was not NULL */
#if LOGGING > 9
	Wms_Qli50_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Wms_Qli50_Command(%s) finished.",command_string);
#endif /* LOGGING */
	return TRUE;
}

/**
 * Command to take the Qli50 out of engineering mode.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Wms_Qli50_Command
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see wms_qli50_general.html#Wms_Qli50_Log_Fix_Control_Chars
 */
int Wms_Qli50_Command_Close(char *class,char *source)
{
	char reply_string[256];
	char fixed_reply_string[256];

	if(!Wms_Qli50_Command(class,source,"CLOSE",reply_string,255,TERMINATOR_CR))
		return FALSE;
	/* think about cr removal */
	if(strncmp(reply_string,"LINE CLOSED",strlen("LINE CLOSED")) != 0)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 105;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Close:Unexpected reply string (%s).",fixed_reply_string);
		return FALSE;		
	}
	return TRUE;
}

/**
 * The Echo command enables or disables the Qli50's character echo.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param onoff A boolean, TRUE if we want to turn the echo on, and FALSE if we want to turn it off.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Wms_Qli50_Command
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see wms_qli50_general.html#Wms_Qli50_Log_Fix_Control_Chars
 */
int Wms_Qli50_Command_Echo(char *class,char *source,int onoff)
{
	char onoff_string[32];
	char command_string[32];
	char reply_string[256];
	char fixed_reply_string[256];
	char reply_onoff_string[32];
	int retval;

	if(!WMS_QLI50_IS_BOOLEAN(onoff))
	{
		Wms_Qli50_Error_Number = 106;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Echo:onoff was not a boolean (%d).",onoff);
		return FALSE;		
	}
	if(onoff)
		strcpy(onoff_string,"ON");
	else
		strcpy(onoff_string,"OFF");
	sprintf(command_string,"ECHO %s",onoff_string);
	if(!Wms_Qli50_Command(class,source,command_string,reply_string,255,TERMINATOR_CR))
		return FALSE;
	retval = sscanf(reply_string,"ECHO %31s",reply_onoff_string);
	if(retval != 1)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 107;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Echo:Unexpected reply string '%s' (%d).",fixed_reply_string,retval);
		return FALSE;		
	}
	if(onoff && (strcmp(reply_onoff_string,"ON") != 0))
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 108;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Echo:Echo ON: Unexpected reply string (%s,%s).",fixed_reply_string,
			reply_onoff_string);
		return FALSE;		
	}
	else if((onoff == FALSE) && (strcmp(reply_onoff_string,"OFF") != 0))
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 109;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Echo:Echo OFF: Unexpected reply string (%s,%s).",fixed_reply_string,
			reply_onoff_string);
		return FALSE;		
	}
	return TRUE;
}

/**
 * The Open command opens a serial connection to the Qli50, placing it in interactive (engineering) mode.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param qli_id A character, identifying the QLI50. This is normally 'A'..
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Wms_Qli50_Command
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see wms_qli50_general.html#Wms_Qli50_Log_Fix_Control_Chars
 */
int Wms_Qli50_Command_Open(char *class,char *source,char qli_id)
{
	char command_string[32];
	char reply_string[256];
	char fixed_reply_string[256];
	char reply_qli_id;
	int retval;

	sprintf(command_string,"OPEN %c",qli_id);
	if(!Wms_Qli50_Command(class,source,command_string,reply_string,255,TERMINATOR_CR))
		return FALSE;
	retval = sscanf(reply_string,"%c OPENED FOR OPERATOR COMMANDS",&reply_qli_id);
	if(retval != 1)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 110;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Open:Unexpected reply string (%s).",fixed_reply_string);
		return FALSE;		
	}
	if(qli_id != reply_qli_id)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 111;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Open: Unexpected reply string (%s,%c,%c).",fixed_reply_string,qli_id,
			reply_qli_id);
		return FALSE;		
	}
	return TRUE;
}


/**
 * The PAR command returns configuration parameters from the Qli50.
 * It rerurns many lines of data, and so cannot use Wms_Qli50_Command.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param reply_string The reply from the QLI50 containing the configuration parameter data.
 * @param reply_string_length The length of the reply_string buffer.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see wms_qli50_command.html#TERMINATOR_CR
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see wms_qli50_general.html#Wms_Qli50_Log_Fix_Control_Chars
 */
int Wms_Qli50_Command_Par(char *class,char *source,char *reply_string,int reply_string_length)
{
	char command_string[32];
	char fixed_command_string[32];
	char message[256];
	int done = FALSE;
	int retval,bytes_read;

	
	strcpy(command_string,"PAR");
	strcat(command_string,TERMINATOR_CR);
	if(!Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,command_string,strlen(command_string)))
	{
		Wms_Qli50_Log_Fix_Control_Chars(command_string,fixed_command_string);
		Wms_Qli50_Error_Number = 112;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command_Par:Failed to write command string '%s'.",
			fixed_command_string);
		return FALSE;
	}
	/* initialise reply string */
	if(reply_string != NULL)
		strcpy(reply_string,"");
	/* read any reply */
	/* We need a Wms_Serial_Read_Line with a timeout.
	** We need to keep reading lines of text until the text stops, and the read times out */
	done = FALSE;
	while(done == FALSE)
	{
		retval = Wms_Serial_Read_Line(class,source,Wms_Qli50_Serial_Handle,TERMINATOR_CR,message,255,
					      &bytes_read);
		message[bytes_read] = '\0';
		if(reply_string != NULL)
		{
			if((strlen(reply_string)+strlen(message)+1) >= reply_string_length)
			{
				Wms_Qli50_Error_Number = 113;
				sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command_Par:"
					"Reply message was too long to fit into reply string ((%lu+%lu) vs %d).",
					strlen(reply_string),strlen(message),reply_string_length);
				return FALSE;
			}
			strcat(reply_string,message);
		}
		done = (retval == FALSE);
	}/* end while */
	return TRUE;
}

/**
 * Command to initiate a hardware reset of the Qli50.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Wms_Qli50_Command
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 */
int Wms_Qli50_Command_Reset(char *class,char *source)
{
	char reply_string[256];

	if(!Wms_Qli50_Command(class,source,"RESET",reply_string,255,TERMINATOR_CR))
		return FALSE;
	return TRUE;
}

/**
 * The STA command returns status from the Qli50.
 * It rerurns many lines of data, and so cannot use Wms_Qli50_Command.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param reply_string The reply from the QLI50 containing the configuration parameter data.
 * @param reply_string_length The length of the reply_string buffer.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see wms_qli50_command.html#TERMINATOR_CR
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see wms_qli50_general.html#Wms_Qli50_Log_Fix_Control_Chars
 */
int Wms_Qli50_Command_Sta(char *class,char *source,char *reply_string,int reply_string_length)
{
	char command_string[32];
	char fixed_command_string[32];
	char message[256];
	int done = FALSE;
	int retval,bytes_read;

	strcpy(command_string,"STA");
	strcat(command_string,TERMINATOR_CR);
	if(!Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,command_string,strlen(command_string)))
	{
		Wms_Qli50_Log_Fix_Control_Chars(command_string,fixed_command_string);
		Wms_Qli50_Error_Number = 114;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command_Sta:Failed to write command string '%s'.",
			fixed_command_string);
		return FALSE;
	}
	/* initialise reply string */
	if(reply_string != NULL)
		strcpy(reply_string,"");
	/* read any reply */
	/* We need a Wms_Serial_Read_Line with a timeout.
	** We need to keep reading lines of text until the text stops, and the read times out */
	done = FALSE;
	while(done == FALSE)
	{
		retval = Wms_Serial_Read_Line(class,source,Wms_Qli50_Serial_Handle,TERMINATOR_CR,message,255,
					      &bytes_read);
		message[bytes_read] = '\0';
		if(reply_string != NULL)
		{
			if((strlen(reply_string)+strlen(message)+1) >= reply_string_length)
			{
				Wms_Qli50_Error_Number = 115;
				sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command_Sta:"
					"Reply message was too long to fit into reply string ((%lu+%lu) vs %d).",
					strlen(reply_string),strlen(message),reply_string_length);
				return FALSE;
			}
			strcat(reply_string,message);
		}
		done = (retval == FALSE);
	}/* end while */
	return TRUE;
}

/**
 * Command the Qli50 to start reading it's sensors. This is done by sending the '<syn> qli_id seq_id <cr>' command.
 * There is no reply to this message.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param qli_id A character representing which QLI50 to use, this is normally 'A'.
 * @param seq_id A character respresenting which saved sequence in the Qli50 to use, this is normally 'A'.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see #CHARACTER_SYN
 * @see #TERMINATOR_CR
 */
int Wms_Qli50_Command_Read_Sensors(char *class,char *source,char qli_id,char seq_id)
{
	char command_string[32];

	sprintf(command_string,"%c%c%c",CHARACTER_SYN,qli_id,seq_id);
	if(!Wms_Qli50_Command(class,source,command_string,NULL,0,NULL))
		return FALSE;
	return TRUE;
}

/**
 * Command the Qli50 to send the results of the last sampling of the sensors.
 * This is done by sending the '<enq> qli_id seq_id <cr>' command.
 * The reply is of the format: "<soh>qli_id seq_id<stx><reading> [,<reading>...]<etx><cr><lf>"
 * The sensor readings for sequence 'A' (the one used by the LT) are configurable, but for the LT the current
 * format is (as defined in Wms.cfg, QLI50_config.txt agrees):
 * <tt>
 * ###############################################################################
 * #Keywd  Symbol    Source   Low   High    N   Crit    Use    Run av  Comment
 * ###############################################################################
 * DEFINE, PT,           WS,  -40,   +60,  10,  FALSE,   TRUE,  1    # Temperature
 * DEFINE, VH,           WS,  0.0,  80.0,  10,   TRUE,   TRUE,  1    # Humidity
 * DEFINE, TDEW,         WS,  -80,   +20,  10,  FALSE,   TRUE,  1    # Dew point
 * DEFINE, F1WS,         WS,  0.0,  17.0,  10,   TRUE,   TRUE,  1    # Wind speed
 * DEFINE, RPWD,         WS,   -2,   360,  10,  FALSE,   TRUE,  1    # Wind direction
 * DEFINE, VPR,          WS,  500,  1100,  10,  FALSE,   TRUE,  1    # Air pressure
 * DEFINE, DSWE,         WS,    2,     5,   5,  FALSE,   TRUE,  1    # Surface wetness (dig)
 * DEFINE, VWE,          WS,    0,    10,   5,   TRUE,   TRUE,  1    # Surface wetness (an)
 * DEFINE, VPY,          WS,    0,  2000,   10, FALSE,   TRUE,  1    # Light
 * DEFINE, UIN,          WS,   11,    13,   10, FALSE,   TRUE,  1    # Internal voltage
 * DEFINE, IR,           WS,  1.1,   1.3,   10, FALSE,   TRUE,  1    # Internal current
 * DEFINE, TIN,          WS,  -50,   100,   10, FALSE,   TRUE,  1    # Internal temperature
 * DEFINE, RT,           WS,  -40,    80,   3,   TRUE,   TRUE,  1    # Reference temperature
 * </tt>
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param qli_id A character representing which QLI50 to use, this is normally 'A'.
 * @param seq_id A character respresenting which saved sequence in the Qli50 to use, this is normally 'A'.
 * @param data The address of a structure of type Wms_Qli50_Data_Struct, on a successful return this is filled in
 *        with the parsed data.
 * @return The procedure returns TRUE if successful, and FALSE if it failed 
 *         (Wms_Qli50_Error_Number and Wms_Qli50_Error_String are filled in on failure).
 * @see #Wms_Qli50_Data_Struct
 * @see #CHARACTER_ENQ
 * @see #CHARACTER_SOH
 * @see #CHARACTER_STX
 * @see #CHARACTER_ETX
 * @see #TERMINATOR_CRLF
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 * @see wms_qli50_general.html#Wms_Qli50_Log_Fix_Control_Chars
 */
int Wms_Qli50_Command_Send_Results(char *class,char *source,char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data)
{
	char command_string[32];
	char reply_string[256];
	char fixed_reply_string[256];
	char *ch_ptr = NULL;
	char *next_element_ptr = NULL;

	if(data == NULL)
	{
		Wms_Qli50_Error_Number = 122;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command_Send_Results:data was NULL.");
		return FALSE;				
	}
	sprintf(command_string,"%c%c%c",CHARACTER_ENQ,qli_id,seq_id);
	if(!Wms_Qli50_Command(class,source,command_string,reply_string,255,TERMINATOR_CRLF))
		return FALSE;
	/* parse the reply data 
	** This is in the format: <soh>qli_id seq_id<stx><reading> [,<reading>...]<etx><cr><lf> */
	if(reply_string[0] != CHARACTER_SOH)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 117;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:<soh> not found in reply string '%s' (%d).",
			fixed_reply_string,(int)(reply_string[0]));
		return FALSE;		
	}
	if(reply_string[1] != qli_id)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 118;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:Wrong qli_id '%c' (vs '%c') found in reply string '%s'.",
			reply_string[1],qli_id,fixed_reply_string);
		return FALSE;		
	}
	if(reply_string[2] != seq_id)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 119;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:Wrong seq_id '%c' (vs '%c') found in reply string '%s'.",
			reply_string[2],seq_id,fixed_reply_string);
		return FALSE;		
	}
	if(reply_string[3] != CHARACTER_STX)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 120;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:<stx> not found in reply string '%s' (%d).",
			fixed_reply_string,(int)(reply_string[3]));
		return FALSE;		
	}
	/* reply values start at reply_string+4 */
	/* find <etx> and terminate string at that position */
	ch_ptr = strchr(reply_string+4,CHARACTER_ETX);
	if(ch_ptr == NULL)
	{
		Wms_Qli50_Log_Fix_Control_Chars(reply_string,fixed_reply_string);
		Wms_Qli50_Error_Number = 121;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:Failed to find <etx> in reply string '%s'.",
			fixed_reply_string);
		return FALSE;		
	}
	(*ch_ptr) = '\0';
	/* reply_string+4 now contains a series of comma-separated values/error codes */
	ch_ptr = reply_string+4;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Temperature",DATA_TYPE_DOUBLE,&(data->Temperature),&next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Humidity",DATA_TYPE_DOUBLE,&(data->Humidity),&next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Dew Point",DATA_TYPE_DOUBLE,&(data->Dew_Point),&next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Wind Speed",DATA_TYPE_DOUBLE,&(data->Wind_Speed),&next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Wind Direction",DATA_TYPE_INT,&(data->Wind_Direction),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Air Pressure",DATA_TYPE_DOUBLE,&(data->Air_Pressure),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Digital Surface Wet",DATA_TYPE_INT,&(data->Digital_Surface_Wet),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Analogue Surface Wet",DATA_TYPE_INT,&(data->Analogue_Surface_Wet),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Light",DATA_TYPE_DOUBLE,&(data->Light),&next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Internal Voltage",DATA_TYPE_DOUBLE,&(data->Internal_Voltage),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Internal Current",DATA_TYPE_DOUBLE,&(data->Internal_Current),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Internal Temperature",DATA_TYPE_DOUBLE,&(data->Internal_Temperature),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	if(!Parse_Reply_Data_Value(class,source,ch_ptr,"Reference Temperature",DATA_TYPE_DOUBLE,&(data->Reference_Temperature),
				   &next_element_ptr))
		return FALSE;		
	ch_ptr = next_element_ptr;
	return TRUE;
}

/* ==========================================
** internal functions 
** ========================================== */
/**
 * Parse the next data value from the comma seperated results string and put the results (parsed as type data_type) 
 * in data_value. Return a character pointer to the end of the parsed data in next_element_ptr. The data value may be
 * either an integer or double, or could instead be an error code of the form 'E00NN' if there was an error getting a value
 * for this parameter.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param ch_ptr Pointer to a character string containing a list of comma separated values, the next of which is to be parsed.
 * @param name The name of the value value we are trying to parse.
 * @param data_type Which data type the value is to be parsed as (either  DATA_TYPE_DOUBLE or DATA_TYPE_INT). 
 * @param data_value The address of a Wms_Qli50_Data_Value struct to hold parsed value in, 
 *        or error code if the value is an error code.
 * @param next_element_ptr A pointer to a character pointer, on successful parsing of the data value this pointer is set to
 * the comma location +1, i.e. the start of the next data value to parse.
 * @see #Wms_Qli50_Data_Type_Enum
 * @see #Wms_Qli50_Data_Value
 */
static int Parse_Reply_Data_Value(char *class,char *source,char *ch_ptr,char *name,enum Wms_Qli50_Data_Type_Enum data_type,
				  struct Wms_Qli50_Data_Value *data_value,char **next_element_ptr)
{
	char data_value_string[256];
	char *end_ptr = NULL;
	char *e_ptr = NULL;
	int retval;
	
	if(name == NULL)
	{
		Wms_Qli50_Error_Number = 124;
		sprintf(Wms_Qli50_Error_String,"Parse_Reply_Data_Value:name was NULL.");
		return FALSE;		
	}
	if(ch_ptr == NULL)
	{
		Wms_Qli50_Error_Number = 125;
		sprintf(Wms_Qli50_Error_String,"Parse_Reply_Data_Value:ch_ptr was NULL for '%s'.",name);
		return FALSE;		
	}
	if((data_type != DATA_TYPE_DOUBLE)&&(data_type != DATA_TYPE_INT))
	{
		Wms_Qli50_Error_Number = 126;
		sprintf(Wms_Qli50_Error_String,"Parse_Reply_Data_Value:data_type was illegal type %d for '%s'.",
			data_type,name);
		return FALSE;		
	}
	if(data_value == NULL)
	{
		Wms_Qli50_Error_Number = 127;
		sprintf(Wms_Qli50_Error_String,"Parse_Reply_Data_Value:data value was NULL for '%s'.",name);
		return FALSE;		
	}
	if(next_element_ptr == NULL)
	{
		Wms_Qli50_Error_Number = 128;
		sprintf(Wms_Qli50_Error_String,"Parse_Reply_Data_Value:next_element_ptr was NULL for '%s'.",name);
		return FALSE;		
	}
	/* find the comma, copy data up to the next comma (or end of string) into data_value_string.
	** set (*next_element_ptr) to either the character following the comma, or the end of string if no commas are found. */
	end_ptr = strchr(ch_ptr,',');
	if(end_ptr == NULL) /* no more commas, the end of this value is the end of the string */
	{
		end_ptr = strchr(ch_ptr,'\0');
		(*next_element_ptr) = end_ptr;
	}
	else
		(*next_element_ptr) = end_ptr+1;
	if((end_ptr-ch_ptr) > 255)
	{
		Wms_Qli50_Error_Number = 129;
		sprintf(Wms_Qli50_Error_String,"Parse_Reply_Data_Value:The data value string was too long for '%s'.",name);
		return FALSE;		
	}
	strncpy(data_value_string,ch_ptr,(end_ptr-ch_ptr));
	data_value_string[(end_ptr-ch_ptr)] = '\0';
	/* check for error code */
	e_ptr = strchr(data_value_string,'E');
	if(e_ptr != NULL)
	{
		/* data value is actually an error code, parse this */
		data_value->Type = DATA_TYPE_ERROR;
		retval = sscanf(data_value_string," E0%d",&(data_value->Value.Error_Code));
		if(retval != 1)
		{
			Wms_Qli50_Error_Number = 130;
			sprintf(Wms_Qli50_Error_String,
				"Parse_Reply_Data_Value:Failed to parse error data value '%s' of data type %d for '%s'.",
				data_value_string,data_type,name);
			return FALSE;		
		}
	}
	else
	{
		/* data value should be a data value */
		if(data_type == DATA_TYPE_DOUBLE)
		{
			data_value->Type = DATA_TYPE_DOUBLE;
			retval = sscanf(data_value_string," %lf",&(data_value->Value.DValue));
		}
		else if(data_type == DATA_TYPE_INT)
		{
			data_value->Type = DATA_TYPE_INT;
			retval = sscanf(data_value_string," %d",&(data_value->Value.IValue));
		}
		else
		{
			Wms_Qli50_Error_Number = 131;
			sprintf(Wms_Qli50_Error_String,"Parse_Reply_Data_Value:Illegal data type %d for '%s'.",data_type,name);
			return FALSE;		
		}
		if(retval != 1)
		{
			Wms_Qli50_Error_Number = 132;
			sprintf(Wms_Qli50_Error_String,
				"Parse_Reply_Data_Value:Failed to parse data value '%s' of data type %d for '%s'.",
				data_value_string,data_type,name);
			return FALSE;		
		}
	}
	return TRUE;
}
