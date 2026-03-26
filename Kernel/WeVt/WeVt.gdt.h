#pragma once

namespace hv {


	void prepare_host_gdt(
		segment_descriptor_32* const gdt,
		task_state_segment_64 const* const tss);


}