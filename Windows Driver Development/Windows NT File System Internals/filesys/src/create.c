/*************************************************************************
*
* File: create.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "Create"/"Open" dispatch entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_CREATE



/*************************************************************************
*
* Function: SFsdCreate()
*
* Description:
*	The I/O Manager will invoke this routine to handle a create/open
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
NTSTATUS SFsdCreate(
PDEVICE_OBJECT		DeviceObject,		// the logical volume device object
PIRP					Irp)					// I/O Request Packet
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext;
	BOOLEAN				AreWeTopLevel = FALSE;

	FsRtlEnterFileSystem();
	ASSERT(DeviceObject);
	ASSERT(Irp);

	// sometimes, we may be called here with the device object representing
	//	the file system (instead of the device object created for a logical
	//	volume. In this case, there is not much we wish to do (this create
	//	typically will happen 'cause some process has to open the FSD device
	//	object so as to be able to send an IOCTL to the FSD)

	//	All of the logical volume device objects we create have a device
	//	extension whereas the device object representing the FSD has no
	//	device extension. This seems like a good enough method to identify
	//	between the two device objects ...
	if (DeviceObject->Size == (unsigned short)(sizeof(DEVICE_OBJECT))) {
		// this is an open of the FSD itself
		Irp->IoStatus.Status = RC;
		Irp->IoStatus.Information = FILE_OPENED;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return(RC);
	}

	// set the top level context
	AreWeTopLevel = SFsdIsIrpTopLevel(Irp);

	try {

		// get an IRP context structure and issue the request
		PtrIrpContext = SFsdAllocateIrpContext(Irp, DeviceObject);
		ASSERT(PtrIrpContext);

		RC = SFsdCommonCreate(PtrIrpContext, Irp);

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
* Function: SFsdCommonCreate()
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
NTSTATUS SFsdCommonCreate(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION	PtrIoStackLocation = NULL;
   PIO_SECURITY_CONTEXT	PtrSecurityContext = NULL;
	PFILE_OBJECT			PtrNewFileObject = NULL;
	PFILE_OBJECT			PtrRelatedFileObject = NULL;
	uint32					AllocationSize = 0; 	// if we create a new file
	PFILE_FULL_EA_INFORMATION	PtrExtAttrBuffer = NULL;
	unsigned long			RequestedOptions = 0;
	unsigned long			RequestedDisposition = 0;
	uint8						FileAttributes = 0;
	unsigned short			ShareAccess = 0;
	unsigned long			ExtAttrLength = 0;
	ACCESS_MASK				DesiredAccess;

	BOOLEAN					DeferredProcessing = FALSE;

   PtrSFsdVCB				PtrVCB = NULL;
	BOOLEAN					AcquiredVCB = FALSE;

	BOOLEAN					DirectoryOnlyRequested = FALSE;
	BOOLEAN					FileOnlyRequested = FALSE;
	BOOLEAN					NoBufferingSpecified = FALSE;
	BOOLEAN					WriteThroughRequested = FALSE;
	BOOLEAN					DeleteOnCloseSpecified = FALSE;
	BOOLEAN					NoExtAttrKnowledge = FALSE;
	BOOLEAN					CreateTreeConnection = FALSE;
	BOOLEAN					OpenByFileId = FALSE;

	// Are we dealing with a page file?
	BOOLEAN					PageFileManipulation = FALSE;

	// Is this open for a target directory (used in rename operations)?
	BOOLEAN					OpenTargetDirectory = FALSE;

	// Should we ignore case when attempting to locate the object?
	BOOLEAN					IgnoreCaseWhenChecking = FALSE;

	PtrSFsdCCB				PtrRelatedCCB = NULL, PtrNewCCB = NULL;
	PtrSFsdFCB				PtrRelatedFCB = NULL, PtrNewFCB = NULL;

	unsigned long			ReturnedInformation;

	UNICODE_STRING			TargetObjectName;
	UNICODE_STRING			RelatedObjectName;

	UNICODE_STRING			AbsolutePathName;

	LARGE_INTEGER			FileAllocationSize, FileEndOfFile;


	ASSERT(PtrIrpContext);
	ASSERT(PtrIrp);

	try {

		AbsolutePathName.Buffer = NULL;
		AbsolutePathName.Length = AbsolutePathName.MaximumLength = 0;

		// First, get a pointer to the current I/O stack location
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(PtrIrp);
		ASSERT(PtrIoStackLocation);

		// If the caller cannot block, post the request to be handled
		//	asynchronously
		if (!(PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_CAN_BLOCK)) {
			// We must defer processing of this request since we could
			//	block anytime while performing the create/open ...
			RC = SFsdPostRequest(PtrIrpContext, PtrIrp);
         DeferredProcessing = TRUE;
			try_return(RC);
		}

		// Now, we can obtain the parameters specified by the user.
		//	Note that the file object is the new object created by the
		//	I/O Manager in anticipation that this create/open request
		//	will succeed.
		PtrNewFileObject	= PtrIoStackLocation->FileObject;
      TargetObjectName	= PtrNewFileObject->FileName;
		PtrRelatedFileObject = PtrNewFileObject->RelatedFileObject;

		// If a related file object is present, get the pointers
		//	to the CCB and the FCB for the related file object
		if (PtrRelatedFileObject) {
			PtrRelatedCCB = (PtrSFsdCCB)(PtrRelatedFileObject->FsContext2);
			ASSERT(PtrRelatedCCB);
			ASSERT(PtrRelatedCCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_CCB);
			// each CCB in turn points to a FCB
			PtrRelatedFCB = PtrRelatedCCB->PtrFCB;
			ASSERT(PtrRelatedFCB);
			ASSERT((PtrRelatedFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_FCB)
					 ||
  					 (PtrRelatedFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB));
			RelatedObjectName = PtrRelatedFileObject->FileName;
		}

		// Allocation size is only used if a new file is created
		//	or a file is superseded.
		AllocationSize    = PtrIrp->Overlay.AllocationSize.LowPart;
		// Note: Some FSD implementations support file sizes > 2 GB.
		//	The following check is only valid if your FSD does not support
		//	a large file size. With NT version 5.0, 64 bit support will
		//	become available and your FSD should ideally support large files ...
		if (PtrIrp->Overlay.AllocationSize.HighPart) {
			RC = STATUS_INVALID_PARAMETER;
			try_return(RC);
		}

		// Get a ptr to the supplied security context
		PtrSecurityContext = PtrIoStackLocation->Parameters.Create.SecurityContext;

		// The desired access can be obtained from the SecurityContext
		DesiredAccess = PtrSecurityContext->DesiredAccess;

		// Two values are supplied in the Create.Options field:
		//	(a) the actual user supplied options
		//	(b) the create disposition
		RequestedOptions = (PtrIoStackLocation->Parameters.Create.Options & FILE_VALID_OPTION_FLAGS);

		// The file disposition is packed with the user options ...
		//	Disposition includes FILE_SUPERSEDE, FILE_OPEN_IF, etc.
		RequestedDisposition = ((PtrIoStackLocation->Parameters.Create.Options >> 24) && 0xFF);

		FileAttributes	= (uint8)(PtrIoStackLocation->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS);
		ShareAccess	= PtrIoStackLocation->Parameters.Create.ShareAccess;

		// If your FSD does not support EA manipulation, you might return
		//	invalid parameter if the following are supplied.
		//	EA arguments are only used if a new file is created or a file is
		//	superseded
		PtrExtAttrBuffer	= PtrIrp->AssociatedIrp.SystemBuffer;
		ExtAttrLength		= PtrIoStackLocation->Parameters.Create.EaLength;

		// Get the options supplied by the user

		// User specifies that returned object MUST be a directory.
		//	Lack of presence of this flag does not mean it *cannot* be a
		//	directory *unless* FileOnlyRequested is set (see below)

		//	Presence of the flag however, does require that the returned object be
		//	a directory (container) object.
		DirectoryOnlyRequested = ((RequestedOptions & FILE_DIRECTORY_FILE) ? TRUE : FALSE);

		// User specifies that returned object MUST NOT be a directory.
		//	Lack of presence of this flag does not mean it *cannot* be a
		//	file *unless* DirectoryOnlyRequested is set (see above).

		//	Presence of the flag however does require that the returned object be
		//	a simple file (non-container) object.
		FileOnlyRequested = ((RequestedOptions & FILE_NON_DIRECTORY_FILE) ? TRUE : FALSE);

		// We cannot cache the file if the following flag is set.
		//	However, things do get a little bit interesting if caching
		//	has been already initiated due to a previous open ...
		//	(maintaining consistency then becomes a little bit more
		//	of a headache - see read/write file descriptions)
		NoBufferingSpecified = ((RequestedOptions & FILE_NO_INTERMEDIATE_BUFFERING) ? TRUE : FALSE);
	
		// Write-through simply means that the FSD must *not* return from
		//	a user write request until the data has been flushed to secondary
		//	storage (either to disks directly connected to the node or across
		//	the network in the case of a redirector)
		WriteThroughRequested = ((RequestedOptions & FILE_WRITE_THROUGH) ? TRUE : FALSE);

		// Not all of the native file system implementations support
		//	the delete-on-close option. All this means is that after the
		//	last close on the FCB has been performed, your FSD should
		//	delete the file. It simply saves the caller from issuing a
		//	separate delete request. Also, some FSD implementations might choose
		//	to implement a Windows NT idiosyncratic behavior wherein you
		//	could create such "delete-on-close" marked files under directories
		//	marked for deletion. Ordinarily, a FSD will not allow you to create
		//	a new file under a directory that has been marked for deletion.
		DeleteOnCloseSpecified = ((RequestedOptions & FILE_DELETE_ON_CLOSE) ? TRUE : FALSE);

		NoExtAttrKnowledge = ((RequestedOptions & FILE_NO_EA_KNOWLEDGE) ? TRUE : FALSE);

		// The following flag is only used by the LAN Manager redirector
		//	to	initiate a "new mapping" to a remote share. Typically,
		//	a FSD will not see this flag (especially disk based FSD's)
		CreateTreeConnection = ((RequestedOptions & FILE_CREATE_TREE_CONNECTION) ? TRUE : FALSE);

		// The NTFS file system for exmaple supports the OpenByFileId option.
		//	Your FSD may also be able to associate a unique numerical ID with
		//	an on-disk object. The caller would get this ID in a "query file
		//	information" call.

		//	Later, the caller might decide to reopen the object, this time
		//	though it may supply your FSD with the file identifier instead of
		//	a file/path name.
		OpenByFileId = ((RequestedOptions & FILE_OPEN_BY_FILE_ID) ? TRUE : FALSE);

		// Are we dealing with a page file?
		PageFileManipulation = ((PtrIoStackLocation->Flags & SL_OPEN_PAGING_FILE) ? TRUE : FALSE);

		// The open target directory flag is used as part of the sequence of
		//	operations performed by the I/O Manager is response to a file/dir
		//	rename operation. See the explanation in the book for details.
		OpenTargetDirectory = ((PtrIoStackLocation->Flags & SL_OPEN_TARGET_DIRECTORY) ? TRUE : FALSE);

		// If your FSD supports case-sensitive file name checks, you may
		//	choose to honor the following flag ...
      IgnoreCaseWhenChecking = ((PtrIoStackLocation->Flags & SL_CASE_SENSITIVE) ? TRUE : FALSE);

		// Ensure that the operation has been directed to a valid VCB ...
		PtrVCB =	(PtrSFsdVCB)(PtrIrpContext->TargetDeviceObject->DeviceExtension);
		ASSERT(PtrVCB);
		ASSERT(PtrVCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB);

		// Use coarse grained locking and acquire the VCB exclusively. This
		//	will lock out all other concurrent create/open requests
		// If the VCB is also acquired by the cleanup/close dispatch routines,
		// you can count on create/opens being synchronized with these functions
		// as well.
		ExAcquireResourceExclusiveLite(&(PtrVCB->VCBResource), TRUE);
      AcquiredVCB = TRUE;

		// Disk based file systems might decide to verify the logical volume
		//	(if required and only if removable media are supported) at this time

		//	Implement your own volume verification routine ...
		//	Read the DDK for more information on when a FSD must verify a
		//	volume ...
		//	if (!NT_SUCCESS(RC = SFsdVerifyVolume(PtrVCB))) {
		//		try_return(RC);
		//	}

		// If the volume has been locked, fail the request
		if (PtrVCB->VCBFlags & SFSD_VCB_FLAGS_VOLUME_LOCKED) {
			RC = STATUS_ACCESS_DENIED;
			try_return(RC);
		}

		// If a volume open is requested, satisfy it now
		if ((PtrNewFileObject->FileName.Length == 0) && ((PtrRelatedFileObject == NULL) ||
			  (PtrRelatedFCB->NodeIdentifier.NodeType == SFSD_NODE_TYPE_VCB))) {
			// If the supplied file name is NULL *and* either there exists
			//	no related file object *or* if a related file object was supplied
			//	but it too refers to a previously opened instance of a logical
			//	volume, this open must be for a logical volume.

			//	Note: your FSD might decide to do "special" things (whatever they
			//	might be) in response to an open request for the logical volume.

			//	Logical volume open requests are done primarily to get/set volume
			//	information, lock the volume, dismount the volume (using the IOCTL
			//	FSCTL_DISMOUNT_VOLUME) etc.

			//	If a volume open is requested, perform checks to ensure that
			//	invalid options have not also been specified ...
			if ((OpenTargetDirectory) || (PtrExtAttrBuffer)) {
				RC = STATUS_INVALID_PARAMETER;
				try_return(RC);
			}

			if (DirectoryOnlyRequested) {
				// a volume is not a directory
				RC = STATUS_NOT_A_DIRECTORY;
				try_return(RC);
			}

			if ((RequestedDisposition != FILE_OPEN) && (RequestedDisposition != FILE_OPEN_IF)) {
				// cannot create a new volume, I'm afraid ...
				RC = STATUS_ACCESS_DENIED;
				try_return(RC);
			}

			RC = SFsdOpenVolume(PtrVCB, PtrIrpContext, PtrIrp, ShareAccess, PtrSecurityContext, PtrNewFileObject);
			ReturnedInformation = PtrIrp->IoStatus.Information;

			try_return(RC);
		}

		// Your FSD might wish to implement the open-by-id option. The "id"
		//	is some unique numerical representation of the on-disk object.
		//	The caller then therefore give you this file id and your FSD
		//	should be completely capable of "opening" the object (it must
		//	exist since the caller received an id for the object from your
		//	FSD in a "query file" call ...

		//	If the file has been deleted in the meantime, you can return
		//	"not found"
		if (OpenByFileId) {
			// perform the open ...
			// RC = SFsdOpenByFileId(PtrIrpContext, PtrIrp ....);
			// try_return(RC);
		}

		// Now determine the starting point from which to begin the parsing
		if (PtrRelatedFileObject) {
			// We have a user supplied related file object.
			//	This implies a "relative" open i.e. relative to the directory
			//	represented by the related file object ...

			//	Note: The only purpose FSD implementations ever have for
			//	the related file object is to determine whether this
			//	is a relative open or not. At all other times (including
			//	during I/O operations), this field is meaningless from
			//	the FSD's perspective.
			if (!(PtrRelatedFCB->FCBFlags & SFSD_FCB_DIRECTORY)) {
				// we must have a directory as the "related" object
				RC = STATUS_INVALID_PARAMETER;
				try_return(RC);
			}

			// So we have a directory, ensure that the name begins with
			//	a "\" i.e. begins at the root and does *not* begin with a "\\"
			//	NOTE: This is just an example of the kind of path-name string
			//	validation that a FSD must do. Although the remainder of
			//	the code may not include such checks, any commercial
			//	FSD *must* include such checking (no one else, including
			//	the I/O Manager will perform checks on your FSD's behalf)
			if ((RelatedObjectName.Length == 0) || (RelatedObjectName.Buffer[0] != L'\\')) {
				RC = STATUS_INVALID_PARAMETER;
				try_return(RC);
			}

			// similarly, if the target file name starts with a "\", it
			//	is wrong since the target file name can no longer be absolute
			if ((TargetObjectName.Length != 0) && (TargetObjectName.Buffer[0] == L'\\')) {
				RC = STATUS_INVALID_PARAMETER;
				try_return(RC);
			}

			// Create an absolute path-name. You could potentially use
			//	the absolute path-name if you cache previously opened
			//	file/directory object names.
			{
				AbsolutePathName.MaximumLength = TargetObjectName.Length + RelatedObjectName.Length + sizeof(WCHAR);
				if (!(AbsolutePathName.Buffer = ExAllocatePool(PagedPool, AbsolutePathName.MaximumLength))) {
					RC = STATUS_INSUFFICIENT_RESOURCES;
					try_return(RC);
				}

				RtlZeroMemory(AbsolutePathName.Buffer, AbsolutePathName.MaximumLength);

				RtlCopyMemory((void *)(AbsolutePathName.Buffer), (void *)(RelatedObjectName.Buffer), RelatedObjectName.Length);
				AbsolutePathName.Length = RelatedObjectName.Length;
				RtlAppendUnicodeToString(&AbsolutePathName, L"\\");
				RtlAppendUnicodeToString(&AbsolutePathName, TargetObjectName.Buffer);
			}

		} else {

			// The suplied path-name must be an absolute path-name i.e.
			//	starting at the root of the file system tree
         if (TargetObjectName.Buffer[0] != L'\\') {
				RC = STATUS_INVALID_PARAMETER;
				try_return(RC);
			}

			{
				AbsolutePathName.MaximumLength = TargetObjectName.Length;
				if (!(AbsolutePathName.Buffer = ExAllocatePool(PagedPool, AbsolutePathName.MaximumLength))) {
					RC = STATUS_INSUFFICIENT_RESOURCES;
					try_return(RC);
				}

				RtlZeroMemory(AbsolutePathName.Buffer, AbsolutePathName.MaximumLength);

				RtlCopyMemory((void *)(AbsolutePathName.Buffer), (void *)(TargetObjectName.Buffer), TargetObjectName.Length);
				AbsolutePathName.Length = TargetObjectName.Length;
			}
		}

		// go into a loop parsing the supplied name

		//	Use the algorithm supplied in the book to implement this
		//	loop.

		//	Note that you may have to "open" intermediate directory objects
		//	while traversing the path. You should try to reuse existing code
		//	whenever possible therefore you should consider using a common
		//	open routine regardless of whether the open is on behalf of the
		//	caller or an intermediate (internal) open performed by your driver.

		//	But first, check if the caller simply wishes to open the root
		//	of the file system tree.
		if (AbsolutePathName.Length == 2) {
			// this is an open of the root directory, ensure that	the caller has not requested a file only
			if (FileOnlyRequested || (RequestedDisposition == FILE_SUPERSEDE) || (RequestedDisposition == FILE_OVERWRITE) ||
				 (RequestedDisposition == FILE_OVERWRITE_IF)) {
				RC = STATUS_FILE_IS_A_DIRECTORY;
				try_return(RC);
			}

			// Insert code to open root directory here. Include creation of a new CCB structure.
			//	e.g. RC = SFsdOpenRootDirectory(...);

			try_return(RC);
		}

		if (PtrRelatedFileObject) {
			// Insert code such that your "start directory" is
			//	the one identified by the related file object
		} else {
			// Insert code to start at the root of the file system
		}

		// NOTE: If your FSD does not support access checking i.e.
		//	your FSD does not check "traversal" privileges,
		//	you could easily maintain a "prefix" cache containing
		//	path-names and open FCB pointers. Then, if the requested
		//	path-name is already present in the cache i.e. someone
		//	had opened it earlier, you can avoid the tedious traversal
		//	of the entire path-name performed below and described in
		//	the book ...

		//	If you do not maintain such a prefix table cache of previously
		//	opened object names or if you do not find the name to be opened
		//	in the cache, then:

		//	Get the "next" component in the name to be parsed. Note that
		//	obtaining the next string component is similar to the strtok
		//	library routine where the separator is a "\"

		//	Your FSD should also always check the validity of the token
		//	to ensure that only valid characters comprise the path/file name

		//	Insert code to open the starting directory here.

		while (TRUE) {
			// Insert code to perform the following tasks here:

			//	(a) acquire the parent directory FCB MainResource exclusively
			//	(b) ensure that the parent directory in which you will perform
			//		 a lookup operation is indeed a directory
			//	(c) if there are no more components left after this one in the
			//		 pathname supplied by the user, break;
			//	(d) attempt to lookup the sub-directory in the parent
			//	(e) if not found, return STATUS_OBJECT_PATH_NOT_FOUND
			//	(f) Otherwise, open the new sub-directory and make it
			//		 the "parent"
			//	(g) go back and repeat the loop for the next component in
			//		 the path

			//	NOTE: If your FSD supports it, you should always check
			//	that the caller has appropriate privileges to traverse
			//	the directories being searched.
		}

		// Now we are down to the last component, check it out to see if it exists ...

		// If "open target directory" was specified
		if (OpenTargetDirectory) {
			if (NT_SUCCESS(RC)) {
				// file exists, set this information in the Information field
				ReturnedInformation = FILE_EXISTS;
			} else {
				RC = STATUS_SUCCESS;

				// Tell the I/O Manager that file does not exit
				ReturnedInformation = FILE_DOES_NOT_EXIST;
			}

			// Now, do the following:

			//	(a) Replace the string in the FileName field in the
			//		 PtrNewFileObject to identify the "target name"
			//		 only (sans the path leading to the object)
				{
					unsigned int	Index = (AbsolutePathName.Length - 1);

					// Back up until we come to the last '\'
					// But first, skip any trailing '\' characters

					while (AbsolutePathName.Buffer[Index] == L'\\') {
						ASSERT(Index >= sizeof(WCHAR));
						Index -= sizeof(WCHAR);
						// Skip this length also
						PtrNewFileObject->FileName.Length -= sizeof(WCHAR);
					}

					while (AbsolutePathName.Buffer[Index] != L'\\') {
						// Keep backing up until we hit one
						ASSERT(Index >= sizeof(WCHAR));
						Index -= sizeof(WCHAR);
					}

					// We must be at a '\' character
					ASSERT(AbsolutePathName.Buffer[Index] == L'\\');
					Index++;

					// We can now determine the new length of the filename
					// and copy the name over
					PtrNewFileObject->FileName.Length -= (unsigned short)(Index*sizeof(WCHAR));
					RtlCopyMemory(&(PtrNewFileObject->FileName.Buffer[0]),
									  &(PtrNewFileObject->FileName.Buffer[Index]),
                             PtrNewFileObject->FileName.Length);
				}

			//	(b) Return with the target's parent directory opened
			//	(c) Update the file object FsContext and FsContext2 fields
			//		 to reflect the fact that the parent directory of the
			//		 target has been opened

			try_return(RC);
		}

		if (!NT_SUCCESS(RC)) {
			// Object was not found, create if requested
			if ((RequestedDisposition == FILE_CREATE) || (RequestedDisposition == FILE_OPEN_IF) ||
				 (RequestedDisposition == FILE_OVERWRITE_IF)) {
				// Create a new file/directory here ...
				// Note that a FCB structure will be allocated at this time
				// and so will a CCB structure. Assume that these are called
				// PtrNewFCB and PtrNewCCB respectively.
				// Further, note that since the file is being created, no other
				// thread can have the file stream open at this time.

				// Open the newly created object

				// Set the allocation size for the object is specified
	
				// Set extended attributes for the file ...

				// Set the Share Access for the file stream.
				// The FCBShareAccess field will be set by the I/O Manager.
				IoSetShareAccess(DesiredAccess, ShareAccess, PtrNewFileObject, &(PtrNewFCB->FCBShareAccess));

				RC = STATUS_SUCCESS;
				ReturnedInformation = FILE_CREATED;
			}

			try_return(RC);

		} else {
			if (RequestedDisposition == FILE_CREATE) {
				ReturnedInformation = FILE_EXISTS;
				RC = STATUS_OBJECT_NAME_COLLISION;
				try_return(RC);
			}

			// Insert code to open the target here, return if failed

			// The FSD will allocate a new FCB structure if no such structure
			// currently exists in memory for the file stream.
			// A new CCB will always be allocated.
			// Assume that these structures are named PtrNewFCB and PtrNewCCB
			// respectively.
			// Further, you should obtain the FCB MainResource exclusively
			// at this time.

			// Check if caller wanted a directory only and target object
			//	not a directory, or caller wanted a file only and target
			//	object not a file ...
			if (FileOnlyRequested && (PtrNewFCB->FCBFlags & SFSD_FCB_DIRECTORY)) {
				// Close the new FCB and leave.
				//	SFsdCloseCCB(PtrNewCCB);
				RC = STATUS_FILE_IS_A_DIRECTORY;
				try_return(RC);
			}

			if ((PtrNewFCB->FCBFlags & SFSD_FCB_DIRECTORY) && ((RequestedDisposition == FILE_SUPERSEDE) ||
				  (RequestedDisposition == FILE_OVERWRITE) || (RequestedDisposition == FILE_OVERWRITE_IF))) {
				RC = STATUS_FILE_IS_A_DIRECTORY;
				try_return(RC);
			}

			if (DirectoryOnlyRequested && !(PtrNewFCB->FCBFlags & SFSD_FCB_DIRECTORY)) {
				// Close the new FCB and leave.
				//	SFsdCloseCCB(PtrNewCCB);
				RC = STATUS_NOT_A_DIRECTORY;
				try_return(RC);
			}

			// Check share access and fail if the share conflicts with an existing
			// open.
			if (PtrNewFCB->OpenHandleCount > 0) {
				// The FCB is currently in use by some thread.
				// We must check whether the requested access/share access
				// conflicts with the existing open operations.

				if (!NT_SUCCESS(RC = IoCheckShareAccess(DesiredAccess, ShareAccess, PtrNewFileObject,
												&(PtrNewFCB->FCBShareAccess), TRUE))) {
					// SFsdCloseCCB(PtrNewCCB);
					try_return(RC);
				}
			} else {
					IoSetShareAccess(DesiredAccess, ShareAccess, PtrNewFileObject, &(PtrNewFCB->FCBShareAccess));
			}

			ReturnedInformation = FILE_OPENED;

			// If a supersede or overwrite was requested, do so now ...
			if (RequestedDisposition == FILE_SUPERSEDE) {
				// Attempt the operation here ...
				//	RC = SFsdSupersede(...);
				if (NT_SUCCESS(RC)) {
					ReturnedInformation = FILE_SUPERSEDED;
				}
			} else if ((RequestedDisposition == FILE_OVERWRITE) || (RequestedDisposition == FILE_OVERWRITE_IF)){
				// Attempt the operation here ...
				//	RC = SFsdOverwrite(...);
				if (NT_SUCCESS(RC)) {
					ReturnedInformation = FILE_OVERWRITTEN;
				}
			}
		}

		try_exit:	NOTHING;

	} finally {
		// Complete the request unless we are here as part of unwinding
		//	when an exception condition was encountered, OR
		//	if the request has been deferred (i.e. posted for later handling)
		if (RC != STATUS_PENDING) {
			// If we acquired any FCB resources, release them now ...

			// If any intermediate (directory) open operations were performed,
			//	implement the corresponding close (do *not* however close
			//	the target you have opened on behalf of the caller ...).

			if (NT_SUCCESS(RC)) {
				// Update the file object such that:
				//	(a) the FsContext field points to the NTRequiredFCB field
				//		 in the FCB
				//	(b) the FsContext2 field points to the CCB created as a
				//		 result of the open operation

				// If write-through was requested, then mark the file object
				//	appropriately
				if (WriteThroughRequested) {
               PtrNewFileObject->Flags |= FO_WRITE_THROUGH;
				}

			} else {
				// Perform failure related post-processing now
			}

			// As long as this unwinding is not being performed as a result of
			//	an exception condition, complete the IRP ...
			if (!(PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_EXCEPTION)) {
				PtrIrp->IoStatus.Status = RC;
				PtrIrp->IoStatus.Information = ReturnedInformation;

				// Free up the Irp Context
				SFsdReleaseIrpContext(PtrIrpContext);
	
				// complete the IRP
				IoCompleteRequest(PtrIrp, IO_DISK_INCREMENT);
			}
		}

		if (AcquiredVCB) {
			ASSERT(PtrVCB);
         SFsdReleaseResource(&(PtrVCB->VCBResource));
			AcquiredVCB = FALSE;
		}

		if (AbsolutePathName.Buffer != NULL) {
			ExFreePool(AbsolutePathName.Buffer);
		}
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFsdOpenVolume()
*
* Description:
*	Open a logical volume for the caller.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFsdOpenVolume(
PtrSFsdVCB				PtrVCB,					// volume to be opened
PtrSFsdIrpContext		PtrIrpContext,			// IRP context
PIRP						PtrIrp,					// original/user IRP
unsigned short			ShareAccess,			// share access
PIO_SECURITY_CONTEXT	PtrSecurityContext,	// caller's context (incl access)
PFILE_OBJECT			PtrNewFileObject)		// I/O Mgr. created file object
{
	NTSTATUS				RC = STATUS_SUCCESS;
	PtrSFsdCCB			PtrCCB = NULL;

	try {
		// check for exclusive open requests (using share modes supplied)
		//	and determine whether it is even possible to open the volume
		//	with the specified share modes (e.g. if caller does not
		//	wish to share read or share write ...)
	
		//	Use IoCheckShareAccess() and IoSetShareAccess() here ...	
		//	They are defined in the DDK.

		//	You might also wish to check the caller's security context
		//	to see whether you wish to allow the volume open or not.
		//	Use the SeAccessCheck() routine described in the DDK for	this purpose.
	
		// create a new CCB structure
		if (!(PtrCCB = SFsdAllocateCCB())) {
			RC = STATUS_INSUFFICIENT_RESOURCES;
			try_return(RC);
		}

		// initialize the CCB
		PtrCCB->PtrFCB = (PtrSFsdFCB)(PtrVCB);
		InsertTailList(&(PtrVCB->VolumeOpenListHead), &(PtrCCB->NextCCB));

		// initialize the CCB to point to the file object
		PtrCCB->PtrFileObject = PtrNewFileObject;

		SFsdSetFlag(PtrCCB->CCBFlags, SFSD_CCB_VOLUME_OPEN);

		// initialize the file object appropriately
		PtrNewFileObject->FsContext = (void *)(PtrVCB);
		PtrNewFileObject->FsContext2 = (void *)(PtrCCB);

		// increment the number of outstanding open operations on this
		//	logical volume (i.e. volume cannot be dismounted)

		//	You might be concerned about 32 bit wrap-around though I would
		//	argue that it is unlikely ... :-)
		(PtrVCB->VCBOpenCount)++;
	
		// now set the IoStatus Information value correctly in the IRP
		//	(caller will set the status field)
		PtrIrp->IoStatus.Information = FILE_OPENED;
	
		try_exit:	NOTHING;
	} finally {
		NOTHING;
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFsdInitializeFCB()
*
* Description:
*	Initialize a new FCB structure and also the sent-in file object
*	(if supplied)
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdInitializeFCB(
PtrSFsdFCB				PtrNewFCB,		// FCB structure to be initialized
PtrSFsdVCB				PtrVCB,			// logical volume (VCB) pointer
PtrSFsdObjectName		PtrObjectName,	// name of the object
uint32					Flags,			// is this a file/directory, etc.
PFILE_OBJECT			PtrFileObject)	// optional file object to be initialized
{
	// Initialize the disk dependent portion as you see fit

	// Initialize the two ERESOURCE objects
	ExInitializeResourceLite(&(PtrNewFCB->NTRequiredFCB.MainResource));
	ExInitializeResourceLite(&(PtrNewFCB->NTRequiredFCB.PagingIoResource));

	PtrNewFCB->PtrVCB = PtrVCB;

	// caller MUST ensure that VCB has been acquired exclusively
	InsertTailList(&(PtrVCB->NextFCB), &(PtrNewFCB->NextFCB));

	// initialize the various list heads
	InitializeListHead(&(PtrNewFCB->NextCCB));

	PtrNewFCB->ReferenceCount = 1;
	PtrNewFCB->OpenHandleCount = 1;

	PtrNewFCB->FCBFlags = Flags;

	PtrNewFCB->FCBName = PtrObjectName;

	if (PtrFileObject) {
		PtrFileObject->FsContext = (void *)(&(PtrNewFCB->NTRequiredFCB));
	}

	return;
}

