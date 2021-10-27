/* qli50_wxt536_main.c
** Program entry point for the qli50_wxt536 program, which emulates a Qli50 weather station 
** by sitting on a serial port reading QLI50 commands,
** and invokes Wxt536 commands on another serial port when weather data is requested.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_udp.h"
#include "wms_qli50_general.h"
#include "wms_wxt536_general.h"
#include "wms_serial_general.h"
#include "qli50_wxt536_config.h"
#include "qli50_wxt536_general.h"
#include "qli50_wxt536_server.h"

/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The log level to use, can be set from the command line arguments, or parsed from the config file.
 */
static int Log_Level = 0;
/**
 * Whether the log level has been set in the command line arguments.
 */
static int Log_Level_Argument_Set = FALSE;

/* internal functions */
static int Qli50_Wxt536_Logging_Initialise(void);
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * <ul>
 * <li>We call Parse_Arguments to parse the command line arguments.
 * <li>We call Qli50_Wxt536_Config_Load to load the config file.
 * <li>We call Qli50_Wxt536_Logging_Initialise to initialise logging.
 * <li>We call Qli50_Wxt536_Server_Initialise to configure the Qli50 server and open the Qli50 serial line.
 * <li>We call Qli50_Wxt536_Server_Start to start waiting on the Qli50 serial line for Qli50 commands.
 * </ul>
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Parse_Arguments
 * @see #Qli50_Wxt536_Logging_Initialise
 * @see qli50_wxt536_general.html#Qli50_Wxt536_General_Error
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_Load
 * @see qli50_wxt536_server.html#Qli50_Wxt536_Server_Initialise
 * @see qli50_wxt536_server.html#Qli50_Wxt536_Server_Start
 */
int main(int argc, char *argv[])
{
	/* parse arguments */
	if(!Parse_Arguments(argc,argv))
		return 1;
	if(!Qli50_Wxt536_Config_Load())
	{
		Qli50_Wxt536_Error();
		return 2;
	}
	if(!Qli50_Wxt536_Logging_Initialise())
	{
		Qli50_Wxt536_Error();
		return 3;
	}
	if(!Qli50_Wxt536_Server_Initialise())
	{
		Qli50_Wxt536_Error();
		return 4;
	}
	if(!Qli50_Wxt536_Server_Start())
	{
		Qli50_Wxt536_Error();
		return 5;
	}
	return 0;
}

/**
 * Routine to initialise the logging. We configure the main program, Qli50 library, Wxt536 library, Serial library,
 * all to use the Qli50_Wxt536_Log_Handler_Stdout and Qli50_Wxt536_Log_Filter_Level_Absolute, and set the Log_Level
 * to Log_Level.
 * @return The routine returns TRUE on success, and FALSE on failure (we fail to get the default log level).
 * @see #Log_Level
 * @see #Log_Level_Argument_Set
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_Int_Get
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Log_Handler_Function_Set
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Log_Filter_Function_Set
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Log_Filter_Level_Set
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Log_Handler_Stdout
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Log_Filter_Level_Absolute
 * @see ../qli50/cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Handler_Function
 * @see ../qli50/cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Filter_Function
 * @see ../qli50/cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Filter_Level
 * @see ../wxt536/cdocs/wms_wxt536_general.html#Wms_Wxt536_Set_Log_Handler_Function
 * @see ../wxt536/cdocs/wms_wxt536_general.html#Wms_Wxt536_Set_Log_Filter_Function
 * @see ../wxt536/cdocs/wms_wxt536_general.html#Wms_Wxt536_Set_Log_Filter_Level
 * @see ../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Handler_Function
 * @see ../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Filter_Function
 * @see ../serial/cdocs/wms_serial_general.html#Wms_Serial_Set_Log_Filter_Level
 */
static int Qli50_Wxt536_Logging_Initialise(void)
{
	if(Log_Level_Argument_Set == FALSE)
	{
		if(!Qli50_Wxt536_Config_Int_Get("log.level.default",&Log_Level))
			return FALSE;
	}
	/* main program */
	Qli50_Wxt536_Log_Handler_Function_Set(Qli50_Wxt536_Log_Handler_Stdout);
	Qli50_Wxt536_Log_Filter_Function_Set(Qli50_Wxt536_Log_Filter_Level_Absolute);
	Qli50_Wxt536_Log_Filter_Level_Set(Log_Level);
	/* Qli50 library */
	Wms_Qli50_Set_Log_Handler_Function(Qli50_Wxt536_Log_Handler_Stdout);
	Wms_Qli50_Set_Log_Filter_Function(Qli50_Wxt536_Log_Filter_Level_Absolute);
	Wms_Qli50_Set_Log_Filter_Level(Log_Level);
	/* Wxt536 library */
	Wms_Wxt536_Set_Log_Handler_Function(Qli50_Wxt536_Log_Handler_Stdout);
	Wms_Wxt536_Set_Log_Filter_Function(Qli50_Wxt536_Log_Filter_Level_Absolute);
	Wms_Wxt536_Set_Log_Filter_Level(Log_Level);
	/* Serial library */
	Wms_Serial_Set_Log_Handler_Function(Qli50_Wxt536_Log_Handler_Stdout);
	Wms_Serial_Set_Log_Filter_Function(Qli50_Wxt536_Log_Filter_Level_Absolute);
	Wms_Serial_Set_Log_Filter_Level(Log_Level);
	return TRUE;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Log_Level
 * @see qli50_wxt536_config.html#Qli50_Wxt536_Config_Filename_Set
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval;

	Log_Level_Argument_Set = FALSE;
	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-c")==0)||(strcmp(argv[i],"-config_file")==0))
		{
			if((i+1)<argc)
			{
				if(!Qli50_Wxt536_Config_Filename_Set(argv[i+1]))
				{
					Qli50_Wxt536_Error();
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Qli50 Wxt536:Parse_Arguments:"
					"-config_file requires a filename argument.\n");
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
				retval = sscanf(argv[i+1],"%d",&Log_Level);
				if(retval != 1)
				{
					fprintf(stderr,"Qli50 Wxt536:Parse_Arguments:"
						"Illegal log level %s.\n",argv[i+1]);
					return FALSE;
				}
				Log_Level_Argument_Set = TRUE;
				i++;
			}
			else
			{
				fprintf(stderr,"Qli50 Wxt536:Parse_Arguments:"
					"Log Level requires a number.\n");
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
	fprintf(stdout,"Qli50 to Wxt536 :Help.\n");
	fprintf(stdout,"This program opens a serial port and waits for Qli50 commands to be sent to it. These are either emulated, or when weather data is requested a Wxt536 on another serial port is queried, and the results translated.\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"qli50_wxt536 [-c[onfig_file] <filename>][-l[og_level] <number>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-log_level specifies the logging(0..5).\n");
}
