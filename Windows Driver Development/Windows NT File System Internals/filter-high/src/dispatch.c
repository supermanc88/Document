/*************************************************************************
*
* File: dispatch.c
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*	Dispatch entry points.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfilter.h"

/* define the file specific bug-check id */
#define			SFILTER_BUG_CHECK_ID				SFILTER_FILE_DISPATCH


/*************************************************************************
*
* Function: SFilterDefaultDispatch()
*
* Description:
*	Simply forward the request to the device we are attached to.
*	Return INVALID_REQUEST if we are not attached to anything.
*  Before forwarding the request, set up a completion routine
*  that can be used to do interesting stuff (if so desired).
*
* Expected Interrupt Level (for execution) :
*
*  Typically (though not guaranteed) @ IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterDefaultDispatch(
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

			// We will specify a default completion routine. This will
			// prevent any completion routine being invoked twice
			// (set by a driver above us in the calling hierarchy) and also
			// allow us the opportunity to do whatever we like once the
			// function processing has been completed.
			// We will specify that our completion routine be invoked regardless
			// of how the IRP is completed/cancelled.
  			IoSetCompletionRoutine(Irp, SFilterDefaultCompletion, PtrDeviceExtension, TRUE, TRUE, TRUE);

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

			KdPrint(("SFilterDefaultDispatch(): Target Device Object = 0x%x, Major function = 0x%x, Minor Function = 0x%x\n",
							PtrTargetDeviceObject, PtrCurrentStackLocation->MajorFunction, PtrCurrentStackLocation->MinorFunction));
	
			if (AcquiredExtension) {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
				AcquiredExtension = FALSE;
			}

			RC = IoCallDriver(PtrTargetDeviceObject, Irp);

			// Note that at this time, the filter driver completion routine
			// does not return STATUS_MORE_PROCESSING_REQUIRED. However, if you
			// do modify this code and use it in your own filter driver and if your
			// completion routine *could* return the STATUS_MORE_PROCESSING_REQUIRED
			// return code, you must not blindly return the return-code obtained from
			// the call to IoCallDriver() above. See Chapter 12 for a discussion of
			// this issue.
			try_return(RC);
		}

		// The filter driver does not particularly care to respond to these requests.
		CompleteIrp = TRUE;
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
* Function: SFilterDefaultCompletion()
*
* Description:
*	Decrement the outstanding I/O count. Then	let it go. Note that you
*	must always be aware that the completion routine could be invoked
*	at high IRQL and in the context of some arbitrary thread.
*
* Expected Interrupt Level (for execution) :
*
*  Typically at high IRQL. Occasionally at IRQL PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/STATUS_MORE_PROCESSING_REQUIRED
*
*************************************************************************/
NTSTATUS SFilterDefaultCompletion(
PDEVICE_OBJECT			PtrDeviceObject,
PIRP						PtrIrp,
void						*Context)
{
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;
	BOOLEAN							CanDetachProceed = FALSE;
	PDEVICE_OBJECT					PtrAssociatedDeviceObject = NULL;

	// acquire the device extension resource exclusively.
	PtrDeviceExtension = (PtrSFilterDeviceExtension)(Context);
	ASSERT(PtrDeviceExtension);

	if (PtrIrp->PendingReturned) {
		IoMarkIrpPending(PtrIrp);
	}

	PtrAssociatedDeviceObject = PtrDeviceExtension->PtrAssociatedDeviceObject;

	// Ensure that this is a valid device object pointer, else return
	// immediately.
	if (PtrAssociatedDeviceObject != PtrDeviceObject) {
		// Bug exposed; see Chapter 12 for details.
		return(STATUS_SUCCESS);
	}

	// Note that you could do all sorts of processing at this point
	// depending upon the results of the operation. Be careful though
	// about what you chose to do, about the fact that this completion
	// routine is being invoked in an arbitrary thread context and probably
	// at high IRQL.

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

	// Although the success return value is hard-coded here, you can
	// return an appropriate value (either success or more-processing-reqd)
	// based upon what it is that you wish to do in your completion routine.
	return(STATUS_SUCCESS);
}

