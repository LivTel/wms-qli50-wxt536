/* qli50_server_test.c
** Open a connection on the specified serial port, and sit on the conenction awaiting commands from a client,
** pretending to be a Vaisala QLI50 weather station.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_udp.h"
#include "wms_qli50_connection.h"
#include "wms_qli50_general.h"
#include "wms_qli50_server.h"
#include "wms_serial_general.h"
/**
 * Open a connection on the specified serial port, and sit on the conenction awaiting commands from a client,
 * pretending to be a Vaisala QLI50 weather station.
 * @author $Author: cjm $
 */
/**
 * Default log level.
 */
#define DEFAULT_LOG_LEVEL       (LOG_VERBOSITY_VERY_VERBOSE)


/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The name of the serial device to open.
 */
char Serial_Device_Name[256];

/* internal functions */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #DEFAULT_LOG_LEVEL
 * @see #Serial_Device_Name
 * @see #Parse_Arguments
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Handler_Function
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Log_Handler_Stdout
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Filter_Function
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Log_Filter_Level_Absolute
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Filter_Level
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Error
 * @see ../cdocs/wms_qli50_connection.html#Wms_Qli50_Connection_Open
 * @see ../cdocs/wms_qli50_connection.html#Wms_Qli50_Connection_Close
 * @see ../cdocs/wms_qli50_server.html#Wms_Qli50_Server_Start
 * @see ../cdocs/wms_qli50_server.html#Wms_Qli50_Server_Loop
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Handler_Function
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Log_Handler_Stdout
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Filter_Function
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Log_Filter_Level_Absolute
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Filter_Level
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Get_Error_Number
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Error
 */
int main(int argc, char *argv[])
{
	fprintf(stdout,"Qli50 Server Test\n");
	/* initialise logging */
	Wms_Qli50_Set_Log_Handler_Function(Wms_Qli50_Log_Handler_Stdout);
	Wms_Qli50_Set_Log_Filter_Function(Wms_Qli50_Log_Filter_Level_Absolute);
	Wms_Qli50_Set_Log_Filter_Level(DEFAULT_LOG_LEVEL);
	Wms_Qli50_Set_Log_Handler_Function(Wms_Serial_Log_Handler_Stdout);
	Wms_Qli50_Set_Log_Filter_Function(Wms_Serial_Log_Filter_Level_Absolute);
	Wms_Qli50_Set_Log_Filter_Level(DEFAULT_LOG_LEVEL);
	fprintf(stdout,"Parsing Arguments.\n");
	/* parse arguments */
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* open interface */
	if(strlen(Serial_Device_Name) < 1)
	{
		fprintf(stdout,"Qli50 Server Test: Specify a serial device filename.\n");
		return 2;
	}
	if(!Wms_Qli50_Server_Start("Qli50 Server Test","qli50_server_test.c",Serial_Device_Name))
	{
		Wms_Qli50_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 3;
	}
	/* start server loop */
	fprintf(stdout,"Starting Qli50 server loop....\n");
	if(!Wms_Qli50_Server_Loop("Qli50 Server Test","qli50_server_test.c"))
	{
		Wms_Qli50_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 4;
	}
	/* close interface */
	if(!Wms_Qli50_Connection_Close("Qli50 Server Test","qli50_server_test.c"))
	{
		Wms_Qli50_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 5;
	}
	fprintf(stdout,"Qli50 Server Test:Finished.\n");
	return 0;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Serial_Device_Name
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Filter_Level
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Filter_Level
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,ivalue;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-h")==0)||(strcmp(argv[i],"-help")==0))
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
					fprintf(stderr,"Qli50 Server Test:Parse_Arguments:"
						"Illegal log level %s.\n",argv[i+1]);
					return FALSE;
				}
				Wms_Qli50_Set_Log_Filter_Level(ivalue);
				Wms_Serial_Set_Log_Filter_Level(ivalue);
				i++;
			}
			else
			{
				fprintf(stderr,"Qli50 Server Test:Parse_Arguments:"
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
				fprintf(stderr,"Qli50 Server Test:Parse_Arguments:"
					"Device filename requires a filename.\n");
				return FALSE;
			}
		}
		else
		{
			fprintf(stderr,"Qli50 Server Test:Parse_Arguments:argument '%s' not recognized.\n",
				argv[i]);
			return FALSE;
		}			
	}
	return TRUE;
}

/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"Qli50 Server Test:Help.\n");
	fprintf(stdout,"Qli50 Server Test opens a connection to the specified serial device and awaits Vaisala Qli50 commands. It emulates a subset of the Vaisala Qli50 command set used by the LT Wms.\n");
	fprintf(stdout,"qli50_server_test [-serial_device|-se <filename>]\n");
	fprintf(stdout,"\t[-l[og_level] <number>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-serial_device specifies the serial device name.\n");
	fprintf(stdout,"\te.g. /dev/ttyS0 for Linux.\n");
	fprintf(stdout,"\t-log_level specifies the logging(0..5).\n");
}
