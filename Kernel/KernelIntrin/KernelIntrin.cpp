#include "KernelIntrin.h"
#include <intrin.h>
#include <softintrin.h>
#include <immintrin.h>

extern "C"
inline
void KernelIntrin__writecr2(unsigned __int64 Data)
{
	::__writecr2(Data);
}

extern "C"
inline
unsigned char KernelIntrin__vmx_vmlaunch(void)
{
	return ::__vmx_vmlaunch();
}

extern "C"
inline
unsigned char KernelIntrin__vmx_on(unsigned __int64* vmxon_region)
{
	return ::__vmx_on(vmxon_region);
}

extern "C"
inline
unsigned char KernelIntrin__vmx_vmresume(void)
{
	return ::__vmx_vmresume();
}

extern "C"
inline
void KernelIntrin__vmx_vmptrst(unsigned __int64* vmcs_physical_address)
{
	::__vmx_vmptrst(vmcs_physical_address);
}

extern "C"
inline
unsigned char KernelIntrin__vmx_vmptrld(unsigned __int64* pa)
{
	return ::__vmx_vmptrld(pa);
}

extern "C"
inline
unsigned char KernelIntrin__vmx_vmclear(unsigned __int64* pa)
{
	return ::__vmx_vmclear(pa);
}

extern "C"
inline
void KernelIntrin__vmx_off(void)
{
	::__vmx_off();
}

extern "C"
inline
void KernelIntrin__stosq(unsigned __int64* Destination, unsigned __int64 Data, size_t Count)
{
	::__stosq(Destination, Data, Count);
}

extern "C"
inline
unsigned char KernelIntrin__vmx_vmread(size_t field, size_t* value)
{
	return ::__vmx_vmread(field, value);
}

extern "C"
inline
unsigned char KernelIntrin__vmx_vmwrite(size_t field, size_t value)
{
	return ::__vmx_vmwrite(field, value);
}

extern "C"
inline
void KernelIntrin__sidt(void* dtr)
{
	::__sidt(dtr);
}

extern "C"
inline
void KernelIntrin__lidt(void* dtr)
{
	::__lidt(dtr);
}

extern "C"
inline
void KernelIntrin__invlpg(void* addr)
{
	::__invlpg(addr);
}

extern "C"
inline
unsigned int KernelIntrin__getcallerseflags(void)
{
	return ::__getcallerseflags();
}

extern "C"
unsigned __int64 __cdecl KernelIntrin_readfsbase_u64(void)
{
	return ::_readfsbase_u64();
}

extern "C" void __cdecl KernelIntrin_writefsbase_u64(unsigned __int64 Value)
{
	_writefsbase_u64(Value);
}

extern "C" void __cdecl KernelIntrin_writegsbase_u64(unsigned __int64 Value)
{
	_writegsbase_u64(Value);
}

extern "C"
inline
void KernelIntrin__inbytestring(unsigned short Port, unsigned char* Buffer, unsigned long Count)
{
	::__inbytestring(Port, Buffer, Count);
}

extern "C"
inline
void KernelIntrin__inwordstring(unsigned short Port, unsigned short* Buffer, unsigned long Count)
{
	::__inwordstring(Port, Buffer, Count);
}

extern "C"
inline
unsigned char KernelIntrin__inbyte(unsigned short Port)
{
	return ::__inbyte(Port);
}

extern "C"
inline
unsigned short KernelIntrin__inword(unsigned short Port)
{
	return ::__inword(Port);
}

extern "C"
inline
unsigned long KernelIntrin__indword(unsigned short Port)
{
	return ::__indword(Port);
}

extern "C"
inline
void KernelIntrin__outbytestring(unsigned short Port, unsigned char* Buffer, unsigned long Count)
{
	::__outbytestring(Port, Buffer, Count);
}

extern "C"
inline
void KernelIntrin__indwordstring(unsigned short Port, unsigned long* Buffer, unsigned long Count)
{
	::__indwordstring(Port, Buffer, Count);
}

extern "C"
inline
void KernelIntrin__outwordstring(unsigned short Port, unsigned short* Buffer, unsigned long Count)
{
	::__outwordstring(Port, Buffer, Count);
}

extern "C"
inline
void KernelIntrin__outdwordstring(unsigned short Port, unsigned long* Buffer, unsigned long Count)
{
	::__outdwordstring(Port, Buffer, Count);
}

extern "C"
inline
void KernelIntrin__outbyte(unsigned short Port, unsigned char Data)
{
	::__outbyte(Port, Data);
}

extern "C"
inline
void KernelIntrin__outword(unsigned short Port, unsigned short Data)
{
	::__outword(Port, Data);
}

extern "C"
inline
void KernelIntrin__outdword(unsigned short Port, unsigned long Data)
{
	::__outdword(Port, Data);
}

extern "C"
inline
unsigned __int64 KernelIntrin__rdtscp(unsigned int* Aux)
{
	return ::__rdtscp(Aux);
}

extern "C"
inline
unsigned long long KernelIntrin__readpmc(unsigned long counter)
{
	return ::__readpmc(counter);
}

extern "C"
inline
unsigned long KernelIntrin__segmentlimit(unsigned long selector)
{
	return ::__segmentlimit(selector);
}

extern "C"
inline
unsigned __int64 KernelIntrin__readmsr(unsigned long __register)
{
	return ::__readmsr(__register);
}

extern "C"
inline
void KernelIntrin__writemsr(unsigned long __register, unsigned __int64 val)
{
	::__writemsr(__register, val);
}

extern "C"
inline
unsigned int __cdecl KernelIntrin_xbegin(void)
{
	return ::_xbegin();
}

extern "C"
inline
void __cdecl KernelIntrin_xend(void)
{
	::_xend();
}

extern "C"
inline
void KernelIntrin__movsb(unsigned char* __dst, unsigned char const* __src, size_t __n)
{
	::__movsb(__dst, __src, __n);
}

extern "C"
inline
void KernelIntrin__cpuid(int CPUInfo[4], int InfoType)
{
	::__cpuid(CPUInfo, InfoType);
}

extern "C"
inline
void KernelIntrin__cpuidex(int CPUInfo[4], int InfoType, int ECXValue)
{
	::__cpuidex(CPUInfo, InfoType, ECXValue);
}

extern "C"
inline
void KernelIntrin__wbinvd(void)
{
	__wbinvd();
}

