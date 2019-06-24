/*************************************************************************
*
* File: protos.h
*
* Module: Sample File System Driver (Kernel mode execution only)
*
* Description:
*	Contains the prototypes for functions in this sample FSD.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#ifndef	_SFSD_PROTOS_H_
#define	_SFSD_PROTOS_H_

/*************************************************************************
* Prototypes for the file sfsdinit.c
*************************************************************************/
extern NTSTATUS DriverEntry(
PDRIVER_OBJECT				DriverObject,		// created by the I/O sub-system
PUNICODE_STRING			RegistryPath);		// path to the registry key

extern void SFsdInitializeFunctionPointers(
PDRIVER_OBJECT				DriverObject);		// created by the I/O sub-system

/*************************************************************************
* Prototypes for the file create.c
*************************************************************************/
extern NTSTATUS SFsdCreate(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS SFsdCommonCreate(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

extern NTSTATUS SFsdOpenVolume(
PtrSFsdVCB					PtrVCB,					// volume to be opened
PtrSFsdIrpContext			PtrIrpContext,			// IRP context
PIRP							PtrIrp,					// original/user IRP
unsigned short				ShareAccess,			// share access
PIO_SECURITY_CONTEXT		PtrSecurityContext,	// caller's context (incl access)
PFILE_OBJECT				PtrNewFileObject);	// I/O Mgr. created file object

extern void SFsdInitializeFCB(
PtrSFsdFCB					PtrNewFCB,		// FCB structure to be initialized
PtrSFsdVCB					PtrVCB,			// logical volume (VCB) pointer
PtrSFsdObjectName			PtrObjectName,	// name of the object
uint32						Flags,			// is this a file/directory, etc.
PFILE_OBJECT				PtrFileObject);// optional file object to be initialized

/*************************************************************************
* Prototypes for the file misc.c
*************************************************************************/
extern NTSTATUS SFsdInitializeZones(
void);

extern void SFsdDestroyZones(
void);

extern BOOLEAN SFsdIsIrpTopLevel(
PIRP							Irp);					// the IRP sent to our dispatch routine

extern long SFsdExceptionFilter(
PtrSFsdIrpContext			PtrIrpContext,
PEXCEPTION_POINTERS		PtrExceptionPointers);

extern NTSTATUS SFsdExceptionHandler(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							Irp);

extern void SFsdLogEvent(
NTSTATUS						SFsdEventLogId,	// the SFsd private message id
NTSTATUS						RC);					// any NT error code we wish to log ...

extern PtrSFsdObjectName SFsdAllocateObjectName(
void);

extern void SFsdReleaseObjectName(
PtrSFsdObjectName			PtrObjectName);

extern PtrSFsdCCB SFsdAllocateCCB(
void);

extern void SFsdReleaseCCB(
PtrSFsdCCB					PtrCCB);

extern PtrSFsdFCB SFsdAllocateFCB(
void);

extern NTSTATUS SFsdCreateNewFCB(
PtrSFsdFCB					*ReturnedFCB,
PLARGE_INTEGER				AllocationSize,
PLARGE_INTEGER				EndOfFile,
PFILE_OBJECT				PtrFileObject,
PtrSFsdVCB					PtrVCB);

extern void SFsdReleaseFCB(
PtrSFsdFCB					PtrFCB);

extern PtrSFsdFileLockInfo SFsdAllocateByteLocks(
void);

extern void SFsdReleaseByteLocks(
PtrSFsdFileLockInfo		PtrByteLocks);

extern PtrSFsdIrpContext SFsdAllocateIrpContext(
PIRP							Irp,
PDEVICE_OBJECT				PtrTargetDeviceObject);

extern void SFsdReleaseIrpContext(
PtrSFsdIrpContext			PtrIrpContext);

extern NTSTATUS SFsdPostRequest(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

extern void SFsdCommonDispatch(
void							*Context);	// actually an IRPContext structure

extern void SFsdInitializeVCB(
PDEVICE_OBJECT				PtrVolumeDeviceObject,
PDEVICE_OBJECT				PtrTargetDeviceObject,
PVPB							PtrVPB);

/*************************************************************************
* Prototypes for the file cleanup.c
*************************************************************************/
extern NTSTATUS SFsdCleanup(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonCleanup(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

/*************************************************************************
* Prototypes for the file close.c
*************************************************************************/
extern NTSTATUS SFsdClose(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonClose(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

/*************************************************************************
* Prototypes for the file read.c
*************************************************************************/
extern NTSTATUS SFsdRead(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonRead(
PtrSFsdIrpContext			PtrIrpContext,
PIRP				      	PtrIrp);

extern void *SFsdGetCallersBuffer(
PIRP							PtrIrp);

extern NTSTATUS SFsdLockCallersBuffer(
PIRP							PtrIrp,
BOOLEAN						IsReadOperation,
uint32						Length);

extern void SFsdMdlComplete(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PIO_STACK_LOCATION		PtrIoStackLocation,
BOOLEAN						ReadCompletion);

/*************************************************************************
* Prototypes for the file write.c
*************************************************************************/
extern NTSTATUS SFsdWrite(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonWrite(
PtrSFsdIrpContext			PtrIrpContext,
PIRP				      	PtrIrp);

extern void SFsdDeferredWriteCallBack (
void							*Context1,			// Should be PtrIrpContext
void							*Context2);			// Should be PtrIrp

/*************************************************************************
* Prototypes for the file fileinfo.c
*************************************************************************/
extern NTSTATUS SFsdFileInfo(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonFileInfo(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

extern NTSTATUS	SFsdGetBasicInformation(
PtrSFsdFCB					PtrFCB,
PFILE_BASIC_INFORMATION	PtrBuffer,
long							*PtrReturnedLength);

extern NTSTATUS	SFsdSetBasicInformation(
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB,
PFILE_OBJECT				PtrFileObject,
PFILE_BASIC_INFORMATION	PtrBuffer);

extern NTSTATUS	SFsdSetDispositionInformation(
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB,
PtrSFsdVCB					PtrVCB,
PFILE_OBJECT				PtrFileObject,
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PFILE_DISPOSITION_INFORMATION	PtrBuffer);

extern NTSTATUS	SFsdSetAllocationInformation(
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB,
PtrSFsdVCB					PtrVCB,
PFILE_OBJECT				PtrFileObject,
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PFILE_ALLOCATION_INFORMATION	PtrBuffer);

/*************************************************************************
* Prototypes for the file flush.c
*************************************************************************/
extern NTSTATUS SFsdFlush(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonFlush(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

extern void SFsdFlushAFile(
PtrSFsdNTRequiredFCB		PtrReqdFCB,
PIO_STATUS_BLOCK			PtrIoStatus);

extern void SFsdFlushLogicalVolume(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PtrSFsdVCB					PtrVCB);

extern NTSTATUS SFsdFlushCompletion(
PDEVICE_OBJECT				PtrDeviceObject,
PIRP							PtrIrp,
PVOID							Context);

/*************************************************************************
* Prototypes for the file dircntrl.c
*************************************************************************/
extern NTSTATUS SFsdDirControl(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonDirControl(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

extern NTSTATUS	SFsdQueryDirectory(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PIO_STACK_LOCATION		PtrIoStackLocation,
PFILE_OBJECT				PtrFileObject,
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB);

extern NTSTATUS	SFsdNotifyChangeDirectory(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp,
PIO_STACK_LOCATION		PtrIoStackLocation,
PFILE_OBJECT				PtrFileObject,
PtrSFsdFCB					PtrFCB,
PtrSFsdCCB					PtrCCB);

/*************************************************************************
* Prototypes for the file devcntrl.c
*************************************************************************/
extern NTSTATUS SFsdDeviceControl(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS SFsdCommonDeviceControl(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

extern NTSTATUS SFsdDevIoctlCompletion(
PDEVICE_OBJECT				PtrDeviceObject,
PIRP							PtrIrp,
void							*Context);

extern NTSTATUS SFsdHandleQueryPath(
void							*BufferPointer);

/*************************************************************************
* Prototypes for the file shutdown.c
*************************************************************************/
extern NTSTATUS SFsdShutdown(
PDEVICE_OBJECT				DeviceObject,		// the logical volume device object
PIRP							Irp);					// I/O Request Packet

extern NTSTATUS	SFsdCommonShutdown(
PtrSFsdIrpContext			PtrIrpContext,
PIRP							PtrIrp);

/*************************************************************************
* Prototypes for the file fastio.c
*************************************************************************/
extern BOOLEAN SFsdFastIoCheckIfPossible(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
IN BOOLEAN						CheckForReadOperation,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoQueryBasicInfo(
IN PFILE_OBJECT					FileObject,
IN BOOLEAN							Wait,
OUT PFILE_BASIC_INFORMATION	Buffer,
OUT PIO_STATUS_BLOCK 			IoStatus,
IN PDEVICE_OBJECT					DeviceObject);

extern BOOLEAN SFsdFastIoQueryStdInfo(
IN PFILE_OBJECT						FileObject,
IN BOOLEAN								Wait,
OUT PFILE_STANDARD_INFORMATION 	Buffer,
OUT PIO_STATUS_BLOCK 				IoStatus,
IN PDEVICE_OBJECT						DeviceObject);

extern BOOLEAN SFsdFastIoLock(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
BOOLEAN							FailImmediately,
BOOLEAN							ExclusiveLock,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoUnlockSingle(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoUnlockAll(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoUnlockAllByKey(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern void SFsdFastIoAcqCreateSec(
IN PFILE_OBJECT				FileObject);

extern void SFsdFastIoRelCreateSec(
IN PFILE_OBJECT				FileObject);

extern BOOLEAN SFsdAcqLazyWrite(
IN PVOID							Context,
IN BOOLEAN						Wait);

extern void SFsdRelLazyWrite(
IN PVOID							Context);

extern BOOLEAN SFsdAcqReadAhead(
IN PVOID							Context,
IN BOOLEAN						Wait);

extern void SFsdRelReadAhead(
IN PVOID							Context);

// the remaining are only valid under NT Version 4.0 and later
#if(_WIN32_WINNT >= 0x0400)

extern BOOLEAN SFsdFastIoQueryNetInfo(
IN PFILE_OBJECT									FileObject,
IN BOOLEAN											Wait,
OUT PFILE_NETWORK_OPEN_INFORMATION 			Buffer,
OUT PIO_STATUS_BLOCK 							IoStatus,
IN PDEVICE_OBJECT									DeviceObject);

extern BOOLEAN SFsdFastIoMdlRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoMdlReadComplete(
IN PFILE_OBJECT				FileObject,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoPrepareMdlWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFsdFastIoMdlWriteComplete(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject);

extern NTSTATUS SFsdFastIoAcqModWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				EndingOffset,
OUT PERESOURCE					*ResourceToRelease,
IN PDEVICE_OBJECT				DeviceObject);

extern NTSTATUS SFsdFastIoRelModWrite(
IN PFILE_OBJECT				FileObject,
IN PERESOURCE					ResourceToRelease,
IN PDEVICE_OBJECT				DeviceObject);

extern NTSTATUS SFsdFastIoAcqCcFlush(
IN PFILE_OBJECT				FileObject,
IN PDEVICE_OBJECT				DeviceObject);

extern NTSTATUS SFsdFastIoRelCcFlush(
IN PFILE_OBJECT				FileObject,
IN PDEVICE_OBJECT				DeviceObject);

#endif	// (_WIN32_WINNT >= 0x0400)

#endif	// _SFSD_PROTOS_H_

