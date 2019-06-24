/*************************************************************************
*
* File: fastio.c
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the various "fast-io" calls.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfilter.h"

/* define the file specific bug-check id */
#define			SFILTER_BUG_CHECK_ID				SFILTER_FILE_FAST_IO



/*************************************************************************
*
* Function: SFilterFastIoCheckIfPossible()
*
* Description:
*	To fast-io or not to fast-io, that is the question ...
*	This routine helps the I/O Manager determine whether the FSD wishes
*	to permit fast-io on a specific file stream.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoCheckIfPossible(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
IN BOOLEAN						CheckForReadOperation,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;
	FAST_IO_POSSIBLE				RC = FastIoIsNotPossible;
	
	try {
		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
      SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				// Note that dynamic detaching is risky anyways.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Note that although I am checking for a valid "FastIoCheckIfPossible"
			// function pointer below, the only reason the caller invoked this
			// function is because the CommonFCBHeader in the file object asked
			// it to check whether fast-IO was possible or not (i.e. the
			// IsFastIoPossible field was set to FastIoIsQuestionable). In that
			// case, the target of our attach operation (if it is the original
			// file system itself) better have a valid FastIoCheckIfPossible
			// field. However. we may have layered ourselves over a filter driver
			// in which case it seems prudent to make the check below.
			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoCheckIfPossible) {
				RC = PtrFastIoDispatch->FastIoCheckIfPossible(FileObject, FileOffset, Length, Wait, LockKey,
							CheckForReadOperation, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit: NOTHING;

	} finally {
		;
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoRead()
*
* Description:
*	Bypass the traditional IRP method to perform a read operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
      SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags &
							SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoRead) {
				RC = PtrFastIoDispatch->FastIoRead(FileObject, FileOffset, Length, Wait, LockKey,
							Buffer, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoWrite()
*
* Description:
*	Bypass the traditional IRP method to perform a write operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{

	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
      SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch =
							PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoWrite) {
				RC = PtrFastIoDispatch->FastIoWrite(FileObject, FileOffset, Length, Wait, LockKey,
							Buffer, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoQueryBasicInfo()
*
* Description:
*	Bypass the traditional IRP method to perform a query basic
*	information operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoQueryBasicInfo(
IN PFILE_OBJECT					FileObject,
IN BOOLEAN							Wait,
OUT PFILE_BASIC_INFORMATION	Buffer,
OUT PIO_STATUS_BLOCK 			IoStatus,
IN PDEVICE_OBJECT					DeviceObject)
{

	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
      SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoQueryBasicInfo) {
				RC = PtrFastIoDispatch->FastIoQueryBasicInfo(FileObject, Wait, Buffer, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoQueryStdInfo()
*
* Description:
*	Bypass the traditional IRP method to perform a query standard
*	information operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoQueryStdInfo(
IN PFILE_OBJECT						FileObject,
IN BOOLEAN								Wait,
OUT PFILE_STANDARD_INFORMATION 	Buffer,
OUT PIO_STATUS_BLOCK 				IoStatus,
IN PDEVICE_OBJECT						DeviceObject)
{

	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoQueryStandardInfo) {
				RC = PtrFastIoDispatch->FastIoQueryStandardInfo(FileObject, Wait, Buffer, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoLock()
*
* Description:
*	Bypass the traditional IRP method to perform a byte range lock
*	operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoLock(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
BOOLEAN							FailImmediately,
BOOLEAN							ExclusiveLock,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoLock) {
				RC = PtrFastIoDispatch->FastIoLock(FileObject, FileOffset, Length, ProcessId, Key, FailImmediately,
							ExclusiveLock,
							IoStatus, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoUnlockSingle()
*
* Description:
*	Bypass the traditional IRP method to perform a byte range unlock
*	operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoUnlockSingle(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoUnlockSingle) {
				RC = PtrFastIoDispatch->FastIoUnlockSingle(FileObject, FileOffset, Length, ProcessId, Key, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoUnlockAll()
*
* Description:
*	Bypass the traditional IRP method to perform multiple byte range unlock
*	operations.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoUnlockAll(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoUnlockAll) {
				RC = PtrFastIoDispatch->FastIoUnlockAll(FileObject, ProcessId, IoStatus, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoUnlockAllByKey()
*
* Description:
*	Bypass the traditional IRP method to perform multiple byte range unlock
*	operations.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoUnlockAllByKey(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoUnlockAllByKey) {
				RC = PtrFastIoDispatch->FastIoUnlockAllByKey(FileObject, ProcessId,
							Key, IoStatus, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}



/*************************************************************************
*
* Function: SFilterFastIoDeviceControl()
*
* Description:
*	If this is our device object, we should process the device control
*	request, otherwise, we should forward it on to the target device
*	object.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoDeviceControl(
IN PFILE_OBJECT 		FileObject,
IN BOOLEAN				Wait,
IN PVOID					InputBuffer,
IN ULONG					InputBufferLength,
OUT PVOID				OutputBuffer,
IN ULONG					OutputBufferLength,
IN ULONG					IoControlCode,
OUT PIO_STATUS_BLOCK	IoStatus,
IN PDEVICE_OBJECT		DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoDeviceControl) {
				RC = PtrFastIoDispatch->FastIoDeviceControl(FileObject,
							Wait, InputBuffer, InputBufferLength, OutputBuffer,
							OutputBufferLength, IoControlCode, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		} else {
			// If you wish to receive device control calls to your FSD, implement
			// support for such calls here.
			NOTHING;
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}




/*************************************************************************
*
* Function: SFilterFastIoAcqCreateSec()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to acquire FSD resources
*	before processing a file map (create section object) request.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL ?
*
* Return Value: None (we must be prepared to handle VMM initiated calls)
*
*************************************************************************/
void SFilterFastIoAcqCreateSec(
IN PFILE_OBJECT			FileObject)
{
	PDEVICE_OBJECT					PtrLocalDeviceObject = NULL;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	// Forward rhe request onwards. Of course, if we had to pre-acquire
	// any local resources, it should be done now. However, it is prudent
	// in the filter not to have any complicated resource acquisition
	// methodologies.
	// Unfortunately, we will have to look at the file object to determine
	// which target device object the request needs to be forwarded to.
	// Do exactly what is done by (say) the FSRTL package.
	PtrLocalDeviceObject = IoGetRelatedDeviceObject(FileObject);

	PtrDeviceExtension = (PtrSFilterDeviceExtension)(PtrLocalDeviceObject->DeviceExtension);
	SFilterAssertExtPtrValid(PtrDeviceExtension);

	//	If the device extension is for a lower device forward the request.
	if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
		PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

		ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
		try {
			// Just to synchronize with the attach and detach operations.
			if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
				PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
			}
		} finally {
			SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
		}

		if (PtrFastIoDispatch && PtrFastIoDispatch->AcquireFileForNtCreateSection) {
			// Forward the request so that the next driver in the hierarchy acquires
			// it's resources.
			PtrFastIoDispatch->AcquireFileForNtCreateSection(FileObject);
		}
	}

	// Nothing we currently wish to do if the request is directed to a non-attached
	// device object.

	return;
}


/*************************************************************************
*
* Function: SFilterFastIoRelCreateSec()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to release FSD resources
*	after processing a file map (create section object) request.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL ?
*
* Return Value: None
*
*************************************************************************/
void SFilterFastIoRelCreateSec(
IN PFILE_OBJECT			FileObject)
{
	PDEVICE_OBJECT					PtrLocalDeviceObject = NULL;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	PtrLocalDeviceObject = IoGetRelatedDeviceObject(FileObject);

	PtrDeviceExtension = (PtrSFilterDeviceExtension)(PtrLocalDeviceObject->DeviceExtension);
	SFilterAssertExtPtrValid(PtrDeviceExtension);

	//	If the device extension is for a lower device forward the request.
	if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
		PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

		ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
		try {
			// Just to synchronize with the attach and detach operations.
			if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
				PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
			}
		} finally {
			SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
		}

		if (PtrFastIoDispatch && PtrFastIoDispatch->ReleaseFileForNtCreateSection) {
			// Forward the request.
			PtrFastIoDispatch->ReleaseFileForNtCreateSection(FileObject);
		}
	}

	return;
}

// The remaining are only valid under NT Version 4.0 and later.
#if(_WIN32_WINNT >= 0x0400)


/*************************************************************************
*
* Function: SFilterFastIoDetachDevice()
*
* Description:
*	The device object we have attached to is being deleted.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: None
*
*************************************************************************/
void SFilterFastIoDetachDevice(
PDEVICE_OBJECT				SourceDeviceObject,		// our device object
PDEVICE_OBJECT				TargetDeviceObject)
{
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	PtrDeviceExtension = (PtrSFilterDeviceExtension)(SourceDeviceObject->DeviceExtension);
	SFilterAssertExtPtrValid(PtrDeviceExtension);

	KdPrint(("SFilterFastIoDetachDevice(): Detach device requested. Source object = 0x%x, Target Object = 0x%x\n",
					SourceDeviceObject, TargetDeviceObject));

	// The following routine will detach our device object and also
	// request a delete operation on our device object which in turn
	// will cause any device object attached to ours to be detached
	// prior to deletion.
   SFilterDetachTarget(SourceDeviceObject, TargetDeviceObject, PtrDeviceExtension);

	return;
}



/*************************************************************************
*
* Function: SFilterFastIoQueryNetInfo()
*
* Description:
*	Get information requested by a redirector across the network. This call
*	will typically originate from the LAN Manager server.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoQueryNetInfo(
IN PFILE_OBJECT									FileObject,
IN BOOLEAN											Wait,
OUT PFILE_NETWORK_OPEN_INFORMATION 			Buffer,
OUT PIO_STATUS_BLOCK 							IoStatus,
IN PDEVICE_OBJECT									DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch
					>= sizeof(FAST_IO_DISPATCH)) && (PtrFastIoDispatch->FastIoQueryNetworkOpenInfo)) {
				RC = PtrFastIoDispatch->FastIoQueryNetworkOpenInfo(FileObject,
							Wait, Buffer, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoMdlRead()
*
* Description:
*	Bypass the traditional IRP method to perform a MDL read operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoMdlRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch
					>= sizeof(FAST_IO_DISPATCH)) && (PtrFastIoDispatch->MdlRead)) {
				RC = PtrFastIoDispatch->MdlRead(FileObject, FileOffset, Length,
							LockKey, MdlChain, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoMdlReadComplete()
*
* Description:
*	Bypass the traditional IRP method to inform the NT Cache Manager and the
*	FSD that the caller no longer requires the data locked in the system cache
*	or the MDL to stay around anymore ..
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoMdlReadComplete(
IN PFILE_OBJECT				FileObject,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->MdlReadComplete)) {
				RC = PtrFastIoDispatch->MdlReadComplete(FileObject, MdlChain, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoPrepareMdlWrite()
*
* Description:
*	Bypass the traditional IRP method to prepare for a MDL write operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoPrepareMdlWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->PrepareMdlWrite)) {
				RC = PtrFastIoDispatch->PrepareMdlWrite(FileObject, FileOffset,
							Length, LockKey, MdlChain, IoStatus,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoMdlWriteComplete()
*
* Description:
*	Bypass the traditional IRP method to inform the NT Cache Manager and the
*	FSD that the caller has updated the contents of the MDL. This data can
*	now be asynchronously written out to secondary storage by the Cache Mgr.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoMdlWriteComplete(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->MdlWriteComplete)) {
				RC = PtrFastIoDispatch->MdlWriteComplete(FileObject, FileOffset, MdlChain, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}



/*************************************************************************
*
* Function: SFilterFastIoReadCompressed()
*
* Description:
*	Bypass the traditional IRP method to perform a compressed read operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoReadCompressed(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
OUT struct _COMPRESSED_DATA_INFO	*CompressedDataInfo,
IN ULONG							CompressedDataInfoLength,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoReadCompressed) {
				RC = PtrFastIoDispatch->FastIoReadCompressed(FileObject, FileOffset, Length, LockKey, Buffer, MdlChain,
							IoStatus, CompressedDataInfo, CompressedDataInfoLength,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}



/*************************************************************************
*
* Function: SFilterFastIoWriteCompressed()
*
* Description:
*	Bypass the traditional IRP method to perform a compressed write operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoWriteCompressed(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
OUT struct _COMPRESSED_DATA_INFO	*CompressedDataInfo,
IN ULONG							CompressedDataInfoLength,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			if (PtrFastIoDispatch && PtrFastIoDispatch->FastIoWriteCompressed) {
				RC = PtrFastIoDispatch->FastIoWriteCompressed(FileObject, FileOffset, Length, LockKey, Buffer, MdlChain,
							IoStatus, CompressedDataInfo, CompressedDataInfoLength,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		// Otherwise, we return FALSE to indicate that the operation cannot
		// be performed via the fast path.

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoMdlReadCompleteCompressed()
*
* Description:
*	Bypass the traditional IRP method to inform the NT Cache Manager and the
*	FSD that the caller no longer requires the data locked in the system cache
*	or the MDL to stay around anymore ..
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoMdlReadCompleteCompressed(
IN PFILE_OBJECT				FileObject,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->MdlReadCompleteCompressed)) {
				RC = PtrFastIoDispatch->MdlReadCompleteCompressed(FileObject, MdlChain, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}



/*************************************************************************
*
* Function: SFilterFastIoMdlWriteCompleteCompressed()
*
* Description:
*	Bypass the traditional IRP method to inform the NT Cache Manager and the
*	FSD that the caller has updated the contents of the MDL. This data can
*	now be asynchronously written out to secondary storage by the Cache Mgr.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: TRUE/FALSE
*
*************************************************************************/
BOOLEAN SFilterFastIoMdlWriteCompleteCompressed(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->MdlWriteCompleteCompressed)) {
				RC = PtrFastIoDispatch->MdlWriteCompleteCompressed(FileObject, FileOffset, MdlChain,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}



/*************************************************************************
*
* Function: SFilterFastIoAcqModWrite()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to acquire FSD resources
*	before initiating a write operation via the Modified Page/Block Writer.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterFastIoAcqModWrite(
IN PFILE_OBJECT					FileObject,
IN PLARGE_INTEGER					EndingOffset,
OUT PERESOURCE						*ResourceToRelease,
IN PDEVICE_OBJECT					DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->AcquireForModWrite)) {
				RC = PtrFastIoDispatch->AcquireForModWrite(FileObject, EndingOffset, ResourceToRelease,
							PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}
	
	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoRelModWrite()
*
* Description:
*	Not really a fast-io operation. Used by the VMM to release FSD resources
*	after processing a modified page/block write operation.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterFastIoRelModWrite(
IN PFILE_OBJECT				FileObject,
IN PERESOURCE					ResourceToRelease,
IN PDEVICE_OBJECT				DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->ReleaseForModWrite)) {
				RC = PtrFastIoDispatch->ReleaseForModWrite(FileObject, ResourceToRelease, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoAcqCcFlush()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to acquire FSD
*	resources before performing a CcFlush() operation on a specific file
*	stream.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterFastIoAcqCcFlush(
IN PFILE_OBJECT			FileObject,
IN PDEVICE_OBJECT			DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->AcquireForCcFlush)) {
				RC = PtrFastIoDispatch->AcquireForCcFlush(FileObject, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoRelCcFlush()
*
* Description:
*	Not really a fast-io operation. Used by the NT Cache Mgr to release FSD
*	resources after performing a CcFlush() operation on a specific file
*	stream.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
NTSTATUS SFilterFastIoRelCcFlush(
IN PFILE_OBJECT			FileObject,
IN PDEVICE_OBJECT			DeviceObject)
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->ReleaseForCcFlush)) {
				RC = PtrFastIoDispatch->ReleaseForCcFlush(FileObject, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}

	return(RC);
}


/*************************************************************************
*
* Function: SFilterFastIoQueryOpen()
*
* Description:
*	Get file information (IRP is sent down!).
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: STATUS_SUCCESS/Error
*
*************************************************************************/
BOOLEAN SFilterFastIoQueryOpen(
IN PIRP										Irp,
OUT PFILE_NETWORK_OPEN_INFORMATION	NetworkInformation,
IN PDEVICE_OBJECT							DeviceObject)
{
	BOOLEAN							RC = FALSE;
   PtrSFilterDeviceExtension	PtrDeviceExtension = NULL;
	PIO_STACK_LOCATION			PtrNextIoStackLocation = NULL;
	PIO_STACK_LOCATION			PtrCurrentStackLocation = NULL;

	try {

		// Get a pointer to the device extension. Must be ours.
		PtrDeviceExtension = (PtrSFilterDeviceExtension)(DeviceObject->DeviceExtension);
		SFilterAssertExtPtrValid(PtrDeviceExtension);

		//	If the device extension is for a lower device forward the request.
		if (PtrDeviceExtension->NodeIdentifier.NodeType == SFILTER_NODE_TYPE_ATTACHED_DEVICE) {
			PFAST_IO_DISPATCH		PtrFastIoDispatch = NULL;

			ExAcquireResourceExclusiveLite(&(PtrDeviceExtension->DeviceExtensionResource), TRUE);
			try {
				// Just to synchronize with the attach and detach operations.
				if (PtrDeviceExtension->DeviceExtensionFlags & SFILTER_DEV_EXT_ATTACHED) {	
					PtrFastIoDispatch = PtrDeviceExtension->TargetDriverObject->FastIoDispatch;
				}
			} finally {
				SFilterReleaseResource(&(PtrDeviceExtension->DeviceExtensionResource));
			}

			// Are we attached to a device that handles the larger fast-IO
			// structure? Note that there are bigger problems if it does not
			// but we should perform the check regardless.
			if (PtrFastIoDispatch && (PtrFastIoDispatch->SizeOfFastIoDispatch >= sizeof(FAST_IO_DISPATCH)) &&
					(PtrFastIoDispatch->FastIoQueryOpen)) {
				// update the device object ptr in the IRP (since we will not use IoCallDriver()).

				PtrCurrentStackLocation = IoGetCurrentIrpStackLocation(Irp);
				PtrCurrentStackLocation->DeviceObject = PtrDeviceExtension->TargetDeviceObject;

				KdPrint(("SFilterFastIoQueryOpen(): Target device = 0x%x\n",
								PtrDeviceExtension->TargetDeviceObject));

				RC = PtrFastIoDispatch->FastIoQueryOpen(Irp, NetworkInformation, PtrDeviceExtension->TargetDeviceObject);
			}
			try_return(RC);
		}

		try_exit:	NOTHING;

	} finally {
		;
	}

	return(RC);
}

#endif	//_WIN32_WINNT >= 0x0400

