// bulkioct.h - ioctl codes for Usb Bulk Device sample
//
//=============================================================================
//
// Compuware Corporation
// NuMega Lab
// 9 Townsend West
// Nashua, NH 03060  USA
//
// Copyright (c) 1998 Compuware Corporation. All Rights Reserved.
// Unpublished - rights reserved under the Copyright laws of the
// United States.
//
//=============================================================================

//------------------------------------------------------------------------------
#ifndef __BULKIOCTH__
#define __BULKIOCT__
	//------------------------------------------------------------------------------
	// IOCTL codes can start at 0x800
	#define IOCTL_GET_CONFIG \
	   CTL_CODE (FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)

	#define DIOC_START \
	   CTL_CODE (FILE_DEVICE_UNKNOWN,0x1,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

	#define DIOC_STOP \
	   CTL_CODE (FILE_DEVICE_UNKNOWN,0x2,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

	#define DIOC_LOAD_BIOS_USB \
	   CTL_CODE (FILE_DEVICE_UNKNOWN,0x3,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

	#define DIOC_SEND_COMMAND    \
		CTL_CODE (FILE_DEVICE_UNKNOWN,0x4,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

	#define DIOC_RESET_PIPE1 \
	   CTL_CODE (FILE_DEVICE_UNKNOWN,0x5,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

	#define DIOC_RESET_PIPE3 \
	   CTL_CODE (FILE_DEVICE_UNKNOWN,0x6,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

#endif

