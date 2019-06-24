/*************************************************************************
*
* File: protos.h
*
* Module: Sample Filter Driver (Kernel mode execution only)
*
* Description:
*	Contains the prototypes for functions in this sample filter driver.
*
* Author: R. Nagar
*
* (c) 1996-97 Rajeev Nagar, All Rights Reserved
*
*************************************************************************/

#ifndef	_SFILTER_PROTOS_H_
#define	_SFILTER_PROTOS_H_

/************************************************************************
* prototypes for the file sfilinit.c
************************************************************************/
extern NTSTATUS DriverEntry(
PDRIVER_OBJECT		DriverObject,		// created by the I/O sub-system
PUNICODE_STRING	RegistryPath);		// path to the registry key

extern void SFilterInitializeFunctionPointers(
PDRIVER_OBJECT		DriverObject);		// created by the I/O sub-system

/************************************************************************
* prototypes for the file fastio.c
************************************************************************/
extern BOOLEAN SFilterFastIoCheckIfPossible(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
IN BOOLEAN						CheckForReadOperation,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN BOOLEAN						Wait,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoQueryBasicInfo(
IN PFILE_OBJECT					FileObject,
IN BOOLEAN							Wait,
OUT PFILE_BASIC_INFORMATION	Buffer,
OUT PIO_STATUS_BLOCK 			IoStatus,
IN PDEVICE_OBJECT					DeviceObject);

extern BOOLEAN SFilterFastIoQueryStdInfo(
IN PFILE_OBJECT						FileObject,
IN BOOLEAN								Wait,
OUT PFILE_STANDARD_INFORMATION 	Buffer,
OUT PIO_STATUS_BLOCK 				IoStatus,
IN PDEVICE_OBJECT						DeviceObject);

extern BOOLEAN SFilterFastIoLock(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
BOOLEAN							FailImmediately,
BOOLEAN							ExclusiveLock,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoUnlockSingle(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN PLARGE_INTEGER				Length,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoUnlockAll(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoUnlockAllByKey(
IN PFILE_OBJECT				FileObject,
PEPROCESS						ProcessId,
ULONG								Key,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoDeviceControl(
IN PFILE_OBJECT 		FileObject,
IN BOOLEAN				Wait,
IN PVOID					InputBuffer,
IN ULONG					InputBufferLength,
OUT PVOID				OutputBuffer,
IN ULONG					OutputBufferLength,
IN ULONG					IoControlCode,
OUT PIO_STATUS_BLOCK	IoStatus,
IN PDEVICE_OBJECT		DeviceObject);

extern void SFilterFastIoAcqCreateSec(
IN PFILE_OBJECT			FileObject);

extern void SFilterFastIoRelCreateSec(
IN PFILE_OBJECT			FileObject);

#if(_WIN32_WINNT >= 0x0400)

extern void SFilterFastIoDetachDevice(
PDEVICE_OBJECT				SourceDeviceObject,		// our device object
PDEVICE_OBJECT				TargetDeviceObject);

extern BOOLEAN SFilterFastIoQueryNetInfo(
IN PFILE_OBJECT									FileObject,
IN BOOLEAN											Wait,
OUT PFILE_NETWORK_OPEN_INFORMATION 			Buffer,
OUT PIO_STATUS_BLOCK 							IoStatus,
IN PDEVICE_OBJECT									DeviceObject);

extern BOOLEAN SFilterFastIoMdlRead(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoMdlReadComplete(
IN PFILE_OBJECT				FileObject,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoPrepareMdlWrite(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoMdlWriteComplete(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoReadCompressed(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
OUT struct _COMPRESSED_DATA_INFO	*CompressedDataInfo,
IN ULONG							CompressedDataInfoLength,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoWriteCompressed(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
IN ULONG							Length,
IN ULONG							LockKey,
OUT PVOID						Buffer,
OUT PMDL							*MdlChain,
OUT PIO_STATUS_BLOCK			IoStatus,
OUT struct _COMPRESSED_DATA_INFO	*CompressedDataInfo,
IN ULONG							CompressedDataInfoLength,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoMdlReadCompleteCompressed(
IN PFILE_OBJECT				FileObject,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject);

extern BOOLEAN SFilterFastIoMdlWriteCompleteCompressed(
IN PFILE_OBJECT				FileObject,
IN PLARGE_INTEGER				FileOffset,
OUT PMDL							MdlChain,
IN PDEVICE_OBJECT				DeviceObject);

extern NTSTATUS SFilterFastIoAcqModWrite(
IN PFILE_OBJECT					FileObject,
IN PLARGE_INTEGER					EndingOffset,
OUT PERESOURCE						*ResourceToRelease,
IN PDEVICE_OBJECT					DeviceObject);

extern NTSTATUS SFilterFastIoRelModWrite(
IN PFILE_OBJECT				FileObject,
IN PERESOURCE					ResourceToRelease,
IN PDEVICE_OBJECT				DeviceObject);

extern NTSTATUS SFilterFastIoAcqCcFlush(
IN PFILE_OBJECT			FileObject,
IN PDEVICE_OBJECT			DeviceObject);

extern NTSTATUS SFilterFastIoRelCcFlush(
IN PFILE_OBJECT			FileObject,
IN PDEVICE_OBJECT			DeviceObject);

extern BOOLEAN SFilterFastIoQueryOpen(
IN PIRP										Irp,
OUT PFILE_NETWORK_OPEN_INFORMATION	NetworkInformation,
IN PDEVICE_OBJECT							DeviceObject);

#endif	//_WIN32_WINNT >= 0x0400

/************************************************************************
* prototypes for the file attach.c
************************************************************************/
extern NTSTATUS SFilterAttachTarget(
PDEVICE_OBJECT				PtrTargetDeviceObject,
PDEVICE_OBJECT				*PtrNewDeviceObject);

extern void SFilterDetachTarget(
PDEVICE_OBJECT					SourceDeviceObject,
PDEVICE_OBJECT					TargetDeviceObject,
PtrSFilterDeviceExtension	PtrDeviceExtension);

/************************************************************************
* prototypes for the file dispatch.c
************************************************************************/
extern NTSTATUS SFilterDefaultDispatch (
PDEVICE_OBJECT		DeviceObject,	// Our device object
PIRP					Irp);				// I/O Request Packet

extern NTSTATUS SFilterDefaultCompletion(
PDEVICE_OBJECT			PtrDeviceObject,
PIRP						PtrIrp,
void						*Context);

/************************************************************************
* prototypes for the file misc.c
************************************************************************/
extern void SFilterInitDevExtension(
PtrSFilterDeviceExtension	PtrDeviceExtension,
PDEVICE_OBJECT					PtrAssociatedDeviceObject,
uint32							NodeType);

extern void SFilterDeleteDevExtension(
PtrSFilterDeviceExtension	PtrDeviceExtension,
BOOLEAN							ResourceAcquired);

extern NTSTATUS SFilterCreateDirectory(
PWCHAR			DirectoryNameStr,
PHANDLE			PtrReturnedHandle,
BOOLEAN			MakeTemporaryObject);

extern void SFilterReinitialize(
PDRIVER_OBJECT			DriverObject, 		// representing the filter driver
void						*Context,			// this filter driver supplies registry path
ULONG						Count);				// # times this function invoked

extern void SFilterFSDNotification(
PDEVICE_OBJECT				PtrTargetFileSystemDeviceObject,
BOOLEAN						DriverActive);

/************************************************************************
* prototypes for the file create.c
************************************************************************/

extern NTSTATUS SFilterCreate(
PDEVICE_OBJECT		DeviceObject,	// Our device object
PIRP					Irp);				// I/O Request Packet

/************************************************************************
* prototypes for the file close.c
************************************************************************/

extern NTSTATUS SFilterCleanupOrClose(
PDEVICE_OBJECT		DeviceObject,	// Our device object
PIRP					Irp);				// I/O Request Packet

/************************************************************************
* prototypes for the file fsctrl.c
************************************************************************/

extern NTSTATUS SFilterFSControl(
PDEVICE_OBJECT		DeviceObject,	// Our device object
PIRP					Irp);				// I/O Request Packet

extern NTSTATUS SFilterMountVolumeCompletion(
PDEVICE_OBJECT			PtrDeviceObject,
PIRP						PtrIrp,
void						*Context);

extern void SFilterMountAttach(
void						*Context);

#endif	// _SFILTER_PROTOS_H_

