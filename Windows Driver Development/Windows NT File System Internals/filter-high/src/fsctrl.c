/*************************************************************************
*
* File: fsctrl.c
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*	File System Control Requests entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfilter.h"

/* define the file specific bug-check id */
#define			SFILTER_BUG_CHECK_ID				SFILTER_FILE_FSCTRL


/*************************************************************************
*
* Function: SFilterFSControl()
*
* Description:
*	Intercept the fsctrl request targeted to the FSD. If you wish to
*  have a user-space application (or kernel-mode driver) communicate
*  with the filter driver, this is where your FSCTL requests would also
*  end up.
*
* Expected Interrupt Level (for execution) :
*
*  Typically (though not guaranteed) @ IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterFSControl(
PDEVICE_OBJECT		DeviceObject,	// Our device object
PIRP					Irp)				// I/O Request Packet
{
	NTSTATUS							RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION			PtrNextIoStackLocation = NULL;
	PIO_STACK_LOCATION			PtrCurrentStackLocation = NULL;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;
	BOOLEAN							AcquiredExtension = FALSE;
	BOOLEAN							CompleteIrp = FALSE;
	PDEVICE_OBJECT					PtrTargetDeviceObject = NULL;
	ULONG								ReturnedInformation = 0;
	PtrSFilterMountCompletion	PtrMountCompletion = NULL;

	try {

		// Get the current I/O stack location.
		PtrCurrentStackLocation = IoGetCurrentIrpStackLocation(Irp);
		ASSERT(PtrCurrentStackLocation);

		// Get a pointer to the device extension that must exist for
		// all of the device objects created by the filter driver.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		// We really should not be here if our resource is not initialized?
		ASSERT(PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_RESOURCE_INITIALIZED);

		// acquire the resource associated with particular device extension for
		// our target device object.
		ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
		AcquiredExtension = TRUE;

		//	If the device extension is for a lower device forward the request.
		if ((PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE)
			&& (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED)) {

			// If we wish to intercept the mount volume, we would definitely like to
			// set a different completion routine. Here is the tedious way
			// of doing this (you could modify the common dispatch routine to
			// do the same).
			if (PtrCurrentStackLocation->MinorFunction != IRP_MN_MOUNT_VOLUME) {
				if (AcquiredExtension) {
					SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
					AcquiredExtension = FALSE;
				}

				// Note that a load FSD request could be being issued to a FS recognizer.
				// If you wish, you can monitor such requests and detach yourself from
				// the recognizer once the FSD has been loaded. That would allow the
				// recognizer to be unloaded.
				// This filter does not implement this behavior, however.
	
				RC = SFilterDefaultDispatch(DeviceObject, Irp);
				try_return(RC);
			} else {

				// Mount request issued.
				KdPrint(("SFilterFSControl(): Mount request intercepted. Target FSD = 0x%x\n",
								PtrDeviceExtension->TargetDriverObject));

				// Be careful about not screwing up badly. This is actually not recommended by the I/O Manager.
				if (Irp->CurrentLocation == 1) {
					// Bad!! Fudge the error code. Break if we can ...
					SFilterBreakPoint();
					CompleteIrp = TRUE;
					try_return(RC = STATUS_INVALID_DEVICE_REQUEST);
				}
	
				PtrNextIoStackLocation = IoGetNextIrpStackLocation(Irp);
	
				// So far, so good! Copy over the contents of the current stack
				// location into the next IRP stack location. Be careful about
				// how we do the copy. The following statement is convenient
				// but will end up screwing up any driver above us who has
				// set a completion routine!
				*PtrNextIoStackLocation = *PtrCurrentStackLocation;

				// We need to allocate memory for the context structure. The information
				// that we will pass on to the mount completion routine must contain
				// the following:
				// (a) A pointer to the device extension
				// (b) A pointer to the VPB structure that we can currently access
				//		 in the current I/O stack location.
				// (c) A work queue item that can be used to post the attach request
				//		 if the mount is successfull.
				// The structure will be freed once the post-processing is completed.
				PtrMountCompletion = (PtrSFilterMountCompletion)ExAllocatePool(NonPagedPool, sizeof(SFilterMountCompletion));
				if (!PtrMountCompletion) {
					CompleteIrp = TRUE;
					try_return(RC = STATUS_INSUFFICIENT_RESOURCES);
				}
				RtlZeroMemory(PtrMountCompletion, sizeof(SFilterMountCompletion));
				PtrMountCompletion->NodeIdentifier.NodeType = SFILTER_NODE_TYPE_MOUNT_COMPLETION;
            PtrMountCompletion->NodeIdentifier.NodeSize = sizeof(SFilterMountCompletion);
				PtrMountCompletion->PtrOriginalIrp = Irp;
				// We will store a pointer to the "real" device object. We can use the
				// VPB associated with the real device object once the mount operation
				// has completed. This *may* be the same VPB structure as the one passed in
				// by the I/O Manager to the FSD. However, it is possible that a "remount"
				// operation will be performed by the FSD in which case the FSD will replace
				// the VPB pointer passed-in by the I/O Manager with the "old" VPB pointer
				// that was already associated with the "real device object".
				// Bottom line is that we do not really know the "VPB" pointer until the
				// the FSD has processed the mount request (note that if this is NOT a
				// remount, the FSD will make the real device VPB pointer be the VPB pointer
				// passed in by the I/O Manager).
				PtrMountCompletion->PtrDeviceObject = PtrCurrentStackLocation->Parameters.MountVolume.Vpb->RealDevice;

				PtrMountCompletion->PtrDeviceExtension = PtrDeviceExtension;
				// might as well initialize the work item now ...
				ExInitializeWorkItem(&(PtrMountCompletion->WorkItem), SFilterMountAttach, PtrMountCompletion);
	
				// We will specify a default completion routine. This will
				// prevent any completion routine being invoked twice
				// (set by a driver above us in the calling hierarchy) and also
				// allow us the opportunity to do whatever we like once the
				// function processing has been completed.
				// We will specify that our completion routine be invoked regardless
				// of how the IRP is completed/cancelled.
				IoSetCompletionRoutine(Irp, SFilterMountVolumeCompletion, PtrMountCompletion, TRUE, TRUE, TRUE);
	
				// Forward the request. Note that if the target does not
				// wish to service the function, the request will get redirected
				// to IopInvalidDeviceRequest() (a routine that completes the
				// IRP with STATUS_INVALID_DEVICE_REQUEST).
				// However, we must release our resources before forwarding the
				// request. That will avoid the sort of problems discussed in
				// Chapter 12 of the text.
	
				PtrTargetDeviceObject = PtrDeviceExtension->TargetDeviceObject;
	
				// Increment the count of outstanding I/O requests. The count will
				// be decremented in the completion routine.
				// Acquire a special end-resource spin-lock to synchronize access.
				SFilterIncrementLargeInteger(PtrDeviceExtension->OutstandingIoRequests,
														(unsigned long)1,
														&(PtrDeviceExtension->IoRequestsSpinLock));
	
				// Clear the fast-IO notification event protected by the resource
				// we have acquired.
				KeClearEvent(&(PtrDeviceExtension->IoInProgressEvent));
	
				if (AcquiredExtension) {
					SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
					AcquiredExtension = FALSE;
				}

				// Note that we will mark the IRP Pending and will return
				// STATUS_MORE_PROCESSING_REQUIRED later in the completion
				// routine.

				IoMarkIrpPending(Irp);

				RC = IoCallDriver(PtrTargetDeviceObject, Irp);

				KdPrint(("SFilterFSControl(): Mount request intercepted. Target FSD Returned = 0x%x\n", RC));

				try_return(RC = STATUS_PENDING);
			}
		} else if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_FILTER_DEVICE) {
			// This request was targeted to us. You may allow such requests
			// if you expect other drivers/applications created/developed by
			// you or others to be able to issue IOCTL requests to your driver.
			// Currently, I expect no such thing.
			CompleteIrp = TRUE;
			try_return(RC = STATUS_INVALID_DEVICE_REQUEST);
		}

		// The filter driver does not particularly care to respond to these requests.
		CompleteIrp = TRUE;
		// does not matter what the Information field contains.
		try_return(RC = STATUS_INVALID_DEVICE_REQUEST);

		try_exit:	NOTHING;

	} finally {

		// Release any resources acquired.
		if (AcquiredExtension) {
			SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			AcquiredExtension = FALSE;
		}

		// Complete the IRP only if we must.
		if (CompleteIrp) {
			Irp->IoStatus.Status = RC;
			Irp->IoStatus.Information = ReturnedInformation;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
		}
	}

	return(RC);
}

/*************************************************************************
*
* Function: SFilterMountVolumeCompletion()
*
* Description:
*	Return STATUS_MORE_PROCESSING_REQUIRED if the request
*	was successfull (mount happened). Else, let it go.
*
* Expected Interrupt Level (for execution) :
*
*  Typically at high IRQL. Occasionally at IRQL PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/STATUS_MORE_PROCESSING_REQUIRED
*
*************************************************************************/
NTSTATUS SFilterMountVolumeCompletion(
PDEVICE_OBJECT			PtrDeviceObject,
PIRP						PtrIrp,
void						*Context)
{
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;
	BOOLEAN							CanDetachProceed = FALSE;
	PDEVICE_OBJECT					PtrAssociatedDeviceObject = NULL;
   PtrSFilterMountCompletion	PtrMountCompletion = NULL;

	PtrMountCompletion = (PtrSFilterMountCompletion)(Context);
	ASSERT(PtrMountCompletion);
	ASSERT(PtrMountCompletion->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_MOUNT_COMPLETION);
	ASSERT(PtrMountCompletion->NodeIdentifier.NodeSize = sizeof(SFilterMountCompletion));

	PtrDeviceExtension = PtrMountCompletion->PtrDeviceExtension;
	ASSERT(PtrDeviceExtension);

	if (PtrIrp->PendingReturned) {
		IoMarkIrpPending(PtrIrp);
	}

	// Get a pointer to the VPB that is the "real" VPB (whether a fresh mount
	// was performed or whether a real mount happened).
	PtrMountCompletion->PtrVPB = PtrMountCompletion->PtrDeviceObject->Vpb;

	PtrAssociatedDeviceObject = PtrDeviceExtension->PtrAssociatedDeviceObject;

	// Ensure that this is a valid device object pointer, else return
	// immediately.
	if (PtrAssociatedDeviceObject != PtrDeviceObject) {
		// Bug exposed; see Chapter 12 for details.
		return(STATUS_SUCCESS);
	}

	if (NT_SUCCESS(PtrIrp->IoStatus.Status)) {
		// Mount succeeded. We will process the remainder asynchronously
		// and will therefore return STATUS_MORE_PROCESSING_REQUIRED.

		// post the request and return.
      ExQueueWorkItem(&(PtrMountCompletion->WorkItem), CriticalWorkQueue);

		return(STATUS_MORE_PROCESSING_REQUIRED);
	} else {
		// who cares; could be that a FSD will be loaded later?
		SFilterDecrementLargeInteger(PtrDeviceExtension->OutstandingIoRequests,
												(unsigned long)1,
												&(PtrDeviceExtension->IoRequestsSpinLock));
	
		// If the outstanding count is 0, signal the appropriate event which will
		// allow any pending detach to proceed.
		SFilterIsLargeIntegerZero(CanDetachProceed, PtrDeviceExtension->OutstandingIoRequests,
												&(PtrDeviceExtension->IoRequestsSpinLock));
	
		if (CanDetachProceed) {
			// signal the event object. Note that this is simply an
			// advisory check we do here (to wake up a sleeping thread).
			// It is the responsibility of the thread performing the detach to
			// ensure that no operations are truly in progress.
			KeSetEvent(&(PtrDeviceExtension->IoInProgressEvent), IO_NO_INCREMENT, FALSE);
		}

		// free up the memory for the mount completion structure
		ExFreePool((void *)PtrMountCompletion);

		return(STATUS_SUCCESS);
	}
}


/*************************************************************************
*
* Function: SFilterMountAttach()
*
* Description:
*	Attach to the newly created device object representing the
*	mounted logical volume. Then complete the original IRP.
*
* Expected Interrupt Level (for execution) :
*
*  At IRQL PASSIVE_LEVEL
*
* Return Value: None.
*
*************************************************************************/
void SFilterMountAttach(
void						*Context)
{
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;
   PtrSFilterDeviceExtension	PtrNewDeviceExtension = NULL;
	BOOLEAN							CanDetachProceed = FALSE;
	PDEVICE_OBJECT					PtrTargetVolumeDeviceObject = NULL;
	PDEVICE_OBJECT					PtrNewDeviceObject = NULL;
   PtrSFilterMountCompletion	PtrMountCompletion = NULL;
	NTSTATUS							RC = STATUS_SUCCESS;

	SFilterBreakPoint();

	PtrMountCompletion = (PtrSFilterMountCompletion)(Context);
	ASSERT(PtrMountCompletion);
	ASSERT(PtrMountCompletion->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_MOUNT_COMPLETION);
	ASSERT(PtrMountCompletion->NodeIdentifier.NodeSize = sizeof(SFilterMountCompletion));

	// Get a pointer to the target device object.
	PtrTargetVolumeDeviceObject =	PtrMountCompletion->PtrVPB->DeviceObject;

	PtrDeviceExtension = PtrMountCompletion->PtrDeviceExtension;
	ASSERT(PtrDeviceExtension);

	// issue the attach request to the target device object.
	RC = SFilterAttachTarget(PtrTargetVolumeDeviceObject, &PtrNewDeviceObject);

	KdPrint(("SFilterFSDNotification(): Result of attaching to device object 0x%x is 0x%x\n",
					PtrTargetVolumeDeviceObject, RC));

	// if we succeeded, remove the "attached-to-fsd" flag (sorry for the hack)
	if (NT_SUCCESS(RC)) {
		ASSERT(PtrNewDeviceObject);

		PtrNewDeviceExtension = (PtrSFilterDeviceExtension)(PtrNewDeviceObject->DeviceExtension);

      SFilterClearFlag(PtrNewDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_ATTACHED_FSD);
	}

	// we are free to detach now ??
	SFilterDecrementLargeInteger(PtrDeviceExtension->OutstandingIoRequests,
											(unsigned long)1,
											&(PtrDeviceExtension->IoRequestsSpinLock));

	// If the outstanding count is 0, signal the appropriate event which will
	// allow any pending detach to proceed.
	SFilterIsLargeIntegerZero(CanDetachProceed, PtrDeviceExtension->OutstandingIoRequests,
											&(PtrDeviceExtension->IoRequestsSpinLock));

	if (CanDetachProceed) {
		// signal the event object. Note that this is simply an
		// advisory check we do here (to wake up a sleeping thread).
		// It is the responsibility of the thread performing the detach to
		// ensure that no operations are truly in progress.
		KeSetEvent(&(PtrDeviceExtension->IoInProgressEvent), IO_NO_INCREMENT, FALSE);
	}

	IoCompleteRequest(PtrMountCompletion->PtrOriginalIrp, IO_NO_INCREMENT);

	// free up the memory for the mount completion structure
	ExFreePool((void *)PtrMountCompletion);

	return;
}

