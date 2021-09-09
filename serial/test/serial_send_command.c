/* serial_send_command.c
** Open a serial port, send some data, and wait for a reply. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_udp.h"
#include "wms_serial_serial.h"
#include "wms_serial_general.h"

/**
 * This program opens a serial port, send a terminated command string, and waits for a reply. 
 * @author $Author: cjm $
 */
/* hash definitions */
/**
 * Default log level.
 */
#define DEFAULT_LOG_LEVEL       (LOG_VERBOSITY_VERY_VERBOSE)
/**
 * The terminator used to delimit the end of each line read. <cr><lf> in this case.
 */
#define TERMINATOR_CRLF         ("\r\n")
/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The name of the serial device to open.
 */
char Serial_Device_Name[256];
/**
 * The command to send over the serial link.
 */
char Command_String[256];

/* internal routines */
static void Remove_Crtl_Characters(char *message);
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #DEFAULT_LOG_LEVEL
 * @see #TERMINATOR_CRLF
 * @see #Remove_Crtl_Characters
 */
int main(int argc, char *argv[])
{
	Wms_Serial_Handle_T serial_handle;
	char message[256];
	int done,bytes_read;
	
	fprintf(stdout,"Serial Send Command\n");
	/* initialise logging */
	Wms_Serial_Set_Log_Handler_Function(Wms_Serial_Log_Handler_Stdout);
	Wms_Serial_Set_Log_Filter_Function(Wms_Serial_Log_Filter_Level_Absolute);
	Wms_Serial_Set_Log_Filter_Level(DEFAULT_LOG_LEVEL);
	fprintf(stdout,"Parsing Arguments.\n");
	/* parse arguments */
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* open interface */
	if(strlen(Serial_Device_Name) < 1)
	{
		fprintf(stdout,"Serial Send Command: Specify a serial device filename.\n");
		return 2;
	}
	strcpy(serial_handle.Device_Name,Serial_Device_Name);
	if(!Wms_Serial_Open("Serial Send Command","serial_send_command.c",&serial_handle))
	{
		Wms_Serial_Error();
		return 3;
	}
	/* send command */
	if(strlen(Command_String) < 1)
	{
		fprintf(stdout,"Serial Send Command: Specify a command to send.\n");
		return 4;
	}
	strcpy(message,Command_String);
	strcat(message,TERMINATOR_CRLF);
	if(!Wms_Serial_Serial_Write("Serial Send Command","serial_send_command.c",serial_handle,
				    message,strlen(message)))
	{
		Wms_Serial_Error();
		Wms_Serial_Close("Serial Send Command","serial_send_command.c",&serial_handle);
		return 5;
	}
	/* read any reply */
	if(!Wms_Serial_Read_Line("Serial Send Command","serial_send_command.c",serial_handle,TERMINATOR_CRLF,message,255,&bytes_read))
	{
		Wms_Serial_Error();
		Wms_Serial_Close("Serial Send Command","serial_send_command.c",&serial_handle);
		return 6;
	}
	message[bytes_read] = '\0';
	Remove_Crtl_Characters(message);
	if(bytes_read > 0)
		fprintf(stdout,"%s\n",message);
	/* close interface */
	if(!Wms_Serial_Close("Serial Send Command","serial_send_command.c",&serial_handle))
	{
		Wms_Serial_Error();
		return 7;
	}
	fprintf(stdout,"Serial Send Command:Finished.\n");
	return 0;
}

/**
 * Remove control characters from the string.
 * @param message The string to modify.
 */
static void Remove_Crtl_Characters(char *message)
{
	int i;

	for(i=0; i < strlen(message); i++)
	{
		if(message[i] < 32)
		{
			fprintf(stderr,"Swapped character %d for space at position %d\n",(int)(message[i]),i);
			message[i] = ' ';
		}
		if(message[i] > 127)
		{
			fprintf(stderr,"Swapped character %d for space at position %d\n",(int)(message[i]),i);
			message[i] = ' ';
		}
	}
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Command_String
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,ivalue;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-c")==0)||(strcmp(argv[i],"-command")==0))
		{
			if((i+1)<argc)
			{
				strcpy(Command_String,argv[i+1]);
				i++;
			}
			else
			{
				fprintf(stderr,"Serial Send Command:Parse_Arguments:"
					"You must specify a command to send.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-h")==0)||(strcmp(argv[i],"-help")==0))
		{
			Help();
			exit(0);
		}
		else if((strcmp(argv[i],"-l")==0)||(strcmp(argv[i],"-log_level")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&ivalue);
				if(retval != 1)
				{
					fprintf(stderr,"Serial Send Command:Parse_Arguments:"
						"Illegal log level %s.\n",argv[i+1]);
					return FALSE;
				}
				Wms_Serial_Set_Log_Filter_Level(ivalue);
				i++;
			}
			else
			{
				fprintf(stderr,"Serial Send Command:Parse_Arguments:"
					"Log Level requires a number.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-se")==0)||(strcmp(argv[i],"-serial_device")==0))
		{
			if((i+1)<argc)
			{
				strcpy(Serial_Device_Name,argv[i+1]);
				i++;
			}
			else
			{
				fprintf(stderr,"Serial Send Command:Parse_Arguments:"
					"Device filename requires a filename.\n");
				return FALSE;
			}
		}
		else
		{
			fprintf(stderr,"Serial Send Command:Parse_Arguments:argument '%s' not recognized.\n",
				argv[i]);
			return FALSE;
		}			
	}
	return TRUE;
}

/**
 * Help routine.
 * @see #DEFAULT_DEVICE_NAME
 */
static void Help(void)
{
	fprintf(stdout,"Serial Send Command:Help.\n");
	fprintf(stdout,"Serial Send Command sends a terminated command string to a device over a serial port and waits for a reply string.\n");
	fprintf(stdout,"serial_send_command [-serial_device|-se <filename>][-c[ommand] <string>]\n");
	fprintf(stdout,"\t[-l[og_level] <number>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-serial_device specifies the serial device name.\n");
	fprintf(stdout,"\t\tTry /dev/ttyS0 for Linux, try /dev/ttyb for Solaris.\n");
	fprintf(stdout,"\t-log_level specifies the logging(0..5).\n");
	fprintf(stdout,"\t-command specifies the string to send. A terminator is added before transmission.\n");
}

