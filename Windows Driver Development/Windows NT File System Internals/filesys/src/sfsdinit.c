/*************************************************************************
*
* File: sfsdinit.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*     This file contains the initialization code for the kernel mode
*     Sample FSD module. The DriverEntry() routine is called by the I/O
*     sub-system to initialize the FSD.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_INIT

#define			SFSD_FS_NAME					L"\\SampleFSD"

// global variables are declared here
SFsdData					SFsdGlobalData;


/*************************************************************************
*
* Function: DriverEntry()
*
* Description:
*	This routine is the standard entry point for all kernel mode drivers.
*	The routine is invoked at IRQL PASSIVE_LEVEL in the context of a
*	system worker thread.
*	All FSD specific data structures etc. are initialized here.
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
	NTSTATUS			RC = STATUS_SUCCESS;
	UNICODE_STRING	DriverDeviceName;
	BOOLEAN			RegisteredShutdown = FALSE;

	SFsdBreakPoint();

	try {
		try {
			// initialize the global data structure
			RtlZeroMemory(&SFsdGlobalData, sizeof(SFsdGlobalData));

			// initialize some required fields
			SFsdGlobalData.NodeIdentifier.NodeType = SFSD_NODE_TYPE_GLOBAL_DATA;
			SFsdGlobalData.NodeIdentifier.NodeSize = sizeof(SFsdGlobalData);

			// initialize the global data resource and remember the fact that
			//	the resource has been initialized
			RC = ExInitializeResourceLite(&(SFsdGlobalData.GlobalDataResource));
			ASSERT(NT_SUCCESS(RC));
			SFsdSetFlag(SFsdGlobalData.SFsdFlags, SFSD_DATA_FLAGS_RESOURCE_INITIALIZED);

			// keep a ptr to the driver object sent to us by the I/O Mgr
			SFsdGlobalData.SFsdDriverObject = DriverObject;

			// initialize the mounted logical volume list head
			InitializeListHead(&(SFsdGlobalData.NextVCB));

			// before we proceed with any more initialization, read in
			//	user supplied configurable values ...
			// if (!NT_SUCCESS(RC = SFsdObtainRegistryValues(RegistryPath))) {
					// in your commercial driver implementation, it would be
					//	advisable for your driver to print an appropriate error
					//	message to the system error log before leaving
			//		try_return(RC);
			//	}

			// we should have the registry data (if any), allocate zone memory ...
			//	This is an example of when FSD implementations try to pre-allocate
			//	some fixed amount of memory to avoid internal fragmentation and/or waiting
			//	later during run-time ...
			if (!NT_SUCCESS(RC = SFsdInitializeZones())) {
				// we failed, print a message and leave ...
				try_return(RC);
			}

			// initialize the IRP major function table, and the fast I/O table
			SFsdInitializeFunctionPointers(DriverObject);

			// create a device object representing the driver itself
			//	so that requests can be targeted to the driver ...
			//	e.g. for a disk-based FSD, "mount" requests will be sent to
			//		  this device object by the I/O Manager.\
			//		  For a redirector/server, you may have applications
			//		  send "special" IOCTL's using this device object ...
			RtlInitUnicodeString(&DriverDeviceName, SFSD_FS_NAME);
			if (!NT_SUCCESS(RC = IoCreateDevice(
					DriverObject,		// our driver object
					0,						// don't need an extension for this object
					&DriverDeviceName,// name - can be used to "open" the driver
               // see the book for alternate choices
					FILE_DEVICE_DISK_FILE_SYSTEM,
					0,						// no special characteristics
               // do not want this as an exclusive device, though you might
					FALSE,
					&(SFsdGlobalData.SFsdDeviceObject)))) {
				// failed to create a device object, leave ...
				try_return(RC);
			}

#ifdef	_THIS_IS_A_NETWORK_REDIR_OR_SERVER_

			// since network redirectors/servers do not register themselves as
			//	"file systems", the I/O Manager does not ordinarily request the
			//	FSD to flush logical volumes at shutdown. To get some notification
			//	at shutdown, use the IoRegisterShutdownNotification() instead ...
			if (!NT_SUCCESS(RC = IoRegisterShutdownNotification(SFsdGlobalData.SFsdDeviceObject))) {
				// failed to register shutdown notification ...
				try_return(RC);
			}
         RegisteredShutdown = TRUE;

			// Register the network FSD with the MUP component.
			if (!NT_SUCCESS(RC = FsRtlRegisterUncProvider(&(SFsdGlobalData.MupHandle), &DriverDeviceName, FALSE))) {
				try_return(RC);
			}

#else		// This is a disk based FSD

			// register the driver with the I/O Manager, pretend as if this is
			//	a physical disk based FSD (or in order words, this FSD manages
			//	logical volumes residing on physical disk drives)
         IoRegisterFileSystem(SFsdGlobalData.SFsdDeviceObject);

#endif	// _THIS_IS_A_NETWORK_REDIR_OR_SERVER_

		} except (EXCEPTION_EXECUTE_HANDLER) {
			// we encountered an exception somewhere, eat it up
			RC = GetExceptionCode();
		}

		try_exit:	NOTHING;
	} finally {
		// start unwinding if we were unsuccessful
		if (!NT_SUCCESS(RC)) {
#ifdef	_THIS_IS_A_NETWORK_REDIR_OR_SERVER_
			if (RegisteredShutdown) {
            IoUnregisterShutdownNotification(SFsdGlobalData.SFsdDeviceObject);
			}
#endif	// _THIS_IS_A_NETWORK_REDIR_OR_SERVER_

			// Now, delete any device objects, etc. we may have created
			if (SFsdGlobalData.SFsdDeviceObject) {
				IoDeleteDevice(SFsdGlobalData.SFsdDeviceObject);
            SFsdGlobalData.SFsdDeviceObject = NULL;
			}

			// free up any memory we might have reserved for zones/lookaside
			//	lists
			if (SFsdGlobalData.SFsdFlags & SFSD_DATA_FLAGS_ZONES_INITIALIZED) {
            SFsdDestroyZones();
			}

			// delete the resource we may have initialized
			if (SFsdGlobalData.SFsdFlags & SFSD_DATA_FLAGS_RESOURCE_INITIALIZED) {
				// un-initialize this resource
				ExDeleteResourceLite(&(SFsdGlobalData.GlobalDataResource));
				SFsdClearFlag(SFsdGlobalData.SFsdFlags, SFSD_DATA_FLAGS_RESOURCE_INITIALIZED);
			}
		}
	}

	return(RC);
}



/*************************************************************************
*
* Function: SFsdInitializeFunctionPointers()
*
* Description:
*	Initialize the IRP... function pointer array in the driver object
*	structure. Also initialize the fast-io function ptr array ...
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsdInitializeFunctionPointers(
PDRIVER_OBJECT		DriverObject)		// created by the I/O sub-system
{
   PFAST_IO_DISPATCH	PtrFastIoDispatch = NULL;
	
	// initialize the function pointers for the IRP major
	//	functions that this FSD is prepared to	handle ...
	//	NT Version 4.0 has 28 possible functions that a
	//	kernel mode driver can handle.
	//	NT Version 3.51 and before has only 22 such functions,
	//	of which 18 are typically interesting to most FSD's.
	
	//	The only interesting new functions that a FSD might
	//	want to respond to beginning with Version 4.0 are the
	//	IRP_MJ_QUERY_QUOTA and the IRP_MJ_SET_QUOTA requests.
	
	//	The code below does not handle quota manipulation, neither
	//	does the NT Version 4.0 operating system (or I/O Manager).
	//	However, you should be on the lookout for any such new
	//	functionality that your FSD might have to implement in
	//	the near future.
	
	DriverObject->MajorFunction[IRP_MJ_CREATE]				= SFsdCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]					= SFsdClose;
	DriverObject->MajorFunction[IRP_MJ_READ]					= SFsdRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE]					= SFsdWrite;

	DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]	= SFsdFileInfo;
	DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]	= SFsdFileInfo;

	DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]		= SFsdFlush;
	// To implement support for querying and modifying volume attributes
	// (volume information query/set operations), enable initialization
	// of the following two function pointers and then implement the supporting
	// functions. Use Chapter 11 in the text to assist you in your efforts.
	// DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = SFsdVolInfo;
	// DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION] = SFsdVolInfo;
	DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]	= SFsdDirControl;
	// To implement support for file system IOCTL calls, enable initialization
	// of the following function pointer and implement appropriate support. Use
	// Chapter 11 in the text to assist you in your efforts.
	// DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SFsdFSControl;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]		= SFsdDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]				= SFsdShutdown;
	// For byte-range lock support, enable initialization of the following
	// function pointer and implement appropriate support. Use Chapter 10
	// in the text to assist you in your efforts.
	// DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]		= SFsdLockControl;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]				= SFsdCleanup;
	// If your FSD supports security attributes, you should provide appropriate
	// dispatch entry points and initialize the function pointers as given below.
	// DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY]		= SFsdSecurity;
	// DriverObject->MajorFunction[IRP_MJ_SET_SECURITY]		= SFsdSecurity;
	// If you support extended attributes, you should provide appropriate
	// dispatch entry points and initialize the function pointers as given below.
	// DriverObject->MajorFunction[IRP_MJ_QUERY_EA]				= SFsdExtendedAttr;
	// DriverObject->MajorFunction[IRP_MJ_SET_EA]				= SFsdExtendedAttr;

	// Now, it is time to initialize the fast-io stuff ...
	PtrFastIoDispatch = DriverObject->FastIoDispatch = &(SFsdGlobalData.SFsdFastIoDispatch);

	// initialize the global fast-io structure
	//	NOTE: The fast-io structure has undergone a substantial revision
	//	in Windows NT Version 4.0. The structure has been extensively expanded.
	//	Therefore, if your driver needs to work on both V3.51 and V4.0+,
	//	you will have to be able to distinguish between the two versions at compile time.
	PtrFastIoDispatch->SizeOfFastIoDispatch	= sizeof(FAST_IO_DISPATCH);
	PtrFastIoDispatch->FastIoCheckIfPossible	= SFsdFastIoCheckIfPossible;
	PtrFastIoDispatch->FastIoRead					= SFsdFastIoRead;
	PtrFastIoDispatch->FastIoWrite				= SFsdFastIoWrite;
	PtrFastIoDispatch->FastIoQueryBasicInfo	= SFsdFastIoQueryBasicInfo;
	PtrFastIoDispatch->FastIoQueryStandardInfo	= SFsdFastIoQueryStdInfo;
	PtrFastIoDispatch->FastIoLock					= SFsdFastIoLock;
	PtrFastIoDispatch->FastIoUnlockSingle		= SFsdFastIoUnlockSingle;
	PtrFastIoDispatch->FastIoUnlockAll			= SFsdFastIoUnlockAll;
	PtrFastIoDispatch->FastIoUnlockAllByKey	= SFsdFastIoUnlockAllByKey;
	PtrFastIoDispatch->AcquireFileForNtCreateSection = SFsdFastIoAcqCreateSec;
	PtrFastIoDispatch->ReleaseFileForNtCreateSection = SFsdFastIoRelCreateSec;

	// the remaining are only valid under NT Version 4.0 and later
#if(_WIN32_WINNT >= 0x0400)
	PtrFastIoDispatch->FastIoQueryNetworkOpenInfo = SFsdFastIoQueryNetInfo;
	PtrFastIoDispatch->AcquireForModWrite		= SFsdFastIoAcqModWrite;
	PtrFastIoDispatch->ReleaseForModWrite		= SFsdFastIoRelModWrite;
	PtrFastIoDispatch->AcquireForCcFlush		= SFsdFastIoAcqCcFlush;
	PtrFastIoDispatch->ReleaseForCcFlush		= SFsdFastIoRelCcFlush;

	// MDL functionality
	PtrFastIoDispatch->MdlRead						= SFsdFastIoMdlRead;
	PtrFastIoDispatch->MdlReadComplete			= SFsdFastIoMdlReadComplete;
	PtrFastIoDispatch->PrepareMdlWrite			= SFsdFastIoPrepareMdlWrite;
	PtrFastIoDispatch->MdlWriteComplete			= SFsdFastIoMdlWriteComplete;

	// although this FSD does not support compressed read/write functionality,
	//	NTFS does, and if you design a FSD that can provide such functionality,
	//	you should consider initializing the fast io entry points for reading
	//	and/or writing compressed data ...
#endif	// (_WIN32_WINNT >= 0x0400)

	// last but not least, initialize the Cache Manager callback functions
	//	which are used in CcInitializeCacheMap()
	SFsdGlobalData.CacheMgrCallBacks.AcquireForLazyWrite = SFsdAcqLazyWrite;
	SFsdGlobalData.CacheMgrCallBacks.ReleaseFromLazyWrite = SFsdRelLazyWrite;
	SFsdGlobalData.CacheMgrCallBacks.AcquireForReadAhead = SFsdAcqReadAhead;
	SFsdGlobalData.CacheMgrCallBacks.ReleaseFromReadAhead = SFsdRelReadAhead;

	return;
}

