// ft2232spi.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/* OS specific libraries */
#ifdef _WIN32
#include<windows.h>
#endif

/* Include D2XX header*/
#include "ftd2xx.h"

#pragma comment(lib, "FTD2XX.lib")
#pragma comment(lib, "libMPSSE.lib")

/* Include libMPSSE header */
#include "libMPSSE_spi.h"

/* Helper macros */

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};

/* Definitions */
#define SPI_DEVICE_BUFFER_SIZE		256
#define SPI_WRITE_COMPLETION_RETRY		10
#define START_ADDRESS_EEPROM 	0x00 /*read/write start address inside the EEPROM*/
#define END_ADDRESS_EEPROM		0x10
#define RETRY_COUNT_EEPROM		10	/* number of retries if read/write fails */
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */
#define SPI_SLAVE_0				0
#define SPI_SLAVE_1				1
#define SPI_SLAVE_2				2
#define DATA_OFFSET				4
#define USE_WRITEREAD			0

/* Globals */
static FT_STATUS ftStatus;
static FT_HANDLE ftHandle;
static uint8 buffer[SPI_DEVICE_BUFFER_SIZE] = {0};


int _tmain(int argc, _TCHAR* argv[])
{
	DWORD count = 0;
	DWORD devIndex = 0;
	char buffer[64];
	DWORD numDevs = 0;

	ftStatus = FT_ListDevices(&numDevs,NULL, FT_LIST_NUMBER_ONLY);
	if (ftStatus == FT_OK) {
		printf("Found %d devices\r\n", numDevs);
	}
	else {
		printf("ERROR, couldn't list devices: %d\r\n", ftStatus);
		return 1;
	}
	ftStatus = FT_ListDevices((PVOID)devIndex,&buffer, FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
	if (ftStatus != FT_OK) {
		printf("ERROR, couldn't list devices: %d\r\n", ftStatus);
		return 1;
	}
	ftStatus = FT_Open(0,&ftHandle);
	if (ftStatus != FT_OK) {
		printf("ERROR, couldn't open FT2232H device %d: %d\r\n", 1, ftStatus);
		return 1;
	}


	return 0;
}

