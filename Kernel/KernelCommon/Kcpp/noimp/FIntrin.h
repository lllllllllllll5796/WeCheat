#pragma once

//https://github.com/asamy/ksm/blob/6ef52b87903b25c62560d80e8fbec0867e5c2a8b/vmx.h#L517
//https://github.com/asamy/ksm/blob/6ef52b87903b25c62560d80e8fbec0867e5c2a8b/vcpu.c#L457
//https://github.com/raphaelsc/Am-I-affected-by-Meltdown/blob/b6f80766da112e3fa09e8c4f73a82f7451f9c8f7/assembly_utils.hh#L47
//https://github.com/intel/ikgt-core/blob/8ee913efe874394ba90bd0d069421bde4d893858/core/host/hw/em64t/em64t_gnu_asm.c#L461
//https://github.com/cheat-engine/cheat-engine/blob/dc08c7db1d9b936e4e486ad78e7aa73308b6bb5b/Cheat%20Engine/tcclib/win32/include/conio.h#L137

#pragma pack(2) //allignemnt of 2 byte
typedef struct lmode_desc 
{
	uint16_t limit;
	uint64_t base;
} lmode_desc_t;
#pragma pack()

typedef struct {
	u64 vpid : 16;
	u64 rsvd : 48;
	u64 gva;
} invvpid_t;

typedef struct {
	u64 ptr;
	u64 gpa;
} invept_t;

//测试成功
// inl void __sidt(void* dtr)
// {
// 	lmode_desc_t* idt = (lmode_desc_t*)dtr;
// 	__asm __volatile__("sidt %0\n\t" : "=m" (*idt) : : "memory");
// }

// inl void __lidt(void* dtr)
// {
// 	lmode_desc_t* idt = (lmode_desc_t*)dtr;
// 	__asm __volatile__("lidt %0" : : "m" (*idt));
// }

// inl void __invlpg(void* addr)
// {
// 	__asm __volatile__("invlpg %0": : "m" (*(uint8_t*)(addr)) : "memory");
// }

// inl u8 __invept(u64 ext, const invept_t* i)
// {
// 	u8 error;
// 	__asm __volatile(ASM_VMX_INVEPT "; setna %0" : "=q" (error) : "d" (i), "c" (ext) : "cc", "memory");
// 	return error;
// }
// 
// inl u8 __invvpid(u64 ext, const invvpid_t* i)
// {
// 	u8 error;
// 	__asm __volatile(ASM_VMX_INVVPID "; setna %0" : "=q" (error) : "d" (i), "c" (ext) : "cc", "memory");
// 	return error;
// }

// unsigned long __segmentlimit(unsigned long selector);

// inl unsigned int __getcallerseflags(void)
// {
// 	uintptr_t result;
// 
// 	__asm __volatile__(
// 		"pushfq\n\t"
// 		"pop %0" : "=r" (result) : : "memory" );
// 
// 	return result;
// }

//测试成功
//INLINE void __writedr(unsigned int Index, unsigned __int64 value)
//{
//	if (Index == 0)
//	{
//		__asm __volatile__("mov %0, %%dr0" : : "r"(value) : "memory");
//	}
//	else if (Index == 1)
//	{
//		__asm __volatile__("mov %0, %%dr1" : : "r"(value) : "memory");
//	}
//	else if (Index == 2)
//	{
//		__asm __volatile__("mov %0, %%dr2" : : "r"(value) : "memory");
//	}
//	else if (Index == 3)
//	{
//		__asm __volatile__("mov %0, %%dr3" : : "r"(value) : "memory");
//	}
//	else if (Index == 4)
//	{
//		__asm __volatile__("mov %0, %%dr4" : : "r"(value) : "memory");
//	}
//	else if (Index == 5)
//	{
//		__asm __volatile__("mov %0, %%dr5" : : "r"(value) : "memory");
//	}
//	else if (Index == 6)
//	{
//		__asm __volatile__("mov %0, %%dr6" : : "r"(value) : "memory");
//	}
//	else if (Index == 7)
//	{
//		__asm __volatile__("mov %0, %%dr7" : : "r"(value) : "memory");
//	}
//}

//测试成功
//INLINE unsigned __int64 __readdr(unsigned int Index)
//{
//	unsigned __int64 addr = 0;
//
//	if (Index == 0)
//	{
//		__asm __volatile__("mov %%dr0, %0" : "=r"(addr) : : "memory");
//	}
//	else if (Index == 1)
//	{
//		__asm __volatile__("mov %%dr1, %0" : "=r"(addr) : : "memory");
//	}
//	else if (Index == 2)
//	{
//		__asm __volatile__("mov %%dr2, %0" : "=r"(addr) : : "memory");
//	}
//	else if (Index == 3)
//	{
//		__asm __volatile__("mov %%dr3, %0" : "=r"(addr) : : "memory");
//	}
//	else if (Index == 4)
//	{
//		__asm __volatile__("mov %%dr4, %0" : "=r"(addr) : : "memory");
//	}
//	else if (Index == 5)
//	{
//		__asm __volatile__("mov %%dr5, %0" : "=r"(addr) : : "memory");
//	}
//	else if (Index == 6)
//	{
//		__asm __volatile__("mov %%dr6, %0" : "=r"(addr) : : "memory");
//	}
//	else if (Index == 7)
//	{
//		__asm __volatile__("mov %%dr7, %0" : "=r"(addr) : : "memory");
//	}
//
//	return addr;
//}

//https://github.com/microsoft/SymCrypt/blob/0f83207858253be11f473dced88b146895004419/lib/linux/intrinsics.c
// inl void __cpuid(int CPUInfo[4], int InfoType)
// {
// 	__asm __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType));
// }
// 
// inl void __cpuidex(int CPUInfo[4], int InfoType, int ECXValue)
// {
// 	__asm __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType), "c" (ECXValue));
// }

// void cpuid(int CPUInfo[4], int InfoType);

// void cpuidex(int CPUInfo[4], int InfoType, int ECXValue);

// inl unsigned __int64 __readmsr(unsigned long __register) {
// 	unsigned long __edx;
// 	unsigned long __eax;
// 	__asm __volatile__("rdmsr" : "=d"(__edx), "=a"(__eax) : "c"(__register) : "memory");
// 	return (((unsigned __int64)__edx) << 32) | (unsigned __int64)__eax;
// }

// inl void __writemsr(unsigned long __register, unsigned __int64 val) {
// 	unsigned long __edx = (unsigned long)(((unsigned __int64)val) << 32);
// 	unsigned long __eax = (unsigned long)val;
// 	__asm __volatile__("wrmsr" : : "a"(__eax), "d"(__edx), "c"(__register) : "memory");
// 	//return (((unsigned __int64)__edx) << 32) | (unsigned __int64)__eax;
// }

// inl void __movsb(unsigned char* __dst, unsigned char const* __src, size_t __n) {
// 	__asm __volatile__("rep movsb" : "+D"(__dst), "+S"(__src), "+c"(__n) : : "memory");
// }
//
//INLINE unsigned __int64 __readcr2() {
//	unsigned __int64 __cr2_val;
//	__asm __volatile__("mov %%cr2, %0" : "=r"(__cr2_val) : : "memory");
//	return __cr2_val;
//}
//
//INLINE unsigned __int64 __readcr3() {
//	unsigned __int64 __cr3_val;
//	__asm __volatile__("mov %%cr3, %0" : "=r"(__cr3_val) : : "memory");
//	return __cr3_val;
//}
//
//INLINE unsigned __int64 __readcr4() {
//	unsigned __int64 __cr4_val;
//	__asm __volatile__("mov %%cr4, %0" : "=r"(__cr4_val) : : "memory");
//	return __cr4_val;
//}
//
//INLINE unsigned __int64 __readcr0() {
//	unsigned __int64 __cr0_val;
//	__asm __volatile__("mov %%cr0, %0" : "=r"(__cr0_val) : : "memory");
//	return __cr0_val;
//}
//
//INLINE unsigned __int64 __readcr8() {
//	unsigned __int64 __cr8_val;
//	__asm __volatile__("mov %%cr8, %0" : "=r"(__cr8_val) : : "memory");
//	return __cr8_val;
//}
//
//INLINE void __writecr0(unsigned __int64 __cr0_val) {
//	__asm __volatile__("mov %0, %%cr0" : : "r"(__cr0_val) : "memory");
//}

// inl void __writecr2(unsigned __int64 Data)
// {
// 	__asm __volatile(
// 		"mov %[Data], %%cr2"
// 		:
// 		: [Data] "q" (Data)
// 		: "memory");
// }
//
//INLINE void __writecr3(unsigned __int64 __cr3_val) {
//	__asm __volatile__("mov %0, %%cr3" : : "r"(__cr3_val) : "memory");
//}
//
//INLINE void __writecr4(unsigned __int64 __cr4_val) {
//	__asm __volatile__("mov %0, %%cr4" : : "r"(__cr4_val) : "memory");
//}
//
//INLINE void __writecr8(unsigned __int64 __cr0_val) {
//	__asm __volatile__("mov %0, %%cr8" : : "r"(__cr0_val) : "memory");
//}
//
//INLINE void _disable() {
//	__asm cli
//}
//
//INLINE void _enable() {
//	__asm sti
//}
//
//INLINE void _cli() {
//	__asm cli
//}
//
//INLINE void _sti() {
//	__asm sti
//}
//
//INLINE naked unsigned __int64 _readeflags() {
//	__asm {
//		pushfq
//		pop rax
//		ret
//	}
//}
//
//INLINE naked void _writeeflags(unsigned __int64 a1) {
//	__asm {
//		push rcx
//		popfq
//		ret
//	}
//}
//
//
//INLINE naked unsigned __int64 _readdr7() {
//	__asm {
//		mov rax, dr7
//		ret
//	}
//}
//
//INLINE naked unsigned __int64 _readdr6() {
//	__asm {
//		mov rax, dr6
//		ret
//	}
//}
//
//INLINE naked unsigned __int64 _writedr7(__int64 a1) {
//	__asm {
//		mov dr7, rcx
//		ret
//	}
//}
//
//INLINE bool KeAreInterruptsEnabled()
//{
//	return (_readeflags() & 0x200) != 0;
//}
//
//INLINE unsigned __int64 KeGetCurrentPrcb() 
//{
//	return __readgsqword(0x20);
//}

// 
// inl void __inbytestring(unsigned short Port, unsigned char* Buffer, unsigned long Count)
// {
// 	__asm __volatile(
// 		"cld ; rep ; insb "
// 		: "=D" (Buffer), "=c" (Count)
// 		: "d"(Port), "0"(Buffer), "1" (Count)
// 	);
// }
// 
// inl void __inwordstring(unsigned short Port, unsigned short* Buffer, unsigned long Count)
// {
// 	__asm __volatile(
// 		"cld ; rep ; insw "
// 		: "=D" (Buffer), "=c" (Count)
// 		: "d"(Port), "0"(Buffer), "1" (Count)
// 	);
// }
// 
// inl unsigned char __inbyte(unsigned short Port)
// {
// 	unsigned char value;
// 	__asm __volatile("inb %w1,%b0"
// 		: "=a" (value)
// 		: "Nd" (Port));
// 	return value;
// }
// 
// inl unsigned short __inword(unsigned short Port)
// {
// 	unsigned short value;
// 	__asm __volatile("inw %w1,%w0"
// 		: "=a" (value)
// 		: "Nd" (Port));
// 	return value;
// }
// 
// inl unsigned long __indword(unsigned short Port)
// {
// 	unsigned long value;
// 	__asm __volatile("inl %w1,%0"
// 		: "=a" (value)
// 		: "Nd" (Port));
// 	return value;
// }
// 
// inl void __outbytestring(unsigned short Port, unsigned char* Buffer, unsigned long Count)
// {
// 	__asm __volatile(
// 		"cld ; rep ; outsb "
// 		: "=S" (Buffer), "=c" (Count)
// 		: "d"(Port), "0"(Buffer), "1" (Count)
// 	);
// }
// 
// inl void __indwordstring(unsigned short Port, unsigned long* Buffer, unsigned long Count)
// {
// 	__asm __volatile(
// 		"cld ; rep ; insl "
// 		: "=D" (Buffer), "=c" (Count)
// 		: "d"(Port), "0"(Buffer), "1" (Count)
// 	);
// }
// 
// inl void __outwordstring(unsigned short Port, unsigned short* Buffer, unsigned long Count)
// {
// 	__asm __volatile(
// 		"cld ; rep ; outsw "
// 		: "=S" (Buffer), "=c" (Count)
// 		: "d"(Port), "0"(Buffer), "1" (Count)
// 	);
// }
// 
// inl void __outdwordstring(unsigned short Port, unsigned long* Buffer, unsigned long Count)
// {
// 	__asm __volatile(
// 		"cld ; rep ; outsl "
// 		: "=S" (Buffer), "=c" (Count)
// 		: "d"(Port), "0"(Buffer), "1" (Count)
// 	);
// }
// 
// inl void __outbyte(unsigned short Port, unsigned char Data)
// {
// 	__asm __volatile("outb %b0,%w1"
// 		:
// 	: "a" (Data), "Nd" (Port));
// }
// 
// inl void __outword(unsigned short Port, unsigned short Data)
// {
// 	__asm __volatile("outw %w0,%w1"
// 		:
// 	: "a" (Data), "Nd" (Port));
// }
// 
// inl void __outdword(unsigned short Port, unsigned long Data)
// {
// 	__asm __volatile("outl %0,%w1"
// 		:
// 	: "a" (Data), "Nd" (Port));
// }
// 
// inl uint64_t __rdtscp(uint32_t* Aux) 
// {
// 	uint32_t aux;
// 	uint64_t rax, rdx;
// 	__asm __volatile("rdtscp\n" : "=a" (rax), "=d" (rdx), "=c" (aux) : : );
// 	*Aux = aux;
// 	return (rdx << 32) + rax;
// }
// 
// inl unsigned long long __readpmc(unsigned long counter)
// {
// 	unsigned long long retval;
// 	__asm __volatile("rdpmc" : "=A"(retval) : "c"(counter));
// 	return retval;
// }