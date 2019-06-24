;/*************************************************************************
;*
;* File:		errmsg.msg
;*
;* Product:		Sample FSD
;*
;* Module:		Sample FSD Event Log Messages
;*
;* Description:
;*     Contains error strings in a format understandable to the message compiler.
;*     Please compile (using mc) with the -c option which will set the
;*     "Customer" bit in all errors.
;*     Use values beginning at 0xA000 (e.g. 0xA001) for the Sample FSD
;*     errors.
;*     Do NOT use %1 for insertion strings. The I/O manager assumes that
;*     the first insertion string is the name of the driver/device.
;*
;* (c) 1996-97 Rajeev Nagar, All Rights Reserved
;*
;*************************************************************************/
MessageIdTypedef=ULONG

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(IO=0x004)

MessageId=0xA001 Facility=IO Severity=Error
SymbolicName=SFSD_ERROR_INTERNAL_ERROR
Language=English
The Sample FSD encountered an internal error. Please check log data information.
.

