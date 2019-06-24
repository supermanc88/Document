/*************************************************************************
*
* File: shutdown.c
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains code to handle the "shutdown notification" dispatch entry point.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#include			"sfsd.h"

// define the file specific bug-check id
#define			SFSD_BUG_CHECK_ID				SFSD_FILE_SHUTDOWN



/*************************************************************************
*
* Function: SFsdShutdown()
*
* Description:
*	All disk-based FSDs can expect to receive this shutdown notification
*	request whenever the system is about to be halted gracefully. If you
*	design and implement a network redirector, you must register explicitly
*	for shutdown notification by invoking the IoRegisterShutdownNotification()
*	routine from your driver entry.
*
*	Note that drivers that register to receive shutdown notification get
*	invoked BEFORE disk-based FSDs are told about the shutdown notification.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: Irrelevant.
*
*************************************************************************/
NTSTATUS SFsdShutdown(
PDEVICE_OBJECT		DeviceObject,		// the logical volume device object
PIRP					Irp)					// I/O Request Packet
{
	NTSTATUS				RC = STATUS_SUCCESS;
   PtrSFsdIrpContext	PtrIrpContext = NULL;
	BOOLEAN				AreWeTopLevel = FALSE;

	FsRtlEnterFileSystem();
	ASSERT(DeviceObject);
	ASSERT(Irp);

	// set the top level context
	AreWeTopLevel = SFsdIsIrpTopLevel(Irp);

	try {

		// get an IRP context structure and issue the request
		PtrIrpContext = SFsdAllocateIrpContext(Irp, DeviceObject);
		ASSERT(PtrIrpContext);

		RC = SFsdCommonShutdown(PtrIrpContext, Irp);

	} except (SFsdExceptionFilter(PtrIrpContext, GetExceptionInformation())) {

		RC = SFsdExceptionHandler(PtrIrpContext, Irp);

		SFsdLogEvent(SFSD_ERROR_INTERNAL_ERROR, RC);
	}

	if (AreWeTopLevel) {
		IoSetTopLevelIrp(NULL);
	}

	FsRtlExitFileSystem();

	return(RC);
}



/*************************************************************************
*
* Function: SFsdCommonShutdown()
*
* Description:
*	The actual work is performed here. Basically, all we do here is
*	internally invoke a flush on all mounted logical volumes. This, in
*	tuen, will result in all open file streams being flushed to disk.
*
* Expected Interrupt Level (for execution) :
*
*  IRQL_PASSIVE_LEVEL
*
* Return Value: Irrelevant
*
*************************************************************************/
NTSTATUS	SFsdCommonShutdown(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp)
{
	NTSTATUS					RC = STATUS_SUCCESS;
	PIO_STACK_LOCATION	PtrIoStackLocation = NULL;
	IO_STATUS_BLOCK		LocalIoStatus;

	try {
		// First, get a pointer to the current I/O stack location
		PtrIoStackLocation = IoGetCurrentIrpStackLocation(PtrIrp);
		ASSERT(PtrIoStackLocation);

		// Here is the algorithm you can follow in implementing your shutdown
		// functionality. Note, of course, that it is quite possible that you
		// may wish to do much more specialized processing in your specific
		// (commercial) FSD at shutdown time; the steps listed below, however,
		// are those that you should implement at a minimum:
		// (a) Block all new "mount volume" requests by acquiring an appropriate
		//		 global resource/lock.
		// (b) Go through your linked list of mounted logical volumes and for
		//		 each such volume, do the following:
		//		 (i) acquire the volume resource exclusively
		//		 (ii) invoke SFsdFlushLogicalVolume() (internally) to flush the
		//				open data streams belonging to the volume from the system
		//				cache
		//		 (iii) Invoke the physical/virtual/logical target device object
		//				on which the volume is mounted and inform this device
		//				about the shutdown request (Use IoBuildSynchronousFsdRequest()
		//				to create an IRP with MajorFunction = IRP_MJ_SHUTDOWN that you
		//				will then issue to the target device object).
		//		 (iv) Wait for the completion of the shutdown processing by the target
		//				device object
		//		 (v) Release the VCB resource you will have acquired in (i) above.

		// Once you have processed all the mounted logical volumes, you can release
		// all acquired global resources and leave (in peace :-)

		try_exit:	NOTHING;

	} finally {

		// See the read/write examples for how to fill in this portion

	} // end of "finally" processing

	return(RC);
}

