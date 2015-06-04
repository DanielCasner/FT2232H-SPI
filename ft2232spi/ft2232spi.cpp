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
#define PROG_BUFFER_SIZE        4000000
#define SPI_DEVICE_BUFFER_SIZE		256
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */
#define SPI_WRITE_COMPLETION_RETRY 100
#define SPI_OPTS               SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
#define GPIO_INIT			   GPIO_DIR /* Everything that's an output should be set high */
//#define RESET_CHIP
#define TEST_COMMUNICATION

int _tmain(int argc, _TCHAR* argv[]) {
	FT_STATUS ftStatus;
	FT_HANDLE ftHandle;
	uint8* wBuffer = new uint8[PROG_BUFFER_SIZE];
	uint8* rBuffer = new uint8[SPI_DEVICE_BUFFER_SIZE];

	FILE* fileHandle;
	size_t fileSize;
	FT_STATUS status = FT_OK;
	FT_DEVICE_LIST_INFO_NODE devList = {0};
	ChannelConfig channelConf = {0};
	uint8 address = 0;
	uint32 channels = 0;
	uint32 sizeTransferred;
	uint16 data = 0;
	uint8 i = 0;
	
	if (wBuffer == NULL) {
		printf("Couln't allocate memeory for write buffer.\r\n");
		exit(1);
	}
	if (rBuffer == NULL) {
		printf("Couln't allocate memeory for read buffer.\r\n");
		exit(1);
	}


	channelConf.ClockRate = 3000000;
	channelConf.LatencyTimer = 255;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS4 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x80B080B0;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/
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
		/* Open the first available channel */
		status = SPI_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
		APP_CHECK_STATUS(status);
		printf("\nhandle=0x%x status=0x%x\n",(unsigned int)ftHandle,status);

		status = SPI_InitChannel(ftHandle,&channelConf);
		APP_CHECK_STATUS(status);

#ifdef RESET_CHIP
		// Reset the FPGA by setting a dummy pin as the chip select with active high which sets both SS and CRESET_B low
		status = SPI_ChangeCS(ftHandle, SPI_CONFIG_OPTION_CS_DBUS5);
		APP_CHECK_STATUS(status);
		SPI_Write(ftHandle, wBuffer, 1, &sizeTransferred, SPI_OPTS);
		APP_CHECK_STATUS(status);

		Sleep(1);

		// Change the chip select back to the actual chip select
		SPI_ChangeCS(ftHandle, SPI_CONFIG_OPTION_CS_DBUS4 | SPI_CONFIG_OPTION_CS_ACTIVELOW);
		APP_CHECK_STATUS(status);

		Sleep(1);
#endif 
		


		// Write some data for the lattice iCE40Ultra EVK demo

		fileHandle = fopen("C:\\Users\\DanielCascner\\Documents\\peripherals\\peripherals_Implmnt\\sbt\\outputs\\bitmap\\top_bitmap.bin", "rb");
		if (fileHandle == NULL) {
			printf("Couldn't open programming file\r\n");
			exit(1);
		}
		fseek(fileHandle, 0, SEEK_END);
		fileSize   = ftell(fileHandle);
		printf("Programming file is %d bytes\r\n", fileSize);
		fseek(fileHandle, 0, SEEK_SET);		

		sizeTransferred = fread(wBuffer, sizeof(uint8), fileSize, fileHandle);
		if (sizeTransferred != fileSize) {
			printf("Didn't read full programming file %d < %d\r\n", sizeTransferred, fileSize);
			exit(1);
		}
		for (int i=0; i<50; ++i) wBuffer[sizeTransferred++] = 0; // Add dummy bytes

		status = SPI_Write(ftHandle, wBuffer, sizeTransferred, &sizeTransferred, SPI_OPTS);
		APP_CHECK_STATUS(status);
		printf("Transferred %d bytes\r\n", sizeTransferred);
		unsigned long retry = 0;
		bool state=FALSE;
		SPI_IsBusy(ftHandle,&state);
		while((FALSE==state) && (retry<SPI_WRITE_COMPLETION_RETRY))
		{
			printf("SPI device is busy(%u)\n",(unsigned)retry);
			SPI_IsBusy(ftHandle,&state);
			retry++;
		}

#ifdef TEST_COMMUNICATION
		Sleep(10);

		for (sizeTransferred = 0; sizeTransferred < 10; sizeTransferred++) wBuffer[sizeTransferred] = sizeTransferred;
		status = SPI_ReadWrite(ftHandle, rBuffer, wBuffer, sizeTransferred, &sizeTransferred, SPI_OPTS);
		APP_CHECK_STATUS(status);
		printf("Transferred %d bytes\r\n", sizeTransferred);
		rBuffer[sizeTransferred] = 0;
		printf("Received %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \r\n", rBuffer[0], rBuffer[1], rBuffer[2], rBuffer[3], rBuffer[4], rBuffer[5], rBuffer[6], rBuffer[7], rBuffer[8], rBuffer[9]);
		
#endif
		status = SPI_CloseChannel(ftHandle);
	}

	delete wBuffer;
	delete rBuffer;

#ifdef _MSC_VER
	Cleanup_libMPSSE();
#endif

#ifndef __linux__
	system("pause");
#endif
	return 0;
}

