/*************************************************************************
*
* File: struct.h
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	This file contains structure definitions for the sample file system
*	driver. Note that all structures are prefixed with the letters
*	"SFsd". The structures are all aligned using normal alignment
*	used by the compiler (typically quad-word aligned).
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#ifndef	_SFSD_STRUCTURES_H_
#define	_SFSD_STRUCTURES_H_

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
struct _SFsdIdentifier;
struct _SFsdObjectName;
struct _SFsdContextControlBlock;
struct _SFsdNTRequiredFCB;
struct _SFsdDiskDependentFCB;
struct _SFsdFileControlBlock;
struct _SFsdFileByteLocks;
struct _SFsdVolumeControlBlock;
struct _SFsdIrpContext;
struct _SFsdData;

/**************************************************************************
	each structure has a unique "node type" or signature associated with it
**************************************************************************/
#define	SFSD_NODE_TYPE_OBJECT_NAME			(0xfdecba01)
#define	SFSD_NODE_TYPE_CCB					(0xfdecba02)
#define	SFSD_NODE_TYPE_FCB					(0xfdecba03)
#define	SFSD_NODE_TYPE_LOCKS					(0xfdecba04)
#define	SFSD_NODE_TYPE_VCB					(0xfdecba05)
#define	SFSD_NODE_TYPE_IRP_CONTEXT			(0xfdecba06)
#define	SFSD_NODE_TYPE_GLOBAL_DATA			(0xfdecba07)

/**************************************************************************
	every structure has a node type, and a node size associated with it.
	The node type serves as a signature field. The size is used for
	consistency checking ...
**************************************************************************/
typedef struct _SFsdIdentifier {
	uint32		NodeType;			// a 32 bit identifier for the structure
	uint32		NodeSize;			// computed as sizeof(structure)
} SFsdIdentifier, *PtrSFsdIdentifier;

/**************************************************************************
	Structures for byte-range lock support.
**************************************************************************/
typedef struct SFsdFileLockAnchor {
	LIST_ENTRY			GrantedFileLockList;
	LIST_ENTRY			PendingFileLockList;
} SFsdFileLockAnchor, *PtrSFsdFileLockAnchor;

typedef struct SFsdFileLockInfo {
	SFsdIdentifier						NodeIdentifier;
	uint32								FileLockFlags;
	PVOID									OwningProcess;
	LARGE_INTEGER						StartingOffset;
	LARGE_INTEGER						Length;
	LARGE_INTEGER						EndingOffset;
	ULONG									Key;
	BOOLEAN								ExclusiveLock;
	PIRP									PendingIRP;
	LIST_ENTRY							NextFileLockEntry;
} SFsdFileLockInfo, *PtrSFsdFileLockInfo;

#define		SFSD_BYTE_LOCK_NOT_FROM_ZONE				(0x80000000)
#define		SFSD_BYTE_LOCK_IS_PENDING					(0x00000001)

/**************************************************************************
	Every open on-disk object must have a name associated with it
	This name has two components:
	(a) the path-name (prefix) that leads to this on-disk object
	(b) the name of the object itself
	Note that with multiply linked objects, a single object might be
	associated with more than one name structure.
	This sample FSD does not correctly support multiply linked objects.

	This structure must be quad-word aligned because it is zone allocated.
**************************************************************************/
typedef struct _SFsdObjectName {
	SFsdIdentifier						NodeIdentifier;
	uint32								ObjectNameFlags;
	// an absolute pathname of the object is stored below
	UNICODE_STRING						ObjectName;
} SFsdObjectName, *PtrSFsdObjectName;

#define		SFSD_OB_NAME_NOT_FROM_ZONE				(0x80000000)

/**************************************************************************
	Each file open instance is represented by a context control block.
	For each successful create/open request; a file object and a CCB will
	be created.
	For open operations performed internally by the FSD, there may not
	exist file objects; but a CCB will definitely be created.

	This structure must be quad-word aligned because it is zone allocated.
**************************************************************************/
typedef struct _SFsdContextControlBlock {
	SFsdIdentifier						NodeIdentifier;
	// ptr to the associated FCB
	struct _SFsdFileControlBlock	*PtrFCB;
	// all CCB structures for a FCB are linked together
	LIST_ENTRY							NextCCB;
	// each CCB is associated with a file object
	PFILE_OBJECT						PtrFileObject;
	// flags (see below) associated with this CCB
	uint32								CCBFlags;
	// current byte offset is required sometimes
	LARGE_INTEGER						CurrentByteOffset;
	// if this CCB represents a directory object open, we may
	//	need to maintain a search pattern
	PSTRING								DirectorySearchPattern;
	// we must maintain user specified file time values
	uint32								UserSpecifiedTime;
} SFsdCCB, *PtrSFsdCCB;


/**************************************************************************
	the following CCBFlags values are relevant. These flag
	values are bit fields; therefore we can test whether
	a bit position is set (1) or not set (0).
**************************************************************************/

// some on-disk file/directories are opened by SFSD itself
//	as opposed to being opened on behalf of a user process
#define	SFSD_CCB_OPENED_BY_SFSD						(0x00000001)
// the file object specified synchronous access at create/open time.
//	this implies that SFSD must maintain the current byte offset
#define	SFSD_CCB_OPENED_FOR_SYNC_ACCESS			(0x00000002)
// file object specified sequential access for this file
#define	SFSD_CCB_OPENED_FOR_SEQ_ACCESS			(0x00000004)
// the CCB has had an IRP_MJ_CLEANUP issued on it. we must
//	no longer allow the file object / CCB to be used in I/O requests.
#define	SFSD_CCB_CLEANED								(0x00000008)
// if we were invoked via the fast i/o path to perform file i/o;
//	we should set the CCB access/modification time at cleanup
#define	SFSD_CCB_ACCESSED								(0x00000010)
#define	SFSD_CCB_MODIFIED								(0x00000020)
// if an application process set the file date time, we must
//	honor that request and *not* overwrite the values at cleanup
#define	SFSD_CCB_ACCESS_TIME_SET					(0x00000040)
#define	SFSD_CCB_MODIFY_TIME_SET					(0x00000080)
#define	SFSD_CCB_CREATE_TIME_SET					(0x00000100)

#define	SFSD_CCB_NOT_FROM_ZONE						(0x80000000)

// this CCB was allocated for a "volume open" operation
#define	SFSD_CCB_VOLUME_OPEN							(0x00000100)

/**************************************************************************
	each open file/directory/volume is represented by a file control block.
	NOTE: Currently, SFSD does not handle multiply linked files correctly.
			In your FSD implementation, you must be careful about handling
			such on-disk files correctly i.e. a single (unique) FCB must
			represent an on-disk file/directory regardless of the path used
			to access the on-disk object.
			With the current SFSD implementation, an on-disk file object
			with more than a single (hard) link will be treated incorrectly!

	Each FCB can logically be divided into two:
	(a) a structure that must have a field of type FSRTL_COMMON_FCB_HEADER
		 as the first field in the structure.
		 This portion should also contain other structures/resources required
		 by the NT Cache Manager
		 We will call this structure the "NT Required" FCB. Note that this
		 portion of the FCB must be allocated from non-paged pool.
	(b) the remainder of the FCB is dependent upon the particular FSD
		 requirements.
		 This portion of the FCB could possibly be allocated from paged
		 memory, though in the sample FSD, it will always be allocated
		 from non-paged pool.

	FCB structures are protected by the MainResource as well as the
	PagingIoResource. Of course, if your FSD implementation requires
	it, you can associate other syncronization structures with the
	FCB.

	This structure must be quad-word aligned because it is zone allocated.
**************************************************************************/
typedef struct _SFsdNTRequiredFCB {
	// see Chapters 6-8 for an explanation of the fields here
   FSRTL_COMMON_FCB_HEADER			CommonFCBHeader;
	SECTION_OBJECT_POINTERS			SectionObject;
	ERESOURCE							MainResource;
	ERESOURCE							PagingIoResource;
} SFsdNTRequiredFCB, *PtrSFsdNTRequiredFCB;

typedef struct _SFsdDiskDependentFCB {
	// although the sample FSD does not maintain on-disk data structures,
	//	this structure serves as a reminder of the logical separation that
	//	your FSD can maintain between the disk dependent and the disk
	//	independent portions of the FCB.
	uint16								DummyField;		// placeholder
} SFsdDiskDependentFCB, *PtrSFsdDiskDependentFCB;

typedef struct _SFsdFileControlBlock {
	SFsdIdentifier						NodeIdentifier;
	// we will go ahead and embed the "NT Required FCB" right here.
	//	Note though that it is just as acceptable to simply allocate
	//	memory separately for the other half of the FCB and store a
	//	pointer to the "NT Required" portion here instead of embedding
	//	it ...
	SFsdNTRequiredFCB					NTRequiredFCB;
  	// the disk dependent portion of the FCB is embedded right here
	SFsdDiskDependentFCB				DiskDependentFCB;
	// this FCB belongs to some mounted logical volume
	struct _SFsdVolumeControlBlock	*PtrVCB;
	// to be able to access all open file(s) for a volume, we will
	//	link all FCB structures for a logical volume together
	LIST_ENTRY							NextFCB;
	// some state information for the FCB is maintained using the
	//	flags field
	uint32								FCBFlags;
	// all CCB's for this particular FCB are linked off the following
	//	list head.
	LIST_ENTRY							NextCCB;
	// NT requires that a file system maintain and honor the various
	//	SHARE_ACCESS modes ...
	SHARE_ACCESS						FCBShareAccess;
	// to identify the lazy writer thread(s) we will grab and store
	//	the thread id here when a request to acquire resource(s)
	//	arrives ..
	uint32								LazyWriterThreadID;
	// whenever a file stream has a create/open operation performed,
	//	the Reference count below is incremented AND the OpenHandle count
	//	below is also incremented.
	//	When an IRP_MJ_CLEANUP is received, the OpenHandle count below
	//	is decremented.
	//	When an IRP_MJ_CLOSE is received, the Reference count below is
	//	decremented.
	//	When the Reference count goes down to zero, the FCB can be de-allocated.
  	//	Note that a zero Reference count implies a zero OpenHandle count.
	//	This invariant must always hold true ... (if it is really an invariant,
	// shoudn't the previous statement be redundant ... hmmm!!!)
	uint32								ReferenceCount;
	uint32								OpenHandleCount;
	// for the sample fsd, there exists a 1-1 correspondence between an
	//	object name structure and a FCB
	PtrSFsdObjectName					FCBName;
	// we will maintain some time information here to make our life easier
	LARGE_INTEGER						CreationTime;
	LARGE_INTEGER						LastAccessTime;
	LARGE_INTEGER						LastWriteTime;
	// Byte-range file lock support (we roll our own)
	SFsdFileLockAnchor				FCBByteRangeLock;
	// The OPLOCK support package requires the following structure
	OPLOCK								FCBOplock;
} SFsdFCB, *PtrSFsdFCB;

/**************************************************************************
	the following FCBFlags values are relevant. These flag
	values are bit fields; therefore we can test whether
	a bit position is set (1) or not set (0).
**************************************************************************/
#define		SFSD_FCB_IN_INIT								(0x00000001)
#define		SFSD_FCB_IN_TEARDOWN							(0x00000002)
#define		SFSD_FCB_PAGE_FILE							(0x00000004)
#define		SFSD_FCB_DIRECTORY							(0x00000008)
#define		SFSD_FCB_ROOT_DIRECTORY						(0x00000018)
#define		SFSD_FCB_WRITE_THROUGH						(0x00000020)
#define		SFSD_FCB_MAPPED								(0x00000040)
#define		SFSD_FCB_FAST_IO_READ_IN_PROGESS			(0x00000080)
#define		SFSD_FCB_FAST_IO_WRITE_IN_PROGESS		(0x00000100)
#define		SFSD_FCB_DELETE_ON_CLOSE					(0x00000200)
#define		SFSD_FCB_MODIFIED								(0x00000400)
#define		SFSD_FCB_ACCESSED								(0x00000800)
#define		SFSD_FCB_READ_ONLY							(0x00001000)

#define		SFSD_INITIALIZED_MAIN_RESOURCE			(0x00002000)
#define		SFSD_INITIALIZED_PAGING_IO_RESOURCE		(0x00004000)

#define		SFSD_FCB_NOT_FROM_ZONE						(0x80000000)

/**************************************************************************
	A logical volume is represented using the following structure.
	This structure is allocated as part of the device extension
	for a device object that this sample FSD will create, to represent
	the mounted logical volume.

	NOTE: If you were to extend this sample FSD to be a "real" FSD,
			you would be worried about allocated clusters/sectiors,
			bitmaps providing such information for the mounted volume,
			dirty/modified clusters/sectiors etc.
			This sample FSD does not maintain such information in the
			in-memory VCB, though you may wish to consider it.
**************************************************************************/
typedef struct _SFsdVolumeControlBlock {
	SFsdIdentifier						NodeIdentifier;
	// a resource to protect the fields contained within the VCB
	ERESOURCE							VCBResource;
	// each VCB is accessible off a global linked list
	LIST_ENTRY							NextVCB;
	// each VCB points to a VPB structure created by the NT I/O Manager
	PVPB									PtrVPB;
	// a set of flags that might mean something useful
	uint32								VCBFlags;
	// A count of the number of open files/directories
	//	As long as the count is != 0, the volume cannot
	//	be dismounted or locked.
	uint32								VCBOpenCount;
	// a global list of all FCB structures associated with the VCB
	LIST_ENTRY							NextFCB;
	// we will maintain a global list of IRP's that are pending
	//	because of a directory notify request.
	LIST_ENTRY							NextNotifyIRP;
	// the above list is protected only by the mutex declared below
	KMUTEX								NotifyIRPMutex;
	// for each mounted volume, we create a device object. Here then
	//	is a back pointer to that device object
	PDEVICE_OBJECT						VCBDeviceObject;
	// We also retain a pointer to the physical device object on which we
	// have mounted ourselves. The I/O Manager passes us a pointer to this
	// device object when requesting a mount operation.
	PDEVICE_OBJECT						TargetDeviceObject;
	// the volume structure contains a pointer to the root directory FCB
	PtrSFsdFCB							PtrRootDirectoryFCB;
	// the complete name of the user visible drive letter we serve
	uint8									*PtrVolumePath;
	// For volume open operations, we do not create a FCB (we use the VCB
	//	directly instead). Therefore, all CCB structures for the volume
	//	open operation are linked directly off the VCB
	LIST_ENTRY							VolumeOpenListHead;
	// Pointer to a stream file object created for the volume information
	// to be more easily read from secondary storage (with the support of
	// the NT Cache Manager).
	PFILE_OBJECT						PtrStreamFileObject;
	// Required to use the Cache Manager.
   SECTION_OBJECT_POINTERS			SectionObject;
	// File sizes required to use the Cache Manager.
	LARGE_INTEGER						AllocationSize;
	LARGE_INTEGER						FileSize;
	LARGE_INTEGER						ValidDataLength;
} SFsdVCB, *PtrSFsdVCB;

// some valid flags for the VCB
#define			SFSD_VCB_FLAGS_VOLUME_MOUNTED			(0x00000001)
#define			SFSD_VCB_FLAGS_VOLUME_LOCKED			(0x00000002)
#define			SFSD_VCB_FLAGS_BEING_DISMOUNTED		(0x00000004)
#define			SFSD_VCB_FLAGS_SHUTDOWN					(0x00000008)
#define			SFSD_VCB_FLAGS_VOLUME_READ_ONLY		(0x00000010)

#define			SFSD_VCB_FLAGS_VCB_INITIALIZED		(0x00000020)

/**************************************************************************
	The IRP context encapsulates the current request. This structure is
	used in the "common" dispatch routines invoked either directly in
	the context of the original requestor, or indirectly in the context
	of a system worker thread.
**************************************************************************/
typedef struct _SFsdIrpContext {
	SFsdIdentifier				NodeIdentifier;
	uint32						IrpContextFlags;
	// copied from the IRP
	uint8							MajorFunction;
	// copied from the IRP
	uint8							MinorFunction;
	// to queue this IRP for asynchronous processing
	WORK_QUEUE_ITEM			WorkQueueItem;
	// the IRP for which this context structure was created
	PIRP							Irp;
	// the target of the request (obtained from the IRP)
	PDEVICE_OBJECT				TargetDeviceObject;
	// if an exception occurs, we will store the code here
	NTSTATUS						SavedExceptionCode;
} SFsdIrpContext, *PtrSFsdIrpContext;

#define			SFSD_IRP_CONTEXT_CAN_BLOCK				(0x00000001)
#define			SFSD_IRP_CONTEXT_WRITE_THROUGH		(0x00000002)
#define			SFSD_IRP_CONTEXT_EXCEPTION				(0x00000004)
#define			SFSD_IRP_CONTEXT_DEFERRED_WRITE		(0x00000008)
#define			SFSD_IRP_CONTEXT_ASYNC_PROCESSING	(0x00000010)
#define			SFSD_IRP_CONTEXT_NOT_TOP_LEVEL		(0x00000020)

#define			SFSD_IRP_CONTEXT_NOT_FROM_ZONE		(0x80000000)

/**************************************************************************
	we will store all of our global variables in one structure.
	Global variables are not specific to any mounted volume BUT
	by definition are required for successful operation of the
	FSD implementation.
**************************************************************************/
typedef struct _SFsdData {
	SFsdIdentifier				NodeIdentifier;
	// the fields in this list are protected by the following resource
	ERESOURCE					GlobalDataResource;
	// each driver has a driver object created for it by the NT I/O Mgr.
	//	we are no exception to this rule.
	PDRIVER_OBJECT				SFsdDriverObject;
	// we will create a device object for our FSD as well ...
	//	Although not really required, it helps if a helper application
	//	writen by us wishes to send us control information via
	//	IOCTL requests ...
	PDEVICE_OBJECT				SFsdDeviceObject;
	// we will keep a list of all logical volumes for our sample FSD
	LIST_ENTRY					NextVCB;
	// the NT Cache Manager, the I/O Manager and we will conspire
	//	to bypass IRP usage using the function pointers contained
	//	in the following structure
	FAST_IO_DISPATCH			SFsdFastIoDispatch;
	// The NT Cache Manager uses the following call backs to ensure
	//	correct locking hierarchy is maintained
	CACHE_MANAGER_CALLBACKS	CacheMgrCallBacks;
	// structures allocated from a zone need some fields here. Note
	//	that under version 4.0, it might be better to use lookaside
	//	lists
	KSPIN_LOCK					ZoneAllocationSpinLock;
	ZONE_HEADER					ObjectNameZoneHeader;
	ZONE_HEADER					CCBZoneHeader;
	ZONE_HEADER					FCBZoneHeader;
	ZONE_HEADER					ByteLockZoneHeader;
	ZONE_HEADER					IrpContextZoneHeader;
	void							*ObjectNameZone;
	void							*CCBZone;
	void							*FCBZone;
	void							*ByteLockZone;
	void							*IrpContextZone;
	// currently, there is a single default zone size value used for
	//	all zones. This should ideally be changed by you to be 1 per
	//	type of zone (e.g. a default size for the FCB zone might be
	//	different from the default size for the ByteLock zone).

	//	Of course, you will need to use different values (min/max)
	//	for lookaside lists (if you decide to use them instead)
	uint32						DefaultZoneSizeInNumStructs;
	// some state information is maintained in the flags field
	uint32						SFsdFlags;
	// Handle returned by the MUP is stored here.
	HANDLE						MupHandle;
} SFsdData, *PtrSFsdData;

// valid flag values for the global data structure
#define		SFSD_DATA_FLAGS_RESOURCE_INITIALIZED		(0x00000001)
#define		SFSD_DATA_FLAGS_ZONES_INITIALIZED			(0x00000002)

// a default size of the number of pages of non-paged pool allocated
//	for each of the zones ...

//	Note that the values are absolutely arbitrary, the only information
//	worth using from the values themselves is that they increase for
//	larger systems (i.e. systems with more memory)
#define		SFSD_DEFAULT_ZONE_SIZE_SMALL_SYSTEM			(0x4)
#define		SFSD_DEFAULT_ZONE_SIZE_MEDIUM_SYSTEM		(0x8)
#define		SFSD_DEFAULT_ZONE_SIZE_LARGE_SYSTEM			(0xc)

// another simplistic (brain dead ? :-) method used is to simply double
//	the values for a "server" machine

//	So, for all you guys who "modified" the registry ;-) to change the
//	wkstation into a server, tough luck !
#define		SFSD_NTAS_MULTIPLE								(0x2)

/***************************************************************************
The following locking hierarchy is maintained in this sample filesystem
driver:

(a) the global structure resource can be acquired at any time. However,
    it is an "end resource" i.e. once acquired, no other resource can
	 be obtained until the global structure resource is released.
(b) the logical volume resource must be acquired (if required) before
	 any of the other resources are acquired.
(c) a file control block can be acquired next (if required). If two
    FCB structures need to be acquired, the FCB "higher" in the directory
	 tree must be acquired first.
	 For a FCB, the "main resource" must be acquired first before a
	 "paging i/o" resource is acquired.

Whenever a file is opened, the logical volume structure is referenced.
This ensures that the volume cannot be dismounted while any file is open.

***************************************************************************/

#endif	_SFSD_STRUCTURES_H_	// has this file been included?

