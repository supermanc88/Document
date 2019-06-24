/*************************************************************************
*
* File: cleanup.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Should contain code to handle the "Cleanup" dispatch entry point.
*	This file serves as a placeholder. Please update this file as part
*	of designing and implementing your FSD.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_CLEANUP



/*************************************************************************
*
* Function: SFsdCleanup()
*
* Description:
*	The I/O Manager will invoke this routine to handle a cleanup
*	request
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL (invocation at higher IRQL will cause execution
*	to be deferred to a worker thread context)
*
* Return Value: Does not matter!
*
*************************************************************************/
NTSTATUS SFsdCleanup(
PDEVICE_OBJECT		DeviceObject,		// the logical volume device object
PIRP					Irp)					// I/O Request Packet
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;
	BOOLEAN				AreWeTopLevel = FALSE;

	FsRtlEnterFileSystem();
	ASSERT(DeviceObject);
	ASSERT(Irp);

	// set the top level context
	AreWeTopLevel = SFsdIsIrpTopLevel(Irp);

	try {

		// get an IRP context structure and issue the request
		PtrIrpContext = SFsdAllocateIrpContext(Irp, DeviceObject);
		ASSERT(PtrIrpContext);

		RC = SFsdCommonCleanup(PtrIrpContext, Irp);

	} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {

		RC = SFsdExceptionHandler(PtrIrpContext, Irp);

		SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);
	}

	if (AreWeTopLevel) {
		IoSetTopLevelIrp(NULL);
	}

	FsRtlExitFileSystem();

	return(RC);
}



/*************************************************************************
*
* Function: SFsdCommonCleanup()
*
* Description:
*	The actual work is performed here. This routine may be invoked in one'
*	of the two possible contexts:
*	(a) in the context of a system worker thread
*	(b) in the context of the original caller
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: Does not matter!
*
*************************************************************************/
NTSTATUS	SFsdCommonCleanup(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION	PtrIoStackLocation = NULL;
	PFILE_OBJECT			PtrFileObject = NULL;
	PtrSFsdFCB				PtrFCB = NULL;
	PtrSFsdCCB				PtrCCB = NULL;
	PtrSFsdVCB				PtrVCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;
	PERESOURCE				PtrResourceAcquired = NULL;
	IO_STATUS_BLOCK		LocalIoStatus;

	BOOLEAN					CompleteIrp = TRUE;
	BOOLEAN					PostRequest = FALSE;

	BOOLEAN					CanWait = FALSE;

	try {
		// First, get a pointer to the current I/O stack location
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(PtrIrp);
		ASSERT(PtrIoStackLocation);

		PtrFileObject = PtrIoStackLocation->FileObject;
		ASSERT(PtrFileObject);

		// Get the FCB and CCB pointers
		PtrCCB = (PtrSFsdCCB)(PtrFileObject->FsContext2);
		ASSERT(PtrCCB);
		PtrFCB = PtrCCB->PtrFCB;
		ASSERT(PtrFCB);

		PtrVCB =	(PtrSFsdVCB)(PtrIrpContext->TargetDeviceObject->DeviceExtension);
		ASSERT(PtrVCB);
		ASSERT(PtrVCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB);

		// Steps you will probably take at this point are:
		// (a) Acquire the VCB exclusively
		// (b) Acquire the file (FCB) exclusively
		// (c) Flush file data to disk
		// (d) Talk to the FSRTL package (if you use it) about pending oplocks.
		// (e) Notify the FSRTL package (if you use it) for use with pending
		//		 notification IRPs
		// (f) Unlock byte-range locks (if any were acquired by process)
		// (g) Update time stamp values (e.g. fast-IO had been performed)
		// (h) Inform the Cache Manager to uninitialize Cache Maps ...
		// and other similar stuff. Chapter 10 in the book describes in further
		// detail the list of functionality expected from a typical cleanup routine.

		try_exit:	NOTHING;

	} finally {

		// See the read/write examples for how to fill in this portion

	} // end of "finally" processing

	return(RC);
}

