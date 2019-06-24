/*************************************************************************
*
* File: devcntrl.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "Device IOCTL" dispatch entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_DEVICE_CONTROL

#if(_WIN32_WINNT < 0x0400)
#define IOCTL_REDIR_QUERY_PATH   CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM, 99, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct _QUERY_PATH_REQUEST {
    ULONG PathNameLength;
    PIO_SECURITY_CONTEXT SecurityContext;
    WCHAR FilePathName[1];
} QUERY_PATH_REQUEST, *PQUERY_PATH_REQUEST;

typedef struct _QUERY_PATH_RESPONSE {
    ULONG LengthAccepted;
} QUERY_PATH_RESPONSE, *PQUERY_PATH_RESPONSE;
#endif


/*************************************************************************
*
* Function: SFsdDeviceControl()
*
* Description:
*	The I/O Manager will invoke this routine to handle a Device IOCTL
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
NTSTATUS SFsdDeviceControl(
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

		RC = SFsdCommonDeviceControl(PtrIrpContext, Irp);

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
* Function: SFsdCommonDeviceControl()
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
NTSTATUS	SFsdCommonDeviceControl(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION	PtrIoStackLocation = NULL;
	PIO_STACK_LOCATION	PtrNextIoStackLocation = NULL;
	PFILE_OBJECT			PtrFileObject = NULL;
	PtrSFsdFCB				PtrFCB = NULL;
	PtrSFsdCCB				PtrCCB = NULL;
	PtrSFsdVCB				PtrVCB = NULL;
	BOOLEAN					CompleteIrp = FALSE;
	ULONG						IoControlCode = 0;
	void						*BufferPointer = NULL;

	try {
		// First, get a pointer to the current I/O stack location
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(PtrIrp);
		ASSERT(PtrIoStackLocation);

		PtrFileObject = PtrIoStackLocation->FileObject;
		ASSERT(PtrFileObject);

		PtrCCB = (PtrSFsdCCB)(PtrFileObject->FsContext2);
		ASSERT(PtrCCB);
		PtrFCB = PtrCCB->PtrFCB;
		ASSERT(PtrFCB);

		if (PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) {
			PtrVCB = (PtrSFsdVCB)(PtrFCB);
		} else {
			PtrVCB = PtrFCB->PtrVCB;
		}

		// Get the IoControlCode value
		IoControlCode = PtrIoStackLocation->Parameters.DeviceIoControl.IoControlCode;

		// You may wish to allow only	volume open operations.

		switch (IoControlCode) {
#ifdef	__THIS_IS_A_NETWORK_REDIR_
		case IOCTL_REDIR_QUERY_PATH:
			// Only for network redirectors.
			BufferPointer = (void *)(PtrIoStackLocation->Parameters.DeviceIoControl.Type3InputBuffer);
			// Invoke the handler for this IOCTL.
			RC = SFsdHandleQueryPath(BufferPointer);
			CompleteIrp = TRUE;
			try_return(RC);
			break;
#endif	// _THIS_IS_A_NETWORK_REDIR_
		default:
			// Invoke the lower level driver in the chain.
			PtrNextIoStackLocation = IoGetNextIrpStackLocation(PtrIrp);
			*PtrNextIoStackLocation = *PtrIoStackLocation;
			// Set a completion routine.
			IoSetCompletionRoutine(PtrIrp, SFsdDevIoctlCompletion, NULL, TRUE, TRUE, TRUE);
			// Send the request.
			RC = IoCallDriver(PtrVCB->TargetDeviceObject, PtrIrp);
			break;
		}

		try_exit:	NOTHING;

	} finally {

		// Release the IRP context
		if (!(PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_EXCEPTION)) {
			// Free up the Irp Context
			SFsdReleaseIrpContext(PtrIrpContext);

			if (CompleteIrp) {
				PtrIrp->IoStatus.Status = RC;
				PtrIrp->IoStatus.Information = 0;
	
				// complete the IRP
				IoCompleteRequest(PtrIrp, IO_DISK_INCREMENT);
			}
		}
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFsdDevIoctlCompletion()
*
* Description:
*	Completion routine.
*	
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS
*
*************************************************************************/
NTSTATUS SFsdDevIoctlCompletion(
PDEVICE_OBJECT			PtrDeviceObject,
PIRP						PtrIrp,
void						*Context)
{
	if (PtrIrp->PendingReturned) {
		IoMarkIrpPending(PtrIrp);
	}

	return(STATUS_SUCCESS);
}


/*************************************************************************
*
* Function: SFsdHandleQueryPath()
*
* Description:
*	Handle the MUP request.
*	
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS
*
*************************************************************************/
NTSTATUS SFsdHandleQueryPath(
void			*BufferPointer)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PQUERY_PATH_REQUEST	RequestBuffer = (PQUERY_PATH_REQUEST)BufferPointer;
	PQUERY_PATH_RESPONSE	ReplyBuffer = (PQUERY_PATH_RESPONSE)BufferPointer;
	ULONG						LengthOfNameToBeMatched = RequestBuffer->PathNameLength;
	ULONG						LengthOfMatchedName = 0;
   WCHAR                *NameToBeMatched = RequestBuffer->FilePathName;

	// So here we are. Simply check the name supplied.
	// You can use whatever algorithm you like to determine whether the
	// sent in name is acceptable.
	// The first character in the name is always a "\"
	// If you like the name sent in (probably, you will like a subset
	// of the name), set the matching length value in LengthOfMatchedName.

	// if (FoundMatch) {
	//		ReplyBuffer->LengthAccepted = LengthOfMatchedName;
	// } else {
	//		RC = STATUS_OBJECT_NAME_NOT_FOUND;
	// }

	return(RC);
}

