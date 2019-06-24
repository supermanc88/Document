/*************************************************************************
*
* File: write.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "Write" dispatch entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_WRITE



/*************************************************************************
*
* Function: SFsdWrite()
*
* Description:
*	The I/O Manager will invoke this routine to handle a write
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
NTSTATUS SFsdWrite(
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

		RC = SFsdCommonWrite(PtrIrpContext, Irp);

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
* Function: SFsdCommonWrite()
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
NTSTATUS	SFsdCommonWrite(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION	PtrIoStackLocation = NULL;
	LARGE_INTEGER			ByteOffset;
	uint32					WriteLength = 0, TruncatedWriteLength = 0;
	uint32					NumberBytesWritten = 0;
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
	BOOLEAN					IsThisADeferredWrite = FALSE;
	BOOLEAN					WritingAtEndOfFile = FALSE;

	try {
		// First, get a pointer to the current I/O stack location
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(PtrIrp);
		ASSERT(PtrIoStackLocation);

		PtrFileObject = PtrIoStackLocation->FileObject;
		ASSERT(PtrFileObject);

  		// If this happens to be a MDL write complete request, then
		// allocated MDL can be freed. This may cause a recursive write
		// back into the FSD.
		if (PtrIoStackLocation->MinorFunction & IRP_MN_COMPLETE) {
			// Caller wants to tell the Cache Manager that a previously
			// allocated MDL can be freed.
			SFsdMdlComplete(PtrIrpContext, PtrIrp, PtrIoStackLocation, FALSE);
			// The IRP has been completed.
			CompleteIrp = FALSE;
			try_return(RC = STATUS_SUCCESS);
		}

		// If this is a request at IRQL DISPATCH_LEVEL, then post
		// the request (your FSD may choose to process it synchronously
		// if you implement the support correctly).
		if (PtrIoStackLocation->MinorFunction & IRP_MN_DPC) {
			CompleteIrp = FALSE;
			PostRequest = TRUE;
			try_return(RC = STATUS_PENDING);
		}

		// Get the FCB and CCB pointers
		PtrCCB = (PtrSFsdCCB)(PtrFileObject->FsContext2);
		ASSERT(PtrCCB);
		PtrFCB = PtrCCB->PtrFCB;
		ASSERT(PtrFCB);

		// Get some of the parameters supplied to us
		ByteOffset = PtrIoStackLocation->Parameters.Write.ByteOffset;
		WriteLength = PtrIoStackLocation->Parameters.Write.Length;

		CanWait = ((PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_CAN_BLOCK) ? TRUE : FALSE);
		PagingIo = ((PtrIrp->Flags & IRP_PAGING_IO) ? TRUE : FALSE);
		NonBufferedIo = ((PtrIrp->Flags & IRP_NOCACHE) ? TRUE : FALSE);
		SynchronousIo = ((PtrFileObject->Flags & FO_SYNCHRONOUS_IO) ? TRUE : FALSE);

		// You might wish to check at this point whether the file object being
		// used for write really did have write permission requested when the
		// create/open operation was performed. Of course, for paging-io write
		// operations, the check is not valid since paging-io (via the VMM) could
		// use any file object (likely the first one with which caching was
		// initiated on the FCB) to perform the write operation

		if (WriteLength == 0) {
			// a 0 byte write can be immediately succeeded
			try_return(RC);
		}

		// NOTE: if your FSD does not support file sizes > 2 GB, you
		//	could validate the start offset here and return end-of-file
		//	if the offset begins beyond the maximum supported length.

		// Is this a write of the volume itself ?
		if (PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) {
			// Yup, we need to send this on to the disk driver after
			//	validation of the offset and length.
			PtrVCB = (PtrSFsdVCB)(PtrFCB);

			// Acquire the volume resource exclusively
			if (!ExAcquireResourceSharedLite(&(PtrVCB->VCBResource), CanWait)) {
				// post the request to be processed in the context of a worker thread
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				try_return(RC = STATUS_PENDING);
			}
			PtrResourceAcquired = &(PtrVCB->VCBResource);

			// Insert code to validate the caller supplied offset here

			// Lock the callers buffer
			if (!NT_SUCCESS(RC = SFsdLockCallersBuffer(PtrIrp, TRUE, WriteLength))) {
				try_return(RC);
			}

			// Forward the request to the lower level driver

			// For synchronous I/O wait here, else return STATUS_PENDING
			// For asynchronous I/O support, read the discussion in Chapter 10

			try_return(RC);
		}

		// Your FSD (if it is a "nice" FSD) should check whether it is
		// convenient to allow the write to proceed by utilizing the
		// CcCanIWrite() function call. If it is not convenient to perform
		// the write at this time, you should defer the request for a while.
		// The check should not however be performed for non-cached write
		// operations. To determine whether we are retrying the operation
		// or now, use the IrpContext structure we have created (See the
		// appendix to this book for a definition of the structure)
      IsThisADeferredWrite = ((PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_DEFERRED_WRITE) ? TRUE : FALSE);
		if (!NonBufferedIo) {
			if (!CcCanIWrite(PtrFileObject, WriteLength, CanWait, IsThisADeferredWrite)) {
				// Cache Manager and/or the VMM does not want us to perform
				// the write at this time. Post the request.
				SFsdSetFlag(PtrIrpContext->IrpContextFlags, SFSD_IRP_CONTEXT_DEFERRED_WRITE);
				CcDeferWrite(PtrFileObject, SFsdDeferredWriteCallBack, PtrIrpContext, PtrIrp, WriteLength, IsThisADeferredWrite);
				CompleteIrp = FALSE;
				try_return(RC = STATUS_PENDING);
			}
		}

		// If the write request is directed to a page file (if your FSD
		// supports paging files), send the request directly to the disk
		// driver. For requests directed to a page file, you have to trust
		// that the offsets will be set correctly by the VMM. You should not
		// attempt to acquire any FSD resources either.
		if (PtrFCB->FCBFlags & SFSD_FCB_PAGE_FILE) {
			IoMarkIrpPending(PtrIrp);
			// You will need to set a completion routine before invoking a lower level driver.
			//	Forward request directly to disk driver.
			// SFsdPageFileIo(PtrIrpContext, PtrIrp);

			CompleteIrp = FALSE;

			try_return(RC = STATUS_PENDING);
		}

		// We can continue. Check whether this write operation is targeted
		// to a directory object in which case the sample FSD will disallow
		// the write request. Once again though, if you create a stream file
		// object to represent a directory in memory, you could come to this
		// point as a result of modifying the directory contents internally
		// by the FSD itself. In that case, you should be able to differentiate
		// the directory write as being an internal, non-cached write operation
		// and allow it to proceed.
		if (PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY) {
			RC = STATUS_INVALID_DEVICE_REQUEST;
			try_return(RC);
		}

		PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

		// Acquire the appropriate FCB resource exclusively
		if (PagingIo) {
			// Try to acquire the FCB PagingIoResource exclusively
			if (!ExAcquireResourceExclusiveLite(&(PtrReqdFCB->PagingIoResource), CanWait)) {
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				try_return(RC = STATUS_PENDING);
			}
			// Remember the resource that was acquired
         PtrResourceAcquired = &(PtrReqdFCB->PagingIoResource);
		} else {
			// Try to acquire the FCB MainResource exclusively
			if (!ExAcquireResourceExclusiveLite(&(PtrReqdFCB->MainResource), CanWait)) {
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				try_return(RC = STATUS_PENDING);
			}
			// Remember the resource that was acquired
         PtrResourceAcquired = &(PtrReqdFCB->MainResource);
		}

		// Validate start offset and length supplied.
		// Here is a special check that determines whether the caller wishes to
		// begin the write at current end-of-file (whatever the value of that
		// offset might be)
		if ((ByteOffset.LowPart == FILE_WRITE_TO_END_OF_FILE) && (ByteOffset.HighPart == 0xFFFFFFFF)) {
         WritingAtEndOfFile = TRUE;
		}

		// There are certain complications that arise when the same file stream
		// has been opened for cached and non-cached access. The FSD is then
		// responsible for maintaining a consistent view of the data seen by
		// the caller.
		// If this happens to be a non-buffered I/O, you should try to flush the
		// cached data (if some other file object has already initiated caching
		// on the file stream). You should also try to purge the cached
		// information though the purge will probably fail if the file has been
		// mapped into some process' virtual address space
		// Read Chapter 10 for more information on the issues involved in
		// maintaining data consistency.
		if (NonBufferedIo && !PagingIo && (PtrReqdFCB->SectionObject.DataSectionObject != NULL)) {
			// Flush and then attempt to purge the cache
			CcFlushCache(&(PtrReqdFCB->SectionObject), &ByteOffset, WriteLength, &(PtrIrp->IoStatus));
			// If the flush failed, return error to the caller
			if (!NT_SUCCESS(RC = PtrIrp->IoStatus.Status)) {
				try_return(RC);
			}

			// Attempt the purge and ignore the return code
			CcPurgeCacheSection(&(PtrReqdFCB->SectionObject), (WritingAtEndOfFile ? &(PtrReqdFCB->CommonFCBHeader.FileSize) : &(ByteOffset)),
										WriteLength, FALSE);
			// We are finished with our flushing and purging
		}

		// Paging I/O write operations are special. If paging i/o write
		// requests begin beyond end-of-file, the request should be no-oped
		// (see the next two chapters for more information). If paging i/o
		// requests extend beyond current end of file, they should be truncated
		// to current end-of-file.
		// Insert code to do this here.

		// This is also a good place to set whether fast-io can be performed
		// on this particular file or not. Your FSD must make it's own
		// determination on whether or not to allow fast-io operations.
		// Commonly, fast-io is not allowed if any byte range locks exist
		// on the file or if oplocks prevent fast-io. Practically any reason
		// choosen by your FSD could result in your setting FastIoIsNotPossible
		// OR FastIoIsQuestionable instead of FastIoIsPossible.
		//
		// PtrReqdFCB->CommonFCBHeader.IsFastIoPossible = FastIoIsPossible;

		// This is a good place for oplock related processing.
		//	Chapter 11 expands upon this topic in greater detail.

		// Check whether the desired write can be allowed depending
		//	on any byte range locks that might exist. Note that for
		//	paging-io, no such checks should be performed.
		if (!PagingIo) {
			// Insert code to perform the check here ...
			//	if (!SFsdCheckForByteLock(PtrFCB, PtrCCB, PtrIrp,
			//		PtrCurrentIoStackLocation)) {
			//	try_return(RC = STATUS_FILE_LOCK_CONFLICT);
			// }
		}

		// Check whether the current request will extend the file size,
		// or the valid data length (if your FSD supports the concept of a
		// valid data length associated with the file stream). In either case,
		// inform the Cache Manager at this time using CcSetFileSizes() about
		// the new file length. Note that real FSD implementations will have to
		// first allocate enough on-disk space at this point (before they
		// inform the Cache Manager about the new size) to ensure that the write
		// will subsequently not fail due to lack of disk space.

		// if ((WritingAtEndOfFile) || ((ByteOffset + TruncatedWriteLength) > PtrReqdFCB->CommonFCBHeader.FileSize)) {
		// 	we are extending the file;
		//		allocate space and inform the Cache Manager;
		// } else if (same test as above for valid data length) {
		// 	we are extending valid data length, inform Cache Manager;
		// }

		
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
				CcPrepareMdlWrite(PtrFileObject, &ByteOffset, TruncatedWriteLength, &(PtrIrp->MdlAddress), &(PtrIrp->IoStatus));
				NumberBytesWritten = PtrIrp->IoStatus.Information;
				RC = PtrIrp->IoStatus.Status;

				try_return(RC);
			}

			// This is a regular run-of-the-mill cached I/O request. Let the
			// Cache Manager worry about it!
			// First though, we need a buffer pointer (address) that is valid
			// More on this in Chapter 10

			// Also, if the request extends the ValidDataLength, use CcZeroData()
			// first to zero out the gap (if any) between current valid data
			// length and the start of the request
			PtrSystemBuffer = SFsdGetCallersBuffer(PtrIrp);
			ASSERT(PtrSystemBuffer);
			if (!CcCopyWrite(PtrFileObject, &(ByteOffset), TruncatedWriteLength, CanWait, PtrSystemBuffer)) {
				// The caller was not prepared to block and data is not immediately
				// available in the system cache
				CompleteIrp = FALSE;
				PostRequest = TRUE;
				// Mark Irp Pending ...
				try_return(RC = STATUS_PENDING);
			} else {
				// We have the data
				PtrIrp->IoStatus.Status = RC;
				PtrIrp->IoStatus.Information = NumberBytesWritten = WriteLength;
			}

		} else {

			// If the request extends beyond valid data length, and if the caller
			// is not the lazy writer, then utilize CcZeroData() to zero out any
			// blocks between current ValidDataLength and the start of the write
			// operation. This method of zeroing data is convenient since it avoids
			// any unneccessary writes to disk. Of course, if your FSD makes no
			// guarantees about reading uninitialized data (native NT FSD
			// implementations guarantee that read operations will receive zeroes
			// if the sectors were not written to, thereby ensuring that old data
			// cannot be re-read unintentionally or maliciously), you can avoid
 			// performing the zeroing operation altogether.
			// You must however be careful about correctly determining the
			// top-level component for the IRP so as to be able to extend valid
			// data length only when appropriate and also avoid any
			// infinite, recursive loops.
			// See Chapter 10 for a discussion on this topic.
			
			// Here is a common method used by Windows NT file system drivers
			// that are in the process of sending a request to the disk driver.
			// First, mark the IRP as pending, then invoke the lower level driver
			// after setting a completion routine.
			// Meanwhile, this particular thread can immediately return	a
			// STATUS_PENDING return code.
			// The completion routine is then responsible for completing the IRP
			// and unlocking appropriate resources

			IoMarkIrpPending(PtrIrp);

			// Invoke a routine to write information to disk at this time
			// You will need to set a completion routine before invoking
			// a lower level driver

			CompleteIrp = FALSE;

			try_return(RC = STATUS_PENDING);
		}

		try_exit:	NOTHING;

	} finally {
		// Post IRP if required
		if (PostRequest) {
			// Implement a routine that will queue up the request to be executed
			// later (asynchronously) in the context of a system worker thread.
			// See Chapter 10 for details.

			if (PtrResourceAcquired) {
				SFsdReleaseResource(PtrResourceAcquired);
			}
		} else if (CompleteIrp && !(RC == STATUS_PENDING)) {
			// For synchronous I/O, the FSD must maintain the current byte offset
			// Do not do this however, if I/O is marked as paging-io
			if (SynchronousIo && !PagingIo && NT_SUCCESS(RC)) {
				PtrFileObject->CurrentByteOffset = RtlLargeIntegerAdd(ByteOffset,
				RtlConvertUlongToLargeInteger((unsigned long)NumberBytesWritten));
			}

			// If the write completed successfully and this was not a paging-io
			// operation, set a flag in the CCB that indicates that a write was
			// performed and that the file time should be updated at cleanup
			if (NT_SUCCESS(RC) && !PagingIo) {
				SFsdSetFlag(PtrCCB->CCBFlags, SFSD_CCB_MODIFIED);
			}

			// If the file size was changed, set a flag in the FCB indicating that
			// this occurred.

			// If the request failed, and we had done some nasty stuff like
			// extending the file size (including informing the Cache Manager
			// about the new file size), and allocating on-disk space etc., undo
			// it at this time.

			// Release resources ...
			if (PtrResourceAcquired) {
				SFsdReleaseResource(PtrResourceAcquired);
			}

			// Can complete the IRP here if no exception was encountered
			if (!(PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_EXCEPTION)) {
				PtrIrp->IoStatus.Status = RC;
				PtrIrp->IoStatus.Information = NumberBytesWritten;

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
* Function: SFsdDeferredWriteCallBack()
*
* Description:
*	Invoked by the cache manager in the context of a worker thread.
*	Typically, you can simply post the request at this point (just
*	as you would have if the original request could not block) to
*	perform the write in the context of a system worker thread.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdDeferredWriteCallBack (
void						*Context1,			// Should be PtrIrpContext
void						*Context2)			// Should be PtrIrp
{
	// You should typically simply post the request to your internal
	// queue of posted requests (just as you would if the original write
	// could not be completed because the caller could not block).
	// Once you post the request, return from this routine. The write
	// will then be retried in the context of a system worker thread
}

