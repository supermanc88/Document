/*************************************************************************
*
* File: struct.h
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	This file contains structure definitions for the sample filter
*	driver. Note that all structures are prefixed with the letters
*	"SFilter". The structures are all aligned using normal alignment
*	used by the compiler (typically quad-word aligned).
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#ifndef	_SFILTER_STRUCTURES_H_
#define	_SFILTER_STRUCTURES_H_

/**************************************************************************
	some useful definitions
**************************************************************************/
#ifdef	_CPU_X86_
typedef	char					int8;
typedef	short					int16;
typedef	int					int32;

typedef	unsigned char		uint8;
typedef	unsigned short		uint16;
typedef	unsigned int		uint32;

// we will use the LARGE_INTEGER structure as defined by NT

#else		// Please define appropriate types here

!!!! You must define your types here for compilation to proceed !!!!

#endif	// if _CPU_X86_

/**************************************************************************
	some empty typedefs defined here so we can reference them easily
**************************************************************************/
struct _SFilterIdentifier;
struct _SFilterData;

/**************************************************************************
	each structure has a unique "node type" or signature associated with it
**************************************************************************/
#define	SFILTER_NODE_TYPE_GLOBAL_DATA			(0xfdecba10)
#define	SFILTER_NODE_TYPE_FILTER_DEVICE		(0xfdecba11)
#define	SFILTER_NODE_TYPE_ATTACHED_DEVICE	(0xfdecba12)
#define	SFILTER_NODE_TYPE_MOUNT_COMPLETION	(0xfdecba13)

/**************************************************************************
	every structure has a node type, and a node size associated with it.
	The node type serves as a signature field. The size is used for
	consistency checking ...
**************************************************************************/
typedef struct _SFilterIdentifier {
	uint32		NodeType;			// a 32 bit identifier for the structure
	uint32		NodeSize;			// computed as sizeof(structure)
} SFilterIdentifier, *PtrSFilterIdentifier;

/**************************************************************************
	the device extension for each device object created by the filter driver
**************************************************************************/
typedef struct _SFilterDeviceExtension {
	// A signature (including device size).
	SFilterIdentifier				NodeIdentifier;
	// This is used to synchronize access to the device extension structure.
	ERESOURCE						DeviceExtensionResource;
	// For convenience, a back ptr to the device object that contains this
	// extension.
	PDEVICE_OBJECT					PtrAssociatedDeviceObject;
	// The sample filter driver keeps a private doubly-linked list of all
	// device objects created by the driver.
	LIST_ENTRY						NextDeviceObject;
	// See Flag definitions below.
	uint32							DeviceExtensionFlags;
	// The device object we are attached to.
	PDEVICE_OBJECT					TargetDeviceObject;
	// Stored for convenience. A pointer to the driver object for the
	// target device object (you can always obtain this information from
	// the target device object).
	PDRIVER_OBJECT					TargetDriverObject;
	// If we are attached to a mounted volume instance, the actual FSD
	// created device object representing the instance of the mounted
	// volume.
	PDEVICE_OBJECT					TargetMountedVolumeDeviceObject;
	// A count of outstanding I/O requests for which we have specified a
	// completion routine.
	LARGE_INTEGER					OutstandingIoRequests;
	// The OutstandingIoRequests field is protected by an Executive spin lock.
   KSPIN_LOCK						IoRequestsSpinLock;
	// The event object is used to synchronize detach requests with pending
	// I/O operations and similar stuff.
	KEVENT							IoInProgressEvent;
	// You can associate other information here.
} SFilterDeviceExtension, *PtrSFilterDeviceExtension;

#define	SFILTER_DEV_EXT_RESOURCE_INITIALIZED		(0x00000001)
#define	SFILTER_DEV_EXT_INSERTED_GLOBAL_LIST		(0x00000002)
#define	SFILTER_DEV_EXT_ATTACHED						(0x00000004)
#define	SFILTER_DEV_EXT_ATTACHED_FSD					(0x00000008)

/**************************************************************************
	we will store all of our global variables in one structure.
**************************************************************************/
typedef struct _SFilterData {
	SFilterIdentifier			NodeIdentifier;
	// The fields in this list are protected by the following resource
	ERESOURCE					GlobalDataResource;
	// Each driver has a driver object created for it by the NT I/O Mgr.
	//	The filter driver is no exception to this rule.
	PDRIVER_OBJECT				SFilterDriverObject;
	// We will create a device object for our filter driver as well ...
	//	Although not really required, it helps if a helper application
	//	writen by us wishes to send us control information via
	//	IOCTL requests ...
	PDEVICE_OBJECT				SFilterDeviceObject;
	// A driver private list of all device objects created by the filter driver.
	LIST_ENTRY					NextDeviceObject;
	// The NT Cache Manager, the I/O Manager and we will conspire
	//	to bypass IRP usage using the function pointers contained
	//	in the following structure
	// We will require this if we layer on top of a file system.
	FAST_IO_DISPATCH			SFilterFastIoDispatch;
	// Some state information is maintained in the flags field
	uint32						SFilterFlags;
	// Handle for the directory object created by the filter driver
	HANDLE						DirectoryHandle;
	// Corresponding handle for the directory visible to the user
	HANDLE						DosDirectoryHandle;
} SFilterData, *PtrSFilterData;

// Valid flag values for the global data structure
#define		SFILTER_DATA_FLAGS_RESOURCE_INITIALIZED		(0x00000001)
#define		SFILTER_DATA_FLAGS_SYMLINK_CREATED				(0x00000002)

/**************************************************************************
	the following structure is used to pass-in as context to the mount
	completion routine.
**************************************************************************/

typedef struct _SFilterMountCompletion {
	// A signature (including device size).
	SFilterIdentifier				NodeIdentifier;
	// pointer to the target device object
	PDEVICE_OBJECT					PtrDeviceObject;
	// pointer to the original IRP
	PIRP								PtrOriginalIrp;
	// a pointer to the VPB structure in the IRP
	PVPB								PtrVPB;
	// a pointer to our device extension (for the device object attached to the FSD)
   PtrSFilterDeviceExtension	PtrDeviceExtension;
	// an embedded work queue item structure
   WORK_QUEUE_ITEM				WorkItem;
} SFilterMountCompletion, *PtrSFilterMountCompletion;


#endif	_SFILTER_STRUCTURES_H_	// has this file been included ?

