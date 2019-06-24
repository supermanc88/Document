/*************************************************************************
*
* File: attach.c
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*		Deals with attaching to and detaching from a target device
*		object. Creates a device object as well to be used in the
*		attach process.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfilter.h"

/* define the file specific bug-check id */
#define			SFILTER_BUG_CHECK_ID				SFILTER_FILE_ATTACH


/*************************************************************************
*
* Function: SFilterAttachTarget()
*
* Description:
*	We have the target FSD device object. We will create a new device object
*	of our own and attach this newly created device object to the target FSD
*	device object (representing the FSD itself or a mounted logical volume).
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterAttachTarget(
PDEVICE_OBJECT				PtrTargetDeviceObject,
PDEVICE_OBJECT				*PtrReturnedDeviceObject)
{

	NTSTATUS							RC = STATUS_SUCCESS;
	PDEVICE_OBJECT					PtrNewDeviceObject = NULL;	// device object we create.
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;
	BOOLEAN							InitializedDeviceObject = FALSE;
	BOOLEAN							AcquiredDeviceObject = FALSE;

	ASSERT(PtrReturnedDeviceObject);

	try {

		// Create a new device object.
		if (!NT_SUCCESS(RC = IoCreateDevice(SFilterGlobalData.SFilterDriverObject,
										sizeof(SFilterDeviceExtension),
										NULL,		// unnamed object
										PtrTargetDeviceObject->DeviceType,
										PtrTargetDeviceObject->Characteristics,
										FALSE,	// Not exclusive.
										&PtrNewDeviceObject))) {
			// failed to create a device object; cannot do much now.
			SFilterBreakPoint();
			try_return(RC);
		}

		// We do this whenever device objects are create on-the-fly (i.e. not as
		// part of driver initialization).
		PtrNewDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

		// Initialize the extension for the device object.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(PtrNewDeviceObject->DeviceExtension);
		SFilterInitDevExtension(PtrDeviceExtension, PtrNewDeviceObject, SFILTER_NODE_TYPE_ATTACHED_DEVICE);
		InitializedDeviceObject = TRUE;
	
		// Acquire the resource exclusively for our newly created device
		// object to ensure that dispatch routines requests are not processed
		// until we are really ready.
		ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
		AcquiredDeviceObject = TRUE;
		
		// attach to the target FSD.
		RC = IoAttachDeviceByPointer(PtrNewDeviceObject, PtrTargetDeviceObject);

		// The only reaon we would fail (and possibly get STATUS_NO_SUCH_DEVICE)
		// is if the target was being initialized or unloaded and neither should
		// be happenning at this time.
		ASSERT(NT_SUCCESS(RC));

		// Note that the AlignmentRequirement, the StackSize, and the SectorSize
		// values will have been automatically initialized for us in the source
		// device object (the I/O Manager does this as part of processing the
		// IoAttachDeviceByPointer() request).

		// We should set the Flags values correctly to indicate whether
		// direct-IO, buffered-IO, or neither is required. Typically, FSDs
		// (especially native FSD implementations) do not want the I/O
		// Manager to touch the user buffer at all.
		PtrNewDeviceObject->Flags |= (PtrTargetDeviceObject->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO));

		// Initialize the TargetDeviceObject field in the extension.
		PtrDeviceExtension->TargetDeviceObject = PtrTargetDeviceObject;
		PtrDeviceExtension->TargetDriverObject = PtrTargetDeviceObject->DriverObject;
		SFilterSetFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_ATTACHED);
		SFilterSetFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_ATTACHED_FSD);

		// We are there now. All I/O requests will start being redirected to
		// us until we detach ourselves.

		try_exit:	NOTHING;

	} finally {
		// Cleanup stuff goes here.
		if (AcquiredDeviceObject) {
			SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			AcquiredDeviceObject = FALSE;
		}

		if (!NT_SUCCESS(RC) && PtrNewDeviceObject) {
			if (InitializedDeviceObject) {
				// The detach routine will take care of everything.
				SFilterDetachTarget(PtrNewDeviceObject, PtrTargetDeviceObject, PtrDeviceExtension);
			}
		} else {
			*PtrReturnedDeviceObject = PtrNewDeviceObject;
		}
	}

	return(RC);
}



/*************************************************************************
*
* Function: SFilterDetachTarget()
*
* Description:
*	This routine will detach from the target device object. It will delete
*	the locally created device object as well. Note that we must ensure that
*	no I/O requests are outstanding when this occurs (or we should drop them).
*	Finally, this routine could be invoked as a result of a fast detach call.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFilterDetachTarget(
PDEVICE_OBJECT					SourceDeviceObject,
PDEVICE_OBJECT					TargetDeviceObject,
PtrSFilterDeviceExtension	PtrDeviceExtension)
{
	BOOLEAN		AcquiredDevice = FALSE;
	BOOLEAN		NoRequestsOutstanding = FALSE;

	ASSERT(SourceDeviceObject);
	ASSERT(PtrDeviceExtension);

	try {
		try {
			ASSERT(PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_RESOURCE_INITIALIZED);

			// We will wait until all IRP-based I/O requests have been completed.

			while (TRUE) {
				// Acquire the device object resource exclusively.
				ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
				AcquiredDevice = TRUE;

				// Check if there are requests outstanding
            SFilterIsLargeIntegerZero(NoRequestsOutstanding, PtrDeviceExtension->OutstandingIoRequests,
										            &(PtrDeviceExtension->IoRequestsSpinLock));

				if (!NoRequestsOutstanding) {
					// Drop the resource and go to sleep.
					SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
					AcquiredDevice = FALSE;

					// Worst case, we will allow a few new I/O requests to slip in ...
					KeWaitForSingleObject((void *)(&(PtrDeviceExtension->IoInProgressEvent)),
													Executive, KernelMode, FALSE, NULL);
				} else {
					break;
				}
			}

			ASSERT(AcquiredDevice);

			// Detach if attached.
			if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
				IoDetachDevice(PtrDeviceExtension->TargetDeviceObject);
				SFilterClearFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_ATTACHED);
				// Blindly clear the attached-to-FSD flag
				SFilterClearFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_ATTACHED_FSD);
			}

			// Delete our device object. But first, take care of the device extension.
         SFilterDeleteDevExtension(PtrDeviceExtension, TRUE);
			AcquiredDevice = FALSE;
			// Note that on 4.0 and later systems, this will result in a recursive fast detach.
			IoDeleteDevice(SourceDeviceObject);

		} except (EXCEPTION_EXECUTE_HANDLER) {
			// Eat it up.
			;
		}

		try_exit:  NOTHING;

	} finally {
		if (AcquiredDevice) {
			SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			AcquiredDevice = FALSE;
		}
	}

	return;
}

