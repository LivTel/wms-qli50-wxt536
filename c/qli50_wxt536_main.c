/* qli50_wxt536_main.c
** Program entry point for the qli50_wxt536 program, which emulates a Qli50 weather station 
** by sitting on a serial port reading QLI50 commands,
** and invokes Wxt536 commands on another serial port when weather data is requested.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_udp.h"
#include "wms_qli50_connection.h"
#include "wms_qli50_general.h"
#include "wms_qli50_server.h"
#include "wms_serial_general.h"

/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id$";

/* internal functions */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * <ul>
 * <li>We call Parse_Arguments to parse the command line arguments.
 * <li>We call Qli50_Wxt526_Config_Load to load the config file.
 * <li>We call Qli50_Wxt526_Server_Start to start waiting on the Qli50 serial line for Qli50 commands.
 * </ul>
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #Parse_Arguments
 * @see qli50_wxt536_general.html#Qli50_Wxt526_General_Error
 * @see qli50_wxt536_config.html#Qli50_Wxt526_Config_Load
 * @see qli50_wxt536_server.html#Qli50_Wxt526_Server_Start
 */
int main(int argc, char *argv[])
{
	/* parse arguments */
	if(!Parse_Arguments(argc,argv))
		return 1;
	if(!Qli50_Wxt526_Config_Load())
	{
		Qli50_Wxt526_General_Error();
		return 1;
	}
	if(!Qli50_Wxt526_Server_Start())
	{
		Qli50_Wxt526_General_Error();
		return 1;
	}
	return 0;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see qli50_wxt536_config.html#Qli50_Wxt526_Config_Filename_Set
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,ivalue;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-c")==0)||(strcmp(argv[i],"-config_file")==0))
		{
			if((i+1)<argc)
			{
				Qli50_Wxt526_Config_Filename_Set(argv[i+1]);
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
	fprintf(stdout,"qli50_wxt536 [-c[onfig_file] <filename>][-h[elp]]\n");
}
