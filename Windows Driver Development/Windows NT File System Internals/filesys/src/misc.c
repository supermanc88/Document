/*************************************************************************
*
* File: misc.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	This file contains some miscellaneous support routines.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_MISC


/*************************************************************************
*
* Function: SFsdInitializeZones()
*
* Description:
*	Allocates some memory for global zones used to allocate FSD structures.
*	Either all memory will be allocated or we will back out gracefully.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFsdInitializeZones(
void)
{
	NTSTATUS				RC = STATUS_SUCCESS;
	uint32				SizeOfZone = SFsdGlobalData.DefaultZoneSizeInNumStructs;
	uint32				SizeOfObjectNameZone = 0;
	uint32				SizeOfCCBZone = 0;
	uint32				SizeOfFCBZone = 0;
	uint32				SizeOfByteLockZone = 0;
	uint32				SizeOfIrpContextZone = 0;

	try {

		// initialize the spinlock protecting the zones
		KeInitializeSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock));

		// determine memory requirements
		switch (MmQuerySystemSize()) {
		case MmSmallSystem:
			// this is just for illustration purposes. I will multiply
			//	number of structures with some arbitrary amount depending
			//	upon available memory in the system ... You should choose a
			// more intelligent method suitable to your memory consumption
			// and the amount of memory available.
			SizeOfObjectNameZone = (2 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdObjectName))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfCCBZone = (2 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdCCB))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfFCBZone = (2 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdFCB))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfByteLockZone = (2 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdFileLockInfo))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfIrpContextZone = (2 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdIrpContext))) + sizeof(ZONE_SEGMENT_HEADER);
			break;
		case MmMediumSystem:
			SizeOfObjectNameZone = (4 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdObjectName))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfCCBZone = (4 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdCCB))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfFCBZone = (4 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdFCB))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfByteLockZone = (4 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdFileLockInfo))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfIrpContextZone = (4 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdIrpContext))) + sizeof(ZONE_SEGMENT_HEADER);
			break;
		case MmLargeSystem:
			SizeOfObjectNameZone = (8 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdObjectName))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfCCBZone = (8 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdCCB))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfFCBZone = (8 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdFCB))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfByteLockZone = (8 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdFileLockInfo))) + sizeof(ZONE_SEGMENT_HEADER);
			SizeOfIrpContextZone = (8 * SizeOfZone * SFsdQuadAlign(sizeof(SFsdIrpContext))) + sizeof(ZONE_SEGMENT_HEADER);
			break;
		}

		// typical NT methodology (at least until *someone* exposed the "difference" between a server and workstation ;-)
		if (MmIsThisAnNtAsSystem()) {
			SizeOfObjectNameZone *= SFSD_NTAS_MULTIPLE;
			SizeOfCCBZone *= SFSD_NTAS_MULTIPLE;
			SizeOfFCBZone *= SFSD_NTAS_MULTIPLE;
			SizeOfByteLockZone *= SFSD_NTAS_MULTIPLE;
			SizeOfIrpContextZone *= SFSD_NTAS_MULTIPLE;
		}

		// allocate memory for each of the zones and initialize the	zones ...
		if (!(SFsdGlobalData.ObjectNameZone = ExAllocatePool(NonPagedPool, SizeOfObjectNameZone))) {
			RC = STATUS_INSUFFICIENT_RESOURCES;
			try_return(RC);
		}

		if (!(SFsdGlobalData.CCBZone = ExAllocatePool(NonPagedPool, SizeOfCCBZone))) {
			RC = STATUS_INSUFFICIENT_RESOURCES;
			try_return(RC);
		}

		if (!(SFsdGlobalData.FCBZone = ExAllocatePool(NonPagedPool, SizeOfFCBZone))) {
			RC = STATUS_INSUFFICIENT_RESOURCES;
			try_return(RC);
		}

		if (!(SFsdGlobalData.ByteLockZone = ExAllocatePool(NonPagedPool, SizeOfByteLockZone))) {
			RC = STATUS_INSUFFICIENT_RESOURCES;
			try_return(RC);
		}

		if (!(SFsdGlobalData.IrpContextZone = ExAllocatePool(NonPagedPool, SizeOfIrpContextZone))) {
			RC = STATUS_INSUFFICIENT_RESOURCES;
			try_return(RC);
		}

		// initialize each of the zone headers ...
		if (!NT_SUCCESS(RC = ExInitializeZone(&(SFsdGlobalData.ObjectNameZoneHeader),
					SFsdQuadAlign(sizeof(SFsdObjectName)),
					SFsdGlobalData.ObjectNameZone, SizeOfObjectNameZone))) {
			// failed the initialization, leave ...
			try_return(RC);
		}

		if (!NT_SUCCESS(RC = ExInitializeZone(&(SFsdGlobalData.CCBZoneHeader),
					SFsdQuadAlign(sizeof(SFsdCCB)),
					SFsdGlobalData.CCBZone,
					SizeOfCCBZone))) {
			// failed the initialization, leave ...
			try_return(RC);
		}

		if (!NT_SUCCESS(RC = ExInitializeZone(&(SFsdGlobalData.FCBZoneHeader),
					SFsdQuadAlign(sizeof(SFsdFCB)),
					SFsdGlobalData.FCBZone,
					SizeOfFCBZone))) {
			// failed the initialization, leave ...
			try_return(RC);
		}

		if (!NT_SUCCESS(RC = ExInitializeZone(&(SFsdGlobalData.ByteLockZoneHeader),
					SFsdQuadAlign(sizeof(SFsdFileLockInfo)),
					SFsdGlobalData.ByteLockZone,
					SizeOfByteLockZone))) {
			// failed the initialization, leave ...
			try_return(RC);
		}

		if (!NT_SUCCESS(RC = ExInitializeZone(&(SFsdGlobalData.IrpContextZoneHeader),
					SFsdQuadAlign(sizeof(SFsdIrpContext)),
					SFsdGlobalData.IrpContextZone,
					SizeOfIrpContextZone))) {
			// failed the initialization, leave ...
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		if (!NT_SUCCESS(RC)) {
			// invoke the destroy routine now ...
			SFsdDestroyZones();
		} else {
			// mark the fact that we have allocated zones ...
			SFsdSetFlag(SFsdGlobalData.SFsdFlags, SFSD_DATA_FLAGS_ZONES_INITIALIZED);
		}
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFsdDestroyZones()
*
* Description:
*	Free up the previously allocated memory. NEVER do this once the
*	driver has been successfully loaded.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdDestroyZones(
void)
{
	try {
		// free up each of the pools
		ExFreePool(SFsdGlobalData.ObjectNameZone);
		ExFreePool(SFsdGlobalData.CCBZone);
		ExFreePool(SFsdGlobalData.FCBZone);
		ExFreePool(SFsdGlobalData.ByteLockZone);
		ExFreePool(SFsdGlobalData.IrpContextZone);

		try_exit:	NOTHING;

	} finally {
		SFsdClearFlag(SFsdGlobalData.SFsdFlags, SFSD_DATA_FLAGS_ZONES_INITIALIZED);
	}

	return;
}


/*************************************************************************
*
* Function: SFsdIsIrpTopLevel()
*
* Description:
*	Helps the FSD determine who the "top level" caller is for this
*	request. A request can originate directly from a user process
*	(in which case, the "top level" will be NULL when this routine
*	is invoked), OR the user may have originated either from the NT
*	Cache Manager/VMM ("top level" may be set), or this could be a
*	recursion into our code in which we would have set the "top level"
*	field the last time around.
*
* Expected Interrupt Level (for execution) :
*
*  whatever level a particular dispatch routine is invoked at.
*
* Return Value: TRUE/FALSE (TRUE if top level was NULL when routine invoked)
*
*************************************************************************/
BOOLEAN SFsdIsIrpTopLevel(
PIRP			Irp)			// the IRP sent to our dispatch routine
{
	BOOLEAN			ReturnCode = FALSE;

	if (IoGetTopLevelIrp() == NULL) {
 		// OK, so we can set ourselves to become the "top level" component
		IoSetTopLevelIrp(Irp);
		ReturnCode = TRUE;
	}

	return(ReturnCode);
}


/*************************************************************************
*
* Function: SFsdExceptionFilter()
*
* Description:
*	This routines allows the driver to determine whether the exception
*	is an "allowed" exception i.e. one we should not-so-quietly consume
*	ourselves, or one which should be propagated onwards in which case
*	we will most likely bring down the machine.
*
*	This routine employs the services of FsRtlIsNtstatusExpected(). This
*	routine returns a BOOLEAN result. A RC of FALSE will cause us to return
*	EXCEPTION_CONTINUE_SEARCH which will probably cause a panic.
*	The FsRtl.. routine returns FALSE iff exception values are (currently) :
*		STATUS_DATATYPE_MISALIGNMENT	||	STATUS_ACCESS_VIOLATION	||
*		STATUS_ILLEGAL_INSTRUCTION	||	STATUS_INSTRUCTION_MISALIGNMENT
*
* Expected Interrupt Level (for execution) :
*
*  ?
*
* Return Value: EXCEPTION_EXECUTE_HANDLER/EXECEPTION_CONTINUE_SEARCH
*
*************************************************************************/
long SFsdExceptionFilter(
PtrSFsdIrpContext				PtrIrpContext,
PEXCEPTION_POINTERS			PtrExceptionPointers)
{
	long							ReturnCode = EXCEPTION_EXECUTE_HANDLER;
	NTSTATUS						ExceptionCode = STATUS_SUCCESS;

	// figure out the exception code
	ExceptionCode = PtrExceptionPointers->ExceptionRecord->ExceptionCode;

	if ((ExceptionCode == STATUS_IN_PAGE_ERROR) && (PtrExceptionPointers->ExceptionRecord->NumberParameters >= 3)) {
		ExceptionCode = PtrExceptionPointers->ExceptionRecord->ExceptionInformation[2];
	}

	if (PtrIrpContext) {
		PtrIrpContext->SavedExceptionCode = ExceptionCode;
		SFsdSetFlag(PtrIrpContext->IrpContextFlags, SFSD_IRP_CONTEXT_EXCEPTION);
	}

	// check if we should propagate this exception or not
	if (!(FsRtlIsNtstatusExpected(ExceptionCode))) {
		// we are not ok, propagate this exception.
		//	NOTE: we will bring down the machine ...
		ReturnCode = EXCEPTION_CONTINUE_SEARCH;

		// better free up the IrpContext now ...
		if (PtrIrpContext) {
			SFsdReleaseIrpContext(PtrIrpContext);
		}
	}

	// if you wish to perform some special processing when
	//	not propagating the exception, set up the state for
	//	special processing now ...

	// return the appropriate code
	return(ReturnCode);
}


/*************************************************************************
*
* Function: SFsdExceptionHandler()
*
* Description:
*	One of the routines in the FSD or in the modules we invoked encountered
*	an exception. We have decided that we will "handle" the exception.
*	Therefore we will prevent the machine from a panic ...
*	You can do pretty much anything you choose to in your commercial
*	driver at this point to ensure a graceful exit. In the sample
*	driver, I will simply free up the IrpContext (if any), set the
*	error code in the IRP and complete the IRP at this time ...
*
* Expected Interrupt Level (for execution) :
*
*  ?
*
* Return Value: Error code
*
*************************************************************************/
NTSTATUS SFsdExceptionHandler(
PtrSFsdIrpContext				PtrIrpContext,
PIRP								Irp)
{
	NTSTATUS						RC;

	ASSERT(Irp);

	if (PtrIrpContext) {
		RC = PtrIrpContext->SavedExceptionCode;
		// Free irp context here
		SFsdReleaseIrpContext(PtrIrpContext);
	} else {
		// must be insufficient resources ...?
		RC = STATUS_INSUFFICIENT_RESOURCES;
	}

	// set the error code in the IRP
	Irp->IoStatus.Status = RC;
	Irp->IoStatus.Information = 0;

	// complete the IRP
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return(RC);
}



/*************************************************************************
*
* Function: SFsdLogEvent()
*
* Description:
*	Log a message in the NT Event Log. This is a rather simplistic log
*	methodology since you can potentially utilize the event log to
*	provide a lot of information to the user (and you should too!)
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdLogEvent(
NTSTATUS					SFsdEventLogId,		// the SFsd private message id
NTSTATUS					RC)						// any NT error code we wish to log ...
{
	try {

		// Implement a call to IoAllocateErrorLogEntry() followed by a call
		// to IoWriteErrorLogEntry(). You should note that the call to IoWriteErrorLogEntry()
		// will free memory for the entry once the write completes (which in actuality
		// is an asynchronous operation).

	} except (EXCEPTION_EXECUTE_HANDLER) {
		// nothing really we can do here, just do not wish to crash ...
		NOTHING;
	}

	return;
}


/*************************************************************************
*
* Function: SFsdAllocateObjectName()
*
* Description:
*	Allocate a new ObjectName structure to represent an open on-disk object.
*	Also initialize the ObjectName structure to NULL.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: A pointer to the ObjectName structure OR NULL.
*
*************************************************************************/
PtrSFsdObjectName SFsdAllocateObjectName(
void)
{
	PtrSFsdObjectName			PtrObjectName = NULL;
	BOOLEAN						AllocatedFromZone = TRUE;
   KIRQL							CurrentIrql;

	// first, try to allocate out of the zone
	KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
	if (!ExIsFullZone(&(SFsdGlobalData.ObjectNameZoneHeader))) {
		// we have enough memory
		PtrObjectName = (PtrSFsdObjectName)ExAllocateFromZone(&(SFsdGlobalData.ObjectNameZoneHeader));

		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);

		// if we failed to obtain from the zone, get it directly from the VMM
		PtrObjectName = (PtrSFsdObjectName)ExAllocatePool(NonPagedPool, SFsdQuadAlign(sizeof(SFsdObjectName)));
      AllocatedFromZone = FALSE;
	}

	// if we could not obtain the required memory, bug-check.
	//	Do NOT do this in your commercial driver, instead handle the error gracefully ...
	if (!PtrObjectName) {
		SFsdPanic(STATUS_INSUFFICIENT_RESOURCES, SFsdQuadAlign(sizeof(SFsdObjectName)), 0);
	}

	// zero out the allocated memory block
	RtlZeroMemory(PtrObjectName, SFsdQuadAlign(sizeof(SFsdObjectName)));

	// set up some fields ...
	PtrObjectName->NodeIdentifier.NodeType	= SFSD_NODE_TYPE_OBJECT_NAME;
	PtrObjectName->NodeIdentifier.NodeSize	= SFsdQuadAlign(sizeof(SFsdObjectName));


	if (!AllocatedFromZone) {
		SFsdSetFlag(PtrObjectName->ObjectNameFlags, SFSD_OB_NAME_NOT_FROM_ZONE);
	}

	return(PtrObjectName);
}


/*************************************************************************
*
* Function: SFsdReleaseObjectName()
*
* Description:
*	Deallocate a previously allocated structure.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdReleaseObjectName(
PtrSFsdObjectName				PtrObjectName)
{
	KIRQL							CurrentIrql;

	ASSERT(PtrObjectName);

	// give back memory either to the zone or to the VMM
	if (!(PtrObjectName->ObjectNameFlags & SFSD_OB_NAME_NOT_FROM_ZONE)) {
		// back to the zone
		KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
      ExFreeToZone(&(SFsdGlobalData.ObjectNameZoneHeader), PtrObjectName);
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		ExFreePool(PtrObjectName);
	}

	return;
}


/*************************************************************************
*
* Function: SFsdAllocateCCB()
*
* Description:
*	Allocate a new CCB structure to represent an open on-disk object.
*	Also initialize the CCB structure to NULL.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: A pointer to the CCB structure OR NULL.
*
*************************************************************************/
PtrSFsdCCB SFsdAllocateCCB(
void)
{
	PtrSFsdCCB					PtrCCB = NULL;
	BOOLEAN						AllocatedFromZone = TRUE;
   KIRQL							CurrentIrql;

	// first, try to allocate out of the zone
	KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
	if (!ExIsFullZone(&(SFsdGlobalData.CCBZoneHeader))) {
		// we have enough memory
		PtrCCB = (PtrSFsdCCB)ExAllocateFromZone(&(SFsdGlobalData.CCBZoneHeader));

		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);

		// if we failed to obtain from the zone, get it directly from the VMM
		PtrCCB = (PtrSFsdCCB)ExAllocatePool(NonPagedPool, SFsdQuadAlign(sizeof(SFsdCCB)));
      AllocatedFromZone = FALSE;
	}

	// if we could not obtain the required memory, bug-check.
	//	Do NOT do this in your commercial driver, instead handle the error gracefully ...
	if (!PtrCCB) {
		SFsdPanic(STATUS_INSUFFICIENT_RESOURCES, SFsdQuadAlign(sizeof(SFsdCCB)), 0);
	}

	// zero out the allocated memory block
	RtlZeroMemory(PtrCCB, SFsdQuadAlign(sizeof(SFsdCCB)));

	// set up some fields ...
	PtrCCB->NodeIdentifier.NodeType	= SFSD_NODE_TYPE_CCB;
	PtrCCB->NodeIdentifier.NodeSize	= SFsdQuadAlign(sizeof(SFsdCCB));


	if (!AllocatedFromZone) {
		SFsdSetFlag(PtrCCB->CCBFlags, SFSD_CCB_NOT_FROM_ZONE);
	}

	return(PtrCCB);
}


/*************************************************************************
*
* Function: SFsdReleaseCCB()
*
* Description:
*	Deallocate a previously allocated structure.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdReleaseCCB(
PtrSFsdCCB						PtrCCB)
{
	KIRQL							CurrentIrql;

	ASSERT(PtrCCB);

	// give back memory either to the zone or to the VMM
	if (!(PtrCCB->CCBFlags & SFSD_CCB_NOT_FROM_ZONE)) {
		// back to the zone
		KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
      ExFreeToZone(&(SFsdGlobalData.CCBZoneHeader), PtrCCB);
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		ExFreePool(PtrCCB);
	}

	return;
}


/*************************************************************************
*
* Function: SFsdAllocateFCB()
*
* Description:
*	Allocate a new FCB structure to represent an open on-disk object.
*	Also initialize the FCB structure to NULL.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: A pointer to the FCB structure OR NULL.
*
*************************************************************************/
PtrSFsdFCB SFsdAllocateFCB(
void)
{
	PtrSFsdFCB					PtrFCB = NULL;
	BOOLEAN						AllocatedFromZone = TRUE;
   KIRQL							CurrentIrql;

	// first, try to allocate out of the zone
	KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
	if (!ExIsFullZone(&(SFsdGlobalData.FCBZoneHeader))) {
		// we have enough memory
		PtrFCB = (PtrSFsdFCB)ExAllocateFromZone(&(SFsdGlobalData.FCBZoneHeader));

		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);

		// if we failed to obtain from the zone, get it directly from the VMM
		PtrFCB = (PtrSFsdFCB)ExAllocatePool(NonPagedPool, SFsdQuadAlign(sizeof(SFsdFCB)));
      AllocatedFromZone = FALSE;
	}

	// if we could not obtain the required memory, bug-check.
	//	Do NOT do this in your commercial driver, instead handle the error gracefully ...
	if (!PtrFCB) {
		SFsdPanic(STATUS_INSUFFICIENT_RESOURCES, SFsdQuadAlign(sizeof(SFsdFCB)), 0);
	}

	// zero out the allocated memory block
	RtlZeroMemory(PtrFCB, SFsdQuadAlign(sizeof(SFsdFCB)));

	// set up some fields ...
	PtrFCB->NodeIdentifier.NodeType	= SFSD_NODE_TYPE_FCB;
	PtrFCB->NodeIdentifier.NodeSize	= SFsdQuadAlign(sizeof(SFsdFCB));


	if (!AllocatedFromZone) {
		SFsdSetFlag(PtrFCB->FCBFlags, SFSD_FCB_NOT_FROM_ZONE);
	}

	return(PtrFCB);
}



/*************************************************************************
*
* Function: SFsdCreateNewFCB()
*
* Description:
*	We want to create a new FCB. We will also create a new CCB (presumably)
*	later. Simply allocate a new FCB structure and initialize fields
*	appropriately.
*	This function also takes the file size values that the caller must
*	have obtained and	will set the file size fields appropriately in the
*	CommonFCBHeader.
*	Finally, this routine will initialize the FileObject structure passed
*	in to this function. If you decide to fail the call later, remember
*	to uninitialize the fields.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: A pointer to the FCB structure OR NULL.
*
*************************************************************************/
NTSTATUS SFsdCreateNewFCB(
PtrSFsdFCB				*ReturnedFCB,
PLARGE_INTEGER			AllocationSize,
PLARGE_INTEGER			EndOfFile,
PFILE_OBJECT			PtrFileObject,
PtrSFsdVCB				PtrVCB)
{
	NTSTATUS							RC = STATUS_SUCCESS;
   PtrSFsdFCB						PtrFCB = NULL;
   PtrSFsdNTRequiredFCB			PtrReqdFCB = NULL;
   PFSRTL_COMMON_FCB_HEADER	PtrCommonFCBHeader = NULL;

	try {
		// Obtain a new FCB structure.
		// The function SFsdAllocateFCB() will obtain a new structure either
		// from a zone or from memory requested directly from the VMM.
		PtrFCB = SFsdAllocateFCB();
		if (!PtrFCB) {
			// Assume lack of memory.
			try_return(RC = STATUS_INSUFFICIENT_RESOURCES);
		}

		// Initialize fields required to interface with the NT Cache Manager.
		// Note that the returned structure has already been zeroed. This means
		// that the SectionObject structure has been zeroed which is a
		// requirement for newly created FCB structures.
		PtrReqdFCB = &(PtrFCB->NTRequiredFCB);

		// Initialize the MainResource and PagingIoResource structures now.
		ExInitializeResourceLite(&(PtrReqdFCB->MainResource));
		SFsdSetFlag(PtrFCB->FCBFlags, SFSD_INITIALIZED_MAIN_RESOURCE);

		ExInitializeResourceLite(&(PtrReqdFCB->PagingIoResource));
		SFsdSetFlag(PtrFCB->FCBFlags, SFSD_INITIALIZED_PAGING_IO_RESOURCE);

		// Start initializing the fields contained in the CommonFCBHeader.
		PtrCommonFCBHeader = &(PtrReqdFCB->CommonFCBHeader);

		// Allow fast-IO for now.
		PtrCommonFCBHeader->IsFastIoPossible = FastIoIsPossible;

		// Initialize the MainResource and PagingIoResource pointers in
		// the CommonFCBHeader structure to point to the ERESOURCE structures we
		// have allocated and already initialized above.
		PtrCommonFCBHeader->Resource = &(PtrReqdFCB->MainResource);
		PtrCommonFCBHeader->PagingIoResource = &(PtrReqdFCB->PagingIoResource);

		// Ignore the Flags field in the CommonFCBHeader for now. Part 3
		// of the book describes it in greater detail.

		// Initialize the file size values here.
		PtrCommonFCBHeader->AllocationSize = *(AllocationSize);
		PtrCommonFCBHeader->FileSize = *(EndOfFile);

		// The following will disable ValidDataLength support. However, your
		// FSD may choose to support this concept.
		PtrCommonFCBHeader->ValidDataLength.LowPart = 0xFFFFFFFF;
		PtrCommonFCBHeader->ValidDataLength.HighPart = 0x7FFFFFFF;

		//	Initialize other fields for the FCB here ...
		PtrFCB->PtrVCB = PtrVCB;
		InitializeListHead(&(PtrFCB->NextCCB));

		// Initialize fields contained in the file object now.
		PtrFileObject->PrivateCacheMap = NULL;
		// Note that we could have just as well taken the value of PtrReqdFCB
		// directly below. The bottom line however is that the FsContext
		// field must point to a FSRTL_COMMON_FCB_HEADER structure.
		PtrFileObject->FsContext = (void *)(PtrCommonFCBHeader);

		// Other initialization continues here ...

		try_exit:	NOTHING;
	} finally {
		;
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFsdReleaseFCB()
*
* Description:
*	Deallocate a previously allocated structure.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdReleaseFCB(
PtrSFsdFCB						PtrFCB)
{
	KIRQL							CurrentIrql;

	ASSERT(PtrFCB);

	// give back memory either to the zone or to the VMM
	if (!(PtrFCB->FCBFlags & SFSD_FCB_NOT_FROM_ZONE)) {
		// back to the zone
		KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
      ExFreeToZone(&(SFsdGlobalData.FCBZoneHeader), PtrFCB);
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		ExFreePool(PtrFCB);
	}

	return;
}


/*************************************************************************
*
* Function: SFsdAllocateByteLocks()
*
* Description:
*	Allocate a new byte range lock structure and initialize it to NULL.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: A pointer to the SFsdByteLocks structure OR NULL.
*
*************************************************************************/
PtrSFsdFileLockInfo SFsdAllocateByteLocks(
void)
{
	PtrSFsdFileLockInfo		PtrByteLocks = NULL;
	BOOLEAN						AllocatedFromZone = TRUE;
   KIRQL							CurrentIrql;

	// first, try to allocate out of the zone
	KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
	if (!ExIsFullZone(&(SFsdGlobalData.ByteLockZoneHeader))) {
		// we have enough memory
		PtrByteLocks = (PtrSFsdFileLockInfo)ExAllocateFromZone(&(SFsdGlobalData.ByteLockZoneHeader));

		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);

		// if we failed to obtain from the zone, get it directly from the VMM
		PtrByteLocks = (PtrSFsdFileLockInfo)ExAllocatePool(NonPagedPool, SFsdQuadAlign(sizeof(SFsdFileLockInfo)));
      AllocatedFromZone = FALSE;
	}

	// if we could not obtain the required memory, bug-check.
	//	Do NOT do this in your commercial driver, instead handle the error gracefully ...
	if (!PtrByteLocks) {
		SFsdPanic(STATUS_INSUFFICIENT_RESOURCES, SFsdQuadAlign(sizeof(SFsdFileLockInfo)), 0);
	}

	// zero out the allocated memory block
	RtlZeroMemory(PtrByteLocks, SFsdQuadAlign(sizeof(PtrSFsdFileLockInfo)));

	if (!AllocatedFromZone) {
		SFsdSetFlag(PtrByteLocks->FileLockFlags, SFSD_BYTE_LOCK_NOT_FROM_ZONE);
	}

	return(PtrByteLocks);
}


/*************************************************************************
*
* Function: SFsdReleaseByteLocks()
*
* Description:
*	Deallocate a previously allocated structure.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdReleaseByteLocks(
PtrSFsdFileLockInfo					PtrByteLocks)
{
	KIRQL							CurrentIrql;

	ASSERT(PtrByteLocks);

	// give back memory either to the zone or to the VMM
	if (!(PtrByteLocks->FileLockFlags & SFSD_BYTE_LOCK_NOT_FROM_ZONE)) {
		// back to the zone
		KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
      ExFreeToZone(&(SFsdGlobalData.ByteLockZoneHeader), PtrByteLocks);
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		ExFreePool(PtrByteLocks);
	}

	return;
}


/*************************************************************************
*
* Function: SFsdAllocateIrpContext()
*
* Description:
*	The sample FSD creates an IRP context for each request received. This
*	routine simply allocates (and initializes to NULL) a SFsdIrpContext
*	structure.
*	Most of the fields in the context structure are then initialized here.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: A pointer to the IrpContext structure OR NULL.
*
*************************************************************************/
PtrSFsdIrpContext SFsdAllocateIrpContext(
PIRP					Irp,
PDEVICE_OBJECT		PtrTargetDeviceObject)
{
	PtrSFsdIrpContext			PtrIrpContext = NULL;
	BOOLEAN						AllocatedFromZone = TRUE;
   KIRQL							CurrentIrql;
	PIO_STACK_LOCATION		PtrIoStackLocation = NULL;

	// first, try to allocate out of the zone
	KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
	if (!ExIsFullZone(&(SFsdGlobalData.IrpContextZoneHeader))) {
		// we have enough memory
		PtrIrpContext = (PtrSFsdIrpContext)ExAllocateFromZone(&(SFsdGlobalData.IrpContextZoneHeader));

		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		// release the spinlock
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);

		// if we failed to obtain from the zone, get it directly from the VMM
		PtrIrpContext = (PtrSFsdIrpContext)ExAllocatePool(NonPagedPool, SFsdQuadAlign(sizeof(SFsdIrpContext)));
      AllocatedFromZone = FALSE;
	}

	// if we could not obtain the required memory, bug-check.
	//	Do NOT do this in your commercial driver, instead handle	the error gracefully ...
	if (!PtrIrpContext) {
		SFsdPanic(STATUS_INSUFFICIENT_RESOURCES, SFsdQuadAlign(sizeof(SFsdIrpContext)), 0);
	}

	// zero out the allocated memory block
	RtlZeroMemory(PtrIrpContext, SFsdQuadAlign(sizeof(SFsdIrpContext)));

	// set up some fields ...
	PtrIrpContext->NodeIdentifier.NodeType	= SFSD_NODE_TYPE_IRP_CONTEXT;
	PtrIrpContext->NodeIdentifier.NodeSize	= SFsdQuadAlign(sizeof(SFsdIrpContext));


	PtrIrpContext->Irp = Irp;
	PtrIrpContext->TargetDeviceObject = PtrTargetDeviceObject;

	// copy over some fields from the IRP and set appropriate flag values
	if (Irp) {
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(Irp);
		ASSERT(PtrIoStackLocation);

		PtrIrpContext->MajorFunction = PtrIoStackLocation->MajorFunction;
		PtrIrpContext->MinorFunction = PtrIoStackLocation->MinorFunction;

		// Often, a FSD cannot honor a request for asynchronous processing
		// of certain critical requests. For example, a "close" request on
		// a file object can typically never be deferred. Therefore, do not
		// be surprised if sometimes your FSD (just like all other FSD
		// implementations on the Windows NT system) has to override the flag
		// below.
		if (IoIsOperationSynchronous(Irp)) {
			SFsdSetFlag(PtrIrpContext->IrpContextFlags, SFSD_IRP_CONTEXT_CAN_BLOCK);
		}
	}

	if (!AllocatedFromZone) {
		SFsdSetFlag(PtrIrpContext->IrpContextFlags, SFSD_IRP_CONTEXT_NOT_FROM_ZONE);
	}

	// Are we top-level ? This information is used by the dispatching code
	// later (and also by the FSD dispatch routine)
	if (IoGetTopLevelIrp() != Irp) {
		// We are not top-level. Note this fact in the context structure
		SFsdSetFlag(PtrIrpContext->IrpContextFlags, SFSD_IRP_CONTEXT_NOT_TOP_LEVEL);
	}

	return(PtrIrpContext);
}


/*************************************************************************
*
* Function: SFsdReleaseIrpContext()
*
* Description:
*	Deallocate a previously allocated structure.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdReleaseIrpContext(
PtrSFsdIrpContext					PtrIrpContext)
{
	KIRQL							CurrentIrql;

	ASSERT(PtrIrpContext);

	// give back memory either to the zone or to the VMM
	if (!(PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_NOT_FROM_ZONE)) {
		// back to the zone
		KeAcquireSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), &CurrentIrql);
      ExFreeToZone(&(SFsdGlobalData.IrpContextZoneHeader), PtrIrpContext);
		KeReleaseSpinLock(&(SFsdGlobalData.ZoneAllocationSpinLock), CurrentIrql);
	} else {
		ExFreePool(PtrIrpContext);
	}

	return;
}


/*************************************************************************
*
* Function: SFsdPostRequest()
*
* Description:
*	Queue up a request for deferred processing (in the context of a system
*	worker thread). The caller must have locked the user buffer (if required)
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_PENDING
*
*************************************************************************/
NTSTATUS SFsdPostRequest(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS			RC = STATUS_PENDING;

	// mark the IRP pending
	IoMarkIrpPending(PtrIrp);

	// queue up the request
	ExInitializeWorkItem(&(PtrIrpContext->WorkQueueItem), SFsdCommonDispatch, PtrIrpContext);

	ExQueueWorkItem(&(PtrIrpContext->WorkQueueItem), CriticalWorkQueue);

	// return status pending
	return(RC);
}


/*************************************************************************
*
* Function: SFsdCommonDispatch()
*
* Description:
*	The common dispatch routine invoked in the context of a system worker
*	thread. All we do here is pretty much case off the major function
*	code and invoke the appropriate FSD dispatch routine for further
*	processing.
*
* Expected Interrupt Level (for execution) :
*
*	IRQL PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdCommonDispatch(
void						*Context)	// actually an IRPContext structure
{
	NTSTATUS						RC = STATUS_SUCCESS;
	PtrSFsdIrpContext			PtrIrpContext = NULL;
	PIRP							PtrIrp = NULL;

	// The context must be a pointer to an IrpContext structure
	PtrIrpContext = (PtrSFsdIrpContext)Context;
	ASSERT(PtrIrpContext);

	// Assert that the Context is legitimate
	if ((PtrIrpContext->NodeIdentifier.NodeType != SFSD_NODE_TYPE_IRP_CONTEXT) || (PtrIrpContext->NodeIdentifier.NodeSize != SFsdQuadAlign(sizeof(SFsdIrpContext)))) {
		// This does not look good
		SFsdPanic(SFSD_ERROR_INTERNAL_ERROR, PtrIrpContext->NodeIdentifier.NodeType, PtrIrpContext->NodeIdentifier.NodeSize);
	}

	//	Get a pointer to the IRP structure
	PtrIrp = PtrIrpContext->Irp;
	ASSERT(PtrIrp);

	// Now, check if the FSD was top level when the IRP was originally invoked
	// and set the thread context (for the worker thread) appropriately
	if (PtrIrpContext->IrpContextFlags & SFSD_IRP_CONTEXT_NOT_TOP_LEVEL) {
		// The FSD is not top level for the original request
		// Set a constant value in TLS to reflect this fact
		IoSetTopLevelIrp((PIRP)FSRTL_FSP_TOP_LEVEL_IRP);
	}

	// Since the FSD routine will now be invoked in the context of this worker
	// thread, we should inform the FSD that it is perfectly OK to block in
	// the context of this thread
	SFsdSetFlag(PtrIrpContext->IrpContextFlags, SFSD_IRP_CONTEXT_CAN_BLOCK);

	FsRtlEnterFileSystem();

	try {

		// Pre-processing has been completed; check the Major Function code value
		// either in the IrpContext (copied from the IRP), or directly from the
		//	IRP itself (we will need a pointer to the stack location to do that),
		//	Then, switch based on the value on the Major Function code
		switch (PtrIrpContext->MajorFunction) {
		case IRP_MJ_CREATE:
			// Invoke the common create routine
			(void)SFsdCommonCreate(PtrIrpContext, PtrIrp);
			break;
		case IRP_MJ_READ:
			// Invoke the common read routine
			(void)SFsdCommonRead(PtrIrpContext, PtrIrp);
			break;
		// Continue with the remaining possible dispatch routines below ...
		default:
			// This is the case where we have an invalid major function
			PtrIrp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
			PtrIrp->IoStatus.Information = 0;
	
			IoCompleteRequest(PtrIrp, IO_NO_INCREMENT);
			break;
		}
	} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {

		RC = SFsdExceptionHandler(PtrIrpContext, PtrIrp);

		SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);
	}

	// Enable preemption
	FsRtlExitFileSystem();

	// Ensure that the "top-level" field is cleared
	IoSetTopLevelIrp(NULL);

	return;
}


/*************************************************************************
*
* Function: SFsdInitializeVCB()
*
* Description:
*	Perform the initialization for a VCB structure.
*
* Expected Interrupt Level (for execution) :
*
*	IRQL PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdInitializeVCB(
PDEVICE_OBJECT			PtrVolumeDeviceObject,
PDEVICE_OBJECT			PtrTargetDeviceObject,
PVPB						PtrVPB)
{
	NTSTATUS				RC = STATUS_SUCCESS;
	PtrSFsdVCB			PtrVCB = NULL;
	BOOLEAN				VCBResourceInitialized = FALSE;

	PtrVCB = (PtrSFsdVCB)(PtrVolumeDeviceObject->DeviceExtension);

	// Zero it out (typically this has already been done by the I/O
	// Manager but it does not hurt to do it again)!
	RtlZeroMemory(PtrVCB, sizeof(SFsdVCB));

	// Initialize the signature fields
	PtrVCB->NodeIdentifier.NodeType = SFSD_NODE_TYPE_VCB;
	PtrVCB->NodeIdentifier.NodeSize = sizeof(SFsdVCB);

	// Initialize the ERESOURCE object.
	RC = ExInitializeResourceLite(&(PtrVCB->VCBResource));
	ASSERT(NT_SUCCESS(RC));
	VCBResourceInitialized = TRUE;

	// We know the target device object.
	// Note that this is not neccessarily a pointer to the actual
	// physical/virtual device on which the logical volume should
	// be mounted. This is actually a pointer to either the actual
	// (real) device or to any device object that may have been
	// attached to it. Any IRPs that we send down should be sent to this
	// device object. However, the "real" physical/virtual device object
	// on which we perform our mount operation can be determined from the
	// RealDevice field in the VPB sent to us.
	PtrVCB->TargetDeviceObject = PtrTargetDeviceObject;

	// We also have a pointer to the newly created device object representing
	// this logical volume (remember that this VCB structure is simply an
	// extension of the created device object).
	PtrVCB->VCBDeviceObject = PtrVolumeDeviceObject;

	// We also have the VPB pointer. This was obtained from the
	// Parameters.MountVolume.Vpb field in the current I/O stack location
	// for the mount IRP.
	PtrVCB->PtrVPB = PtrVPB;

	// Initialize the list anchor (head) for some lists in this VCB.
	InitializeListHead(&(PtrVCB->NextFCB));
	InitializeListHead(&(PtrVCB->NextNotifyIRP));
	InitializeListHead(&(PtrVCB->VolumeOpenListHead));

	// Initialize the notify IRP list mutex
	KeInitializeMutex(&(PtrVCB->NotifyIRPMutex), 0);

	// Set the initial file size values appropriately. Note that your FSD may
	// wish to guess at the initial amount of information you would like to
	// read from the disk until you have really determined that this a valid
	// logical volume (on disk) that you wish to mount.
	// PtrVCB->FileSize = PtrVCB->AllocationSize = ??

	// You typically do not want to bother with valid data length callbacks
	// from the Cache Manager for the file stream opened for volume metadata
	// information
	PtrVCB->ValidDataLength.LowPart = 0xFFFFFFFF;
	PtrVCB->ValidDataLength.HighPart = 0x7FFFFFFF;

	// Create a stream file object for this volume.
	PtrVCB->PtrStreamFileObject = IoCreateStreamFileObject(NULL,
												PtrVCB->PtrVPB->RealDevice);
	ASSERT(PtrVCB->PtrStreamFileObject);

	// Initialize some important fields in the newly created file object.
	PtrVCB->PtrStreamFileObject->FsContext = (void *)PtrVCB;
   PtrVCB->PtrStreamFileObject->FsContext2 = NULL;
   PtrVCB->PtrStreamFileObject->SectionObjectPointer = &(PtrVCB->SectionObject);

	PtrVCB->PtrStreamFileObject->Vpb = PtrVPB;

	// Link this chap onto the global linked list of all VCB structures.
	ExAcquireResourceExclusiveLite(&(SFsdGlobalData.GlobalDataResource), TRUE);
	InsertTailList(&(SFsdGlobalData.NextVCB), &(PtrVCB->NextVCB));

	// Initialize caching for the stream file object.
	CcInitializeCacheMap(PtrVCB->PtrStreamFileObject, (PCC_FILE_SIZES)(&(PtrVCB->AllocationSize)),
								TRUE,		// We will use pinned access.
								&(SFsdGlobalData.CacheMgrCallBacks), PtrVCB);
								
	SFsdReleaseResource(&(SFsdGlobalData.GlobalDataResource));

	// Mark the fact that this VCB structure is initialized.
   SFsdSetFlag(PtrVCB->VCBFlags, SFSD_VCB_FLAGS_VCB_INITIALIZED);

	return;
}


