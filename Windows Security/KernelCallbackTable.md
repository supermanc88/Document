## 怎样获取KernelCallbackTable


首先KernelCallbackTable是存放在`PEB`中

```
kd> !PEB
Wow64 PEB32 at 7efde000
    InheritedAddressSpace:    No
    ReadImageFileExecOptions: No
    BeingDebugged:            No
    ImageBaseAddress:         0000000000400000
    NtGlobalFlag:             0
    NtGlobalFlag2:            0
    Ldr                       0000000077b90200
    ***  _PEB_LDR_DATA type was not found...
    *** unable to read Ldr table at 0000000077b90200
    SubSystemData:     0000000000000000
    ProcessHeap:       0000000000520000
    ProcessParameters: 0000000000521268
    ***  _CURDIR type was not found...
    WindowTitle:  '< Name not readable >'
    ImageFile:    '< Name not readable >'
    CommandLine:  '< Name not readable >'
    DllPath:      '< Name not readable >'
    Environment:  0000000000000000
       Unable to read Environment string.


Wow64 PEB at 000000007efdf000
...
```

查看`PEB`结构体：

```
kd> dt nt!_PEB 000000007efdf000
   +0x000 InheritedAddressSpace : 0 ''
   +0x001 ReadImageFileExecOptions : 0 ''
   +0x002 BeingDebugged    : 0 ''
   +0x003 BitField         : 0 ''
   +0x003 ImageUsesLargePages : 0y0
   +0x003 IsProtectedProcess : 0y0
   +0x003 IsLegacyProcess  : 0y0
   +0x003 IsImageDynamicallyRelocated : 0y0
   +0x003 SkipPatchingUser32Forwarders : 0y0
   +0x003 SpareBits        : 0y000
   +0x008 Mutant           : 0xffffffff`ffffffff Void
   +0x010 ImageBaseAddress : 0x00000000`00400000 Void
   +0x018 Ldr              : 0x00000000`779e2640 _PEB_LDR_DATA
   +0x020 ProcessParameters : 0x00000000`002c1d50 _RTL_USER_PROCESS_PARAMETERS
   +0x028 SubSystemData    : (null) 
   +0x030 ProcessHeap      : 0x00000000`002c0000 Void
   +0x038 FastPebLock      : 0x00000000`779ea900 _RTL_CRITICAL_SECTION
   +0x040 AtlThunkSListPtr : (null) 
   +0x048 IFEOKey          : (null) 
   +0x050 CrossProcessFlags : 1
   +0x050 ProcessInJob     : 0y1
   +0x050 ProcessInitializing : 0y0
   +0x050 ProcessUsingVEH  : 0y0
   +0x050 ProcessUsingVCH  : 0y0
   +0x050 ProcessUsingFTH  : 0y0
   +0x050 ReservedBits0    : 0y000000000000000000000000000 (0)
   +0x058 KernelCallbackTable : 0x00000000`752b1510 Void
   +0x058 UserSharedInfoPtr : 0x00000000`752b1510 Void
   +0x060 SystemReserved   : [1] 0
   +0x064 AtlThunkSListPtr32 : 0
   +0x068 ApiSetMap        : 0x00000000`00040000 Void
   +0x070 TlsExpansionCounter : 0
   +0x078 TlsBitmap        : 0x00000000`779e2590 Void
   +0x080 TlsBitmapBits    : [2] 0x7ffff
   +0x088 ReadOnlySharedMemoryBase : 0x00000000`7efe0000 Void
   +0x090 HotpatchInformation : (null) 
   +0x098 ReadOnlyStaticServerData : 0x00000000`7efe0a90  -> (null) 
   +0x0a0 AnsiCodePageData : 0x00000000`7efa0000 Void
   +0x0a8 OemCodePageData  : 0x00000000`7efa0000 Void
   +0x0b0 UnicodeCaseTableData : 0x00000000`7efd0028 Void
   +0x0b8 NumberOfProcessors : 1
   +0x0bc NtGlobalFlag     : 0
   +0x0c0 CriticalSectionTimeout : _LARGE_INTEGER 0xffffe86d`079b8000
   +0x0c8 HeapSegmentReserve : 0x100000
   +0x0d0 HeapSegmentCommit : 0x2000
   +0x0d8 HeapDeCommitTotalFreeThreshold : 0x10000
   +0x0e0 HeapDeCommitFreeBlockThreshold : 0x1000
   +0x0e8 NumberOfHeaps    : 2
   +0x0ec MaximumNumberOfHeaps : 0x10
   +0x0f0 ProcessHeaps     : 0x00000000`779ea6c0  -> 0x00000000`002c0000 Void
   +0x0f8 GdiSharedHandleTable : 0x00000000`007b0000 Void
   +0x100 ProcessStarterHelper : (null) 
   +0x108 GdiDCAttributeList : 0x14
   +0x110 LoaderLock       : 0x00000000`779e7490 _RTL_CRITICAL_SECTION
   +0x118 OSMajorVersion   : 6
   +0x11c OSMinorVersion   : 1
   +0x120 OSBuildNumber    : 0x1db1
   +0x122 OSCSDVersion     : 0x100
   +0x124 OSPlatformId     : 2
   +0x128 ImageSubsystem   : 2
   +0x12c ImageSubsystemMajorVersion : 4
   +0x130 ImageSubsystemMinorVersion : 0
   +0x138 ActiveProcessAffinityMask : 1
   +0x140 GdiHandleBuffer  : [60] 0
   +0x230 PostProcessInitRoutine : (null) 
   +0x238 TlsExpansionBitmap : 0x00000000`779e2580 Void
   +0x240 TlsExpansionBitmapBits : [32] 1
   +0x2c0 SessionId        : 1
   +0x2c8 AppCompatFlags   : _ULARGE_INTEGER 0x0
   +0x2d0 AppCompatFlagsUser : _ULARGE_INTEGER 0x0
   +0x2d8 pShimData        : (null) 
   +0x2e0 AppCompatInfo    : (null) 
   +0x2e8 CSDVersion       : _UNICODE_STRING ""
   +0x2f8 ActivationContextData : (null) 
   +0x300 ProcessAssemblyStorageMap : (null) 
   +0x308 SystemDefaultActivationContextData : 0x00000000`00190000 _ACTIVATION_CONTEXT_DATA
   +0x310 SystemAssemblyStorageMap : (null) 
   +0x318 MinimumStackCommit : 0
   +0x320 FlsCallback      : (null) 
   +0x328 FlsListHead      : _LIST_ENTRY [ 0x00000000`7efdf328 - 0x00000000`7efdf328 ]
   +0x338 FlsBitmap        : 0x00000000`779e2570 Void
   +0x340 FlsBitmapBits    : [4] 1
   +0x350 FlsHighIndex     : 0
   +0x358 WerRegistrationData : (null) 
   +0x360 WerShipAssertPtr : (null) 
   +0x368 pContextData     : 0x00000000`001a0000 Void
   +0x370 pImageHeaderHash : (null) 
   +0x378 TracingFlags     : 0
   +0x378 HeapTracingEnabled : 0y0
   +0x378 CritSecTracingEnabled : 0y0
   +0x378 SpareTracingBits : 0y000000000000000000000000000000 (0)
```

由上面的结果可知`KernelCallbackTable`是在地址`0x00000000752b1510`处。


查看`KernelCallbackTable`中的内容：

```
kd> dqs 0x00000000`752b1510 L100
00000000`752b1510  00000000`752e2634 wow64win!whcbfnCOPYDATA
00000000`752b1518  00000000`752e27c8 wow64win!whcbfnCOPYGLOBALDATA
00000000`752b1520  00000000`752e290c wow64win!whcbfnDWORD
00000000`752b1528  00000000`752e2bb8 wow64win!whcbfnNCDESTROY
00000000`752b1530  00000000`752e2d04 wow64win!whcbfnDWORDOPTINLPMSG
00000000`752b1538  00000000`752e2e74 wow64win!whcbfnINOUTDRAG
00000000`752b1540  00000000`752e3050 wow64win!whcbfnGETTEXTLENGTHS
00000000`752b1548  00000000`752e31c0 wow64win!whcbfnINCNTOUTSTRING
00000000`752b1550  00000000`752e3354 wow64win!whcbfnINCNTOUTSTRINGNULL
00000000`752b1558  00000000`752e34e0 wow64win!whcbfnINLPCOMPAREITEMSTRUCT
00000000`752b1560  00000000`752e3688 wow64win!whcbfnINLPCREATESTRUCT
00000000`752b1568  00000000`752e3888 wow64win!whcbfnINLPDELETEITEMSTRUCT
00000000`752b1570  00000000`752e39f8 wow64win!whcbfnINLPDRAWITEMSTRUCT
00000000`752b1578  00000000`752e3b84 wow64win!whcbfnINLPHELPINFOSTRUCT
00000000`752b1580  00000000`752e3d10 wow64win!whcbfnINLPHLPSTRUCT
00000000`752b1588  00000000`752e3e9c wow64win!whcbfnINLPMDICREATESTRUCT
00000000`752b1590  00000000`752e406c wow64win!whcbfnINOUTLPMEASUREITEMSTRUCT
00000000`752b1598  00000000`752e4244 wow64win!whcbfnINLPWINDOWPOS
00000000`752b15a0  00000000`752e43c0 wow64win!whcbfnINOUTLPPOINT5
00000000`752b15a8  00000000`752e4560 wow64win!whcbfnINOUTLPSCROLLINFO
00000000`752b15b0  00000000`752e4700 wow64win!whcbfnINOUTLPRECT
00000000`752b15b8  00000000`752e48b4 wow64win!whcbfnINOUTNCCALCSIZE
00000000`752b15c0  00000000`752e4bc0 wow64win!whcbfnINOUTLPWINDOWPOS
00000000`752b15c8  00000000`752e4da4 wow64win!whcbfnINPAINTCLIPBRD
00000000`752b15d0  00000000`752e4f30 wow64win!whcbfnINSIZECLIPBRD
00000000`752b15d8  00000000`752e508c wow64win!whcbfnINDESTROYCLIPBRD
00000000`752b15e0  00000000`752e51e4 wow64win!whcbfnINSTRING
00000000`752b15e8  00000000`752e53d4 wow64win!whcbfnINSTRINGNULL
00000000`752b15f0  00000000`752e55a0 wow64win!whcbfnINDEVICECHANGE
00000000`752b15f8  00000000`752e57ec wow64win!whcbfnPOWERBROADCAST
00000000`752b1600  00000000`752e5964 wow64win!whcbfnINOUTNEXTMENU
00000000`752b1608  00000000`752e5b0c wow64win!whcbfnOPTOUTLPDWORDOPTOUTLPDWORD
00000000`752b1610  00000000`752e5c74 wow64win!whcbfnOUTDWORDDWORD
00000000`752b1618  00000000`752e5dc0 wow64win!whcbfnOUTDWORDINDWORD
00000000`752b1620  00000000`752e5f18 wow64win!whcbfnOUTLPRECT
00000000`752b1628  00000000`752e606c wow64win!whcbfnOUTSTRING
00000000`752b1630  00000000`752e61f8 wow64win!whcbfnPOPTINLPUINT
00000000`752b1638  00000000`752e637c wow64win!whcbfnPOUTLPINT
00000000`752b1640  00000000`752e652c wow64win!whcbfnSENTDDEMSG
00000000`752b1648  00000000`752e668c wow64win!whcbfnINOUTSTYLECHANGE
00000000`752b1650  00000000`752e6818 wow64win!whcbfnHkINDWORD
00000000`752b1658  00000000`752e699c wow64win!whcbfnHkINLPCBTACTIVATESTRUCT
00000000`752b1660  00000000`752e6af4 wow64win!whcbfnHkINLPCBTCREATESTRUCT
00000000`752b1668  00000000`752e6ef8 wow64win!whcbfnHkINLPDEBUGHOOKSTRUCT
00000000`752b1670  00000000`752e7188 wow64win!whcbfnHkINLPMOUSEHOOKSTRUCTEX
00000000`752b1678  00000000`752e72e8 wow64win!whcbfnHkINLPKBDLLHOOKSTRUCT
00000000`752b1680  00000000`752e74a8 wow64win!whcbfnHkINLPMSLLHOOKSTRUCT
00000000`752b1688  00000000`752e766c wow64win!whcbfnHkINLPMSG
00000000`752b1690  00000000`752e780c wow64win!whcbfnHkINLPRECT
00000000`752b1698  00000000`752e7990 wow64win!whcbfnHkOPTINLPEVENTMSG
00000000`752b16a0  00000000`752e7b68 wow64win!whcbClientCopyDDEIn1
00000000`752b16a8  00000000`752e7d18 wow64win!whcbClientCopyDDEIn2
00000000`752b16b0  00000000`752e7e58 wow64win!whcbClientCopyDDEOut1
00000000`752b16b8  00000000`752e8020 wow64win!whcbClientCopyDDEOut2
00000000`752b16c0  00000000`752e8160 wow64win!whcbClientCopyImage
00000000`752b16c8  00000000`752e8288 wow64win!whcbClientEventCallback
00000000`752b16d0  00000000`752e83cc wow64win!whcbClientFindMnemChar
00000000`752b16d8  00000000`752e852c wow64win!whcbClientFreeDDEHandle
00000000`752b16e0  00000000`752e8644 wow64win!whcbClientFreeLibrary
00000000`752b16e8  00000000`752e8754 wow64win!whcbClientGetCharsetInfo
00000000`752b16f0  00000000`752e888c wow64win!whcbClientGetDDEFlags
00000000`752b16f8  00000000`752e89a4 wow64win!whcbClientGetDDEHookData
00000000`752b1700  00000000`752e8b0c wow64win!whcbClientGetListboxString
00000000`752b1708  00000000`752e8cb4 wow64win!whcbClientGetMessageMPH
00000000`752b1710  00000000`752e8e28 wow64win!whcbClientLoadImage
00000000`752b1718  00000000`752e8fac wow64win!whcbClientLoadLibrary
00000000`752b1720  00000000`752e9110 wow64win!whcbClientLoadMenu
00000000`752b1728  00000000`752e9264 wow64win!whcbClientLoadLocalT1Fonts
00000000`752b1730  00000000`752e934c wow64win!whcbClientPSMTextOut
00000000`752b1738  00000000`752e94b8 wow64win!whcbClientLpkDrawTextEx
00000000`752b1740  00000000`752e9668 wow64win!whcbClientExtTextOutW
00000000`752b1748  00000000`752e97e8 wow64win!whcbClientGetTextExtentPointW
00000000`752b1750  00000000`752e9980 wow64win!whcbClientCharToWchar
00000000`752b1758  00000000`752e9a6c wow64win!whcbClientAddFontResourceW
00000000`752b1760  00000000`752e9bdc wow64win!whcbClientThreadSetup
00000000`752b1768  00000000`752e9cc4 wow64win!whcbClientDeliverUserApc
00000000`752b1770  00000000`752e9dac wow64win!whcbClientNoMemoryPopup
00000000`752b1778  00000000`752e9e94 wow64win!whcbClientMonitorEnumProc
00000000`752b1780  00000000`752e9fc0 wow64win!whcbClientCallWinEventProc
00000000`752b1788  00000000`752ea0fc wow64win!whcbClientWaitMessageExMPH
00000000`752b1790  00000000`752ea1e8 wow64win!whcbClientWOWGetProcModule
00000000`752b1798  00000000`752ea314 wow64win!whcbClientWOWTask16SchedNotify
00000000`752b17a0  00000000`752ea400 wow64win!whcbClientImmLoadLayout
00000000`752b17a8  00000000`752ea64c wow64win!whcbClientImmProcessKey
00000000`752b17b0  00000000`752ea774 wow64win!whcbfnIMECONTROL
00000000`752b17b8  00000000`752ea93c wow64win!whcbfnINWPARAMDBCSCHAR
00000000`752b17c0  00000000`752eaa9c wow64win!whcbfnGETDBCSTEXTLENGTHS
00000000`752b17c8  00000000`752eac0c wow64win!whcbfnINLPKDRAWSWITCHWND
00000000`752b17d0  00000000`752ead9c wow64win!whcbClientLoadStringW
00000000`752b17d8  00000000`752eaeec wow64win!whcbClientLoadOLE
00000000`752b17e0  00000000`752eafd4 wow64win!whcbClientRegisterDragDrop
00000000`752b17e8  00000000`752eb0e0 wow64win!whcbClientRevokeDragDrop
00000000`752b17f0  00000000`752eb1ec wow64win!whcbfnINOUTMENUGETOBJECT
00000000`752b17f8  00000000`752eb38c wow64win!whcbClientPrinterThunk
00000000`752b1800  00000000`752eb478 wow64win!whcbfnOUTLPCOMBOBOXINFO
00000000`752b1808  00000000`752eb688 wow64win!whcbfnOUTLPSCROLLBARINFO
00000000`752b1810  00000000`752eb7e0 wow64win!whcbfnINLPUAHDRAWMENU
00000000`752b1818  00000000`752eb938 wow64win!whcbfnINLPUAHDRAWMENUITEM
00000000`752b1820  00000000`752ebb04 wow64win!whcbfnINLPUAHINITMENU
00000000`752b1828  00000000`752ebc5c wow64win!whcbfnINOUTLPUAHMEASUREMENUITEM
00000000`752b1830  00000000`752ebeac wow64win!whcbfnINLPUAHNCPAINTMENUPOPUP
00000000`752b1838  00000000`752ec004 wow64win!whcbfnOUTLPTITLEBARINFOEX
00000000`752b1840  00000000`752ec168 wow64win!whcbfnTOUCH
00000000`752b1848  00000000`752ec2d8 wow64win!whcbfnGESTURE
00000000`752b1850  00000000`752ec448 wow64win!whcbfnINPGESTURENOTIFYSTRUCT
00000000`752b1858  00000000`00000000
00000000`752b1860  00000000`00000000
00000000`752b1868  00000000`00000000
00000000`752b1870  00000000`00000001
00000000`752b1878  00000000`00000004
00000000`752b1880  00000000`00000002
00000000`752b1888  00000000`00000004
...
```

> Windbg 命令 `dqs`： 会显示给定范围的内存内容。当该内存是符号表中的一系列地址时，相应的符号也会显示出来。