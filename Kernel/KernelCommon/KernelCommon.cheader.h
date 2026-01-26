#pragma once

//CÍ·ÎÄ¼þ
//#define NDIS_SUPPORT_NDIS61 1
#include <initguid.h>
#include <fltKernel.h>
#include <ntifs.h>
#include <fwpmk.h>
#include <ntddk.h>
#include <wdm.h>
#include <Wdmsec.h>
#include <stdarg.h>
#include <ntdef.h>
#include <ntimage.h>
#define NTSTRSAFE_LIB
#define NTSTRSAFE_NO_CB_FUNCTIONS
#include <ntstrsafe.h>
#include <ntdddisk.h>
#include <mountdev.h>
#include <ntddvol.h>
#include <Aux_klib.h>
#include <wsk.h>         
#include <minwindef.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <wdmguid.h>
#include <ntddkbd.h>
#include <ntddscsi.h>
#include <srb.h>
#include <scsi.h>
#include <basetsd.h>
#include <immintrin.h>
#include <hidclass.h>