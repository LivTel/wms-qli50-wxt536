/* wms_serial_serial.h
 */

#ifndef WMS_SERIAL_SERIAL_H
#define WMS_SERIAL_SERIAL_H
#include <termios.h>
#include <unistd.h>

/* hash defines */
/**
 * The length of string to use for holding the name of the serial device.
 */
#define WMS_SERIAL_DEVICE_NAME_STRING_LENGTH (256)

/* structures */
/**
 * Structure holding local data pertinent to the serial module. This consists of:
 * <ul>
 * <li><b>Device_Name</b> The device name string of the serial port (e.g. /dev/ttyS0). 
 *     Maximum length WMS_SERIAL_DEVICE_NAME_STRING_LENGTH.
 * <li><b>Serial_Options_Saved</b> The saved set of serial options.
 * <li><b>Serial_Options</b> The set of serial options configured.
 * <li><b>Serial_Fd</b> The opened serial port's file descriptor.
 * </ul>
 * @see #WMS_SERIAL_DEVICE_NAME_STRING_LENGTH
 */
typedef struct Wms_Serial_Handle_Struct
{
	char Device_Name[WMS_SERIAL_DEVICE_NAME_STRING_LENGTH];
	struct termios Serial_Options_Saved;
	struct termios Serial_Options;
	int Serial_Fd;
} Wms_Serial_Handle_T;

extern int Wms_Serial_Baud_Rate_Set(int baud_rate);
extern int Wms_Serial_Input_Flags_Set(int flags);
extern int Wms_Serial_Output_Flags_Set(int flags);
extern int Wms_Serial_Control_Flags_Set(int flags);
extern int Wms_Serial_Local_Flags_Set(int flags);

extern int Wms_Serial_Open(char *class,char *source,Wms_Serial_Handle_T *handle);
extern int Wms_Serial_Close(char *class,char *source,Wms_Serial_Handle_T *handle);
extern int Wms_Serial_Write(char *class,char *source,Wms_Serial_Handle_T handle,void *message,
			    size_t message_length);
extern int Wms_Serial_Read(char *class,char *source,Wms_Serial_Handle_T handle,void *message,
			   int message_length,int *bytes_read);
extern int Wms_Serial_Read_Line(char *class,char *source,Wms_Serial_Handle_T handle,
				char *terminator,char *message,int message_length,int *bytes_read);


#endif
