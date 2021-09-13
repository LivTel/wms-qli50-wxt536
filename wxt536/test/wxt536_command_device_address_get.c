/* wxt536_command_device_address_get.c
** Open a connection to the Vaisala Wxt536, send a get device address command, and parse the reply.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_udp.h"
#include "wms_wxt536_command.h"
#include "wms_wxt536_connection.h"
#include "wms_wxt536_general.h"
#include "wms_serial_general.h"

/**
 * Open a connection to the Vaisala Wxt536, send a get device address command, and parse the reply.
 * @author $Author: cjm $
 */
/* hash definitions */
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

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #DEFAULT_LOG_LEVEL
 */
int main(int argc, char *argv[])
{
	char reply_string[256];
	int device_address;

	fprintf(stdout,"Wxt536 Get Device Address\n");
	/* initialise logging */
	Wms_Wxt536_Set_Log_Handler_Function(Wms_Wxt536_Log_Handler_Stdout);
	Wms_Wxt536_Set_Log_Filter_Function(Wms_Wxt536_Log_Filter_Level_Absolute);
	Wms_Wxt536_Set_Log_Filter_Level(DEFAULT_LOG_LEVEL);
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
		fprintf(stdout,"Wxt536 Get Device Address: Specify a serial device filename.\n");
		return 2;
	}
	if(!Wms_Wxt536_Connection_Open("Wxt536 Get Device Address","wxt536_command_device_address_get.c",Serial_Device_Name))
	{
		Wms_Wxt536_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 3;
	}
	/* send command and read reply */
	if(!Wms_Wxt536_Command_Device_Address_Get("Wxt536 Get Device Address",
						  "wxt536_command_device_address_get.c",&device_address))
	{
		Wms_Wxt536_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 4;
	}
	fprintf(stdout,"The Wxt536 Device Address is %d\n",device_address);
	/* close interface */
	if(!Wms_Wxt536_Connection_Close("Wxt536 Get Device Address","wxt536_command_device_address_get.c"))
	{
		Wms_Wxt536_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 5;
	}
	fprintf(stdout,"Wxt536 Get Device Address:Finished.\n");
	return 0;
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
					fprintf(stderr,"Wxt536 Get Device Address:Parse_Arguments:"
						"Illegal log level %s.\n",argv[i+1]);
					return FALSE;
				}
				Wms_Wxt536_Set_Log_Filter_Level(ivalue);
				Wms_Serial_Set_Log_Filter_Level(ivalue);
				i++;
			}
			else
			{
				fprintf(stderr,"Wxt536 Get Device Address:Parse_Arguments:"
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
				fprintf(stderr,"Wxt536 Get Device Address:Parse_Arguments:"
					"Device filename requires a filename.\n");
				return FALSE;
			}
		}
		else
		{
			fprintf(stderr,"Wxt536 Get Device Address:Parse_Arguments:argument '%s' not recognized.\n",
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
	fprintf(stdout,"Wxt536 Get Device Address:Help.\n");
	fprintf(stdout,"Wxt536 Get Device Address queries the Vaisala Wxt536 and gets it's device address.\n");
	fprintf(stdout,"wxt536_send_command [-serial_device|-se <filename>][-l[og_level] <number>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-serial_device specifies the serial device name.\n");
	fprintf(stdout,"\te.g. /dev/ttyS0 for Linux.\n");
	fprintf(stdout,"\t-log_level specifies the logging(0..5).\n");
}
