/* wxt536_command_wind_data_get.c
** Open a connection to the Vaisala Wxt536, retrieve the current wind data and print it out.
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
 * Open a connection to the Vaisala Wxt536, retrieve the current wind data and print it out.
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
/**
 * A character identifying the Vaisala Wxt536 device address.
 */
char Device_Address = ' ';

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #DEFAULT_LOG_LEVEL
 * @see #Serial_Device_Name
 * @see #Device_Address
 * @see #Parse_Arguments
 * @see ../cdocs/wms_wxt536_general.html#Wms_Wxt536_Set_Log_Handler_Function
 * @see ../cdocs/wms_wxt536_general.html#Wms_Wxt536_Log_Handler_Stdout
 * @see ../cdocs/wms_wxt536_general.html#Wms_Wxt536_Set_Log_Filter_Function
 * @see ../cdocs/wms_wxt536_general.html#Wms_Wxt536_Log_Filter_Level_Absolute
 * @see ../cdocs/wms_wxt536_general.html#Wms_Wxt536_Set_Log_Filter_Level
 * @see ../cdocs/wms_wxt536_general.html#Wms_Wxt536_Error
 * @see ../cdocs/wms_wxt536_connection.html#Wms_Wxt536_Connection_Open
 * @see ../cdocs/wms_wxt536_connection.html#Wms_Wxt536_Connection_Close
 * @see ../cdocs/wms_wxt536_command.html#Wxt536_Command_Wind_Data_Struct
 * @see ../cdocs/wms_wxt536_command.html#Wms_Wxt536_Command_Wind_Data_Get
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
	struct Wxt536_Command_Wind_Data_Struct wind_data;
	
	fprintf(stdout,"Wxt536 Get Wind Data\n");
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
		fprintf(stdout,"Wxt536 Get Wind Data: Specify a serial device filename.\n");
		return 2;
	}
	if(!Wms_Wxt536_Connection_Open("Wxt536 Get Wind Data","wxt536_command_wind_data_get.c",
				       Serial_Device_Name))
	{
		Wms_Wxt536_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 3;
	}
	/* send command and read reply */
	fprintf(stdout,"Getting Wind Data from Wxt536 with Device Address '%c'.\n",Device_Address);
	if(!Wms_Wxt536_Command_Wind_Data_Get("Wxt536 Get Wind Data","wxt536_command_wind_data_get.c",
					     Device_Address,&wind_data))
	{
		Wms_Wxt536_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 4;
	}
	fprintf(stdout,"Minimum wind direction %.2f degrees.\n",wind_data.Wind_Direction_Minimum);
	fprintf(stdout,"Average wind direction %.2f degrees.\n",wind_data.Wind_Direction_Average);
	fprintf(stdout,"Maximum wind direction %.2f degrees.\n",wind_data.Wind_Direction_Maximum);
	fprintf(stdout,"Minimum wind speed %.2f m/s.\n",wind_data.Wind_Speed_Minimum);
	fprintf(stdout,"Average wind speed %.2f m/s.\n",wind_data.Wind_Speed_Average);
	fprintf(stdout,"Maximum wind speed %.2f m/s.\n",wind_data.Wind_Speed_Maximum);
	/* close interface */
	if(!Wms_Wxt536_Connection_Close("Wxt536 Get Wind Data","wxt536_command_wind_data_get.c"))
	{
		Wms_Wxt536_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 5;
	}
	fprintf(stdout,"Wxt536 Get Wind Data:Finished.\n");
	return 0;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Device_Address
 * @see #Serial_Device_Name
 * @see ../cdocs/wms_wxt536_general.html#Wms_Wxt536_Set_Log_Filter_Level
 * @see ../../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Filter_Level
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,ivalue;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-d")==0)||(strcmp(argv[i],"-device_address")==0))
		{
			if((i+1)<argc)
			{
				if(strlen(argv[i+1]) != 1)
				{
					fprintf(stderr,"Wxt536 Get Wind Data:Parse_Arguments:"
						"Illegal device address '%s'.\n",argv[i+1]);
					return FALSE;
				}
				Device_Address = argv[i+1][0];
				i++;
			}
			else
			{
				fprintf(stderr,"Wxt536 Get Wind Data:Parse_Arguments:"
					"Device Address requires a character.\n");
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
					fprintf(stderr,"Wxt536 Get Wind Data:Parse_Arguments:"
						"Illegal log level %s.\n",argv[i+1]);
					return FALSE;
				}
				Wms_Wxt536_Set_Log_Filter_Level(ivalue);
				Wms_Serial_Set_Log_Filter_Level(ivalue);
				i++;
			}
			else
			{
				fprintf(stderr,"Wxt536 Get Wind Data:Parse_Arguments:"
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
				fprintf(stderr,"Wxt536 Get Wind Data:Parse_Arguments:"
					"Device filename requires a filename.\n");
				return FALSE;
			}
		}
		else
		{
			fprintf(stderr,"Wxt536 Get Wind Data:Parse_Arguments:argument '%s' not recognized.\n",
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
	fprintf(stdout,"Wxt536 Get Wind Data:Help.\n");
	fprintf(stdout,"Wxt536 Get Wind Data queries the Vaisala Wxt536 and gets the current wind data.\n");
	fprintf(stdout,"wxt536_command_wind_data_get [-serial_device|-se <filename>][-d[evice_address] <character>]\n");
	fprintf(stdout,"\t[-l[og_level] <number>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-serial_device specifies the serial device name.\n");
	fprintf(stdout,"\te.g. /dev/ttyS0 for Linux.\n");
	fprintf(stdout,"\t-device_address specifies the Wxt536. This character is normally '0'.\n");
	fprintf(stdout,"\t-log_level specifies the logging(0..5).\n");
}
