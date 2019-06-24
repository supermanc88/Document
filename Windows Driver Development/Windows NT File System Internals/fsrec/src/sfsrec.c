/*************************************************************************
*
* File: sfsrec.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*     This file contains the initialization code for the kernel mode
*     Sample FSD recognizer module
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsrec.h"

#define			SFSD_FS_NAME					L"\\SampleFSDRecognizer"

PDEVICE_OBJECT	PtrFSRecDeviceObject = NULL;
unsigned int	SFsRecDidLoadFail = 0;

extern NTSTATUS ZwLoadDriver(
IN PUNICODE_STRING		DriverName);


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
PDRIVER_OBJECT		DriverObject,
PUNICODE_STRING	RegistryPath)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	UNICODE_STRING			DriverDeviceName;

	UNICODE_STRING 		FileSystemName;
	OBJECT_ATTRIBUTES		ObjectAttributes;
	HANDLE					FileSystemHandle = NULL;
	IO_STATUS_BLOCK		IoStatus;
   PtrSFsRecDeviceExtension	PtrExtension = NULL;

	try {
		try {

			// Initialize the IRP major function table
			DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SFsRecFsControl;
			DriverObject->DriverUnload = SFsRecUnload;

			// Before creating a device object, check whether the FSD has been
			// already loaded. Obviously you should know the name of the FSD that
			// this recognizer has been created for.
			RtlInitUnicodeString(&FileSystemName, L"\\SampleFSD");
			InitializeObjectAttributes(&ObjectAttributes, &FileSystemName, OBJ_CASE_INSENSITIVE, NULL, NULL);
			// Try to open the file system now.
			RC = ZwCreateFile(&FileSystemHandle, SYNCHRONIZE, &ObjectAttributes, &IoStatus, NULL, 0,
										FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, 0, NULL, 0);
			if (RC != STATUS_OBJECT_NAME_NOT_FOUND) {
				// The FSD must have been already loaded.
				if (NT_SUCCESS(RC)) {
					ZwClose(FileSystemHandle);
				}
				RC = STATUS_IMAGE_ALREADY_LOADED;
				try_return(RC);
			}

			// Create a device object representing the file system recognizer.
			// Mount requests are sent to this device object.
			RtlInitUnicodeString(&DriverDeviceName, L"\\SampleFSDRecognizer");

			if (!NT_SUCCESS(RC = IoCreateDevice(DriverObject,	// Driver object for the file system rec.
					sizeof(SFsRecDeviceExtension),	// Did a load fail?
					&DriverDeviceName,					// Name used above
					FILE_DEVICE_DISK_FILE_SYSTEM,
					0,											// No special characteristics
					FALSE,
					&(PtrFSRecDeviceObject)))) {
				try_return(RC);
			}

			PtrExtension = (PtrSFsRecDeviceExtension)(PtrFSRecDeviceObject->DeviceExtension);

			PtrExtension->DidLoadFail = FALSE;

			// Register the device object with the I/O Manager.
         IoRegisterFileSystem(PtrFSRecDeviceObject);

		} except (EXCEPTION_EXECUTE_HANDLER) {
			// we encountered an exception somewhere, eat it up
			RC = GetExceptionCode();
		}

		try_exit:	NOTHING;
	} finally {
		// start unwinding if we were unsuccessful
		if (!NT_SUCCESS(RC) && PtrFSRecDeviceObject) {
			IoDeleteDevice(PtrFSRecDeviceObject);
			PtrFSRecDeviceObject = NULL;
		}
	}

	return(RC);
}



/*************************************************************************
*
* Function: SFsRecUnload()
*
* Description:
*	Unload the FS recognizer.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFsRecUnload(
PDRIVER_OBJECT		PtrFsRecDriverObject)
{
	// Simple. Unregister the device object, and delete it.
	if (PtrFSRecDeviceObject) {
		IoUnregisterFileSystem(PtrFSRecDeviceObject);
		IoDeleteDevice(PtrFSRecDeviceObject);
	
		PtrFSRecDeviceObject = NULL;
	}

	return;
}


/*************************************************************************
*
* Function: SFsRecFsControl()
*
* Description:
*	The single dispatch routine provided.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
NTSTATUS SFsRecFsControl(
PDEVICE_OBJECT		DeviceObject,
PIRP					Irp)
{
	NTSTATUS							RC = STATUS_UNRECOGNIZED_VOLUME;
	PIO_STACK_LOCATION			PtrIoStackLocation = NULL;
   PtrSFsRecDeviceExtension	PtrExtension = NULL;
	PDEVICE_OBJECT					PtrTargetDeviceObject = NULL;
	UNICODE_STRING					DriverName;

	FsRtlEnterFileSystem();

	try {

		try {

			PtrIoStackLocation = IoGetCurrentIrpStackLocation(Irp);
			ASSERT(PtrIoStackLocation);
	
			// Get a pointer to the device object extension.
			PtrExtension = (PtrSFsRecDeviceExtension)(PtrFSRecDeviceObject->DeviceExtension);
	
			switch (PtrIoStackLocation->MinorFunction) {
			case IRP_MN_MOUNT_VOLUME:
				// Fail the request immediately if a previous load has failed.
				//  You are not required to do this however in your driver.
				if (PtrExtension->DidLoadFail) {
					try_return(RC);
				}
	
				// Get a pointer to the target physical/virtual device object.
				PtrTargetDeviceObject = PtrIoStackLocation->Parameters.MountVolume.DeviceObject;
	
				// The operations that you perform here are highly FSD specific.
				// Typically, you would invoke an internal function that would
				// (a) Get the disk geometry by issuing an IOCTL
				// (b) Read the first few sectors (or appropriate sectors)
				//		 to verify the on-disk metadata information.
				// To get the drive geometry, use the documented I/O Manager
				// routine called IoBuildDeviceIoControlRequest() to create an
				// IRP. Supply an event with this request that you will wait on
				// in case the lower-level driver returns STATUS_PENDING.
				// Similarly, to actually read on-disk sectors, create an IRP
				// using the IoBuildSynchronousFsdRequest() function call with
				// a major function of IRP_MJ_READ.
	
				// After you have obtained on-disk information, verify the metadata.
				// RC = SFsRecGetDiskInfoAndVerify(PtrTargetDeviceObject);
	
				if (NT_SUCCESS(RC)) {
					// Everything looks good. Prepare to load the driver.
					try_return(RC = STATUS_FS_DRIVER_REQUIRED);
				}
				break;
			case IRP_MN_LOAD_FILE_SYSTEM:
				// OK. So we processed a mount request and returned
				// STATUS_FS_DRIVER_REQUIRED to the I/O Manager.
				// This is the result!
				RtlInitUnicodeString(&DriverName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\SFsd");
				RC = ZwLoadDriver(&DriverName);
				if ((!NT_SUCCESS(RC)) && (RC != STATUS_IMAGE_ALREADY_LOADED)) {
					PtrExtension->DidLoadFail = TRUE;
				} else {
					// Load succeeded. Mission accomplished!
					IoUnregisterFileSystem(PtrFSRecDeviceObject);
				}
				break;
			default:
				RC = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}
	
		} except (EXCEPTION_EXECUTE_HANDLER) {
			RC = GetExceptionCode();
		}

		try_exit:	NOTHING;

	} finally {
		// Complete the IRP.
		Irp->IoStatus.Status = RC;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}

	FsRtlExitFileSystem();

	return(RC);
}

