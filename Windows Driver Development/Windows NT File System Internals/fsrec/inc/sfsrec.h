/*************************************************************************
*
* File: sfsrec.h
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	The main include file for the sample file system driver recognizer.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#ifndef	_SFSREC_SFSREC_H_
#define	_SFSREC_SFSREC_H_

// Common include files; should be in the include dir of the MS supplied IFS Kit.
#include	<ntifs.h>

typedef struct SFsRecDeviceExtension {
	BOOLEAN		DidLoadFail;
} SFsRecDeviceExtension, *PtrSFsRecDeviceExtension;

// try-finally simulation
#define try_return(S)	{ S; goto try_exit; }
#define try_return1(S)	{ S; goto try_exit1; }
#define try_return2(S)	{ S; goto try_exit2; }

// Prototypes
extern void SFsRecUnload(
PDRIVER_OBJECT		PtrFsRecDriverObject);

extern NTSTATUS SFsRecFsControl(
PDEVICE_OBJECT		DeviceObject,
PIRP					Irp);

#endif	// _SFSREC_SFSREC_H_

