/* qli50_wxt536_general.c
** Weather monitoring system (qli50 -> wxt536 conversion), config file routines.
*/
/**
 * Routines to load and return values from a configuration file. This is implemented as a wrapper to the estar_config
 * library.
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
#include <time.h>
#include "estar_config.h"
#include "qli50_wxt536_general.h"
#include "qli50_wxt536_config.h"

/* defines */
/**
 * Length of the filename string.
 */
#define FILENAME_LENGTH           (256)
/**
 * The default configuration filename.
 */
#define DEFAULT_FILENAME          ("/home/dev/src/wms-qli50-wxt536/config/qli50_wxt536.conf")
/* external variables */

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The filename used to store the configuration data.
 */
static char Config_Filename[FILENAME_LENGTH] = DEFAULT_FILENAME;
/**
 * The structure holding the configuration properties loaded from the Config_Filename.
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Properties_t
 */
static eSTAR_Config_Properties_t Config_Properties;

/* =======================================================
** external functions 
** ======================================================= */
/**
 * Routine to set the configuration filename to be loaded/parsed.
 * @param filename The filename to use, as a string.
 * @return The routine returns TRUE on success, and FALSE on failure. 
 *         Qli50_Wxt536_Error_Number / Qli50_Wxt536_Error_String are set on failure. The routine can fail
 *         if filename is NULL, or it is too long for the allocated array length (FILENAME_LENGTH).
 * @see #FILENAME_LENGTH
 * @see #Config_Filename
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 */
int Qli50_Wxt536_Config_Filename_Set(char *filename)
{
	if(filename == NULL)
	{
		Qli50_Wxt536_Error_Number = 1;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Filename_Set:filename was NULL.");
		return FALSE;
	}
	if(strlen(filename) >= FILENAME_LENGTH)
	{
		Qli50_Wxt536_Error_Number = 2;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Filename_Set:filename was too long (%d).",
			strlen(filename));
		return FALSE;
	}
	strcpy(Config_Filename,filename);
	return TRUE;
}

/**
 * Routine load the configuration from the Config_Filename into memory, and store in Config_Properties, using
 * eSTAR_Config_Parse_File.
 * @return The routine returns TRUE on success, and FALSE on failure. 
 *         Qli50_Wxt536_Error_Number / Qli50_Wxt536_Error_String are set on failure. 
 * @see #Config_Filename
 * @see #Config_Properties
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Properties_t
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Parse_File
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Error_To_String
 */
int Qli50_Wxt536_Config_Load(void)
{
	char estar_error_string[512];
	int retval;

	retval = eSTAR_Config_Parse_File(Config_Filename,&Config_Properties);
	if(retval == FALSE)
	{
		eSTAR_Config_Error_To_String(estar_error_string);
		Qli50_Wxt536_Error_Number = 3;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Load:eSTAR_Config_Parse_File failed (%s).",
			estar_error_string);
		return FALSE;
	}
	return TRUE;
}

/**
 * Routine to get a string value for a specified keyword in the previously loaded set of configuration values.
 * @param keyword The keyword of the configuration item.
 * @param value A previosly allocated string array to store the string value of the configuration item 
 *        with the specified keyword.
 * @param value_length The length in bytes of the area allocated for value.
 * @return The routine returns TRUE on success, and FALSE on failure. 
 *         Qli50_Wxt536_Error_Number / Qli50_Wxt536_Error_String are set on failure. 
 * @see #Config_Properties
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Get_String
 */
int Qli50_Wxt536_Config_String_Get(char *keyword,char *value,int value_length)
{
	char estar_error_string[512];
	char *estar_config_string = NULL;
	int retval;
	
	if(keyword == NULL)
	{
		Qli50_Wxt536_Error_Number = 4;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_String_Get:keyword was NULL.");
		return FALSE;
	}
	if(value == NULL)
	{
		Qli50_Wxt536_Error_Number = 5;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_String_Get:value was NULL.");
		return FALSE;
	}
	retval = eSTAR_Config_Get_String(&Config_Properties,keyword,&estar_config_string);
	if(retval == FALSE)
	{
		eSTAR_Config_Error_To_String(estar_error_string);
		Qli50_Wxt536_Error_Number = 6;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Config_String_Get:eSTAR_Config_Get_String(keyword='%s') failed (%s).",
			keyword,estar_error_string);
		return FALSE;
	}
	if(strlen(estar_config_string) >= value_length)
	{
		Qli50_Wxt536_Error_Number = 7;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Config_String_Get:keyword='%s' had value too long (%d) for allocated space %d.",
			keyword,strlen(estar_config_string),value_length);
		return FALSE;		
	}
	strcpy(value,estar_config_string);
	return TRUE;
}

/**
 * Routine to get an integer value for a specified keyword in the previously loaded set of configuration values.
 * @param keyword The keyword of the configuration item.
 * @param value The address of an integer to store the integer value of the configuration item 
 *        with the specified keyword.
 * @return The routine returns TRUE on success, and FALSE on failure. 
 *         Qli50_Wxt536_Error_Number / Qli50_Wxt536_Error_String are set on failure. 
 * @see #Config_Properties
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Get_Int
 */
int Qli50_Wxt536_Config_Int_Get(char *keyword,int *value)
{
	char estar_error_string[512];
	int retval;
	
	if(keyword == NULL)
	{
		Qli50_Wxt536_Error_Number = 8;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Int_Get:keyword was NULL.");
		return FALSE;
	}
	if(value == NULL)
	{
		Qli50_Wxt536_Error_Number = 9;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Int_Get:value was NULL.");
		return FALSE;
	}
	retval = eSTAR_Config_Get_Int(&Config_Properties,keyword,value);
	if(retval == FALSE)
	{
		eSTAR_Config_Error_To_String(estar_error_string);
		Qli50_Wxt536_Error_Number = 10;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Config_Int_Get:eSTAR_Config_Get_Int(keyword='%s') failed (%s).",
			keyword,estar_error_string);
		return FALSE;
	}
	return TRUE;
}

/**
 * Routine to get a double value for a specified keyword in the previously loaded set of configuration values.
 * @param keyword The keyword of the configuration item.
 * @param value The address of a double to store the double value of the configuration item 
 *        with the specified keyword.
 * @return The routine returns TRUE on success, and FALSE on failure. 
 *         Qli50_Wxt536_Error_Number / Qli50_Wxt536_Error_String are set on failure. 
 * @see #Config_Properties
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Get_Double
 */
int Qli50_Wxt536_Config_Double_Get(char *keyword,double *value)
{
	char estar_error_string[512];
	int retval;
	
	if(keyword == NULL)
	{
		Qli50_Wxt536_Error_Number = 11;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Double_Get:keyword was NULL.");
		return FALSE;
	}
	if(value == NULL)
	{
		Qli50_Wxt536_Error_Number = 12;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Double_Get:value was NULL.");
		return FALSE;
	}
	retval = eSTAR_Config_Get_Double(&Config_Properties,keyword,value);
	if(retval == FALSE)
	{
		eSTAR_Config_Error_To_String(estar_error_string);
		Qli50_Wxt536_Error_Number = 13;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Config_Double_Get:eSTAR_Config_Get_Double(keyword='%s') failed (%s).",
			keyword,estar_error_string);
		return FALSE;
	}
	return TRUE;
}

/**
 * Routine to get a boolean value for a specified keyword in the previously loaded set of configuration values.
 * @param keyword The keyword of the configuration item.
 * @param value The address of an integer to store the boolean value of the configuration item 
 *        with the specified keyword.
 * @return The routine returns TRUE on success, and FALSE on failure. 
 *         Qli50_Wxt536_Error_Number / Qli50_Wxt536_Error_String are set on failure. 
 * @see #Config_Properties
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_Number
 * @see qli50_wxt536_general.html#Qli50_Wxt536_Error_String
 * @see ../../estar/config/cdocs/estar_config.html#eSTAR_Config_Get_Boolean
 */
int Qli50_Wxt536_Config_Boolean_Get(char *keyword,int *value)
{
	char estar_error_string[512];
	int retval;
	
	if(keyword == NULL)
	{
		Qli50_Wxt536_Error_Number = 14;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Boolean_Get:keyword was NULL.");
		return FALSE;
	}
	if(value == NULL)
	{
		Qli50_Wxt536_Error_Number = 15;
		sprintf(Qli50_Wxt536_Error_String,"Qli50_Wxt536_Config_Boolean_Get:value was NULL.");
		return FALSE;
	}
	retval = eSTAR_Config_Get_Boolean(&Config_Properties,keyword,value);
	if(retval == FALSE)
	{
		eSTAR_Config_Error_To_String(estar_error_string);
		Qli50_Wxt536_Error_Number = 16;
		sprintf(Qli50_Wxt536_Error_String,
			"Qli50_Wxt536_Config_Boolean_Get:eSTAR_Config_Get_Boolean(keyword='%s') failed (%s).",
			keyword,estar_error_string);
		return FALSE;
	}
	return TRUE;
}

