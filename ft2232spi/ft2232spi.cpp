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
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */
#define SPI_WRITE_COMPLETION_RETRY 100
#define SPI_OPTS               SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE

/* Globals */
static FT_STATUS ftStatus;
static FT_HANDLE ftHandle;
static uint8 wBuffer[SPI_DEVICE_BUFFER_SIZE] = {0};
static uint8 rBuffer[SPI_DEVICE_BUFFER_SIZE] = {0};


int _tmain(int argc, _TCHAR* argv[]) {
	FT_STATUS status = FT_OK;
	FT_DEVICE_LIST_INFO_NODE devList = {0};
	ChannelConfig channelConf = {0};
	uint8 address = 0;
	uint32 channels = 0;
	uint16 data = 0;
	uint8 i = 0;
	
	channelConf.ClockRate = 400000;
	channelConf.LatencyTimer = 255;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS4 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0xFFff0000;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/
	channelConf.reserved = 0;

	/* init library */
#ifdef _MSC_VER
	Init_libMPSSE();
#endif
	status = SPI_GetNumChannels(&channels);
	APP_CHECK_STATUS(status);
	printf("Number of available SPI channels = %d\n",(int)channels);

	if(channels>0)
	{
#if 0
		for(i=0;i<channels;i++)
		{
			status = SPI_GetChannelInfo(i,&devList);
			APP_CHECK_STATUS(status);
			printf("Information on channel number %d:\n",i);
			/* print the dev info */
			printf("		Flags=0x%x\n",devList.Flags);
			printf("		Type=0x%x\n",devList.Type);
			printf("		ID=0x%x\n",devList.ID);
			printf("		LocId=0x%x\n",devList.LocId);
			printf("		SerialNumber=%s\n",devList.SerialNumber);
			printf("		Description=%s\n",devList.Description);
			printf("		ftHandle=0x%x\n",(unsigned int)devList.ftHandle);/*is 0 unless open*/
		}
#endif

		/* Open the first available channel */
		status = SPI_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
		APP_CHECK_STATUS(status);
		printf("\nhandle=0x%x status=0x%x\n",(unsigned int)ftHandle,status);
		status = SPI_InitChannel(ftHandle,&channelConf);
		APP_CHECK_STATUS(status);

		// Write some data for the lattice iCE40Ultra EVK demo
		uint32 sizeTransferred;

		wBuffer[0] = 0x7e; // Programming sync pattern
		wBuffer[1] = 0xaa;
		wBuffer[2] = 0x99;
		wBuffer[3] = 0x7e;
		status = SPI_ReadWrite(ftHandle, rBuffer, wBuffer, 4, &sizeTransferred, SPI_OPTS);
		APP_CHECK_STATUS(status);
		printf("Wrote/Read %d bytes\r\n", sizeTransferred);
		unsigned long retry = 0;
		bool state=FALSE;
		SPI_IsBusy(ftHandle,&state);
		while((FALSE==state) && (retry<SPI_WRITE_COMPLETION_RETRY))
		{
			printf("SPI device is busy(%u)\n",(unsigned)retry);
			SPI_IsBusy(ftHandle,&state);
			retry++;
		}
		printf("Read %02x %02x %02x\r\n", rBuffer[0], rBuffer[1], rBuffer[2]);

		status = SPI_CloseChannel(ftHandle);
	}

#ifdef _MSC_VER
	Cleanup_libMPSSE();
#endif

#ifndef __linux__
	system("pause");
#endif
	return 0;
}

