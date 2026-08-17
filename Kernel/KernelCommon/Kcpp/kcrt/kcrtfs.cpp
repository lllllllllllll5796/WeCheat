#include "KernelCommon.pch.h"
// #include <ntifs.h>
// #include <errno.h>
#include <stdio.h>

#include "kcrt.h"

#ifndef _countof
#    define _countof(x) (sizeof(x) / sizeof(x[0]))
#endif

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

char _CRT_CWD[260] = {0};

extern pfn_vsnprintf g_vsnprintf;

//EXTERN_C POBJECT_TYPE *IoDriverObjectType;

//EXTERN_C
//NTKERNELAPI
//NTSTATUS
//ObReferenceObjectByName(
//    IN PUNICODE_STRING ObjectName,
//    IN ULONG Attributes,
//    IN PACCESS_STATE PassedAccessState OPTIONAL,
//    IN ACCESS_MASK DesiredAccess OPTIONAL,
//    IN POBJECT_TYPE ObjectType OPTIONAL,
//    IN KPROCESSOR_MODE AccessMode,
//    IN OUT PVOID ParseContext OPTIONAL,
//    OUT PVOID *Object);

#define KCRTFS_POOL_DEFAULT_TAG 'sfck' // kcrt

const char *
_get_cwd()
{
    return _CRT_CWD;
}

void
_set_cwd(const char *path)
{
    strncpy(_CRT_CWD, path, _countof(_CRT_CWD));
}

BOOLEAN
_init_cwd() //һ����˵���õ�C:\Windows
{
    BOOLEAN ok;
    NTSTATUS st;
    HANDLE KeyHandle;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING KeyPath;

    wchar_t wpath[] = {//   \\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion
                       L'\\', L'R', L'e', L'g', L'i',  L's', L't', L'r', L'y',  L'\\', L'M',  L'a', L'c',
                       L'h',  L'i', L'n', L'e', L'\\', L'S', L'O', L'F', L'T',  L'W',  L'A',  L'R', L'E',
                       L'\\', L'M', L'i', L'c', L'r',  L'o', L's', L'o', L'f',  L't',  L'\\', L'W', L'i',
                       L'n',  L'd', L'o', L'w', L's',  L' ', L'N', L'T', L'\\', L'C',  L'u',  L'r', L'r',
                       L'e',  L'n', L't', L'V', L'e',  L'r', L's', L'i', L'o',  L'n',  0,     0};

    ok = FALSE;
    RtlInitUnicodeString(&KeyPath, wpath);
    InitializeObjectAttributes(&oa, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
    st = ZwOpenKey(&KeyHandle, KEY_READ, &oa);

    if (NT_SUCCESS(st))
    {
        ULONG nRes = 0;
        UNICODE_STRING ValueName;

        wchar_t wval[] = {// SystemRoot
                          L'S',
                          L'y',
                          L's',
                          L't',
                          L'e',
                          L'm',
                          L'R',
                          L'o',
                          L'o',
                          L't',
                          0,
                          0};

        RtlInitUnicodeString(&ValueName, wval);
        ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, NULL, 0, &nRes);
        if (nRes > 0)
        {
            KEY_VALUE_PARTIAL_INFORMATION *kvpi;
            kvpi = (KEY_VALUE_PARTIAL_INFORMATION *)ExAllocatePoolWithTag(PagedPool, nRes, 'sfc_');

            if (kvpi)
            {
                st = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, kvpi, nRes, &nRes);

                if (NT_SUCCESS(st) && (kvpi->Type == REG_SZ))
                {
                    ANSI_STRING ValueDataA;
                    UNICODE_STRING ValueDataW;

                    RtlInitUnicodeString(&ValueDataW, (PWSTR)kvpi->Data);
                    st = RtlUnicodeStringToAnsiString(&ValueDataA, &ValueDataW, TRUE);

                    if (NT_SUCCESS(st))
                    {
                        _set_cwd(ValueDataA.Buffer);
                        RtlFreeAnsiString(&ValueDataA);
                    }
                }

                ExFreePoolWithTag(kvpi, 'sfc_');
            }
        }

        ZwClose(KeyHandle);
    }

    return ok;
}

void __cdecl _cinitfs(void)
{
    _init_cwd();
}

extern "C" size_t
path_sanitize(char *s, size_t sz, const char *base, const char *expr);

typedef struct _FILE_CONTROL_BLOCK
{
    HANDLE hFile;
    NTSTATUS err;
} FILE_CONTROL_BLOCK;

FILE_CONTROL_BLOCK *
fget_core(FILE *f)
{
    return (FILE_CONTROL_BLOCK *)f->_Placeholder;
}

void
fset_core(FILE *f, FILE_CONTROL_BLOCK *fcb)
{
    f->_Placeholder = fcb;
}

static PDEVICE_OBJECT
GetNtfsDevice()
{
    PDRIVER_OBJECT drvObj = NULL;
    PDEVICE_OBJECT devObj = NULL;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING ntfsName;
    wchar_t wval[] = {//  \\FileSystem\\Ntfs
                      L'\\',
                      L'F',
                      L'i',
                      L'l',
                      L'e',
                      L'S',
                      L'y',
                      L's',
                      L't',
                      L'e',
                      L'm',
                      L'\\',
                      L'N',
                      L't',
                      L'f',
                      L's',
                      0,
                      0};
    RtlInitUnicodeString(&ntfsName, wval);

    status = ObReferenceObjectByName(
        &ntfsName, OBJ_CASE_INSENSITIVE, 0, 0, *IoDriverObjectType, KernelMode, 0, (PVOID *)&drvObj);
    if (NT_SUCCESS(status) && drvObj)
    {
        devObj = drvObj->DeviceObject;
        ObDereferenceObject(drvObj);
    }
    return devObj;
}

// �˺��������·��������\\??\\c:\\����������
static PDEVICE_OBJECT
GetBaseDevByPath(PUNICODE_STRING Path)
{
    NTSTATUS status;
    HANDLE fileHandle = NULL;
    PFILE_OBJECT fileObject = NULL;
    UNICODE_STRING devName;
    OBJECT_ATTRIBUTES oa;
    PDEVICE_OBJECT devObj = NULL;
    IO_STATUS_BLOCK iosb;
    WCHAR tmp[] = {// "\\DosDevices\\*:\\"
                   L'\\',
                   L'D',
                   L'o',
                   L's',
                   L'D',
                   L'e',
                   L'v',
                   L'i',
                   L'c',
                   L'e',
                   L's',
                   L'\\',
                   L'*',
                   L':',
                   L'\\',
                   0,
                   0};

    do
    {
        if (!Path || Path->Length < 7)
        {
            break;
        }
        // ���̷��滻��*
        tmp[12] = Path->Buffer[4];

        RtlInitUnicodeString(&devName, tmp);

        InitializeObjectAttributes(&oa, &devName, OBJ_KERNEL_HANDLE, NULL, NULL);

        // ���̷��ļ�
        status = IoCreateFile(&fileHandle,
            GENERIC_READ | SYNCHRONIZE,
            &oa,
            &iosb,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0,
            CreateFileTypeNone,
            NULL,
            IO_NO_PARAMETER_CHECKING);

        if (!NT_SUCCESS(status) || !fileHandle)
        {
            break;
        }
        // ��ȡ�ļ�����
        status = ObReferenceObjectByHandle(fileHandle, FILE_READ_ACCESS, *IoFileObjectType, KernelMode, (PVOID *)&fileObject, 0);
        if (!NT_SUCCESS(status) || !fileObject)
        {
            break;
        }

        devObj = IoGetBaseFileSystemDeviceObject(fileObject);

    } while (0);
    if (fileObject)
    {
        ObfDereferenceObject(fileObject);
    }
    if (fileHandle)
    {
        ZwClose(fileHandle);
    }

    return devObj;
}

void *__cdecl fopenSafe(char const *_FileName, char const *_Mode, int _IsSanitizePath)
{
    NTSTATUS st;
    HANDLE hFile = NULL;
    char *path = NULL;
    IO_STATUS_BLOCK isb;
    OBJECT_ATTRIBUTES oa;
    ANSI_STRING FileNameA;
    UNICODE_STRING FileNameW;
    PDEVICE_OBJECT baseDev = NULL;

    ULONG AccessMask = 0;
    ULONG ShareAccess = 0;
    ULONG CreateOptions = 0;
    ULONG CreateDisposition = 0;
    {
        char c_r[] = {'r', 0};
        char c_rb[] = {'r', 'b', 0};
        char c_w[] = {'w', 0};
        char c_wb[] = {'w', 'b', 0};
        char c_rw[] = {'r', 'w', 0};
        char c_rwb[] = {'r', 'w', 'b', 0};
        char c_ajia[] = {'a', '+', 0};
        char c_ajiab[] = {'a', '+', 'b', 0};
        char c_abjia[] = {'a', 'b', '+', 0};

        const char *mode = _Mode;

        if (!strcmp(mode, c_r) || !strcmp(mode, c_rb))
        {
            AccessMask = FILE_READ_DATA | FILE_READ_ATTRIBUTES;
            CreateDisposition = FILE_OPEN;
        }
        else if (!strcmp(mode, c_w) || !strcmp(mode, c_wb))
        {
            AccessMask = FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES;
            CreateDisposition = FILE_SUPERSEDE;
        }
        else if (!strcmp(mode, c_rw) || !strcmp(mode, c_rwb))
        {
            AccessMask = FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES;
            CreateDisposition = FILE_OPEN_IF;
        }
        else if (!strcmp(mode, c_ajia) || !strcmp(mode, c_ajiab) || !strcmp(mode, c_abjia))
        {
            AccessMask =
                FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_APPEND_DATA;
            CreateDisposition = FILE_OPEN_IF;
        }
        else
        {
            return NULL;
        }

        AccessMask |= SYNCHRONIZE;
        ShareAccess = FILE_SHARE_READ;
        CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    }

    path = (char *)ExAllocatePoolWithTag(PagedPool, 512, KCRTFS_POOL_DEFAULT_TAG);
    if (!path)
        return NULL;
    RtlSecureZeroMemory(path, 512);

    if (_IsSanitizePath)
    {
        //תR3����·����Ŀǰֻ��ǰ����˸�\\??\\���ѡ�����

        const char *cwd = "";
        if (_FileName[0] == '.') //��ǰĿ��O�õ���C:\Windows
            cwd = _get_cwd();
        if (!path_sanitize(path, 512, cwd, _FileName))
        {
            ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
            return NULL;
        }
    }
    else
    {
        RtlCopyMemory(path, _FileName, min(512, strlen(_FileName)));
    }

    RtlInitAnsiString(&FileNameA, path);
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&FileNameW, &FileNameA, TRUE)))
    {
        ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
        return NULL;
    }
    // ��ȡ����ļ����̵���ײ��豸����
    baseDev = GetBaseDevByPath(&FileNameW);

    InitializeObjectAttributes(&oa, &FileNameW, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (baseDev)
    {
        // ��������ײ��豸����ͨ�ŵķ�ʽ���ļ����
        st = IoCreateFileSpecifyDeviceObjectHint(&hFile,
            AccessMask,
            &oa,
            &isb,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            ShareAccess,
            CreateDisposition,
            CreateOptions,
            // FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0,
            CreateFileTypeNone,
            NULL,
            IO_IGNORE_SHARE_ACCESS_CHECK,
            baseDev);
    }
    if (NULL == hFile)
    {
        st = ZwCreateFile(&hFile,
            AccessMask,
            &oa,
            &isb,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            ShareAccess,
            CreateDisposition,
            CreateOptions,
            NULL,
            0);
    }
    ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
    path = NULL;
    RtlFreeUnicodeString(&FileNameW);

    if (!NT_SUCCESS(st))
        return NULL;

    FILE *f = (FILE *)ExAllocatePoolWithTag(PagedPool, sizeof(FILE), KCRTFS_POOL_DEFAULT_TAG);
    FILE_CONTROL_BLOCK *fcb =
        (FILE_CONTROL_BLOCK *)ExAllocatePoolWithTag(PagedPool, sizeof(FILE_CONTROL_BLOCK), KCRTFS_POOL_DEFAULT_TAG);
    fcb->err = 0;
    fcb->hFile = hFile;
    fset_core(f, fcb);

    return (void *)f;
}

//����3�����Ƿ�Ҫת����R3������·��
/*FILE**/ void *__cdecl fopen2(char const *_FileName, char const *_Mode, int _IsSanitizePath)
{
    NTSTATUS st;
    HANDLE hFile;
    char *path = NULL;
    IO_STATUS_BLOCK isb;
    OBJECT_ATTRIBUTES oa;
    ANSI_STRING FileNameA;
    UNICODE_STRING FileNameW;

    ULONG AccessMask = 0;
    ULONG ShareAccess = 0;
    ULONG CreateOptions = 0;
    ULONG CreateDisposition = 0;
    {
        char c_r[] = {'r', 0};
        char c_rb[] = {'r', 'b', 0};
        char c_w[] = {'w', 0};
        char c_wb[] = {'w', 'b', 0};
        char c_rw[] = {'r', 'w', 0};
        char c_rwb[] = {'r', 'w', 'b', 0};
        char c_ajia[] = {'a', '+', 0};
        char c_ajiab[] = {'a', '+', 'b', 0};
        char c_abjia[] = {'a', 'b', '+', 0};

        const char *mode = _Mode;

        if (!strcmp(mode, c_r) || !strcmp(mode, c_rb))
        {
            AccessMask = FILE_READ_DATA | FILE_READ_ATTRIBUTES;
            CreateDisposition = FILE_OPEN;
        }
        else if (!strcmp(mode, c_w) || !strcmp(mode, c_wb))
        {
            AccessMask = FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES;
            CreateDisposition = FILE_SUPERSEDE;
        }
        else if (!strcmp(mode, c_rw) || !strcmp(mode, c_rwb))
        {
            AccessMask = FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES;
            CreateDisposition = FILE_OPEN_IF;
        }
        else if (!strcmp(mode, c_ajia) || !strcmp(mode, c_ajiab) || !strcmp(mode, c_abjia))
        {
            AccessMask =
                FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_APPEND_DATA;
            CreateDisposition = FILE_OPEN_IF;
        }
        else
        {
            return NULL;
        }

        AccessMask |= SYNCHRONIZE;
        ShareAccess = FILE_SHARE_READ;
        CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    }

    path = (char *)ExAllocatePoolWithTag(PagedPool, 512, KCRTFS_POOL_DEFAULT_TAG);
    if (!path)
        return NULL;
    RtlSecureZeroMemory(path, 512);

    if (_IsSanitizePath)
    {
        //תR3����·����Ŀǰֻ��ǰ����˸�\\??\\���ѡ�����

        const char *cwd = "";
        if (_FileName[0] == '.') //��ǰĿ��O�õ���C:\Windows
            cwd = _get_cwd();
        if (!path_sanitize(path, 512, cwd, _FileName))
        {
            ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
            return NULL;
        }
    }
    else
    {
        RtlCopyMemory(path, _FileName, min(512, strlen(_FileName)));
    }

    RtlInitAnsiString(&FileNameA, path);
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&FileNameW, &FileNameA, TRUE)))
    {
        ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
        return NULL;
    }

    InitializeObjectAttributes(&oa, &FileNameW, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    st = ZwCreateFile(&hFile,
        AccessMask,
        &oa,
        &isb,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        ShareAccess,
        CreateDisposition,
        CreateOptions,
        NULL,
        0);
    ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
    path = NULL;
    RtlFreeUnicodeString(&FileNameW);

    if (!NT_SUCCESS(st))
        return NULL;

    FILE *f = (FILE *)ExAllocatePoolWithTag(PagedPool, sizeof(FILE), KCRTFS_POOL_DEFAULT_TAG);
    FILE_CONTROL_BLOCK *fcb =
        (FILE_CONTROL_BLOCK *)ExAllocatePoolWithTag(PagedPool, sizeof(FILE_CONTROL_BLOCK), KCRTFS_POOL_DEFAULT_TAG);
    fcb->err = 0;
    fcb->hFile = hFile;
    fset_core(f, fcb);

    return (void *)f;
}

FILE *__cdecl fopen(char const *_FileName, char const *_Mode)
{
    return (FILE *)fopen2(_FileName, _Mode, 1);
}

int __cdecl fclose(FILE *f)
{
    FILE_CONTROL_BLOCK *fcb = fget_core(f);

    ZwClose(fcb->hFile);
    ExFreePoolWithTag(fcb, KCRTFS_POOL_DEFAULT_TAG);
    ExFreePoolWithTag(f, KCRTFS_POOL_DEFAULT_TAG);

    return 0;
}

//����2�����Ƿ�Ҫת����R3������·��
int __cdecl remove2(char const *_FileName, int _IsSanitizePath)
{
    UNICODE_STRING usFIleNameW;
    ANSI_STRING usFIleNameA;
    char *path = NULL;
    IO_STATUS_BLOCK iosb;
    OBJECT_ATTRIBUTES oa = {sizeof(oa)};
    long status = -1;
    FILE_BASIC_INFORMATION fbi;
    HANDLE hFile = NULL;
    char *cwd = (char*)"";

    RtlSecureZeroMemory(&iosb, sizeof(iosb));
    RtlSecureZeroMemory(&fbi, sizeof(fbi));

    path = (char *)ExAllocatePoolWithTag(PagedPool, 512, KCRTFS_POOL_DEFAULT_TAG);
    if (!path)
    {
        return -1;
    }
    RtlSecureZeroMemory(path, 512);

    if (_IsSanitizePath)
    {
        //תR3����·����Ŀǰֻ��ǰ����˸�\\??\\���ѡ�����

        if (_FileName[0] == '.')
        {
            //��ǰĿ��O�õ���C:\Windows
            cwd = (char *)_get_cwd();
        }

        if (!path_sanitize(path, 512, cwd, _FileName))
        {
            ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
            return -1;
        }
    }
    else
    {
        RtlCopyMemory(path, _FileName, min(512, strlen(_FileName)));
    }

    RtlInitAnsiString(&usFIleNameA, path);
    if (RtlAnsiStringToUnicodeString(&usFIleNameW, &usFIleNameA, TRUE) < 0)
    {
        ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
        return -1;
    }

    oa.Attributes = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
    oa.ObjectName = &usFIleNameW;

    //���Ȱ���Ҫɾ�����ļ�����������Ϊ��ͨ����������ɾ��

    status = ZwCreateFile(&hFile,
        //�����������д�������޷���ֻ���ļ�..
        SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES, //��д����Ȩ��
        &oa,
        &iosb, //�ļ�IO״̬
        NULL,
        FILE_ATTRIBUTE_READONLY |   // read only
            FILE_ATTRIBUTE_NORMAL | //�ļ�����һ��дNORMAL
            FILE_ATTRIBUTE_HIDDEN | //Ҫ�������ļ���ҲҪ��д
            FILE_ATTRIBUTE_SYSTEM,  //Ҫ��ϵͳ�ļ���ҲҪ��д
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN, //ֻ�򿪴��ڵ��ļ������ļ������ڣ���ʧ��
                   // �ļ���Ŀ¼����
                   // �����ļ�����ֱ��д���ļ�ϵͳ��ֱ��д�벻�����������ʱ���⣬��IOռ�úࣩܶ
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
        // Eabuf������дNULL�������ļ��������������豸�Ľ���������EAдNULL,EA����Ҳ��0
        NULL,
        0);

    if (status < 0)
    {
        goto _end;
    }

    status = ZwQueryInformationFile(hFile, &iosb, &fbi, sizeof(fbi), FileBasicInformation);

    if (status < 0)
    {
        goto _end;
    }

    fbi.FileAttributes = FILE_ATTRIBUTE_NORMAL;

    status = ZwSetInformationFile(hFile, &iosb, &fbi, sizeof(fbi), FileBasicInformation);

    if (status < 0)
    {
        goto _end;
    }

    //ɾ����ʱ��һ��Ҫ�رյ����
    ZwClose(hFile);

    status = ZwDeleteFile(&oa);

_end:
    if (path)
    {
        ExFreePoolWithTag(path, KCRTFS_POOL_DEFAULT_TAG);
        path = NULL;
    }

    RtlFreeUnicodeString(&usFIleNameW);

    if (status == 0)
    {
        return 0;
    }

    return -1;
}

int __cdecl remove(char const *_FileName)
{
    return remove2(_FileName, 1);
}

int __cdecl feof(FILE *f)
{
    IO_STATUS_BLOCK isb;
    FILE_STANDARD_INFORMATION fsi;
    FILE_CONTROL_BLOCK *fcb = fget_core(f);

    fcb->err = ZwQueryInformationFile(fcb->hFile, &isb, &fsi, sizeof(fsi), FileStandardInformation);
    if (NT_SUCCESS(fcb->err))
    {
        FILE_POSITION_INFORMATION fpi;

        fcb->err = ZwQueryInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
        if (NT_SUCCESS(fcb->err))
        {
            return (fpi.CurrentByteOffset.QuadPart + 1 >= fsi.EndOfFile.QuadPart);
        }
    }

    return 1;
}

#include <stdarg.h>

int __cdecl kfprintf(void *f, char const *const _Format, ...)
{
    char s[512] = {0};
    size_t sz = sizeof(s);
    int n = 0;
    va_list _ArgList;

    va_start(_ArgList, _Format);
    n = g_vsnprintf(s, sz, _Format, _ArgList);
    va_end(_ArgList);
    fwrite(s, n, 1, (FILE*)f);

    return n;
}

int __cdecl fgetc(FILE *f)
{
    char c = EOF;
    IO_STATUS_BLOCK isb;
    FILE_CONTROL_BLOCK *fcb = fget_core(f);

    fcb->err = ZwReadFile(fcb->hFile, NULL, NULL, NULL, &isb, &c, 1, NULL, NULL);
    if (NT_SUCCESS(fcb->err))
    {
        return (c & 0xFF);
    }
    else
    {
        return EOF;
    }
}

int __cdecl ungetc(int c, FILE *f)
{
    if (c != EOF)
    {
        if (!fseek(f, -1, SEEK_CUR))
        {
            return c;
        }
    }

    return EOF;
}

size_t __cdecl fread(void *Buffer, size_t ElementSize, size_t ElementCount, FILE *f)
{
    IO_STATUS_BLOCK isb;
    FILE_CONTROL_BLOCK *fcb = fget_core(f);
    size_t rd_size = ElementSize * ElementCount;

    RtlSecureZeroMemory(&isb, sizeof(isb));
    fcb->err = ZwReadFile(fcb->hFile, NULL, NULL, NULL, &isb, Buffer, (ULONG)rd_size, NULL, NULL);
    return isb.Information;
}

size_t __cdecl fwrite(void const *Buffer, size_t ElementSize, size_t ElementCount, FILE *f)
{
    IO_STATUS_BLOCK isb;
    FILE_CONTROL_BLOCK *fcb = fget_core(f);
    size_t wt_size = ElementSize * ElementCount;

    RtlSecureZeroMemory(&isb, sizeof(isb));
    fcb->err = ZwWriteFile(fcb->hFile, NULL, NULL, NULL, &isb, (PVOID)Buffer, (ULONG)wt_size, NULL, NULL);
    return isb.Information;
}

int __cdecl ferror(FILE *f)
{
    return fget_core(f)->err;
}

void __cdecl clearerr(FILE *f)
{
    fget_core(f)->err = 0;
}

long __cdecl ftell(FILE *f)
{
    return (long)_ftelli64(f);
}

int __cdecl fseek(FILE *_Stream, long _Offset, int _Origin)
{
    return _fseeki64(_Stream, _Offset, _Origin);
}

__int64 __cdecl _ftelli64(FILE *_Stream)
{
    IO_STATUS_BLOCK isb;
    FILE_POSITION_INFORMATION fpi;
    FILE_CONTROL_BLOCK *fcb = fget_core(_Stream);

    fcb->err = ZwQueryInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
    if (NT_SUCCESS(fcb->err))
    {
        return fpi.CurrentByteOffset.QuadPart;
    }
    else
    {
        return -1;
    }
}

int __cdecl _fseeki64(FILE *_Stream, __int64 _Offset, int _Origin)
{
    IO_STATUS_BLOCK isb;
    FILE_POSITION_INFORMATION fpi;
    FILE_CONTROL_BLOCK *fcb = fget_core(_Stream);

    switch (_Origin)
    {
    case SEEK_CUR: {
        long cur_pos = ftell(_Stream);

        if (cur_pos >= 0)
        {
            fpi.CurrentByteOffset.QuadPart = cur_pos + _Offset;
            fcb->err = ZwSetInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
            if (NT_SUCCESS(fcb->err))
            {
                return 0;
            }
        }
    }
    break;
    case SEEK_END: {
        FILE_STANDARD_INFORMATION fsi;

        fcb->err = ZwQueryInformationFile(fcb->hFile, &isb, &fsi, sizeof(fsi), FileStandardInformation);
        if (NT_SUCCESS(fcb->err))
        {
            fpi.CurrentByteOffset.QuadPart = fsi.EndOfFile.QuadPart + _Offset;

            fcb->err = ZwSetInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
            if (NT_SUCCESS(fcb->err))
            {
                return 0;
            }
        }
    }
    break;
    case SEEK_SET: {
        fpi.CurrentByteOffset.QuadPart = _Offset;
        fcb->err = ZwSetInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
        if (NT_SUCCESS(fcb->err))
        {
            return 0;
        }
    }
    break;
    default:
        break;
    }

    return EIO;
}

void __cdecl exit(_In_ int _Code)
{

}

void __cdecl _exit(_In_ int _Code)
{

}
