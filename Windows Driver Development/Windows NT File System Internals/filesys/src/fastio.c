/*************************************************************************
*
* File: fastio.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the various "fast-io" calls.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_FAST_IO



/*************************************************************************
*
* Function: SFsdFastIoCheckIfPossible()
*
* Description:
*	To fast-io or not to fast-io, that is the question ...
*	This routine helps the I/O Manager determine whether the FSD wishes
*	to permit fast-io on a specific file stream.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoCheckIfPossible(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
IN BOOLEAN						CheckForReadOperation,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;
	PtrSFsdFCB			PtrFCB = NULL;
	PtrSFsdCCB			PtrCCB = NULL;
	LARGE_INTEGER		IoLength;

	// Obtain a pointer to the FCB and CCB for the file stream.
	PtrCCB = (PtrSFsdCCB)(FileObject->FsContext2);
	ASSERT(PtrCCB);
	PtrFCB = PtrCCB->PtrFCB;
	ASSERT(PtrFCB);
	
	// Validate that this is a fast-IO request to a regular file.
	// The sample FSD for example, will not allow fast-IO requests
	// to volume objects, or to directories.
	if ((PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) ||
		 (PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY)) {
		// This is not allowed.
		return(ReturnedStatus);
	}

	IoLength = RtlConvertUlongToLargeInteger(Length);
	
	// Your FSD can determine the checks that it needs to perform.
	// Typically, a FSD will check whether there exist any byte-range
	// locks that would prevent a fast-IO operation from proceeding.
	
	// ... (FSD specific checks go here).
	
	if (CheckForReadOperation) {
		// Chapter 11 describes how to use the FSRTL package for byte-range
		// lock requests. The following routine is exported by the FSRTL
		// package and it returns TRUE if the read operation should be
		// allowed to proceed based on the status of the current byte-range
		// locks on the file stream. If you do not use the FSRTL package
		// for byte-range locking support, then you must substitute your
		// own checks over here.
		// ReturnedStatus = FsRtlFastCheckLockForRead(&(PtrFCB->FCBByteRangeLock),
		//							FileOffset, &IoLength, LockKey, FileObject,
      //                     PsGetCurrentProcess());
	} else {
		// This is a write request. Invoke the FSRTL byte-range lock package
		// to see whether the write should be allowed to proceed.
		// ReturnedStatus = FsRtlFastCheckLockForWrite(&(PtrFCB->FCBByteRangeLock),
		//							FileOffset, &IoLength, LockKey, FileObject,
      //                     PsGetCurrentProcess());
	}
	
	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoRead()
*
* Description:
*	Bypass the traditional IRP method to perform a read operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {

			// Chapter 11 describes how to roll your own fast-IO entry points.
			// Typically, you will acquire appropriate resources here and
			// then (maybe) forward the request to FsRtlCopyRead().
			// If you are a suitably complex file system, you may even choose
			// to do some pre-processing (e.g. prefetching data from someplace)
			// before passing on the request to the FSRTL package.

			// Of course, you also have the option of bypassing the FSRTL
			// package completely and simply forwarding the request directly
			// to the NT Cache Manager.

			// Bottom line is that you have complete flexibility on determining
			// what you decide to do here. Read Chapter 11 well (and obviously
			// other related issues) before filling in this and other fast-IO
			// dispatch entry points.

			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoWrite()
*
* Description:
*	Bypass the traditional IRP method to perform a write operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {

			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoQueryBasicInfo()
*
* Description:
*	Bypass the traditional IRP method to perform a query basic
*	information operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoQueryBasicInfo(
IN PFILE_OBJECT					FileObject,
IN BOOLEAN							Wait,
OUT PFILE_BASIC_INFORMATION	Buffer,
OUT PIO_STATUS_BLOCK 			IoStatus,
IN PDEVICE_OBJECT					DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoQueryStdInfo()
*
* Description:
*	Bypass the traditional IRP method to perform a query standard
*	information operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoQueryStdInfo(
IN PFILE_OBJECT						FileObject,
IN BOOLEAN								Wait,
OUT PFILE_STANDARD_INFORMATION 	Buffer,
OUT PIO_STATUS_BLOCK 				IoStatus,
IN PDEVICE_OBJECT						DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoLock()
*
* Description:
*	Bypass the traditional IRP method to perform a byte range lock
*	operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoLock(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
BOOLEAN							FailImmediately,
BOOLEAN							ExclusiveLock,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoUnlockSingle()
*
* Description:
*	Bypass the traditional IRP method to perform a byte range unlock
*	operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoUnlockSingle(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoUnlockAll()
*
* Description:
*	Bypass the traditional IRP method to perform multiple byte range unlock
*	operations.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoUnlockAll(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoUnlockAllByKey()
*
* Description:
*	Bypass the traditional IRP method to perform multiple byte range unlock
*	operations.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoUnlockAllByKey(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoAcqCreateSec()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to acquire FSD resources
*	before processing a file map (create section object) request.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None (we must be prepared to handle VMM initiated calls)
*
*************************************************************************/
void SFsdFastIoAcqCreateSec(
IN PFILE_OBJECT			FileObject)
{
	PtrSFsdFCB			PtrFCB = NULL;
	PtrSFsdCCB			PtrCCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;

	// Obtain a pointer to the FCB and CCB for the file stream.
	PtrCCB = (PtrSFsdCCB)(FileObject->FsContext2);
	ASSERT(PtrCCB);
	PtrFCB = PtrCCB->PtrFCB;
	ASSERT(PtrFCB);
	PtrReqdFCB = &(PtrFCB->NTRequiredFCB);
	
	// Acquire the MainResource exclusively for the file stream
	ExAcquireResourceExclusiveLite(&(PtrReqdFCB->MainResource), TRUE);

	// Although this is typically not required, the sample FSD will
	// also acquire the PagingIoResource exclusively at this time
	// to conform with the resource acquisition described in the set
	// file information routine. Once again though, you will probably
	// not need to do this.
	ExAcquireResourceExclusiveLite(&(PtrReqdFCB->PagingIoResource), TRUE);

	return;
}


/*************************************************************************
*
* Function: SFsdFastIoRelCreateSec()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to release FSD resources
*	after processing a file map (create section object) request.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdFastIoRelCreateSec(
IN PFILE_OBJECT			FileObject)
{

	PtrSFsdFCB			PtrFCB = NULL;
	PtrSFsdCCB			PtrCCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;

	// Obtain a pointer to the FCB and CCB for the file stream.
	PtrCCB = (PtrSFsdCCB)(FileObject->FsContext2);
	ASSERT(PtrCCB);
	PtrFCB = PtrCCB->PtrFCB;
	ASSERT(PtrFCB);
	PtrReqdFCB = &(PtrFCB->NTRequiredFCB);
	
	// Release the PagingIoResource for the file stream
	SFsdReleaseResource(&(PtrReqdFCB->PagingIoResource));

	// Release the MainResource for the file stream
	SFsdReleaseResource(&(PtrReqdFCB->MainResource));

	return;
}


/*************************************************************************
*
* Function: SFsdAcqLazyWrite()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to acquire FSD
*	resources before performing a delayed write (write behind/lazy write)
*	operation.
*	NOTE: this function really must succeed since the Cache Manager will
*			typically ignore failure and continue on ...
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE (Cache Manager does not tolerate FALSE well)
*
*************************************************************************/
BOOLEAN SFsdAcqLazyWrite(
IN PVOID							Context,
IN BOOLEAN						Wait)
{
	BOOLEAN				ReturnedStatus = TRUE;

	PtrSFsdFCB			PtrFCB = NULL;
	PtrSFsdCCB			PtrCCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;

	// The context is whatever we passed to the Cache Manager when invoking
	// the CcInitializeCacheMaps() function. In the case of the sample FSD
	// implementation, this context is a pointer to the CCB structure.

	ASSERT(Context);
	PtrCCB = (PtrSFsdCCB)(Context);
	ASSERT(PtrCCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_CCB);

	PtrFCB = PtrCCB->PtrFCB;
	ASSERT(PtrFCB);
	PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

	// Acquire the MainResource in the FCB exclusively. Then, set the
	// lazy-writer thread id in the FCB structure for identification when
	// an actual write request is received by the FSD.
	// Note: The lazy-writer typically always supplies WAIT set to TRUE.
	if (!ExAcquireResourceExclusiveLite(&(PtrReqdFCB->MainResource),
													  Wait)) {
		ReturnedStatus = FALSE;
	} else {
		// Now, set the lazy-writer thread id.
		ASSERT(!(PtrFCB->LazyWriterThreadID));
		PtrFCB->LazyWriterThreadID = (unsigned int)(PsGetCurrentThread());
	}

	// If your FSD needs to perform some special preparations in anticipation
	// of receving a lazy-writer request, do so now.

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdRelLazyWrite()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to release FSD
*	resources after performing a delayed write (write behind/lazy write)
*	operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdRelLazyWrite(
IN PVOID							Context)
{
	BOOLEAN				ReturnedStatus = TRUE;

	PtrSFsdFCB			PtrFCB = NULL;
	PtrSFsdCCB			PtrCCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;

	// The context is whatever we passed to the Cache Manager when invoking
	// the CcInitializeCacheMaps() function. In the case of the sample FSD
	// implementation, this context is a pointer to the CCB structure.

	ASSERT(Context);
	PtrCCB = (PtrSFsdCCB)(Context);
	ASSERT(PtrCCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_CCB);

	PtrFCB = PtrCCB->PtrFCB;
	ASSERT(PtrFCB);
	PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

	// Remove the current thread-id from the FCB and release the MainResource.
	ASSERT((PtrFCB->LazyWriterThreadID) == (unsigned int)PsGetCurrentThread());
	PtrFCB->LazyWriterThreadID = 0;

	// Release the acquired resource.
	SFsdReleaseResource(&(PtrReqdFCB->MainResource));

	// Of course, your FSD should undo whatever else seems appropriate at this
	// time.

	return;
}


/*************************************************************************
*
* Function: SFsdAcqReadAhead()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to acquire FSD
*	resources before performing a read-ahead operation.
*	NOTE: this function really must succeed since the Cache Manager will
*			typically ignore failure and continue on ...
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE (Cache Manager does not tolerate FALSE well)
*
*************************************************************************/
BOOLEAN SFsdAcqReadAhead(
IN PVOID							Context,
IN BOOLEAN						Wait)
{

	BOOLEAN				ReturnedStatus = TRUE;

	PtrSFsdFCB			PtrFCB = NULL;
	PtrSFsdCCB			PtrCCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;

	// The context is whatever we passed to the Cache Manager when invoking
	// the CcInitializeCacheMaps() function. In the case of the sample FSD
	// implementation, this context is a pointer to the CCB structure.

	ASSERT(Context);
	PtrCCB = (PtrSFsdCCB)(Context);
	ASSERT(PtrCCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_CCB);

	PtrFCB = PtrCCB->PtrFCB;
	ASSERT(PtrFCB);
	PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

	// Acquire the MainResource in the FCB shared.
	// Note: The read-ahead thread typically always supplies WAIT set to TRUE.
	if (!ExAcquireResourceSharedLite(&(PtrReqdFCB->MainResource),
													  Wait)) {
		ReturnedStatus = FALSE;
	}

	// If your FSD needs to perform some special preparations in anticipation
	// of receving a read-ahead request, do so now.

	return(ReturnedStatus);
}



/*************************************************************************
*
* Function: SFsdRelReadAhead()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to release FSD
*	resources after performing a read-ahead operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdRelReadAhead(
IN PVOID							Context)
{
	BOOLEAN				ReturnedStatus = TRUE;

	PtrSFsdFCB			PtrFCB = NULL;
	PtrSFsdCCB			PtrCCB = NULL;
	PtrSFsdNTRequiredFCB	PtrReqdFCB = NULL;

	// The context is whatever we passed to the Cache Manager when invoking
	// the CcInitializeCacheMaps() function. In the case of the sample FSD
	// implementation, this context is a pointer to the CCB structure.

	ASSERT(Context);
	PtrCCB = (PtrSFsdCCB)(Context);
	ASSERT(PtrCCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_CCB);

	PtrFCB = PtrCCB->PtrFCB;
	ASSERT(PtrFCB);
	PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

	// Release the acquired resource.
	SFsdReleaseResource(&(PtrReqdFCB->MainResource));

	// Of course, your FSD should undo whatever else seems appropriate at this
	// time.

	return;
}


/* the remaining are only valid under NT Version 4.0 and later */
#if(_WIN32_WINNT >= 0x0400)


/*************************************************************************
*
* Function: SFsdFastIoQueryNetInfo()
*
* Description:
*	Get information requested by a redirector across the network. This call
*	will originate from the LAN Manager server.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoQueryNetInfo(
IN PFILE_OBJECT									FileObject,
IN BOOLEAN											Wait,
OUT PFILE_NETWORK_OPEN_INFORMATION 			Buffer,
OUT PIO_STATUS_BLOCK 							IoStatus,
IN PDEVICE_OBJECT									DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoMdlRead()
*
* Description:
*	Bypass the traditional IRP method to perform a MDL read operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoMdlRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
	
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoMdlReadComplete()
*
* Description:
*	Bypass the traditional IRP method to inform the NT Cache Manager and the
*	FSD that the caller no longer requires the data locked in the system cache
*	or the MDL to stay around anymore ..
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoMdlReadComplete(
IN PFILE_OBJECT				FileObject,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
		
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoPrepareMdlWrite()
*
* Description:
*	Bypass the traditional IRP method to prepare for a MDL write operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoPrepareMdlWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
		
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoMdlWriteComplete()
*
* Description:
*	Bypass the traditional IRP method to inform the NT Cache Manager and the
*	FSD that the caller has updated the contents of the MDL. This data can
*	now be asynchronously written out to secondary storage by the Cache Mgr.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFsdFastIoMdlWriteComplete(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN				ReturnedStatus = FALSE;		// fast i/o failed/not allowed
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {
	
			// See description in SFsdFastIoRead() before filling-in the
			// stub here.
			NOTHING;
		
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(ReturnedStatus);
}


/*************************************************************************
*
* Function: SFsdFastIoAcqModWrite()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to acquire FSD resources
*	before initiating a write operation via the Modified Page/Block Writer.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error (try not to return an error, will 'ya ? :-)
*
*************************************************************************/
NTSTATUS SFsdFastIoAcqModWrite(
IN PFILE_OBJECT					FileObject,
IN PLARGE_INTEGER					EndingOffset,
OUT PERESOURCE						*ResourceToRelease,
IN PDEVICE_OBJECT					DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {

			// You must determine which resource(s) you would like to
			// acquire at this time. You know that a write is imminent;
			// you will probably therefore acquire appropriate resources
			// exclusively.

			// You must first get the FCB and CCB pointers from the file object
			// that is passed in to this function (as an argument). Note that
			// the ending offset (when examined in conjunction with current valid data
			// length) may help you in determining the appropriate resource(s) to acquire.

			// For example, if the ending offset is beyond current valid data length,
			// you may decide to acquire *both* the MainResource and the PagingIoResource
			// exclusively; otherwise, you may decide simply to acquire the PagingIoResource.

			// Consult the text for more information on synchronization in FSDs.

			// One final note; the VMM expects that you will return a pointer to
			// the resource that you acquired (single return value). This pointer
			// will be returned back to you in the release call (below).

			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(RC);
}


/*************************************************************************
*
* Function: SFsdFastIoRelModWrite()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to release FSD resources
*	after processing a modified page/block write operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error (an error returned here is really not expected!)
*
*************************************************************************/
NTSTATUS SFsdFastIoRelModWrite(
IN PFILE_OBJECT				FileObject,
IN PERESOURCE					ResourceToRelease,
IN PDEVICE_OBJECT				DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {

			// The MPW has complete the write for modified pages and therefore
			// wants you to release pre-acquired resource(s).

			// You must undo here whatever it is that you did in the
			// SFsdFastIoAcqModWrite() call above.

			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(RC);
}


/*************************************************************************
*
* Function: SFsdFastIoAcqCcFlush()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to acquire FSD
*	resources before performing a CcFlush() operation on a specific file
*	stream.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFsdFastIoAcqCcFlush(
IN PFILE_OBJECT			FileObject,
IN PDEVICE_OBJECT			DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {

			// Acquire appropriate resources that will allow correct synchronization
			// with a flush call (and avoid deadlock).
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(RC);
}


/*************************************************************************
*
* Function: SFsdFastIoRelCcFlush()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to acquire FSD
*	resources before performing a CcFlush() operation on a specific file
*	stream.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFsdFastIoRelCcFlush(
IN PFILE_OBJECT			FileObject,
IN PDEVICE_OBJECT			DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;

	FsRtlEnterFileSystem();

	try {

		try {

			// Release resources acquired in SFsdFastIoAcqCcFlush() above.
			NOTHING;
	
		} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {
	
			RC = SFsdExceptionHandler(PtrIrpContext, NULL);
	
			SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);

		}

		try_exit:	NOTHING;

	} finally {

	}
	
	FsRtlExitFileSystem();

	return(RC);
}

#endif	//_WIN32_WINNT >= 0x0400

