/*************************************************************************
*
* File: sfilter.h
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*	The main include file for the sample filter driver. This file contains
*	a mish-mash of things.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#ifndef	_SFILTER_SFILTER_H_
#define	_SFILTER_SFILTER_H_

// some constant definitions
#define		SFILTER_PANIC_IDENTIFIER		(0x86427532)

#define		SFILTER_DRV_DIR			L"\\Device\\SFilter"
#define		SFILTER_DOS_DRV_DIR		L"\\DosDevices\\SFilter"
#define		SFILTER_DRV_NAME			L"\\Device\\SFilter\\SampleFilterDrv"
#define		SFILTER_DOS_DRV_NAME		L"\\DosDevices\\SFilter\\SampleFilterDrv"


// Common include files - should be in the include dir of the MS supplied FSDDK
#include	<ntifs.h>

// the following include files should be in the inc sub-dir associated with this driver
#include	"struct.h"
#include	"protos.h"
// #include "errmsg.h"

// global variables - minimize these
extern SFilterData				SFilterGlobalData;


/**********************************************************************************
*
* The following macros make life a little easier
*
**********************************************************************************/

// try-finally simulation
#define try_return(S)	{ S; goto try_exit; }
#define try_return1(S)	{ S; goto try_exit1; }
#define try_return2(S)	{ S; goto try_exit2; }

// Flag (bit-field) manipulation
#define	SFilterSetFlag(Flag, Value)	((Flag) |= (Value))
#define	SFilterClearFlag(Flag, Value)	((Flag) &= ~(Value))

// Align a value along a 4-byte boundary.
#define	SFilterQuadAlign(Value)			((((uint32)(Value)) + 7) & 0xfffffff8)

// to perform a bug-check (panic), the following macro is used
// it allows us to print out the file identifier, line #, and upto 3
// additional DWORD arguments.
#define	SFilterPanic(arg1, arg2, arg3)					\
	(KeBugCheckEx(SFILTER_PANIC_IDENTIFIER, SFILTER_BUG_CHECK_ID | __LINE__, (uint32)(arg1), (uint32)(arg2), (uint32)(arg3)))

// Release a resource in the context of the thread that originally acquired the
// resource.
#define	SFilterReleaseResource(Resource)	(ExReleaseResourceForThreadLite((Resource), ExGetCurrentResourceThread()))

// the following check asserts that the passed-in device extension
// pointer is valid
#define	SFilterAssertExtPtrValid(PtrExtension)			{													\
		ASSERT((PtrExtension));																						\
		ASSERT((((PtrExtension)->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_FILTER_DEVICE) ||	\
				  ((PtrExtension)->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE)) &&\
				 ((PtrExtension)->NodeIdentifier.NodeSize == sizeof(SFilterDeviceExtension)));	\
}

// the following macro allows us to increment a large integer value atomically.
// we expect an unsigned long to be supplied as the increment value.
// a spin lock should be passed-in to synchronize operations
#define	SFilterIncrementLargeInteger(LargeIntegerOp, ULongIncrement, PtrSpinLock)	{			\
	KIRQL				OldIrql;																							\
	KeAcquireSpinLock(PtrSpinLock, &OldIrql);																	\
	RtlLargeIntegerAdd((LargeIntegerOp),(RtlConvertUlongToLargeInteger((ULongIncrement))));	\
	KeReleaseSpinLock(PtrSpinLock, OldIrql);																	\
}

// the following macro allows us to decrement a large integer value atomically.
// we expect an unsigned long to be supplied as the decrement value.
// a spin lock should be passed-in to synchronize operations
#define	SFilterDecrementLargeInteger(LargeIntegerOp, ULongIncrement, PtrSpinLock)	{			\
	KIRQL				OldIrql;																							\
	KeAcquireSpinLock(PtrSpinLock, &OldIrql);																	\
	RtlLargeIntegerSubtract((LargeIntegerOp),(RtlConvertUlongToLargeInteger((ULongIncrement))));	\
	KeReleaseSpinLock(PtrSpinLock, OldIrql);																	\
}

// the following macro allows us to check if the large integer value is zero,
// atomically. Note that I have added (for convenience) a check to ensure that
// the value is non-negative.
#define	SFilterIsLargeIntegerZero(ReturnValue, LargeIntegerOp, PtrSpinLock)	{					\
	KIRQL				OldIrql;																							\
	KeAcquireSpinLock(PtrSpinLock, &OldIrql);																	\
	ASSERT(RtlLargeIntegerGreaterOrEqualToZero((LargeIntegerOp)));										\
	ReturnValue = RtlLargeIntegerEqualToZero((LargeIntegerOp));											\
	KeReleaseSpinLock(PtrSpinLock, OldIrql);																	\
}

/**********************************************************************************
*
* End of macro definitions
*
**********************************************************************************/

// each file has a unique bug-check identifier associated with it.
//	Here is a list of constant definitions for these identifiers
#define	SFILTER_FILE_INIT										(0x00000001)
#define	SFILTER_FILE_ATTACH									(0x00000002)
#define	SFILTER_FILE_FAST_IO									(0x00000003)
#define	SFILTER_FILE_MISC										(0x00000004)
#define	SFILTER_FILE_DISPATCH								(0x00000005)
#define	SFILTER_FILE_CREATE									(0x00000006)
#define	SFILTER_FILE_CLOSE									(0x00000007)
#define	SFILTER_FILE_FSCTRL									(0x00000008)

#if DBG
#define	SFilterBreakPoint()	DbgBreakPoint()
#else
#define	SFilterBreakPoint()
#endif

#endif	// _SFILTER_SFILTER_H_

