/*************************************************************************
*
* File: read.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "Read" dispatch entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_READ



/*************************************************************************
*
* Function: SFsdRead()
*
* Description:
*	The I/O Manager will invoke this routine to handle a read
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
NTSTATUS SFsdRead(
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

		RC = SFsdCommonRead(PtrIrpContext, Irp);

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
* Function: SFsdCommonRead()
*
* Description:
*	The actual work is performed here. This routine may be invoked in one
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
NTSTATUS	SFsdCommonRead(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION	PtrIoStackLocation = NULL;
	LARGE_INTEGER			ByteOffset;
	uint32					ReadLength = 0, TruncatedReadLength = 0;
	uint32					NumberBytesRead = 0;
	PFILE_OBJECT			PtrFileObject = NULL;
	PtrSFsdFCB				PtrFCB = NULL;
	PtrSFsdCCB				PtrCCB = NULL;
	PtrSFsdVCB				PtrVCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;
	PERESOURCE				PtrResourceAcquired = NULL;
	IO_STATUS_BLOCK		LocalIoStatus;
	void						*PtrSystemBuffer = NULL;
	uint32					KeyValue = 0;

	BOOLEAN					CompleteIrp = TRUE;
	BOOLEAN					PostRequest = FALSE;

	BOOLEAN					CanWait = FALSE;
	BOOLEAN					PagingIo = FALSE;
	BOOLEAN					NonBufferedIo = FALSE;
	BOOLEAN					SynchronousIo = FALSE;

	try {
		// First, get a pointer to the current I/O stack location
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(PtrIrp);
		ASSERT(PtrIoStackLocation);

  		// If this happens to be a MDL read complete request, then
		// there is not much processing that the FSD has to do.
		if (PtrIoStackLocation->MinorFunction & IRP_MN_COMPLETE) {
			// Caller wants to tell the Cache Manager that a previously
			// allocated MDL can be freed.
			SFsdMdlComplete(PtrIrpContext, PtrIrp, PtrIoStackLocation, TRUE);
			// The IRP has been completed.
			CompleteIrp = FALSE;
			try_return(RC = STATUS_SUCCESS);
		}

		// If this is a request at IRQL DISPATCH_LEVEL, then post
		// the request (your FSD may choose to process it synchronously
		// if you implement the support correctly; obviously you will be
		// quite constrained in what you can do at such IRQL).
		if (PtrIoStackLocation->MinorFunction & IRP_MN_DPC) {
			CompleteIrp = FALSE;
			PostRequest = TRUE;
			try_return(RC = STATUS_PENDING);
		}

		PtrFileObject = PtrIoStackLocation->FileObject;
		ASSERT(PtrFileObject);

		// Get the FCB and CCB pointers
		PtrCCB = (PtrSFsdCCB)(PtrFileObject->FsContext2);
		ASSERT(PtrCCB);
		PtrFCB = PtrCCB->PtrFCB;
		ASSERT(PtrFCB);

		// Get some of the parameters supplied to us
		ByteOffset = PtrIoStackLocation->Parameters.Read.ByteOffset;
		ReadLength = PtrIoStackLocation->Parameters.Read.Length;

		CanWait = ((PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_CAN_BLOCK) ? TRUE : FALSE);
		PagingIo = ((PtrIrp->Flags & IRP_PAGING_IO) ? TRUE : FALSE);
		NonBufferedIo = ((PtrIrp->Flags & IRP_NOCACHE) ? TRUE : FALSE);
		SynchronousIo = ((PtrFileObject->Flags & FO_SYNCHRONOUS_IO) ? TRUE : FALSE);

		if (ReadLength == 0) {
			// a 0 byte read can be immediately succeeded
			try_return(RC);
		}

		// NOTE: if your FSD does not support file sizes > 2 GB, you
		//	could validate the start offset here and return end-of-file
		//	if the offset begins beyond the maximum supported length.

		// Is this a read of the volume itself ?
		if (PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) {
			// Yup, we need to send this on to the disk driver after
			//	validation of the offset and length.
			PtrVCB = (PtrSFsdVCB)(PtrFCB);

			// Acquire the volume resource shared ...
			if (!ExAcquireResourceSharedLite(&(PtrVCB->VCBResource), CanWait)) {
				// post the request to be processed in the context of a worker thread
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				try_return(RC = STATUS_PENDING);
			}
			PtrResourceAcquired = &(PtrVCB->VCBResource);

			// Insert code to validate the caller supplied offset here

			// Lock the callers buffer
			if (!NT_SUCCESS(RC = SFsdLockCallersBuffer(PtrIrp, TRUE, ReadLength))) {
				try_return(RC);
			}

			// Forward the request to the lower level driver
			// For synchronous I/O wait here, else return STATUS_PENDING
			// For asynchronous I/O support, read the discussion in Chapter 10

			try_return(RC);
		}

		// If the read request is directed to a page file (if your FSD
		// supports paging files), send the request directly to the disk
		// driver. For requests directed to a page file, you have to trust
		// that the offsets will be set correctly by the VMM. You should not
		// attempt to acquire any FSD resources either.
		if (PtrFCB->FCBFlags & SFSD_FCB_PAGE_FILE) {
			IoMarkIrpPending(PtrIrp);
			// You will need to set a completion routine before invoking
			// a lower level driver
			//	forward request directly to disk driver
			// SFsdPageFileIo(PtrIrpContext, PtrIrp);

			CompleteIrp = FALSE;

			try_return(RC = STATUS_PENDING);
		}


		// If this read is directed to a directory, it is not allowed
		//	by the sample FSD. Note that you may choose to create a stream
		// file for FSD (internal) directory read/write operations in which
		// case you should modify the check below to allow reading (directly
		// from disk) directories as long as the read originated from within your FSD
		if (PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY) {
			RC = STATUS_INVALID_DEVICE_REQUEST;
			try_return(RC);
		}

		PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

		// This is a good place for oplock related processing.
		//	Chapter 11 expands upon this topic in greater detail.

		// Check whether the desired read can be allowed depending
		//	on any byte range locks that might exist. Note that for
		//	paging-io, no such checks should be performed.
		if (!PagingIo) {
			// Insert code to perform the check here ...
			//	if (!SFsdCheckForByteLock(PtrFCB, PtrCCB, PtrIrp, PtrCurrentIoStackLocation)) {
			//		try_return(RC = STATUS_FILE_LOCK_CONFLICT);
			// }
		}

		// There are certain complications that arise when the same file stream
		// has been opened for cached and non-cached access. The FSD is then
		// responsible for maintaining a consistent view of the data seen by
		// the caller.
		// Also, it is possible for file streams to be mapped in both as data files
		// and as an executable. This could also lead to consistency problems since
		// there now exist two separate sections (and pages) containing file
		// information.
		// Read Chapter 10 for more information on the issues involved in
		// maintaining data consistency.
		// The test below flushes the data cached in system memory if the current
		// request madates non-cached access (file stream must be cached) and
		// (a) the current request is not paging-io which indicates it is not
		//		 a recursive I/O operation OR originating in the Cache Manager
		// (b) OR the current request is paging-io BUT it did not originate via
		//		 the Cache Manager (or is a recursive I/O operation) and we do
		//		 have an image section that has been initialized.
#define	SFSD_REQ_NOT_VIA_CACHE_MGR(ptr)	(!MmIsRecursiveIoFault() && ((ptr)->ImageSectionObject != NULL))

		if (NonBufferedIo && (PtrReqdFCB->SectionObject.DataSectionObject != NULL)) {
			if	(!PagingIo || (SFSD_REQ_NOT_VIA_CACHE_MGR(&(PtrReqdFCB->SectionObject)))) {
            CcFlushCache(&(PtrReqdFCB->SectionObject), &ByteOffset, ReadLength, &(PtrIrp->IoStatus));
				// If the flush failed, return error to the caller
				if (!NT_SUCCESS(RC = PtrIrp->IoStatus.Status)) {
					try_return(RC);
				}
			}
		}

		// Acquire the appropriate FCB resource shared
		if (PagingIo) {
			// Try to acquire the FCB PagingIoResource shared
			if (!ExAcquireResourceSharedLite(&(PtrReqdFCB->PagingIoResource), CanWait)) {
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				try_return(RC = STATUS_PENDING);
			}
			// Remember the resource that was acquired
         PtrResourceAcquired = &(PtrReqdFCB->PagingIoResource);
		} else {
			// Try to acquire the FCB MainResource shared
			if (!ExAcquireResourceSharedLite(&(PtrReqdFCB->MainResource), CanWait)) {
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				try_return(RC = STATUS_PENDING);
			}
			// Remember the resource that was acquired
         PtrResourceAcquired = &(PtrReqdFCB->MainResource);
		}

		// Validate start offset and length supplied.
		//	If start offset is > end-of-file, return an appropriate error. Note
		// that since a FCB resource has already been acquired, and since all
		// file size changes require acquisition of both FCB resources (see
		// Chapter 10), the contents of the FCB and associated data structures
		// can safely be examined.

		//	Also note that I am using the file size in the "Common FCB Header"
		// to perform the check. However, your FSD might decide to keep a
		// separate copy in the FCB (or some other representation of the
		//	file associated with the FCB).
		if (RtlLargeIntegerGreaterThan(ByteOffset, PtrReqdFCB->CommonFCBHeader.FileSize)) {
			// Starting offset is > file size
			try_return(RC = STATUS_END_OF_FILE);
		}

		// We can also go ahead and truncate the read length here
		//	such that it is contained within the file size

		// This is also a good place to set whether fast-io can be performed
		// on this particular file or not. Your FSD must make it's own
		// determination on whether or not to allow fast-io operations.
		// Commonly, fast-io is not allowed if any byte range locks exist
		// on the file or if oplocks prevent fast-io. Practically any reason
		// choosen by your FSD could result in your setting FastIoIsNotPossible
		// OR FastIoIsQuestionable instead of FastIoIsPossible.
		//
		// PtrReqdFCB->CommonFCBHeader.IsFastIoPossible = FastIoIsPossible;

		
		//	Branch here for cached vs non-cached I/O
		if (!NonBufferedIo) {

			// The caller wishes to perform cached I/O. Initiate caching if
			// this is the first cached I/O operation using this file object
			if (PtrFileObject->PrivateCacheMap == NULL) {
				// This is the first cached I/O operation. You must ensure
				// that the FCB Common FCB Header contains valid sizes at this time
				CcInitializeCacheMap(PtrFileObject, (PCC_FILE_SIZES)(&(PtrReqdFCB->CommonFCBHeader.AllocationSize)),
					FALSE,		// We will not utilize pin access for this file
					&(SFsdGlobalData.CacheMgrCallBacks), // callbacks
					PtrCCB);		// The context used in callbacks
			}

			// Check and see if this request requires a MDL returned to the caller
			if (PtrIoStackLocation->MinorFunction & IRP_MN_MDL) {
				// Caller does want a MDL returned. Note that this mode
				// implies that the caller is prepared to block
				CcMdlRead(PtrFileObject, &ByteOffset, TruncatedReadLength, &(PtrIrp->MdlAddress), &(PtrIrp->IoStatus));
				NumberBytesRead = PtrIrp->IoStatus.Information;
				RC = PtrIrp->IoStatus.Status;

				try_return(RC);
			}

			// This is a regular run-of-the-mill cached I/O request. Let the
			// Cache Manager worry about it!
			// First though, we need a buffer pointer (address) that is valid
			// More on this in Chapter 10
			PtrSystemBuffer = SFsdGetCallersBuffer(PtrIrp);
			ASSERT(PtrSystemBuffer);
			if (!CcCopyRead(PtrFileObject, &(ByteOffset), ReadLength, CanWait, PtrSystemBuffer, &(PtrIrp->IoStatus))) {
				// The caller was not prepared to block and data is not immediately
				// available in the system cache
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				// Mark Irp Pending ...
				try_return(RC = STATUS_PENDING);
			}

			// We have the data
			RC = PtrIrp->IoStatus.Status;
			NumberBytesRead = PtrIrp->IoStatus.Information;

			try_return(RC);

		} else {

			// Send the request to lower level drivers

			// For paging-io, the FSD has to trust the VMM to do the right thing

			// Here is a common method used by Windows NT native file systems
			// that are in the process of sending a request to the disk driver.
			// First, mark the IRP as pending, then invoke the lower level driver
			// after setting a completion routine.
			// Meanwhile, this particular thread can immediately return	a
			// STATUS_PENDING return code.
			// The completion routine is then responsible for completing the IRP
			// and unlocking appropriate resources

			// Also, at this point, your FSD might choose to utilize the
			// information contained in the ValidDataLength field to simply
			// return zeroes to the caller for reads extending beyond current
			// valid data length.

			IoMarkIrpPending(PtrIrp);

			// Invoke a routine to read disk information at this time
			// You will need to set a completion routine before invoking
			// a lower level driver

			CompleteIrp = FALSE;

			try_return(RC = STATUS_PENDING);
		}

		try_exit:	NOTHING;

	} finally {
		// Post IRP if required
		if (PostRequest) {

			// Release any resources acquired here ...
			if (PtrResourceAcquired) {
				SFsdReleaseResource(PtrResourceAcquired);
			}

			// Implement a routine that will queue up the request to be executed
			// later (asynchronously) in the context of a system worker thread.
			// See Chapter 10 for details.

			// Lock the callers buffer here. Then invoke a common routine to
			// perform the post operation.
			if (!(PtrIoStackLocation->MinorFunction & IRP_MN_MDL)) {
				RC = SFsdLockCallersBuffer(PtrIrp, TRUE, ReadLength);
				ASSERT(NT_SUCCESS(RC));
			}

			// Perform the post operation which will mark the IRP pending
			// and will return STATUS_PENDING back to us
			RC = SFsdPostRequest(PtrIrpContext, PtrIrp);

		} else if (CompleteIrp && !(RC == STATUS_PENDING)) {
			// For synchronous I/O, the FSD must maintain the current byte offset
			// Do not do this however, if I/O is marked as paging-io
			if (SynchronousIo && !PagingIo && NT_SUCCESS(RC)) {
				PtrFileObject->CurrentByteOffset = RtlLargeIntegerAdd(ByteOffset, RtlConvertUlongToLargeInteger((unsigned long)NumberBytesRead));
			}

			// If the read completed successfully and this was not a paging-io
			// operation, set a flag in the CCB that indicates that a read was
			// performed and that the file time should be updated at cleanup
			if (NT_SUCCESS(RC) && !PagingIo) {
				SFsdSetFlag(PtrCCB->CCBFlags, SFSD_CCB_ACCESSED);
			}

			if (PtrResourceAcquired) {
				SFsdReleaseResource(PtrResourceAcquired);
			}

			// Can complete the IRP here if no exception was encountered
			if (!(PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_EXCEPTION)) {
				PtrIrp->IoStatus.Status = RC;
				PtrIrp->IoStatus.Information = NumberBytesRead;

				// Free up the Irp Context
				SFsdReleaseIrpContext(PtrIrpContext);
	
				// complete the IRP
				IoCompleteRequest(PtrIrp, IO_DISK_INCREMENT);
			}
		} // can we complete the IRP ?
	} // end of "finally" processing

	return(RC);
}




/*************************************************************************
*
* Function: SFsdGetCallersBuffer()
*
* Description:
*	Obtain a pointer to the caller's buffer.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
void *SFsdGetCallersBuffer(
PIRP							PtrIrp)
{
	void			*ReturnedBuffer = NULL;

	// If an MDL is supplied, use it.
	if (PtrIrp->MdlAddress) {
      ReturnedBuffer = MmGetSystemAddressForMdl(PtrIrp->MdlAddress);
	} else {
		ReturnedBuffer = PtrIrp->UserBuffer;
	}

	return(ReturnedBuffer);
}



/*************************************************************************
*
* Function: SFsdLockCallersBuffer()
*
* Description:
*	Obtain a MDL that describes the buffer. Lock pages for I/O
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFsdLockCallersBuffer(
PIRP				PtrIrp,
BOOLEAN			IsReadOperation,
uint32			Length)
{
	NTSTATUS			RC = STATUS_SUCCESS;
	PMDL				PtrMdl = NULL;

	ASSERT(PtrIrp);
	
	try {
		// Is a MDL already present in the IRP
		if (!(PtrIrp->MdlAddress)) {
			// Allocate a MDL
			if (!(PtrMdl = IoAllocateMdl(PtrIrp->UserBuffer, Length, FALSE, FALSE, PtrIrp))) {
				RC = STATUS_INSUFFICIENT_RESOURCES;
				try_return(RC);
			}

			// Probe and lock the pages described by the MDL
			// We could encounter an exception doing so, swallow the exception
			// NOTE: The exception could be due to an unexpected (from our
			// perspective), invalidation of the virtual addresses that comprise
			// the passed in buffer
			try {
				MmProbeAndLockPages(PtrMdl, PtrIrp->RequestorMode, (IsReadOperation ? IoWriteAccess:IoReadAccess));
			} except(EXCEPTION_EXECUTE_HANDLER) {
				RC = STATUS_INVALID_USER_BUFFER;
			}
		}

		try_exit:	NOTHING;

	} finally {
		if (!NT_SUCCESS(RC) && PtrMdl) {
			IoFreeMdl(PtrMdl);
			// You MUST NULL out the MdlAddress field in the IRP after freeing
			// the MDL, else the I/O Manager will also attempt to free the MDL
			// pointed to by that field during I/O completion. Obviously, the
			// pointer becomes invalid once you free the allocated MDL and hence
			// you will encounter a system crash during IRP completion.
			PtrIrp->MdlAddress = NULL;
		}
	}

	return(RC);
}



/*************************************************************************
*
* Function: SFsdMdlComplete()
*
* Description:
*	Tell Cache Manager to release MDL (and possibly flush).
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None.
*
*************************************************************************/
void SFsdMdlComplete(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PIO_STACK_LOCATION		PtrIoStackLocation,
BOOLEAN						ReadCompletion)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PFILE_OBJECT			PtrFileObject = NULL;

	PtrFileObject = PtrIoStackLocation->FileObject;
	ASSERT(PtrFileObject);

	// Not much to do here.
	if (ReadCompletion) {
		CcMdlReadComplete(PtrFileObject, PtrIrp->MdlAddress);
	} else {
		// The Cache Manager needs the byte offset in the I/O stack location.
      CcMdlWriteComplete(PtrFileObject, &(PtrIoStackLocation->Parameters.Write.ByteOffset), PtrIrp->MdlAddress);
	}

	// Clear the MDL address field in the IRP so the IoCompleteRequest()
	// does not try to play around with the MDL.
	PtrIrp->MdlAddress = NULL;

	// Free up the Irp Context.
	SFsdReleaseIrpContext(PtrIrpContext);

	// Complete the IRP.
	PtrIrp->IoStatus.Status = RC;
	PtrIrp->IoStatus.Information = 0;
	IoCompleteRequest(PtrIrp, IO_NO_INCREMENT);

	return;
}

