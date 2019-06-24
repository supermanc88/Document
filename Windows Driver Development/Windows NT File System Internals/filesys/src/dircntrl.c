/*************************************************************************
*
* File: dircntrl.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "directory control" dispatch entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_DIR_CONTROL



/*************************************************************************
*
* Function: SFsdDirControl()
*
* Description:
*	The I/O Manager will invoke this routine to handle a directory control
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
NTSTATUS SFsdDirControl(
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

		RC = SFsdCommonDirControl(PtrIrpContext, Irp);

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
* Function: SFsdCommonDirControl()
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
NTSTATUS	SFsdCommonDirControl(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION	PtrIoStackLocation = NULL;
	PFILE_OBJECT			PtrFileObject = NULL;
	PtrSFsdFCB				PtrFCB = NULL;
	PtrSFsdCCB				PtrCCB = NULL;
	PtrSFsdVCB				PtrVCB = NULL;

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

	// Get some of the parameters supplied to us
	switch (PtrIoStackLocation->MinorFunction) {
	case IRP_MN_QUERY_DIRECTORY:
		RC = SFsdQueryDirectory(PtrIrpContext, PtrIrp, PtrIoStackLocation, PtrFileObject, PtrFCB, PtrCCB);
		break;
	case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
		RC = SFsdNotifyChangeDirectory(PtrIrpContext, PtrIrp, PtrIoStackLocation, PtrFileObject, PtrFCB, PtrCCB);
		break;
	default:
		// This should not happen.
		RC = STATUS_INVALID_DEVICE_REQUEST;
		PtrIrp->IoStatus.Status = RC;
		PtrIrp->IoStatus.Information = 0;

		// Free up the Irp Context
		SFsdReleaseIrpContext(PtrIrpContext);

		// complete the IRP
		IoCompleteRequest(PtrIrp, IO_NO_INCREMENT);
		break;
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFsdQueryDirectory()
*
* Description:
*	Query directory request.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS	SFsdQueryDirectory(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PIO_STACK_LOCATION		PtrIoStackLocation,
PFILE_OBJECT				PtrFileObject,
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	BOOLEAN					CompleteRequest = TRUE;
	BOOLEAN					PostRequest = FALSE;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;
	BOOLEAN					CanWait = FALSE;
	PtrSFsdVCB				PtrVCB = NULL;
	BOOLEAN					AcquiredFCB = FALSE;
	unsigned long			BufferLength = 0;
	PSTRING					PtrSearchPattern = NULL;
	FILE_INFORMATION_CLASS FileInformationClass;
	unsigned long			FileIndex = 0;
	BOOLEAN					RestartScan = FALSE;
	BOOLEAN					ReturnSingleEntry = FALSE;
	BOOLEAN					IndexSpecified = FALSE;
	unsigned char			*Buffer = NULL;
	BOOLEAN					FirstTimeQuery = FALSE;
	unsigned long			StartingIndexForSearch = 0;
	unsigned int			BytesReturned = 0;

	try {

		// Validate the sent-in FCB
		if ((PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) || !(PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY)) {
			// We will only allow notify requests on directories.
			RC = STATUS_INVALID_PARAMETER;
		}

		PtrReqdFCB = &(PtrFCB->NTRequiredFCB);
		CanWait = ((PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_CAN_BLOCK) ? TRUE : FALSE);
      PtrVCB = PtrFCB->PtrVCB;

		// If the caller does not wish to block, it would be easier to
		// simply post the request now.
		if (!CanWait) {
			PostRequest = TRUE;
			try_return(RC = STATUS_PENDING);
		}

		// Obtain the callers parameters
		BufferLength = PtrIoStackLocation->Parameters.QueryDirectory.Length;
		PtrSearchPattern = PtrIoStackLocation->Parameters.QueryDirectory.FileName;
		FileInformationClass = PtrIoStackLocation->Parameters.QueryDirectory.FileInformationClass;
		FileIndex = PtrIoStackLocation->Parameters.QueryDirectory.FileIndex;

		// Some additional arguments that affect the FSD behavior
		RestartScan       = (PtrIoStackLocation->Flags & SL_RESTART_SCAN);
		ReturnSingleEntry = (PtrIoStackLocation->Flags & SL_RETURN_SINGLE_ENTRY);
		IndexSpecified    = (PtrIoStackLocation->Flags & SL_INDEX_SPECIFIED);

		// I will acquire exclusive access to the FCB.
		// This is not mandatory, however, and your FSD could choose to acquire
		// the resource shared for increased parallelism.
		ExAcquireResourceExclusiveLite(&(PtrReqdFCB->MainResource), TRUE);
		AcquiredFCB = TRUE;

		// We must determine the buffer pointer to be used. Since this
		// routine could either be invoked directly in the context of the
		// calling thread, or in the context of a worker thread, here is
		// a general way of determining what we should use.
		if (PtrIrp->MdlAddress) {
			Buffer = MmGetSystemAddressForMdl(PtrIrp->MdlAddress);
		} else {
			Buffer = PtrIrp->UserBuffer;
		}

		// The method of determining where to look from and what to look for is
		// unfortunately extremely confusing. However, here is a methodology you
		// you can broadly adopt:
		// (a) You have to maintain a search buffer per CCB structure.
		// (b) This search buffer is initialized the very first time
		//		 a query directory operation is performed using the file object.
		// (For the sample FSD, the search buffer is stored in the
		//	 DirectorySearchPattern field)
		// However, the caller still has the option of "overriding" this stored
		// search pattern by supplying a new one in a query directory operation.
		//
		if (PtrSearchPattern == NULL) {
			// User has supplied a search pattern
			// Now validate that the search pattern is legitimate; this is
			// dependent upon the character set acceptable to your FSD.

			// Once you have validated the search pattern, you must
			// check whether you need to store this search pattern in
			// the CCB.
			if (PtrCCB->DirectorySearchPattern == NULL) {
				// This must be the very first query request.
            FirstTimeQuery = TRUE;

				// Now, allocate enough memory to contain the caller
				// supplied search pattern and fill in the DirectorySearchPattern
				// field in the CCB
				// PtrCCB->DirectorySearchPattern = ExAllocatePool(...);
			} else {
				// We should ignore the search pattern in the CCB and instead,
				// use the user-supplied pattern for this particular query
				// directory request.
			}

		} else if (PtrCCB->DirectorySearchPattern == NULL) {
			// This MUST be the first directory query operation (else the
			// DirectorySearchPattern field would never be NULL. Also, the caller
			// has neglected to provide a pattern so we MUST invent one.
			// Use "*" (following NT conventions) as your search pattern
			// and store it in the PtrCCB->DirectorySearchPattern field.

			PtrCCB->DirectorySearchPattern = ExAllocatePool(PagedPool, sizeof(L"*"));
			ASSERT(PtrCCB->DirectorySearchPattern);

			FirstTimeQuery = TRUE;
		} else {
			// The caller has not supplied any search pattern that we are
			// forced to use. However, the caller had previously supplied
			// a pattern (or we must have invented one) and we will use it.
			// This is definitely not the first query operation on this
			// directory using this particular file object.

			PtrSearchPattern = PtrCCB->DirectorySearchPattern;
		}

		// There is one other piece of information that your FSD must store
		// in the CCB structure for query directory support. This is the index
		// value (i.e. the offset in your on-disk directory structure) from
		// which you should start searching.
		// However, the flags supplied with the IRP can make us override this
		// as well.

		if (FileIndex) {
			// Caller has told us wherefrom to begin.
			// You may need to round this to an appropriate directory entry
			// entry alignment value.
         StartingIndexForSearch = FileIndex;
		} else if (RestartScan) {
         StartingIndexForSearch = 0;
		} else {
			// Get the starting offset from the CCB.
			// Remember to update this value on your way out from this function.
			// But, do not update the CCB CurrentByteOffset field if you reach
			// the end of the directory (or get an error reading the directory)
			// while performing the search.
         StartingIndexForSearch = PtrCCB->CurrentByteOffset.LowPart;
		}

		// Now, your FSD must determine the best way to read the directory
		// contents from disk and search through them.

		// If ReturnSingleEntry is TRUE, please return information on only
		// one matching entry.

		// One final note though:
		// If you do not find a directory entry OR while searching you reach the
		// end of the directory, then the return code should be set as follows:

		// (a) If any files have been returned (i.e. ReturnSingleEntry was FALSE
		//		 and you did find at least one match), then return STATUS_SUCCESS
		//	(b) If no entry is being returned then:
		//		 (i) If this is the first query i.e. FirstTimeQuery is TRUE
		//			  then return STATUS_NO_SUCH_FILE
		//		 (ii) Otherwise, return STATUS_NO_MORE_FILES

		try_exit:	NOTHING;

		// Remember to update the CurrentByteOffset field in the CCB if required.

		// You should also set a flag in the FCB indicating that the directory
		// contents were accessed.

	} finally {
		if (PostRequest) {
			if (AcquiredFCB) {
				SFsdReleaseResource(&(PtrReqdFCB->MainResource));
			}

			// Map the users buffer and then post the request.
			RC = SFsdLockCallersBuffer(PtrIrp, TRUE, BufferLength);
			ASSERT(NT_SUCCESS(RC));

			RC = SFsdPostRequest(PtrIrpContext, PtrIrp);

		} else if (!(PtrIrpContext->IrpContextFlags &
							SFSD_IRP_CONTEXT_EXCEPTION)) {
			if (AcquiredFCB) {
				SFsdReleaseResource(&(PtrReqdFCB->MainResource));
			}

			// Complete the request.
			PtrIrp->IoStatus.Status = RC;
			PtrIrp->IoStatus.Information = BytesReturned;

			// Free up the Irp Context
			SFsdReleaseIrpContext(PtrIrpContext);

			// complete the IRP
			IoCompleteRequest(PtrIrp, IO_DISK_INCREMENT);
		}
	}

	return(RC);
}



/*************************************************************************
*
* Function: SFsdNotifyChangeDirectory()
*
* Description:
*	Handle the notify request.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS	SFsdNotifyChangeDirectory(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PIO_STACK_LOCATION		PtrIoStackLocation,
PFILE_OBJECT				PtrFileObject,
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	BOOLEAN					CompleteRequest = FALSE;
	BOOLEAN					PostRequest = FALSE;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;
	BOOLEAN					CanWait = FALSE;
	ULONG						CompletionFilter = 0;
	BOOLEAN					WatchTree = FALSE;
	PtrSFsdVCB				PtrVCB = NULL;
	BOOLEAN					AcquiredFCB = FALSE;

	try {

		// Validate the sent-in FCB
		if ((PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) || !(PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY)) {
			// We will only allow notify requests on directories.
			RC = STATUS_INVALID_PARAMETER;
			CompleteRequest = TRUE;
		}

		PtrReqdFCB = &(PtrFCB->NTRequiredFCB);
		CanWait = ((PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_CAN_BLOCK) ? TRUE : FALSE);
      PtrVCB = PtrFCB->PtrVCB;

		// Acquire the FCB resource shared
		if (!ExAcquireResourceSharedLite(&(PtrReqdFCB->MainResource), CanWait)) {
			PostRequest = TRUE;
			try_return(RC = STATUS_PENDING);
		}
      AcquiredFCB = TRUE;

		// Obtain some parameters sent by the caller
		CompletionFilter = PtrIoStackLocation->Parameters.NotifyDirectory.CompletionFilter;
		WatchTree = (PtrIoStackLocation->Flags & SL_WATCH_TREE ? TRUE : FALSE);

		// If you wish to capture the subject context, you can do so as
		// follows:
		// {
		//		PSECURITY_SUBJECT_CONTEXT SubjectContext;
 		// 	SubjectContext = ExAllocatePool(PagedPool,
		//									sizeof(SECURITY_SUBJECT_CONTEXT));
		//		SeCaptureSubjectContext(SubjectContext);
		//	}

		FsRtlNotifyFullChangeDirectory(&(PtrVCB->NotifyIRPMutex), &(PtrVCB->NextNotifyIRP), (void *)PtrCCB,
							(PSTRING)(PtrFCB->FCBName->ObjectName.Buffer), WatchTree, FALSE, CompletionFilter, PtrIrp,
							NULL,		// SFsdTraverseAccessCheck(...) ?
							NULL);	// SubjectContext ?

		RC = STATUS_PENDING;

		try_exit:	NOTHING;

	} finally {

		if (PostRequest) {
			// Perform appropriate post related processing here
			if (AcquiredFCB) {
				SFsdReleaseResource(&(PtrReqdFCB->MainResource));
				AcquiredFCB = FALSE;
			}
			RC = SFsdPostRequest(PtrIrpContext, PtrIrp);
		} else if (CompleteRequest) {
			PtrIrp->IoStatus.Status = RC;
			PtrIrp->IoStatus.Information = 0;

			// Free up the Irp Context
			SFsdReleaseIrpContext(PtrIrpContext);

			// complete the IRP
			IoCompleteRequest(PtrIrp, IO_DISK_INCREMENT);
		} else {
			// Simply free up the IrpContext since the IRP has been queued
			SFsdReleaseIrpContext(PtrIrpContext);
		}

		// Release the FCB resources if acquired.
		if (AcquiredFCB) {
			SFsdReleaseResource(&(PtrReqdFCB->MainResource));
			AcquiredFCB = FALSE;
		}

	}

	return(RC);
}

