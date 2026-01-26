#pragma once

extern "C"
inline
void KernelIntrin__writecr2(unsigned __int64 Data);

extern "C"
inline
unsigned char KernelIntrin__vmx_vmlaunch(void);

extern "C"
inline
unsigned char KernelIntrin__vmx_on(unsigned long long* vmxon_region);

extern "C"
inline
unsigned char KernelIntrin__vmx_vmresume(void);

extern "C"
inline
void KernelIntrin__vmx_vmptrst(unsigned __int64* vmcs_physical_address);

extern "C"
inline
unsigned char KernelIntrin__vmx_vmptrld(unsigned __int64* pa);

extern "C"
inline
unsigned char KernelIntrin__vmx_vmclear(unsigned __int64* pa);

extern "C"
inline
void KernelIntrin__vmx_off(void);

extern "C"
inline
void KernelIntrin__stosq(unsigned __int64* __dst, unsigned __int64 __x, size_t __n);

extern "C"
inline
unsigned char KernelIntrin__vmx_vmread(size_t field, size_t* value);

extern "C"
inline
unsigned char KernelIntrin__vmx_vmwrite(size_t field, size_t value);

extern "C"
inline
void KernelIntrin__sidt(void* dtr);

extern "C"
inline
void KernelIntrin__lidt(void* dtr);

extern "C"
inline
void KernelIntrin__invlpg(void* addr);

extern "C"
inline
unsigned int KernelIntrin__getcallerseflags(void);

extern "C"
unsigned __int64 __cdecl KernelIntrin_readfsbase_u64(void);

extern "C" void __cdecl KernelIntrin_writefsbase_u64(unsigned __int64 Value);
extern "C" void __cdecl KernelIntrin_writegsbase_u64(unsigned __int64 Value);

extern "C"
inline
void KernelIntrin__inbytestring(unsigned short Port, unsigned char* Buffer, unsigned long Count);

extern "C"
inline
void KernelIntrin__inwordstring(unsigned short Port, unsigned short* Buffer, unsigned long Count);

extern "C"
inline
unsigned char KernelIntrin__inbyte(unsigned short Port);

extern "C"
inline
unsigned short KernelIntrin__inword(unsigned short Port);

extern "C"
inline
unsigned long KernelIntrin__indword(unsigned short Port);

extern "C"
inline
void KernelIntrin__outbytestring(unsigned short Port, unsigned char* Buffer, unsigned long Count);

extern "C"
inline
void KernelIntrin__indwordstring(unsigned short Port, unsigned long* Buffer, unsigned long Count);

extern "C"
inline
void KernelIntrin__outwordstring(unsigned short Port, unsigned short* Buffer, unsigned long Count);

extern "C"
inline
void KernelIntrin__outdwordstring(unsigned short Port, unsigned long* Buffer, unsigned long Count);

extern "C"
inline
void KernelIntrin__outbyte(unsigned short Port, unsigned char Data);

extern "C"
inline
void KernelIntrin__outword(unsigned short Port, unsigned short Data);

extern "C"
inline
void KernelIntrin__outdword(unsigned short Port, unsigned long Data);

extern "C"
inline
unsigned __int64 KernelIntrin__rdtscp(unsigned int* Aux);

extern "C"
inline
unsigned long long KernelIntrin__readpmc(unsigned long counter);

extern "C"
inline
unsigned long KernelIntrin__segmentlimit(unsigned long selector);

extern "C"
inline
unsigned __int64 KernelIntrin__readmsr(unsigned long __register);

extern "C"
inline
void KernelIntrin__writemsr(unsigned long __register, unsigned __int64 val);

extern "C"
inline
unsigned int     __cdecl KernelIntrin_xbegin(void);

extern "C"
inline
void             __cdecl KernelIntrin_xend(void);

extern "C"
inline
void KernelIntrin__movsb(unsigned char* __dst, unsigned char const* __src, size_t __n);

extern "C"
inline
void KernelIntrin__cpuid(int CPUInfo[4], int InfoType);

extern "C"
inline
void KernelIntrin__cpuidex(int CPUInfo[4], int InfoType, int ECXValue);

extern "C"
inline
void KernelIntrin__wbinvd(void);