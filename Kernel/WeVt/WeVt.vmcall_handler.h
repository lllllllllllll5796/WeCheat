#pragma once

void restore_segment_registers();
void call_vmxoff(__vcpu* vcpu);
void vmexit_vmcall_handler(__vcpu* vcpu);
