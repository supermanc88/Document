/*************************************************************************
*
* File: fileinfo.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "set/query file information" dispatch
*	entry points.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_INFORMATION



/*************************************************************************
*
* Function: SFsdFileInfo()
*
* Description:
*	The I/O Manager will invoke this routine to handle a set/query file
*	information request
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL (invocation at higher IRQL will cause execution
*	to be deferred to a worker thread context)
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFsdFileInfo(
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

		RC = SFsdCommonFileInfo(PtrIrpContext, Irp);

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
* Function: SFsdCommonFileInfo()
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
NTSTATUS	SFsdCommonFileInfo(
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
	BOOLEAN					MainResourceAcquired = FALSE;
	BOOLEAN					VCBResourceAcquired = FALSE;
	BOOLEAN					PagingIoResourceAcquired = FALSE;
	IO_STATUS_BLOCK		LocalIoStatus;
	void						*PtrSystemBuffer = NULL;
	long						BufferLength = 0;
	FILE_INFORMATION_CLASS	FunctionalityRequested;
	BOOLEAN					CanWait = FALSE;
	BOOLEAN					PostRequest = FALSE;

	try {
		// First, get a pointer to the current I/O stack location.
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(PtrIrp);
		ASSERT(PtrIoStackLocation);

		PtrFileObject = PtrIoStackLocation->FileObject;
		ASSERT(PtrFileObject);

		// Get the FCB and CCB pointers.
		PtrCCB = (PtrSFsdCCB)(PtrFileObject->FsContext2);
		ASSERT(PtrCCB);
		PtrFCB = PtrCCB->PtrFCB;
		ASSERT(PtrFCB);

		PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

		CanWait = ((PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_CAN_BLOCK) ? TRUE : FALSE);

		// If the caller has opened a logical volume and is attempting to
		// query information for it as a file stream, return an error.
		if (PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB) {
			// This is not allowed. Caller must use get/set volume information instead.
			RC = STATUS_INVALID_PARAMETER;
			try_return(RC);
		}

		ASSERT(PtrFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_FCB);

		// The NT I/O Manager always allocates and supplies a system
		// buffer for query and set file information calls.
		// Copying information to/from the user buffer and the system
		// buffer is performed by the I/O Manager and the FSD need not worry about it.
		PtrSystemBuffer = PtrIrp->AssociatedIrp.SystemBuffer;	

		if (PtrIoStackLocation->MajorFunction == IRP_MJ_QUERY_INFORMATION) {
			// Now, obtain some parameters.
			BufferLength = PtrIoStackLocation->Parameters.QueryFile.Length;
         FunctionalityRequested = PtrIoStackLocation->Parameters.QueryFile.FileInformationClass;

			// Acquire the MainResource shared (NOTE: for paging-IO on a
			// page file, we should avoid acquiring any resources and simply
			// trust the VMM to do the right thing, else we could possibly
			// run into deadlocks).
			if (!(PtrFCB->FCBFlags & SFSD_FCB_PAGE_FILE)) {
				// Acquire the MainResource shared.
				if (!ExAcquireResourceSharedLite(&(PtrReqdFCB->MainResource), CanWait)) {
					PostRequest = TRUE;
					try_return(RC = STATUS_PENDING);
				}
				MainResourceAcquired = TRUE;
			}

			// Do whatever the caller asked us to do
			switch (FunctionalityRequested) {
			case FileBasicInformation:
				RC = SFsdGetBasicInformation(PtrFCB, (PFILE_BASIC_INFORMATION)PtrSystemBuffer, &BufferLength);
				break;
			case FileStandardInformation:
				// RC = SFsdGetStandardInformation(PtrFCB, PtrCCB, ...);
				break;
			// Similarly, implement all of the other query information routines
			// that your FSD can support.
#if(_WIN32_WINNT >= 0x0400)
			case FileNetworkOpenInformation:
				// RC = SFsdGetNetworkOpenInformation(...);
				break;
#endif	// _WIN32_WINNT >= 0x0400
			case FileInternalInformation:
				// RC = SFsdGetInternalInformation(...);
				break;
			case FileEaInformation:
				// RC = SFsdGetEaInformation(...);
				break;
			case FileNameInformation:
				// RC = SFsdGetFullNameInformation(...);
				break;
			case FileAlternateNameInformation:
				// RC = SFsdGetAltNameInformation(...);
				break;
			case FileCompressionInformation:
				// RC = SFsdGetCompressionInformation(...);
				break;
			case FilePositionInformation:
				// This is fairly simple. Copy over the information from the
				// file object.
				{
               PFILE_POSITION_INFORMATION		PtrFileInfoBuffer;

					PtrFileInfoBuffer = (PFILE_POSITION_INFORMATION)PtrSystemBuffer;

					ASSERT(BufferLength >= sizeof(FILE_POSITION_INFORMATION));
					PtrFileInfoBuffer->CurrentByteOffset = PtrFileObject->CurrentByteOffset;
					// Modify the local variable for BufferLength appropriately.
					BufferLength -= sizeof(FILE_POSITION_INFORMATION);
				}
				break;
			case FileStreamInformation:
				// RC = SFsdGetFileStreamInformation(...);
				break;
			case FileAllInformation:
				// The I/O Manager supplies the Mode, Access, and Alignment
				// information. The rest is up to us to provide.
				// Therefore, decrement the BufferLength appropriately (assuming
				// that the above 3 types on information are already in the
				// buffer)
				{

               PFILE_POSITION_INFORMATION		PtrFileInfoBuffer;
					PFILE_ALL_INFORMATION			PtrAllInfo = (PFILE_ALL_INFORMATION)PtrSystemBuffer;

					BufferLength -= (sizeof(FILE_MODE_INFORMATION) + sizeof(FILE_ACCESS_INFORMATION) + sizeof(FILE_ALIGNMENT_INFORMATION));

					// Fill in the position information.

					PtrFileInfoBuffer = (PFILE_POSITION_INFORMATION)&(PtrAllInfo->PositionInformation);

					PtrFileInfoBuffer->CurrentByteOffset = PtrFileObject->CurrentByteOffset;

					// Modify the local variable for BufferLength appropriately.
					ASSERT(BufferLength >= sizeof(FILE_POSITION_INFORMATION));
					BufferLength -= sizeof(FILE_POSITION_INFORMATION);

					// Get the remaining stuff.
					if (!NT_SUCCESS(RC = SFsdGetBasicInformation(PtrFCB, (PFILE_BASIC_INFORMATION)&(PtrAllInfo->BasicInformation),
																					&BufferLength))) {
						// Another method you may wish to use to avoid the
						// multiple checks for success/failure is to have the
						// called routine simply raise an exception instead.
						try_return(RC);
					}
					// Similarly, get all of the others ...
				}
				break;
			default:
				RC = STATUS_INVALID_PARAMETER;
				try_return(RC);
			}

			// If we completed successfully, the return the amount of information transferred.
			if (NT_SUCCESS(RC)) {
				PtrIrp->IoStatus.Information = PtrIoStackLocation->Parameters.QueryFile.Length - BufferLength;
			} else {
				PtrIrp->IoStatus.Information = 0;
			}

		} else {
			ASSERT(PtrIoStackLocation->MajorFunction == IRP_MJ_SET_INFORMATION);

			// Now, obtain some parameters.
         FunctionalityRequested = PtrIoStackLocation->Parameters.SetFile.FileInformationClass;

			//	If your FSD supports opportunistic locking (described in
			// Chapter 11), then you should check whether the oplock state
			// allows the caller to proceed.

			// Rename, and link operations require creation of a directory
			// entry and possibly deletion of another directory entry. Since,
			// we acquire the VCB resource exclusively during create operations,
			// we should acquire it exclusively for link and/or rename operations
			// as well.

			// Similarly, marking a directory entry for deletion should cause us
			// to acquire the VCB exclusively as well.

			if ((FunctionalityRequested == FileDispositionInformation) || (FunctionalityRequested == FileRenameInformation) ||
				 (FunctionalityRequested == FileLinkInformation)) {
				if (!ExAcquireResourceExclusiveLite(&(PtrVCB->VCBResource), CanWait)) {
					PostRequest = TRUE;
					try_return(RC = STATUS_PENDING);
				}
				// We have the VCB acquired exclusively.
				VCBResourceAcquired = TRUE;
			}

			// Unless this is an operation on a page file, we should go ahead and
			// acquire the FCB exclusively at this time. Note that we will pretty
			// much block out anything being done to the FCB from this point on.
			if (!(PtrFCB->FCBFlags & SFSD_FCB_PAGE_FILE)) {
				// Acquire the MainResource shared.
				if (!ExAcquireResourceExclusiveLite(&(PtrReqdFCB->MainResource), CanWait)) {
					PostRequest = TRUE;
					try_return(RC = STATUS_PENDING);
				}
				MainResourceAcquired = TRUE;
			}

			// The only operations that could conceivably proceed from this point
			// on are paging-IO read/write operations. For delete link (rename),
			// set allocation size, and set EOF, should also acquire the paging-IO
			// resource, thereby synchronizing with paging-IO requests. In your
			// FSD, you should ideally acquire the resource only when processing
			// such requests; here however, I will go ahead and block out all
			// paging-IO read/write operations at this time (for convenience).
			if (!ExAcquireResourceExclusiveLite(&(PtrReqdFCB->PagingIoResource), CanWait)) {
				PostRequest = TRUE;
				try_return(RC = STATUS_PENDING);
			}

			// Do whatever the caller asked us to do
			switch (FunctionalityRequested) {
			case FileBasicInformation:
				RC = SFsdSetBasicInformation(PtrFCB, PtrCCB, PtrFileObject, (PFILE_BASIC_INFORMATION)PtrSystemBuffer);
				break;
			case FilePositionInformation:
				// Check	if no intermediate buffering has been specified.
				// If it was specified, do not allow non-aligned set file
				// position requests to succeed.
				{
               PFILE_POSITION_INFORMATION		PtrFileInfoBuffer;

					PtrFileInfoBuffer = (PFILE_POSITION_INFORMATION)PtrSystemBuffer;

					if (PtrFileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
						if (PtrFileInfoBuffer->CurrentByteOffset.LowPart & PtrIoStackLocation->DeviceObject->AlignmentRequirement) {
							// Invalid alignment.
							try_return(RC = STATUS_INVALID_PARAMETER);
						}
					}

					PtrFileObject->CurrentByteOffset = PtrFileInfoBuffer->CurrentByteOffset;
				}
				break;
			case FileDispositionInformation:
				RC = SFsdSetDispositionInformation(PtrFCB, PtrCCB, PtrVCB, PtrFileObject, PtrIrpContext, PtrIrp,
							(PFILE_DISPOSITION_INFORMATION)PtrSystemBuffer);
				break;
			case FileRenameInformation:
			case FileLinkInformation:
				// When you implement your rename/link routine, be careful to
				// check the following two arguments:
				// TargetFileObject = PtrIoStackLocation->Parameters.SetFile.FileObject;
				// ReplaceExistingFile = PtrIoStackLocation->Parameters.SetFile.ReplaceIfExists;
				//
				// The TargetFileObject argument is a pointer to the "target
				// directory" file object obtained during the "create" routine
				// invoked by the NT I/O Manager with the SL_OPEN_TARGET_DIRECTORY
				// flag specified. Remember that it is quite possible that if the
				// rename/link is contained within a single directory, the target
				// and source directories will be the same.
				// The ReplaceExistingFile argument should be used by you to
				// determine if the caller wishes to replace the target (if it
				// currently exists) with the new link/renamed file. If this
				// value is FALSE, and if the target directory entry (being
				// renamed-to, or the target of the link) exists, you should
				// return a STATUS_OBJECT_NAME_COLLISION error to the caller.

				// RC = SFsdRenameOrLinkFile(PtrFCB, PtrCCB, PtrFileObject,	PtrIrpContext,
				//						PtrIrp, (PFILE_RENAME_INFORMATION)PtrSystemBuffer);

				// Once you have completed the rename/link operation, do not
				// forget to notify any "notify IRPs" about the actions you have
				// performed.
				// An example is if you renamed across directories, you should
				// report that a new entry was added with the FILE_ACTION_ADDED
				// action type. The actual modification would then be reported
				// as either FILE_NOTIFY_CHANGE_FILE_NAME (if a file was renamed)
				// or FILE_NOTIFY_CHANGE_DIR_NAME (if a directory was renamed).
				break;
			case FileAllocationInformation:
				RC = SFsdSetAllocationInformation(PtrFCB, PtrCCB, PtrVCB, PtrFileObject,
																PtrIrpContext, PtrIrp, PtrSystemBuffer);
				break;
			case FileEndOfFileInformation:
				// RC = SFsdSetEOF(...);
				break;
			default:
				RC = STATUS_INVALID_PARAMETER;
				try_return(RC);
			}
		}

		try_exit:	NOTHING;

	} finally {

		if (PagingIoResourceAcquired) {
			SFsdReleaseResource(&(PtrReqdFCB->PagingIoResource));
			PagingIoResourceAcquired = FALSE;
		}

		if (MainResourceAcquired) {
			SFsdReleaseResource(&(PtrReqdFCB->MainResource));
			MainResourceAcquired = FALSE;
		}

		if (VCBResourceAcquired) {
			SFsdReleaseResource(&(PtrVCB->VCBResource));
			VCBResourceAcquired = FALSE;
		}

		// Post IRP if required
		if (PostRequest) {

			// Since, the I/O Manager gave us a system buffer, we do not
			// need to "lock" anything.

			// Perform the post operation which will mark the IRP pending
			// and will return STATUS_PENDING back to us
			RC = SFsdPostRequest(PtrIrpContext, PtrIrp);

		} else {

			// Can complete the IRP here if no exception was encountered
			if (!(PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_EXCEPTION)) {
				PtrIrp->IoStatus.Status = RC;

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
* Function: SFsdGetBasicInformation()
*
* Description:
*	Return some time-stamps and file attributes to the caller.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS	SFsdGetBasicInformation(
PtrSFsdFCB					PtrFCB,
PFILE_BASIC_INFORMATION	PtrBuffer,
long							*PtrReturnedLength)
{
	NTSTATUS			RC = STATUS_SUCCESS;

	try {
		if (*PtrReturnedLength < sizeof(FILE_BASIC_INFORMATION)) {
			try_return(RC = STATUS_BUFFER_OVERFLOW);
		}

		// Zero out the supplied buffer.
		RtlZeroMemory(PtrBuffer, sizeof(FILE_BASIC_INFORMATION));

		// Note: If your FSD wishes to be even more precise about time
		// stamps, you may wish to consider the effects of fast-IO on the
		// file stream. Typically, the FSD simply states a flag indicating
		// that fast-IO read/write has occured. Time stamps are then updated
		// when a cleanup is received for the file stream. However, if the
		// user performs fast-IO and subsequently issues a request to query
		// basic information, your FSD could query the current system time
		// using KeQuerySystemTime(), and update the FCB time-stamps before
		// returning the values to the caller. This gives the caller a slightly
		// more accurate value.

		// Get information from the FCB.
		PtrBuffer->CreationTime = PtrFCB->CreationTime;
		PtrBuffer->LastAccessTime = PtrFCB->LastAccessTime;
		PtrBuffer->LastWriteTime = PtrFCB->LastWriteTime;
		// Assume that the sample FSD does not support a "change time".

		// Now fill in the attributes.
      PtrBuffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;

		if (PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY) {
			PtrBuffer->FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
		}

		// Similarly, fill in attributes indicating a hidden file, system
		// file, compressed file, temporary file, etc. if your FSD supports
		// such file attribute values.

		try_exit: NOTHING;
	} finally {
		if (NT_SUCCESS(RC)) {
			// Return the amount of information filled in.
			*PtrReturnedLength -= sizeof(FILE_BASIC_INFORMATION);
		}
	}
	return(RC);
}


/*************************************************************************
*
* Function: SFsdSetBasicInformation()
*
* Description:
*	Set some time-stamps and file attributes supplied by the caller.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS	SFsdSetBasicInformation(
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB,
PFILE_OBJECT				PtrFileObject,
PFILE_BASIC_INFORMATION	PtrBuffer)
{
	NTSTATUS			RC = STATUS_SUCCESS;
	BOOLEAN			CreationTimeChanged = FALSE;
	BOOLEAN			AttributesChanged = FALSE;

	try {

		// Obtain a pointer to the directory entry associated with
		// the FCB being modifed. The directory entry is obviously
		// part of the data associated with the parent directory that
		// contains this particular file stream.
		// Note well that no other modifications
		// are currently allowed to the directory entry since we have
		// the VCB resource exclusively acquired (as a matter of fact,
		// NO directory on the logical volume can be currently modified).
		// PtrDirectoryEntry = SFsdGetDirectoryEntryPtr(...);

		if (RtlLargeIntegerNotEqualToZero(PtrBuffer->CreationTime)) {
			// Modify the directory entry timestamp.
			// ...

			// Also note that fact that the timestamp has changed
			// so that any directory notifications can be performed.
			CreationTimeChanged = TRUE;

			// The interesting thing here is that the user has set certain time
			// fields. However, before doing this, the user may have performed
			// I/O which in turn would have caused your FSD to mark the fact that
			// write/access time should be modifed at cleanup.
			// You might wish to mark the fact that such updates are no longer
			// required since the user has explicitly specified the values she
			// wishes to see associated with the file stream.
			SFsdSetFlag(PtrCCB->CCBFlags, SFSD_CCB_CREATE_TIME_SET);
		}

		// Similarly, check for all the time-stamp values that your
		// FSD cares about. Ignore the ones that you do not support.
		// ...


		// Now come the attributes.
		if (PtrBuffer->FileAttributes) {
			// We have a non-zero attribute value.
			// The presence of a particular attribute indicates that the
			// user wishes to set the attribute value. The absence indicates
			// the user wishes to clear the particular attribute.

			// Before we start examining attribute values, you may wish
			// to clear any unsupported attribute flags to reduce confusion.
			
			SFsdClearFlag(PtrBuffer->FileAttributes, ~FILE_ATTRIBUTE_VALID_SET_FLAGS);
			SFsdClearFlag(PtrBuffer->FileAttributes, FILE_ATTRIBUTE_NORMAL);

			// Similarly, you should pick out other invalid flag values.
			// SFsdClearFlag(PtrBuffer->FileAttributes, FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_ATOMIC_WRITE ...);


			if (PtrBuffer->FileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
				SFsdSetFlag(PtrFileObject->Flags, FO_TEMPORARY_FILE);
			} else {
				SFsdClearFlag(PtrFileObject->Flags, FO_TEMPORARY_FILE);
			}

			// If your FSD supports file compression, you may wish to
			// note the user's preferences for compressing/not compressing
			// the file at this time.
		}

		try_exit: NOTHING;
	} finally {
		;
	}
	return(RC);
}


/*************************************************************************
*
* Function: SFsdSetDispositionInformation()
*
* Description:
*	Mark/Unmark a file for deletion.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS	SFsdSetDispositionInformation(
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB,
PtrSFsdVCB					PtrVCB,
PFILE_OBJECT				PtrFileObject,
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PFILE_DISPOSITION_INFORMATION	PtrBuffer)
{
	NTSTATUS			RC = STATUS_SUCCESS;

	try {
		if (!PtrBuffer->DeleteFile) {
			// "un-delete" the file.
			SFsdClearFlag(PtrFCB->FCBFlags, SFSD_FCB_DELETE_ON_CLOSE);
			PtrFileObject->DeletePending = FALSE;
			try_return(RC);
		}

		// The easy part is over. Now, we know that the user wishes to
		// delete the corresponding directory entry (of course, if this
		// is the only link to the file stream, any on-disk storage space
		// associated with the file stream will also be released when the
		// (only) link is deleted!)

		// Do some checking to see if the file can even be deleted.

		if (PtrFCB->FCBFlags & SFSD_FCB_DELETE_ON_CLOSE) {
			// All done!
			try_return(RC);
		}

		if (PtrFCB->FCBFlags & SFSD_FCB_READ_ONLY) {
			try_return(RC = STATUS_CANNOT_DELETE);
		}

		if (PtrVCB->VCBFlags & SFSD_VCB_FLAGS_VOLUME_READ_ONLY) {
			try_return(RC = STATUS_CANNOT_DELETE);
		}

		// An important step is to check if the file stream has been
		// mapped by any process. The delete cannot be allowed to proceed
		// in this case.
		if (!MmFlushImageSection(&(PtrFCB->NTRequiredFCB.SectionObject), MmFlushForDelete)) {
			try_return(RC = STATUS_CANNOT_DELETE);
		}

		// It would not be prudent to allow deletion of either a root
		// directory or a directory that is not empty.
		if (PtrFCB->FCBFlags & SFSD_FCB_ROOT_DIRECTORY) {
			try_return(RC = STATUS_CANNOT_DELETE);
		}

		if (PtrFCB->FCBFlags & SFSD_FCB_DIRECTORY) {
			// Perform your check to determine whether the directory
			// is empty or not.
			// if (!SFsdIsDirectoryEmpty(PtrFCB, PtrCCB, PtrIrpContext)) {
			//		try_return(RC = STATUS_DIRECTORY_NOT_EMPTY);
			// }
		}

		// Set a flag to indicate that this directory entry will become history
		// at cleanup.
		SFsdSetFlag(PtrFCB->FCBFlags, SFSD_FCB_DELETE_ON_CLOSE);
		PtrFileObject->DeletePending = TRUE;

		try_exit: NOTHING;
	} finally {
		;
	}
	return(RC);
}



/*************************************************************************
*
* Function: SFsdSetAllocationInformation()
*
* Description:
*	Mark/Unmark a file for deletion.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS	SFsdSetAllocationInformation(
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB,
PtrSFsdVCB					PtrVCB,
PFILE_OBJECT				PtrFileObject,
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PFILE_ALLOCATION_INFORMATION	PtrBuffer)
{
	NTSTATUS			RC = STATUS_SUCCESS;
	BOOLEAN			TruncatedFile = FALSE;
	BOOLEAN			ModifiedAllocSize = FALSE;

	try {
		// Increasing the allocation size associated with a file stream
		// is relatively easy. All you have to do is execute some FSD
		// specific code to check whether you have enough space available
		// (and if your FSD supports user/volume quotas, whether the user
		// is not exceeding quota), and then increase the file size in the
		// corresponding on-disk and in-memory structures.
		// Then, all you should do is inform the Cache Manager about the
		// increased allocation size.

		// First, do whatever error checking is appropriate here (e.g. whether
		// the caller is trying the change size for a directory, etc.).

		// Are we increasing the allocation size?
		if (RtlLargeIntegerLessThan(
				PtrFCB->NTRequiredFCB.CommonFCBHeader.AllocationSize,
				PtrBuffer->AllocationSize)) {

			// Yes. Do the FSD specific stuff i.e. increase reserved
			// space on disk.
			// RC = SFsdTruncateFileAllocationSize(...)

			ModifiedAllocSize = TRUE;

		} else if (RtlLargeIntegerGreaterThan(PtrFCB->NTRequiredFCB.CommonFCBHeader.AllocationSize,
																PtrBuffer->AllocationSize)) {
			// This is the painful part. See if the VMM will allow us to proceed.
			// The VMM will deny the request if:
			// (a) any image section exists OR
			// (b) a data section exists and the size of the user mapped view
			//		 is greater than the new size
			// Otherwise, the VMM should allow the request to proceed.
			if (!MmCanFileBeTruncated(&(PtrFCB->NTRequiredFCB.SectionObject), &(PtrBuffer->AllocationSize))) {
				// VMM said no way!
				try_return(RC = STATUS_USER_MAPPED_FILE);
			}

			// Perform your directory entry modifications. Release any on-disk
			// space you may need to in the process.
			// RC = SFsdTruncateFileAllocationSize(...);

			ModifiedAllocSize = TRUE;
			TruncatedFile = TRUE;
		}

		try_exit:

			// This is a good place to check if we have performed a truncate
			// operation. If we have perform a truncate (whether we extended
			// or reduced file size), you should update file time stamps.

			// Last, but not the lease, you must inform the Cache Manager of file size changes.
			if (ModifiedAllocSize && NT_SUCCESS(RC)) {
				// Update the FCB Header with the new allocation size.
				PtrFCB->NTRequiredFCB.CommonFCBHeader.AllocationSize = PtrBuffer->AllocationSize;

				// If we decreased the allocation size to less than the
				// current file size, modify the file size value.
				// Similarly, if we decreased the value to less than the
				// current valid data length, modify that value as well.
				if (TruncatedFile) {
					if (RtlLargeIntegerLessThan(PtrFCB->NTRequiredFCB.CommonFCBHeader.FileSize, PtrBuffer->AllocationSize)) {
						// Decrease the file size value.
						PtrFCB->NTRequiredFCB.CommonFCBHeader.FileSize = PtrBuffer->AllocationSize;
					}

					if (RtlLargeIntegerLessThan(PtrFCB->NTRequiredFCB.CommonFCBHeader.ValidDataLength, PtrBuffer->AllocationSize)) {
						// Decrease the valid data length value.
						PtrFCB->NTRequiredFCB.CommonFCBHeader.ValidDataLength = PtrBuffer->AllocationSize;
					}
				}


				// If the FCB has not had caching initiated, it is still valid
				// for you to invoke the NT Cache Manager. It is possible in such
				// situations for the call to be no'oped (unless some user has
				// mapped in the file)

				// NOTE: The invocation to CcSetFileSizes() will quite possibly
				//	result in a recursive call back into the file system.
				//	This is because the NT Cache Manager will typically
				//	perform a flush before telling the VMM to purge pages
				//	especially when caching has not been initiated on the
				//	file stream, but the user has mapped the file into
				//	the process' virtual address space.
				CcSetFileSizes(PtrFileObject, (PCC_FILE_SIZES)&(PtrFCB->NTRequiredFCB.CommonFCBHeader.AllocationSize));

				// Inform any pending IRPs (notify change directory).
			}

	} finally {
		;
	}
	return(RC);
}

