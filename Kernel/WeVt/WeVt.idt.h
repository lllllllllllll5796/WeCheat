#pragma once

namespace hv
{
	void prepare_host_idt(segment_descriptor_interrupt_gate_64* const idt);
}