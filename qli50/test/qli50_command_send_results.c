/* qli50_command_read_sensors.c
** Open a connection to the Vaisala Qli50, and send a send results command.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_udp.h"
#include "wms_qli50_command.h"
#include "wms_qli50_connection.h"
#include "wms_qli50_general.h"
#include "wms_serial_general.h"

/**
 * Open a connection to the Vaisala Qli50, and send a send results command.
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
/**
 * A character representing the QLI50 Identifier to send the command to. Usually 'A'.
 */
char Qli50_Id = 'A';
/**
 * A character representing the QLI50 sequence to use when reading sensors. Usually 'A'.
 */
char Seq_Id = 'A';

/* internal routines */
static void Print_Result(char *name,struct Wms_Qli50_Data_Value data_value);
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #DEFAULT_LOG_LEVEL
 * @see #Serial_Device_Name
 * @see #Qli50_Id
 * @see #Seq_Id
 * @see #Parse_Arguments
 * @see #Print_Result
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Handler_Function
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Log_Handler_Stdout
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Filter_Function
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Log_Filter_Level_Absolute
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Set_Log_Filter_Level
 * @see ../cdocs/wms_qli50_general.html#Wms_Qli50_Error
 * @see ../cdocs/wms_qli50_connection.html#Wms_Qli50_Connection_Open
 * @see ../cdocs/wms_qli50_connection.html#Wms_Qli50_Connection_Close
 * @see ../cdocs/wms_qli50_command.html#Wms_Qli50_Command_Send_Results
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
	struct Wms_Qli50_Data_Struct data;
	
	fprintf(stdout,"Qli50 Send Results\n");
	/* initialise logging */
	Wms_Qli50_Set_Log_Handler_Function(Wms_Qli50_Log_Handler_Stdout);
	Wms_Qli50_Set_Log_Filter_Function(Wms_Qli50_Log_Filter_Level_Absolute);
	Wms_Qli50_Set_Log_Filter_Level(DEFAULT_LOG_LEVEL);
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
		fprintf(stdout,"Qli50 Send Results: Specify a serial device filename.\n");
		return 2;
	}
	if(!Wms_Qli50_Connection_Open("Qli50 Send Results","qli50_command_send_results.c",Serial_Device_Name))
	{
		Wms_Qli50_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 3;
	}
	/* send command and read reply */
	fprintf(stdout,"Sending Send Results with sequence id '%c' to Qli50 with Id '%c'.\n",Seq_Id,Qli50_Id);
	if(!Wms_Qli50_Command_Send_Results("Qli50 Send Results","qli50_command_send_results.c",Qli50_Id,Seq_Id,&data))
	{
		Wms_Qli50_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 4;
	}
	fprintf(stdout,"Qli50 send results has been sent.\n");
	Print_Result("Temperature",data.Temperature);
	Print_Result("Humidity",data.Humidity);
	Print_Result("Dew Point",data.Dew_Point);
	Print_Result("Wind Speed",data.Wind_Speed);
	Print_Result("Wind Direction",data.Wind_Direction);
	Print_Result("Air Pressure",data.Air_Pressure);
	Print_Result("Digital Surface Wet",data.Digital_Surface_Wet);
	Print_Result("Analogue Surface Wet",data.Analogue_Surface_Wet);
	Print_Result("Light",data.Light);
	Print_Result("Internal Voltage",data.Internal_Voltage);
	Print_Result("Internal Current",data.Internal_Current);
	Print_Result("Internal Temperature",data.Internal_Temperature);
	Print_Result("Reference Temperature",data.Reference_Temperature);
	/* close interface */
	if(!Wms_Qli50_Connection_Close("Qli50 Send Results","qli50_command_send_results.c"))
	{
		Wms_Qli50_Error();
		if(Wms_Serial_Get_Error_Number() != 0)
			Wms_Serial_Error();
		return 5;
	}
	fprintf(stdout,"Qli50 Send Results:Finished.\n");
	return 0;
}

/**
 * Print out a data value returned from the Qli50.
 * @param name The name of the datum to print out.
 * @param data_value The data value (or error code) to print out.
 * @see #Wms_Qli50_Data_Value
 * @see #Wms_Qli50_Data_Type_Enum
 */
static void Print_Result(char *name,struct Wms_Qli50_Data_Value data_value)
{
	switch(data_value.Type)
	{
		case DATA_TYPE_DOUBLE:
			fprintf(stdout,"%s : %.2f.\n",name,data_value.Value.DValue);
			break;
		case DATA_TYPE_INT:
			fprintf(stdout,"%s : %d.\n",name,data_value.Value.IValue);
			break;
		case DATA_TYPE_ERROR:
			fprintf(stdout,"%s : Error: %d.\n",name,data_value.Value.Error_Code);
			break;
	}
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Qli50_Id
 * @see #Seq_Id
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
					fprintf(stderr,"Qli50 Send Results:Parse_Arguments:"
						"Illegal log level %s.\n",argv[i+1]);
					return FALSE;
				}
				Wms_Qli50_Set_Log_Filter_Level(ivalue);
				Wms_Serial_Set_Log_Filter_Level(ivalue);
				i++;
			}
			else
			{
				fprintf(stderr,"Qli50 Send Results:Parse_Arguments:"
					"Log Level requires a number.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-qli50_id")==0))
		{
			if((i+1)<argc)
			{
				if(strlen(argv[i+1]) == 1)
					Qli50_Id = argv[i+1][0];
				else
				{
					fprintf(stderr,"Qli50 Send Results:Parse_Arguments:"
						"Qli50_Id parameter must be a single letter.\n");
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Qli50 Send Results:Parse_Arguments:"
					"Qli50 Id requires an Id.\n");
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
				fprintf(stderr,"Qli50 Send Results:Parse_Arguments:"
					"Device filename requires a filename.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-seq_id")==0)||(strcmp(argv[i],"-sequence_id")==0))
		{
			if((i+1)<argc)
			{
				if(strlen(argv[i+1]) == 1)
					Seq_Id = argv[i+1][0];
				else
				{
					fprintf(stderr,"Qli50 Send Results:Parse_Arguments:"
						"Seq_Id parameter must be a single letter.\n");
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Qli50 Send Results:Parse_Arguments:"
					"Seq Id requires an Id.\n");
				return FALSE;
			}
		}
		else
		{
			fprintf(stderr,"Qli50 Send Results:Parse_Arguments:argument '%s' not recognized.\n",
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
	fprintf(stdout,"Qli50 Send Results:Help.\n");
	fprintf(stdout,"Qli50 Send Results sends an send results command to the Qli50.\n");
	fprintf(stdout,"qli50_command_send_results [-serial_device|-se <filename>][-qli_id <id>][-seq_id <id>]\n");
	fprintf(stdout,"\t[-l[og_level] <number>][-h[elp]]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-qli_id sets the QLI50 Id parameter to the send results command - this should be a single letter.\n");
	fprintf(stdout,"\t-seq_id sets the sequence Id parameter to the send results command - this should be a single letter.\n");
	fprintf(stdout,"\t-serial_device specifies the serial device name.\n");
	fprintf(stdout,"\te.g. /dev/ttyS0 for Linux.\n");
	fprintf(stdout,"\t-log_level specifies the logging(0..5).\n");
}
