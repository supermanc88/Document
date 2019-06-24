/*************************************************************************
*
* File: create.c
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*	Create/open entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfilter.h"

/* define the file specific bug-check id */
#define			SFILTER_BUG_CHECK_ID				SFILTER_FILE_CREATE


/*************************************************************************
*
* Function: SFilterCreate()
*
* Description:
*	Intercept the create/open request targeted to the FSD mounted logical
*	volume. This could also be a create targeted to this filter driver in
*	which case we simply return success.
*
* Expected Interrupt Level (for execution) :
*
*  Typically (though not guaranteed) @ IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterCreate(
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
	ULONG								ReturnedInformation = FILE_OPENED;

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

			// Currently, this filter driver simply forwards the request.
			// However, you could do all sorts of stuff when you intercept a
			// create/open request. For example, if you wish, you could "massage"
			// the name and return reparse to the caller which would allow you
			// to essentially redirect create/open requests.

			if (AcquiredExtension) {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
				AcquiredExtension = FALSE;
			}

			RC = SFilterDefaultDispatch(DeviceObject, Irp);
			try_return(RC);
		} else if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_FILTER_DEVICE) {
			// This request was targeted to us. Simply return success.
			// You may wish to ensure that an open/open-if type of request
			// was sent. You may also wish to ensure that the caller has
			// appropriate privileges. Currently, the filter driver does
			// not perform such checks.
			CompleteIrp = TRUE;
			try_return(RC);	
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

