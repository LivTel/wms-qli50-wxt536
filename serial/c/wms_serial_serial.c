/* wms_serial_serial.c
** Weather monitoring system (qli50 -> wxt536 conversion), serial interface library, serial routines.
*/
/**
 * Basic operations, open close etc.
 * @author Chris Mottram
 * @version $Revision$
 */
#include <errno.h>   /* Error number definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include "log_udp.h"
#include "wms_serial_general.h"
#include "wms_serial_serial.h"

/* hash defines */
/**
 * The number of times to attempt a read which returns 0 bytes read, before failing and timing out,
 * when trying to read a line of data using Wms_Serial_Read_Line.
 */
#define READ_LINE_TIMEOUT (10)

/* data types */
/**
 * Data structure containing default baud rates and flags, an instance of which is used
 * to initialise new serial connections.
 * <dl>
 * <dt>Baud_Rate</dt> <dd>The baud rate of the serial connection : usually B19200 or B9600 from termios.h.</dd>
 * <dt>Input_Flags</dt> <dd>Flags to or into c_iflag.</dd>
 * <dt>Output_Flags</dt> <dd>Flags to or into c_oflag.</dd>
 * <dt>Control_Flags</dt> <dd>Flags to or into c_cflag.</dd>
 * <dt>Local_Flags</dt> <dd>Flags to or into c_lflag.</dd>
 * </dl>
 */
struct Serial_Attribute_Struct
{
	int Baud_Rate;
	int Input_Flags;
	int Output_Flags;
	int Control_Flags;
	int Local_Flags;
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * Instance of Serial_Attribute_Struct containing attributes to initialise opened serial connections with:
 * <dl>
 * <dt>Baud_Rate</dt> <dd>B19200</dd>
 * <dt>Input_Flags</dt> <dd>IGNPAR</dd>
 * <dt>Output_Flags</dt> <dd>0</dd>
 * <dt>Control_Flags</dt> <dd>CS8 | CLOCAL | CREAD</dd>
 * <dt>Local_Flags</dt> <dd>0</dd>
 * </dl>
 * @see #Serial_Attribute_Struct
 */
static struct Serial_Attribute_Struct Serial_Attribute_Data = 
{
	B19200,IGNPAR,0,CS8 | CLOCAL | CREAD, 0

};

/* external functions */
/**
 * Set the baud rate to be set for subsequently opened serial handles.
 * @param baud_rate The baud rate to use, usually B9600 or B19200 from termios.h - man tcsetattr to see all of them.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Serial_Attribute_Data
 */
int Wms_Serial_Baud_Rate_Set(int baud_rate)
{
	/* check baud rate.
	** This is a subset of all allowable values, man tcsetattr to see all of them */
	if((baud_rate != B1200)&&(baud_rate != B1800)&&(baud_rate != B2400)&&(baud_rate != B4800)&&
	   (baud_rate != B9600)&&(baud_rate != B19200)&&(baud_rate != B38400)&&(baud_rate != B57600)&&
	   (baud_rate != B115200))
	{
		Wms_Serial_Error_Number = 15;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Baud_Rate_Set: Illegal baud rate %d.",baud_rate);
		return FALSE;
	}
	Serial_Attribute_Data.Baud_Rate = baud_rate;
	return TRUE;
}

/**
 * Set the input flags to be set for subsequently opened serial handles.
 * @param flags The input flags to use, default is IGNPAR from termios.h - man tcsetattr to see all of them.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Serial_Attribute_Data
 */
int Wms_Serial_Input_Flags_Set(int flags)
{
	Serial_Attribute_Data.Input_Flags = flags;
	return TRUE;
}

/**
 * Set the output flags to be set for subsequently opened serial handles.
 * @param flags The input flags to use, default is 0 from termios.h - man tcsetattr to see all of them.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Serial_Attribute_Data
 */
int Wms_Serial_Output_Flags_Set(int flags)
{
	Serial_Attribute_Data.Output_Flags = flags;
	return TRUE;
}

/**
 * Set the control flags to be set for subsequently opened serial handles.
 * @param flags The control flags to use, default is CS8 | CLOCAL | CREAD from termios.h - 
 *        man tcsetattr to see all of them.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Serial_Attribute_Data
 */
int Wms_Serial_Control_Flags_Set(int flags)
{
	Serial_Attribute_Data.Control_Flags = flags;
	return TRUE;
}

/**
 * Set the local flags to be set for subsequently opened serial handles.
 * @param flags The local flags to use, default is 0 from termios.h - man tcsetattr to see all of them.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Serial_Attribute_Data
 */
int Wms_Serial_Local_Flags_Set(int flags)
{
	Serial_Attribute_Data.Local_Flags = flags;
	return TRUE;
}

/**
 * Open the serial device, and configure accordingly,using default in Serial_Attribute_Data.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle The address of a Wms_Serial_Handle_T structure to fill in.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Wms_Serial_Handle_T
 * @see #Serial_Attribute_Data
 */
int Wms_Serial_Open(char *class,char *source,Wms_Serial_Handle_T *handle)
{
	int open_errno,retval;

	if(handle == NULL)
	{
		Wms_Serial_Error_Number = 1;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Open: Device handle was NULL.");
		return FALSE;
	}
	/* Open serial device, read/write, not a controlling terminal, don't wait for DCD signal line. */
#if LOGGING > 0
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Serial_Serial_Open(%s).",handle->Device_Name);
#endif /* LOGGING */
	handle->Serial_Fd = open(handle->Device_Name, O_RDWR | O_NOCTTY );
	if(handle->Serial_Fd < 0)
	{
		open_errno = errno;
		Wms_Serial_Error_Number = 2;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Serial_Open: Device %s failed to open (%d,%d = %s).",
			handle->Device_Name,handle->Serial_Fd,open_errno,strerror(open_errno));
		return FALSE;
	}
#if LOGGING > 1
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Serial_Open with FD %d.",
			     handle->Serial_Fd);
#endif /* LOGGING */
	/* get current serial options */
	retval = tcgetattr(handle->Serial_Fd,&(handle->Serial_Options_Saved));
	if(retval != 0)
	{
		open_errno = errno;
		Wms_Serial_Error_Number = 3;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Open: tcgetattr failed (%d = %s).",open_errno,
			strerror(open_errno));
		return FALSE;
	}
	/* initialise new serial options */
	bzero(&(handle->Serial_Options), sizeof(handle->Serial_Options));
	/* set control flags and baud rate */
	handle->Serial_Options.c_cflag = Serial_Attribute_Data.Baud_Rate|Serial_Attribute_Data.Control_Flags;
	/* select raw input, clear ICANON to switch off canonical mode (character not line input). */
	handle->Serial_Options.c_lflag = Serial_Attribute_Data.Local_Flags & ~(ICANON);
	/* ignore parity errors */
	handle->Serial_Options.c_iflag = Serial_Attribute_Data.Input_Flags;
	/* set raw output */
	handle->Serial_Options.c_oflag = Serial_Attribute_Data.Output_Flags;
	/* wait for up to 10 deciseconds before timing out input */
	handle->Serial_Options.c_cc[VMIN]=0;
	handle->Serial_Options.c_cc[VTIME]=10;
	/* set input and output speeds again */
	retval = cfsetispeed(&(handle->Serial_Options),Serial_Attribute_Data.Baud_Rate);
	if(retval != 0)
	{
		open_errno = errno;
		Wms_Serial_Error_Number = 20;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Open: cfsetispeed failed (%d = %s).",open_errno,
			strerror(open_errno));
		return FALSE;
	}
	retval = cfsetospeed(&(handle->Serial_Options),Serial_Attribute_Data.Baud_Rate);
	if(retval != 0)
	{
		open_errno = errno;
		Wms_Serial_Error_Number = 21;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Open: cfsetospeed failed (%d = %s).",open_errno,
			strerror(open_errno));
		return FALSE;
	}
#if LOGGING > 2
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
	      "Wms_Serial_Open:New Attr:Input:%#x,Output:%#x,Local:%#x,Control:%#x,Min:%c,Time:%c.",
		       handle->Serial_Options.c_iflag,handle->Serial_Options.c_oflag,
		       handle->Serial_Options.c_lflag,handle->Serial_Options.c_cflag,
		       handle->Serial_Options.c_cc[VMIN],handle->Serial_Options.c_cc[VTIME]);
#endif /* LOGGING */
 	/* set new options */
#if LOGGING > 1
	Wms_Serial_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Open:Setting serial options.");
#endif /* LOGGING */
	tcflush(handle->Serial_Fd, TCIFLUSH);
	retval = tcsetattr(handle->Serial_Fd,TCSANOW,&(handle->Serial_Options));
	if(retval != 0)
	{
		open_errno = errno;
		Wms_Serial_Error_Number = 11;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Open: tcsetattr failed (%d = %s).",open_errno,
			strerror(open_errno));
		return FALSE;
	}
	/* re-get current serial options to see what was set */
#if LOGGING > 1
	Wms_Serial_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Open:Re-Getting new serial options.");
#endif /* LOGGING */
	retval = tcgetattr(handle->Serial_Fd,&(handle->Serial_Options));
	if(retval != 0)
	{
		open_errno = errno;
		Wms_Serial_Error_Number = 12;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Open: re-get tcgetattr failed (%d = %s).",
			open_errno,strerror(open_errno));
		return FALSE;
	}
#if LOGGING > 2
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
	      "Wms_Serial_Serial_Open:New Get Attr:Input:%#x,Output:%#x,Local:%#x,Control:%#x,Min:%c,Time:%c.",
		       handle->Serial_Options.c_iflag,handle->Serial_Options.c_oflag,
		       handle->Serial_Options.c_lflag,handle->Serial_Options.c_cflag,
		       handle->Serial_Options.c_cc[VMIN],handle->Serial_Options.c_cc[VTIME]);
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
			      "Wms_Serial_Serial_Open:Input Baud Rate:%d,B1200 = %d.",cfgetispeed(&(handle->Serial_Options)),B1200);
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
			      "Wms_Serial_Serial_Open:Output Baud Rate:%d.",cfgetospeed(&(handle->Serial_Options)));
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
			      "Wms_Serial_Serial_Open:Character bits %d, CS7 = %d.",
			      handle->Serial_Options.c_cflag&CSIZE,CS7);
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
			      "Wms_Serial_Serial_Open:Parity %d,Odd Parity = %d.",
			      handle->Serial_Options.c_cflag&PARENB,handle->Serial_Options.c_cflag&PARODD);
#endif /* LOGGING */
#if LOGGING > 0
	Wms_Serial_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Serial_Serial_Open:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to close a previously open serial device. The serial options are first reset.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle The address of a Wms_Serial_Handle_T structure to close.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Wms_Serial_Handle_T
 */
int Wms_Serial_Close(char *class,char *source,Wms_Serial_Handle_T *handle)
{
	int retval,close_errno;

#if LOGGING > 0
	Wms_Serial_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Serial_Close:Started.");
#endif /* LOGGING */
#if LOGGING > 1
	Wms_Serial_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Close:Resetting serial options.");
#endif /* LOGGING */
	retval = tcsetattr(handle->Serial_Fd,TCSANOW,&(handle->Serial_Options_Saved));
	if(retval != 0)
	{
		close_errno = errno;
		Wms_Serial_Error_Number = 13;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Close: tcsetattr failed (%d = %s).",close_errno,
			strerror(close_errno));
		return FALSE;
	}
#if LOGGING > 1
	Wms_Serial_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Serial_Close:Closing file descriptor.");
#endif /* LOGGING */
	retval = close(handle->Serial_Fd);
	if(retval < 0)
	{
		close_errno = errno;
		Wms_Serial_Error_Number = 4;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Close: failed (%d,%d,%d = %s).",
			handle->Serial_Fd,retval,close_errno,strerror(close_errno));
		return FALSE;
	}
#if LOGGING > 0
	Wms_Serial_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Wms_Serial_Serial_Close:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to write a message to the opened serial link.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle An instance of Wms_Serial_Handle_T containing connection information to write to.
 * @param message A pointer to an allocated buffer containing the bytes to write.
 * @param message_length The length of the message to write.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Wms_Serial_Handle_T
 */
int Wms_Serial_Write(char *class,char *source,Wms_Serial_Handle_T handle,void *message,size_t message_length)
{
	int write_errno,retval;

	if(message == NULL)
	{
		Wms_Serial_Error_Number = 5;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Write:Message was NULL.");
		return FALSE;
	}
#if LOGGING > 0
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Write(%d bytes).",
			     message_length);
#endif /* LOGGING */
	retval = write(handle.Serial_Fd,message,message_length);
#if LOGGING > 1
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Write returned %d.",retval);
#endif /* LOGGING */
	if(retval != message_length)
	{
		write_errno = errno;
		Wms_Serial_Error_Number = 6;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Write: failed (%d,%d,%d = %s).",
			handle.Serial_Fd,retval,write_errno,strerror(write_errno));
		return FALSE;
	}
#if LOGGING > 0
	Wms_Serial_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Write:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to read a message from the opened serial link. 
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle An instance of Wms_Serial_Handle_T containing connection information to read from.
 * @param message A buffer of message_length bytes, to fill with any serial data returned.
 * @param message_length The length of the message buffer.
 * @param bytes_read The address of an integer. On return this will be filled with the number of bytes read from
 *        the serial interface. The address can be NULL, if this data is not needed.
 * @return TRUE if succeeded, FALSE otherwise.
 */
int Wms_Serial_Read(char *class,char *source,Wms_Serial_Handle_T handle,void *message,int message_length,
			  int *bytes_read)
{
	int read_errno,retval;

	/* check input parameters */
	if(message == NULL)
	{
		Wms_Serial_Error_Number = 7;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Read:Message was NULL.");
		return FALSE;
	}
	if(message_length < 0)
	{
		Wms_Serial_Error_Number = 8;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Read:Message length was too small:%d.",
			message_length);
		return FALSE;
	}
	/* initialise bytes_read */
	if(bytes_read != NULL)
		(*bytes_read) = 0;
#if LOGGING > 0
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Read:Max length %d.",
			     message_length);
#endif /* LOGGING */
	retval = read(handle.Serial_Fd,message,message_length);
#if LOGGING > 1
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Read:returned %d.",retval);
#endif /* LOGGING */
	if(retval < 0)
	{
		read_errno = errno;
		/* if the errno is EAGAIN, a non-blocking read has failed to return any data. */
		if(read_errno != EAGAIN)
		{
			Wms_Serial_Error_Number = 9;
			sprintf(Wms_Serial_Error_String,"Wms_Serial_Read: failed (%d,%d,%d = %s).",
				handle.Serial_Fd,retval,read_errno,strerror(read_errno));
			return FALSE;
		}
		else
		{
			if(bytes_read != NULL)
				(*bytes_read) = 0;
		}
	}
	else
	{
		if(bytes_read != NULL)
			(*bytes_read) = retval;
	}
#if LOGGING > 0
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Read:returned %d of %d.",retval,
			     message_length);
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to read a message from the opened serial link. 
 * We have setup each individual read to timeout after 1 second, if no data arrives. We loop until we get
 * a terminator in the received data string, or we timeout after 10 reads attempts with no new data read (i.e. 10s).
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle An instance of Wms_Serial_Handle_T containing connection information to read from.
 * @param terminator A NULL terminated string containing the characters that are at the end of the current input line.
 * @param message A buffer of message_length bytes, to fill with any serial data returned.
 * @param message_length The length of the message buffer.
 * @param bytes_read The address of an integer. On return this will be filled with the number of bytes read from
 *        the serial interface. 
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #READ_LINE_TIMEOUT
 * @see #Wms_Serial_Handle_T
 */
int Wms_Serial_Read_Line(char *class,char *source,Wms_Serial_Handle_T handle,char *terminator,char *message,
			 int message_length, int *bytes_read)
{
	int read_errno,retval,timeout;

	/* check input parameters */
	if(message == NULL)
	{
		Wms_Serial_Error_Number = 16;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Read_Line:Message was NULL.");
		return FALSE;
	}
	if(message_length < 0)
	{
		Wms_Serial_Error_Number = 17;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Read_Line:Message length was too small:%d.",
			message_length);
		return FALSE;
	}
	if(bytes_read == NULL)
	{
		Wms_Serial_Error_Number = 18;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Read_Line:bytes_read was NULL.");
		return FALSE;
	}
#if LOGGING > 5
	Wms_Serial_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Read_line:starting.");
#endif /* LOGGING */
	/* initialise bytes_read */
	(*bytes_read) = 0;
	message[(*bytes_read)] = '\0';
	timeout = 0;
	while((strstr(message,terminator) == NULL)&&(timeout < READ_LINE_TIMEOUT))
	{
#if LOGGING > 10
		Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
				      "Wms_Serial_Read_line:starting read, current length %d bytes.",(*bytes_read));
#endif /* LOGGING */
		retval = read(handle.Serial_Fd,message+strlen(message),message_length-strlen(message));
		if(retval < 0)
		{
			read_errno = errno;
			/* if the errno is EAGAIN, a non-blocking read has failed to return any data. */
			if(read_errno != EAGAIN)
			{
				Wms_Serial_Error_Number = 19;
				sprintf(Wms_Serial_Error_String,"Wms_Serial_Read_Line: failed (%d,%d,%d = %s).",
					handle.Serial_Fd,retval,read_errno,strerror(read_errno));
				return FALSE;
			}
		}
		else
		{
			(*bytes_read) += retval;
			message[(*bytes_read)] = '\0';
			if(retval > 0)
				timeout = 0;
			else
				timeout++;
		}
	}/* end while */
	message[(*bytes_read)] = '\0';
	if(timeout >= READ_LINE_TIMEOUT)
	{
		Wms_Serial_Error_Number = 10;
		sprintf(Wms_Serial_Error_String,"Wms_Serial_Read_Line: Timed out after %d reads and %d bytes read.",
			timeout,(*bytes_read));
		return FALSE;
	}
#if LOGGING > 0
	Wms_Serial_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Wms_Serial_Read_line:read %d bytes.",(*bytes_read));
#endif /* LOGGING */
	return TRUE;
}

