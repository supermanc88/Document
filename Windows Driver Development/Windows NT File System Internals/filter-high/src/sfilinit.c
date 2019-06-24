/*************************************************************************
*
* File: sfilinit.c
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*     This file contains the initialization code for the kernel mode
*     Sample Filter module. The DriverEntry() routine is called by the I/O
*     sub-system to initialize the filter driver. The filter driver is
*		designed to layer over FSD device objects.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

// Common include file
#include			"sfilter.h"

// Define the file-specific bug-check id
#define			SFILTER_BUG_CHECK_ID				SFILTER_FILE_INIT

// Global variables are declared here (minimize these)
SFilterData					SFilterGlobalData;


/*************************************************************************
*
* Function: DriverEntry()
*
* Description:
*	This routine is the standard entry point for most kernel mode drivers.
*	The routine is invoked at IRQL PASSIVE_LEVEL in the context of a
*	system worker thread.
*	All Filter driver specific data structures etc. are initialized here.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error (will cause driver to be unloaded).
*
*************************************************************************/
NTSTATUS DriverEntry(
PDRIVER_OBJECT		DriverObject,		// created by the I/O sub-system
PUNICODE_STRING	RegistryPath)		// path to the registry key
{
	NTSTATUS							RC = STATUS_SUCCESS;
	BOOLEAN							RegisteredShutdown = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	UNICODE_STRING					UserVisibleName;
	UNICODE_STRING					DriverDeviceName;

	// Allows us to break into the filter driver (if required) at
	// driver initialization (only with the debug/checked build).
	SFilterBreakPoint();

	try {
		try {
			// initialize the global data structure
			RtlZeroMemory(&SFilterGlobalData, sizeof(SFilterGlobalData));

			// initialize some required fields
			SFilterGlobalData.NodeIdentifier.NodeType = SFILTER_NODE_TYPE_GLOBAL_DATA;
			SFilterGlobalData.NodeIdentifier.NodeSize = sizeof(SFilterGlobalData);

			// initialize the global data resource and remember the fact that
			//	the resource has been initialized
			RC = ExInitializeResourceLite(&(SFilterGlobalData.GlobalDataResource));
			ASSERT(NT_SUCCESS(RC));
			SFilterSetFlag(SFilterGlobalData.SFilterFlags, SFILTER_DATA_FLAGS_RESOURCE_INITIALIZED);

			// keep a ptr to the driver object sent to us by the I/O Mgr
			SFilterGlobalData.SFilterDriverObject = DriverObject;

			// initialize the layered device object list head
			InitializeListHead(&(SFilterGlobalData.NextDeviceObject));

			// initialize the IRP major function table, and the fast I/O table
			SFilterInitializeFunctionPointers(DriverObject);

			// It would be nice to create directories	in which to create
			// named (temporary) filter driver objects.

			// Start with a directory in the NT Object Name Space.
			if (!NT_SUCCESS(RC = SFilterCreateDirectory(SFILTER_DRV_DIR,
													&(SFilterGlobalData.DirectoryHandle),
													TRUE))) {
				SFilterBreakPoint();
				try_return(RC);
			}

			// Now create a directory in the DOS (Win32) visible name space.
			if (!NT_SUCCESS(RC = SFilterCreateDirectory(SFILTER_DOS_DRV_DIR,
													&(SFilterGlobalData.DosDirectoryHandle),
													TRUE))) {
				SFilterBreakPoint();
				try_return(RC);
			}

			// create a device object representing the driver itself
			//	so that requests can be targeted to the driver ...
			RtlInitUnicodeString(&DriverDeviceName, SFILTER_DRV_NAME);
			if (!NT_SUCCESS(RC = IoCreateDevice(
					DriverObject,
					sizeof(SFilterDeviceExtension),
					&DriverDeviceName,
					FILE_DEVICE_FILE_SYSTEM,	// For lack of anything better ?
					0,
					FALSE,	// Not exclusive.
					&(SFilterGlobalData.SFilterDeviceObject)))) {
				// failed to create a device object, leave.
				SFilterBreakPoint();
				try_return(RC);
			}

			// Initialize the extension for the device object.
			PtrDeviceExtension = (PtrSFilterDeviceExtension)(SFilterGlobalData.SFilterDeviceObject->DeviceExtension);
			SFilterInitDevExtension(PtrDeviceExtension, SFilterGlobalData.SFilterDeviceObject, SFILTER_NODE_TYPE_FILTER_DEVICE);

			// In order to allow user-space helper applications to access our
			// device object for the filter driver, create a symbolic link to
			// the object.
			RtlInitUnicodeString(&UserVisibleName, SFILTER_DOS_DRV_NAME);
			if (!NT_SUCCESS(RC = IoCreateSymbolicLink(&UserVisibleName, &DriverDeviceName))) {
				SFilterBreakPoint();
				try_return(RC);
			}
			SFilterSetFlag(SFilterGlobalData.SFilterFlags, SFILTER_DATA_FLAGS_SYMLINK_CREATED);


			// The filter driver is not currently unloadable. However, you could
			// extend the driver to support such functionality if so desired.
			// You must be careful about any pending I/O requests, must unattach
			// any previously attached device objects, release all resources.
			// All of the above is not easy but certainly doable for someone
			// sufficiently motivated :-)

#if(_WIN32_WINNT >= 0x0400)
			// For version 4.0+ of the operating system only:
			// We will ask the I/O Manager to inform us whenever a FSD registers
			// itself. We should be able to get to all native local file system drivers
			// in this manner (and likely all network redirectors).

			// For versions prior to 4.0, see the reinitialization function
			// specified by this filter driver.
			RC = IoRegisterFsRegistrationChange(DriverObject, SFilterFSDNotification);
#endif	//(_WIN32_WINNT >= 0x0400)

		} except (EXCEPTION_EXECUTE_HANDLER) {
			// We encountered an exception somewhere, eat it up.
			RC = GetExceptionCode();
			SFilterBreakPoint();
		}

		try_exit:	NOTHING;
	} finally {
		// Start unwinding if we were unsuccessful.
		if (!NT_SUCCESS(RC)) {

			if (SFilterGlobalData.SFilterFlags & SFILTER_DATA_FLAGS_SYMLINK_CREATED) {
				IoDeleteSymbolicLink(&UserVisibleName);
			}

			// Now, delete any device objects, etc. we may have created
			if (SFilterGlobalData.SFilterDeviceObject) {
				if (PtrDeviceExtension) {
               SFilterDeleteDevExtension(PtrDeviceExtension, FALSE);
				}
				// nothing (no-one) should be attached to our device
				// object at this time.
				IoDeleteDevice(SFilterGlobalData.SFilterDeviceObject);
            SFilterGlobalData.SFilterDeviceObject = NULL;
			}

			// Delete the directories we may have created.
			if (SFilterGlobalData.DirectoryHandle) {
				ZwClose(SFilterGlobalData.DirectoryHandle);
            SFilterGlobalData.DirectoryHandle = NULL;
			}

			if (SFilterGlobalData.DosDirectoryHandle) {
				ZwClose(SFilterGlobalData.DosDirectoryHandle);
            SFilterGlobalData.DosDirectoryHandle = NULL;
			}

			// delete the resource we may have initialized
			if (SFilterGlobalData.SFilterFlags & SFILTER_DATA_FLAGS_RESOURCE_INITIALIZED) {
				// uninitialize this resource
				ExDeleteResourceLite(&(SFilterGlobalData.GlobalDataResource));
				SFilterClearFlag(SFilterGlobalData.SFilterFlags, SFILTER_DATA_FLAGS_RESOURCE_INITIALIZED);
			}
		} else {
  			// This driver is loaded at system boot time. Therefore, not all
			// of the registry is available at that time. We will set a
			// a reinitialization function that will allow us to read user-specified
			// data.
			// For versions previous to version 4.0, we can also attach to
			// "known" FSDs in the reinitialization function (as long as the
			// driver loads suitably late in the cycle).
			IoRegisterDriverReinitialization(DriverObject, SFilterReinitialize, RegistryPath);
		}
	}

	return(RC);
}



/*************************************************************************
*
* Function: SFilterInitializeFunctionPointers()
*
* Description:
*	Initialize the IRP major function pointer array in the driver object
*	structure. Also initialize the fast-IO function ptr array.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFilterInitializeFunctionPointers(
PDRIVER_OBJECT		DriverObject)		// created by the I/O sub-system
{
   PFAST_IO_DISPATCH	PtrFastIoDispatch = NULL;
	unsigned int		Index = 0;

	// You must initialize the function pointer table such that your
	// filter driver supports all of the functions that the target
	// would originally support unless you really want to prevent the user
	// from being able to invoke a specific function. Even then, a better
	// way to deny the specific functionality	is to initialize the specific
	// major function dispatch table entry to refer to your function and
	// then return an error from within your dispatch function.
	// Here, all I want to do is trap the read/write routines and let the
	// others go through.
	// However, create, cleanup, and close should also be appropriately
	// initialized to allow your user-space helper application to open
	// the device object representing the filter device.

	// First, initialize all function pointers to a default value.
	for (Index = 0; Index <= IRP_MJ_MAXIMUM_FUNCTION; Index++) {
		DriverObject->MajorFunction[Index] = SFilterDefaultDispatch;
	}

	// Initialize the ones we care about with unique function pointers.
	// Note that the functions your driver will care about will depend upon
	// exactly what it is that your filter wishes to do.
	// Currently, all we really care about is intercepting mount/verify/dismount
	// requests.
	DriverObject->MajorFunction[IRP_MJ_CREATE]				= SFilterCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]					= SFilterCleanupOrClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]				= SFilterCleanupOrClose;
	DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SFilterFSControl;

	// Now, it is time to initialize the fast-IO stuff.
	PtrFastIoDispatch = DriverObject->FastIoDispatch = &(SFilterGlobalData.SFilterFastIoDispatch);

	// Initialize the global fast-IO structure.
	//	NOTE: The fast-IO structure has undergone a substantial revision
	//			in Windows NT Version 4.0. The structure has been extensively
	//			expanded.
	//
	//			Therefore, if your driver needs to work on both V3.51 and V4.0+,
	//			you will have to be able to distinguish between the two versions
	//			at compile time.
	PtrFastIoDispatch->SizeOfFastIoDispatch	= sizeof(FAST_IO_DISPATCH);
	PtrFastIoDispatch->FastIoCheckIfPossible	= SFilterFastIoCheckIfPossible;
	PtrFastIoDispatch->FastIoRead					= SFilterFastIoRead;
	PtrFastIoDispatch->FastIoWrite				= SFilterFastIoWrite;
	PtrFastIoDispatch->FastIoQueryBasicInfo	= SFilterFastIoQueryBasicInfo;
	PtrFastIoDispatch->FastIoQueryStandardInfo	= SFilterFastIoQueryStdInfo;
	PtrFastIoDispatch->FastIoLock					= SFilterFastIoLock;
	PtrFastIoDispatch->FastIoUnlockSingle		= SFilterFastIoUnlockSingle;
	PtrFastIoDispatch->FastIoUnlockAll			= SFilterFastIoUnlockAll;
	PtrFastIoDispatch->FastIoUnlockAllByKey	= SFilterFastIoUnlockAllByKey;
	PtrFastIoDispatch->FastIoDeviceControl		= SFilterFastIoDeviceControl;

	PtrFastIoDispatch->AcquireFileForNtCreateSection = SFilterFastIoAcqCreateSec;
	PtrFastIoDispatch->ReleaseFileForNtCreateSection = SFilterFastIoRelCreateSec;

	// The remaining are only valid under NT Version 4.0 and later.
#if(_WIN32_WINNT >= 0x0400)
	PtrFastIoDispatch->FastIoDetachDevice		= SFilterFastIoDetachDevice;

	PtrFastIoDispatch->FastIoQueryNetworkOpenInfo = SFilterFastIoQueryNetInfo;

	/* MDL functionality */
	PtrFastIoDispatch->MdlRead						= SFilterFastIoMdlRead;
	PtrFastIoDispatch->MdlReadComplete			= SFilterFastIoMdlReadComplete;
	PtrFastIoDispatch->PrepareMdlWrite			= SFilterFastIoPrepareMdlWrite;
	PtrFastIoDispatch->MdlWriteComplete			= SFilterFastIoMdlWriteComplete;
	PtrFastIoDispatch->FastIoReadCompressed	= SFilterFastIoReadCompressed;
	PtrFastIoDispatch->FastIoWriteCompressed	= SFilterFastIoWriteCompressed;
	PtrFastIoDispatch->MdlReadCompleteCompressed	=
									SFilterFastIoMdlReadCompleteCompressed;
	PtrFastIoDispatch->MdlWriteCompleteCompressed =
									SFilterFastIoMdlWriteCompleteCompressed;

	// The only fast-IO request (currently) that actually takes an IRP.
	PtrFastIoDispatch->FastIoQueryOpen			= SFilterFastIoQueryOpen;

	PtrFastIoDispatch->AcquireForModWrite		= SFilterFastIoAcqModWrite;
	PtrFastIoDispatch->ReleaseForModWrite		= SFilterFastIoRelModWrite;
	PtrFastIoDispatch->AcquireForCcFlush		= SFilterFastIoAcqCcFlush;
	PtrFastIoDispatch->ReleaseForCcFlush		= SFilterFastIoRelCcFlush;
#endif	//(_WIN32_WINNT >= 0x0400)

	return;
}

