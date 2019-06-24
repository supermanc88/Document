/*************************************************************************
*
* File: flush.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "Flush Buffers" dispatch entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_FLUSH



/*************************************************************************
*
* Function: SFsdFlush()
*
* Description:
*	The I/O Manager will invoke this routine to handle a flush buffers
*	request
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL (invocation at higher IRQL will cause execution
*	to be deferred to a worker thread context)
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFsdFlush(
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

		RC = SFsdCommonFlush(PtrIrpContext, Irp);

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
* Function: SFsdCommonFlush()
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
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS	SFsdCommonFlush(
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
	BOOLEAN					AcquiredFCB = FALSE;
	BOOLEAN					PostRequest = FALSE;
	BOOLEAN					CanWait = TRUE;

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
		PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

		// Get some of the parameters supplied to us
		CanWait = ((PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_CAN_BLOCK) ? TRUE : FALSE);

		// If we cannot wait, post the request immediately since a flush is inherently blocking/synchronous.
		if (!CanWait) {
			PostRequest = TRUE;
			try_return(RC);
		}

		// Check the type of object passed-in. That will determine the course of
		// action we take.
		if ((PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) || (PtrFCB->FCBFlags & SFSD_FCB_ROOT_DIRECTORY)) {

			if (PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) {
				PtrVCB = (PtrSFsdVCB)(PtrFCB);
			} else {
				PtrVCB = PtrFCB->PtrVCB;
			}

			// The caller wishes to flush all files for the mounted
			// logical volume. The flush volume routine below should simply
			// walk through all of the open file streams, acquire the
			// FCB resource, and request the flush operation from the Cache
			// Manager. Basically, the sequence of operations listed below
			// for a single file should be executed on all open files.

			SFsdFlushLogicalVolume(PtrIrpContext, PtrIrp, PtrVCB);

			try_return(RC);
		}

		if (!(PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY)) {
			// This is a regular file.
			ExAcquireResourceExclusiveLite(&(PtrReqdFCB->MainResource), TRUE);
			AcquiredFCB = TRUE;

			// Request the Cache Manager to perform a flush operation.
			// Further, instruct the Cache Manager that we wish to flush the
			// entire file stream.
			SFsdFlushAFile(PtrReqdFCB, &(PtrIrp->IoStatus));
			RC = PtrIrp->IoStatus.Status;
			// All done. You may want to also flush the directory entry for the
			// file stream at this time.

			// Some log-based FSD implementations may wish to flush their
			// log files at this time. Finally, you should update the time-stamp
			// values for the file stream appropriately. This would involve
			// obtaining the current time and modifying the appropriate directory
			// entry fields.
		}

		try_exit:

		if (AcquiredFCB) {
			SFsdReleaseResource(&(PtrReqdFCB->MainResource));
			AcquiredFCB = FALSE;
		}

		if (!PostRequest) {
			PIO_STACK_LOCATION		PtrNextIoStackLocation = NULL;
			NTSTATUS						RC1 = STATUS_SUCCESS;

			// Send the request down at this point.
			// To do this, you must set the next IRP stack location, and
			// maybe set a completion routine.
			// Be careful about marking the IRP pending if the lower level
			// driver returned pending and you do have a completion routine!
			PtrNextIoStackLocation = IoGetNextIrpStackLocation(PtrIrp);
			*PtrNextIoStackLocation = *PtrIoStackLocation;

			// Set the completion routine to "eat-up" any
			// STATUS_INVALID_DEVICE_REQUEST error code returned by the lower
			// level driver.
			IoSetCompletionRoutine(PtrIrp, SFsdFlushCompletion, NULL, TRUE, TRUE, TRUE);

			RC1 = IoCallDriver(PtrVCB->TargetDeviceObject, PtrIrp);

			RC = ((RC1 == STATUS_INVALID_DEVICE_REQUEST) ? RC : RC1);
		}

	} finally {
		if (PostRequest) {
			// Nothing to lock now.
			RC = SFsdPostRequest(PtrIrpContext, PtrIrp);
		} else {
			// Release the IRP context at this time.
  			SFsdReleaseIrpContext(PtrIrpContext);
		}
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFsdFlushAFile()
*
* Description:
*	Tell the Cache Manager to perform a flush.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdFlushAFile(
PtrSFsdNTRequiredFCB	PtrReqdFCB,
PIO_STATUS_BLOCK		PtrIoStatus)
{
	CcFlushCache(&(PtrReqdFCB->SectionObject), NULL, 0, PtrIoStatus);
	return;
}


/*************************************************************************
*
* Function: SFsdFlushLogicalVolume()
*
* Description:
*	Flush everything beginning at root directory.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdFlushLogicalVolume(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PtrSFsdVCB					PtrVCB)
{
	BOOLEAN			AcquiredVCB = FALSE;
	PtrSFsdFCB		PtrFCB = NULL;
	PLIST_ENTRY		PtrNextFCB = NULL;

	try {
		ExAcquireResourceExclusiveLite(&(PtrVCB->VCBResource), TRUE);
		AcquiredVCB = TRUE;

		// Go through the list of FCB's. You would probably
		// flush all of the files. Then, you could flush the
		// directories that you may have have pinned into memory.

		// NOTE: This function may also be invoked internally as part of
		// processing a shutdown request.

	} finally {

		if (AcquiredVCB) {
			SFsdReleaseResource(&(PtrVCB->VCBResource));
		}
	}

	return;
}


/*************************************************************************
*
* Function: SFsdFlushCompletion()
*
* Description:
*	Eat up any bad errors.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
NTSTATUS SFsdFlushCompletion(
PDEVICE_OBJECT	PtrDeviceObject,
PIRP				PtrIrp,
PVOID				Context)
{
	NTSTATUS		RC = STATUS_SUCCESS;

	if (PtrIrp->PendingReturned) {
		IoMarkIrpPending(PtrIrp);
	}

	if (PtrIrp->IoStatus.Status == STATUS_INVALID_DEVICE_REQUEST) {
		// cannot do much here, can we?
		PtrIrp->IoStatus.Status = STATUS_SUCCESS;
	}

	return(STATUS_SUCCESS);
}


