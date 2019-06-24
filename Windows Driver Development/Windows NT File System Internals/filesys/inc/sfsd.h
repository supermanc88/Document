/*************************************************************************
*
* File: sfsd.h
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	The main include file for the sample file system driver.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#ifndef	_SFSD_SFSD_H_
#define	_SFSD_SFSD_H_

// some constant definitions
#define	SFSD_PANIC_IDENTIFIER		(0x86427531)

// any directory information SFSD obtains from the local file system
//	will use a buffer of the following size ... (in KB)
#define	SFSD_READ_DIR_BUFFER_LENGTH	(512)

// Common include files - should be in the include dir of the MS supplied IFS Kit
#include	<ntifs.h>

// the following include files should be in the inc sub-dir associated with this driver
#include	"struct.h"
#include	"protos.h"
#include "errmsg.h"

// global variables - minimize these
extern SFsdData				SFsdGlobalData;

// try-finally simulation
#define try_return(S)	{ S; goto try_exit; }
#define try_return1(S)	{ S; goto try_exit1; }
#define try_return2(S)	{ S; goto try_exit2; }

// some global (helpful) macros
#define	SFsdSetFlag(Flag, Value)	((Flag) |= (Value))
#define	SFsdClearFlag(Flag, Value)	((Flag) &= ~(Value))

#define	SFsdQuadAlign(Value)			((((uint32)(Value)) + 7) & 0xfffffff8)

// to perform a bug-check (panic), the following macro is used
#define	SFsdPanic(arg1, arg2, arg3)					\
	(KeBugCheckEx(SFSD_PANIC_IDENTIFIER, SFSD_BUG_CHECK_ID | __LINE__, (uint32)(arg1), (uint32)(arg2), (uint32)(arg3)))

// a convenient macro (must be invoked in the context of the thread that acquired the resource)
#define	SFsdReleaseResource(Resource)	\
	(ExReleaseResourceForThreadLite((Resource), ExGetCurrentResourceThread()))

// each file has a unique bug-check identifier associated with it.
//	Here is a list of constant definitions for these identifiers
#define	SFSD_FILE_INIT										(0x00000001)
#define	SFSD_FILE_REGISTRY								(0x00000002)
#define	SFSD_FILE_CREATE									(0x00000003)
#define	SFSD_FILE_CLEANUP									(0x00000004)
#define	SFSD_FILE_CLOSE									(0x00000005)
#define	SFSD_FILE_READ										(0x00000006)
#define	SFSD_FILE_WRITE									(0x00000007)
#define	SFSD_FILE_INFORMATION							(0x00000008)
#define	SFSD_FILE_FLUSH									(0x00000009)
#define	SFSD_FILE_VOL_INFORMATION						(0x0000000A)
#define	SFSD_FILE_DIR_CONTROL							(0x0000000B)
#define	SFSD_FILE_FILE_CONTROL							(0x0000000C)
#define	SFSD_FILE_DEVICE_CONTROL						(0x0000000D)
#define	SFSD_FILE_SHUTDOWN								(0x0000000E)
#define	SFSD_FILE_LOCK_CONTROL							(0x0000000F)
#define	SFSD_FILE_SECURITY								(0x00000010)
#define	SFSD_FILE_EXT_ATTR								(0x00000011)
#define	SFSD_FILE_MISC										(0x00000012)
#define	SFSD_FILE_FAST_IO									(0x00000013)

#if DBG
#define	SFsdBreakPoint()	DbgBreakPoint()
#else
#define	SFsdBreakPoint()
#endif

#endif	// _SFSD_SFSD_H_

