/*************************************************************************
*
* File: misc.c
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*		Contains miscelleneous functions.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfilter.h"

/* define the file specific bug-check id */
#define			SFILTER_BUG_CHECK_ID				SFILTER_FILE_MISC



/*************************************************************************
*
* Function: SFilterInitDevExtension()
*
* Description:
*	Performs some rudimentary work to initialize the device extension
*	presumably for a newly created device object.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None.
*
*************************************************************************/
void SFilterInitDevExtension(
PtrSFilterDeviceExtension	PtrDeviceExtension,
PDEVICE_OBJECT					PtrAssociatedDeviceObject,
uint32							NodeType)
{
	RtlZeroMemory(PtrDeviceExtension, sizeof(SFilterDeviceExtension));

	PtrDeviceExtension->NodeIdentifier.NodeType = NodeType;
	PtrDeviceExtension->NodeIdentifier.NodeSize = sizeof(SFilterDeviceExtension);

	PtrDeviceExtension->PtrAssociatedDeviceObject = PtrAssociatedDeviceObject;

	ExInitializeResourceLite(&(PtrDeviceExtension->DeviceExtensionResource));
	// Set a flag indicating that the resource has been initialized.
	SFilterSetFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_RESOURCE_INITIALIZED);

	try {
		ExAcquireResourceExclusiveLite(&(SFilterGlobalData.GlobalDataResource), TRUE);
		// Insert the object into the global list.
		InsertTailList(&(SFilterGlobalData.NextDeviceObject), &(PtrDeviceExtension->NextDeviceObject));
		// Mark the fact that the device object has been inserted.
		SFilterSetFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_INSERTED_GLOBAL_LIST);
	} finally {
		SFilterReleaseResource(&(SFilterGlobalData.GlobalDataResource));
	}

	// Initialize the Executive spin lock for this device extension.
   KeInitializeSpinLock(&(PtrDeviceExtension->IoRequestsSpinLock));

	// Initialize the event object used to denote I/O in progress.
	// When set, the event signals that no I/O is currently in progress
	// excluding (currently) fast-IO operations.
	// Event is cleared in a safe fashion (protected by the resource
	// initialized above) BUT is set in an unsafe fashion. Therefore, the
	// waiting thread is responsible for rechecking that the condition
	// is still true (after acquiring the resource) once it has been awoken.
	KeInitializeEvent(&(PtrDeviceExtension->IoInProgressEvent), NotificationEvent, FALSE);

	return;
}


/*************************************************************************
*
* Function: SFilterDeleteDevExtension()
*
* Description:
*	Performs some rudimentary work to uninitialize the device extension
*	for an object presumably being deleted.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None.
*
*************************************************************************/
void SFilterDeleteDevExtension(
PtrSFilterDeviceExtension	PtrDeviceExtension,
BOOLEAN							ResourceAcquired)
{

	// If the caller did not invoke this routine with the device extension
	// resource acquired, we will acquire it here.
	if (!ResourceAcquired && (PtrDeviceExtension->DeviceExtensionFlags &
			SFILTER_DEV_EXT_RESOURCE_INITIALIZED)) {
		ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
		ResourceAcquired = TRUE;
	}

	// assert that the device object associated with the
	// extension to be deleted is not attached to anything.
	ASSERT(!(PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED));

	// Remove the device extension from the global list.
	try {
		ExAcquireResourceExclusiveLite(&(SFilterGlobalData.GlobalDataResource), TRUE);
		if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_INSERTED_GLOBAL_LIST) {
			RemoveEntryList(&(PtrDeviceExtension->NextDeviceObject));
			SFilterClearFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_INSERTED_GLOBAL_LIST);
		}
	} finally {
		SFilterReleaseResource(&(SFilterGlobalData.GlobalDataResource));
	}

	if (ResourceAcquired) {
		SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
	}

	PtrDeviceExtension->NodeIdentifier.NodeType = 0;
	PtrDeviceExtension->NodeIdentifier.NodeSize = 0;

	// Uninitialize the resource.
	ExDeleteResourceLite(&(PtrDeviceExtension->DeviceExtensionResource));
	SFilterClearFlag(PtrDeviceExtension->DeviceExtensionFlags, SFILTER_DEV_EXT_RESOURCE_INITIALIZED);

	return;
}




/*************************************************************************
*
* Function: SFilterCreateDirectory()
*
* Description:
*	Create a directory in the object name space. Make the directory
*	a temporary object if requested by the caller.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/an appropriate error
*
*************************************************************************/
NTSTATUS SFilterCreateDirectory(
PWCHAR			DirectoryNameStr,
PHANDLE			PtrReturnedHandle,
BOOLEAN			MakeTemporaryObject)
{
	NTSTATUS							RC = STATUS_SUCCESS;
	UNICODE_STRING					DirectoryName;
	OBJECT_ATTRIBUTES				DirectoryAttributes;

	try {
		// Create a Unicode string.
		RtlInitUnicodeString(&DirectoryName, DirectoryNameStr);
	
		// Create an object attributes structure.
		InitializeObjectAttributes(&DirectoryAttributes,
											&DirectoryName, OBJ_PERMANENT,
											NULL, NULL);
	
		// The following call will fail if we do not have appropriate privileges.
		if (!NT_SUCCESS(RC = ZwCreateDirectoryObject(PtrReturnedHandle,
										DIRECTORY_ALL_ACCESS,
										&DirectoryAttributes))) {
			try_return(RC);
		}
	
		// Make the named directory object a temporary object.
		ZwMakeTemporaryObject(PtrReturnedHandle);

		try_exit: NOTHING;

	} finally {
		NOTHING;
	}

	return(RC);	
}


/*************************************************************************
*
* Function: SFilterReinitialize()
*
* Description:
*	You can read the registry if you like at this time, or do anything else
*	that could not be done because the filter loads at system boot time.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFilterReinitialize(
PDRIVER_OBJECT			DriverObject, 		// representing the filter driver
void						*Context,			// this filter driver supplies registry path
ULONG						Count)				// # times this function invoked
{
   PUNICODE_STRING	RegistryPath = NULL;

	// Currently, this function does not do anything
	// (except break into a debugger if appropriate). You could, however,
	// modify this function to suit your purposes (e.g. read the registry
	// contents for user-configurable parameters)

	// Note that you can specify a reinitialization routine from within this
	// routine to keep getting invoked whenever a new driver gets loaded.
	// In this manner, you can check for known FSDs that load in versions
	// previous to version 4.0

	RegistryPath = (PUNICODE_STRING)(Context);

	return;
}


/*************************************************************************
*
* Function: SFilterFSDNotification
*
* Description:
*	This function is invoked by the I/O Manager in version 4.0+ of the
*	OS whenever a FSD registers or unregisters itself.
*	This function could also be invoked internally by the filter driver
*	from the reinitialization function for versions prior to 4.0.
*	Currently, the filter driver will attach itself to the target device
*	object when the function is invoked.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL (in the context of a system worker thread)
*
* Return Value: None
*
*************************************************************************/
void SFilterFSDNotification(
PDEVICE_OBJECT				PtrTargetFileSystemDeviceObject,
BOOLEAN						DriverActive)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PDEVICE_OBJECT			PtrNewDeviceObject = NULL;

	if (DriverActive) {
		// Create a new device object and attach to the supplied device object.
		// Then, we will intercept all requests to the FSD including (but not
		// limited to) mount requests. Therefore, we will be able to attach to
		// device objects created on-the-fly by the FSD representing instances
		// of mounted logical volumes.

		RC = SFilterAttachTarget(PtrTargetFileSystemDeviceObject, &PtrNewDeviceObject);

		KdPrint(("SFilterFSDNotification(): Result of attaching to device object 0x%x is 0x%x\n",
						PtrTargetFileSystemDeviceObject, RC));
	} else {
		// Nothing much to do here. Remember that it will be a rare FSD indeed
		// that is unloadable (though you can run into problems in versions 3.51
		// and before if the FSD indeed tries to unload itself).
		// You may wish to add code here that traverses the filter driver's linked
		// list of device objects attached to FSDs and delete the appropriate
		// (attached) device object.
		NOTHING;
	}

	return;
}

