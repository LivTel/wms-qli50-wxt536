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
/* hash defines */
/**
 * The terminator used to delimit the end of each line read. <cr> in this case.
 */
#define TERMINATOR_CR         ("\r")
/**
 * The terminator used to delimit the end of each line read. <cr><lf> in this case.
 */
#define TERMINATOR_CRLF         ("\r\n")
/**
 * ENQ (Enquiry) control character (Ctrl-E).
 */
#define CHARACTER_ENQ         ('\x05')
/**
 * ETX (End of text) control character (Ctrl-C).
 */
#define CHARACTER_ETX         ('\x03')
/**
 * SOH (Start of header) control character (Ctrl-A).
 */
#define CHARACTER_SOH         ('\x01')
/**
 * STX (Start of text) control character (Ctrl-B).
 */
#define CHARACTER_STX         ('\x02')
/**
 * SYN (Synchronous idle) control character (Ctrl-V).
 */
#define CHARACTER_SYN         ('\x16')

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* external functions */

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
 */
int Wms_Qli50_Command_Close(char *class,char *source)
{
	char reply_string[256];

	if(!Wms_Qli50_Command(class,source,"CLOSE",reply_string,255,TERMINATOR_CR))
		return FALSE;
	/* think about cr removal */
	if(strncmp(reply_string,"LINE CLOSED",strlen("LINE CLOSED")) != 0)
	{
		Wms_Qli50_Error_Number = 105;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Close:Unexpected reply string (%s).",reply_string);
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
 */
int Wms_Qli50_Command_Echo(char *class,char *source,int onoff)
{
	char onoff_string[32];
	char command_string[32];
	char reply_string[256];
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
	retval = sscanf(reply_string,"ECHO: %31s",reply_onoff_string);
	if(retval != 1)
	{
		Wms_Qli50_Error_Number = 107;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Echo:Unexpected reply string (%s).",reply_string);
		return FALSE;		
	}
	if(onoff && (strcmp(reply_onoff_string,"ON") != 0))
	{
		Wms_Qli50_Error_Number = 108;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Echo:Echo ON: Unexpected reply string (%s,%s).",reply_string,
			reply_onoff_string);
		return FALSE;		
	}
	else if((onoff == FALSE) && (strcmp(reply_onoff_string,"OFF") != 0))
	{
		Wms_Qli50_Error_Number = 109;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Echo:Echo OFF: Unexpected reply string (%s,%s).",reply_string,
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
 */
int Wms_Qli50_Command_Open(char *class,char *source,char qli_id)
{
	char command_string[32];
	char reply_string[256];
	char reply_qli_id;
	int retval;

	sprintf(command_string,"OPEN %c",qli_id);
	if(!Wms_Qli50_Command(class,source,command_string,reply_string,255,TERMINATOR_CR))
		return FALSE;
	retval = sscanf(reply_string,"%c OPENED FOR OPERATOR COMMANDS",&reply_qli_id);
	if(retval != 1)
	{
		Wms_Qli50_Error_Number = 110;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Open:Unexpected reply string (%s).",reply_string);
		return FALSE;		
	}
	if(qli_id != reply_qli_id)
	{
		Wms_Qli50_Error_Number = 111;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Open: Unexpected reply string (%s,%c,%c).",reply_string,qli_id,
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
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 */
int Wms_Qli50_Command_Par(char *class,char *source,char *reply_string,int reply_string_length)
{
	char command_string[32];
	char message[256];
	int done = FALSE;
	int retval,bytes_read;

	
	strcpy(command_string,"PAR");
	if(!Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,command_string,strlen(command_string)))
	{
		Wms_Qli50_Error_Number = 112;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command_Par:Failed to write command string '%s'.",
			command_string);
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
 * @see wms_qli50_general.html#Wms_Qli50_Log
 * @see wms_qli50_general.html#Wms_Qli50_Log_Format
 * @see wms_qli50_general.html#Wms_Qli50_Error_Number
 * @see wms_qli50_general.html#Wms_Qli50_Error_String
 */
int Wms_Qli50_Command_Sta(char *class,char *source,char *reply_string,int reply_string_length)
{
	char command_string[32];
	char message[256];
	int done = FALSE;
	int retval,bytes_read;

	strcpy(command_string,"STA");
	if(!Wms_Serial_Write(class,source,Wms_Qli50_Serial_Handle,command_string,strlen(command_string)))
	{
		Wms_Qli50_Error_Number = 114;
		sprintf(Wms_Qli50_Error_String,"Wms_Qli50_Command_Sta:Failed to write command string '%s'.",
			command_string);
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
 */
int Wms_Qli50_Command_Send_Results(char *class,char *source,char qli_id,char seq_id,struct Wms_Qli50_Data_Struct *data)
{
	char command_string[32];
	char reply_string[256];
	char *ch_ptr = NULL;
	int retval;

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
		Wms_Qli50_Error_Number = 117;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:<soh> not found in reply string '%s' (%d).",
			reply_string,(int)(reply_string[0]));
		return FALSE;		
	}
	if(reply_string[1] != qli_id)
	{
		Wms_Qli50_Error_Number = 118;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:Wrong qli_id '%c' (vs '%c') found in reply string '%s'.",
			reply_string[1],qli_id,reply_string);
		return FALSE;		
	}
	if(reply_string[2] != seq_id)
	{
		Wms_Qli50_Error_Number = 119;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:Wrong seq_id '%c' (vs '%c') found in reply string '%s'.",
			reply_string[2],seq_id,reply_string);
		return FALSE;		
	}
	if(reply_string[3] != CHARACTER_STX)
	{
		Wms_Qli50_Error_Number = 120;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:<stx> not found in reply string '%s' (%d).",
			reply_string,(int)(reply_string[3]));
		return FALSE;		
	}
	/* reply values start at reply_string+4 */
	/* find <etx> and terminate string at that position */
	ch_ptr = strchr(reply_string+4,CHARACTER_ETX);
	if(ch_ptr == NULL)
	{
		Wms_Qli50_Error_Number = 121;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:Failed to find <etx> in reply string '%s'.",
			reply_string);
		return FALSE;		
	}
	(*ch_ptr) = '\0';
	/* reply_string+4 now contains a series of comma-separated values/error codes */
	retval = sscanf(reply_string+4,"%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d,%lf,%lf,%lf,%lf",
			&(data->Temperature),&(data->Humidity),&(data->Dew_Point),
			&(data->Wind_Speed),&(data->Wind_Direction),&(data->Air_Pressure),
			&(data->Digital_Surface_Wet),&(data->Analogue_Surface_Wet),&(data->Light),
			&(data->Internal_Voltage),&(data->Internal_Current),
			&(data->Internal_Temperature),&(data->Reference_Temperature));
	if(retval != 13)
	{
		Wms_Qli50_Error_Number = 123;
		sprintf(Wms_Qli50_Error_String,
			"Wms_Qli50_Command_Send_Results:Failed to parse reply string '%s' (%d).",
			reply_string+4,retval);
		return FALSE;		
	}
	return TRUE;
}
