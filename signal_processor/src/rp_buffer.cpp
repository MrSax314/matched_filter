#include <asm-generic/errno.h>
#include <cuda_runtime.h>
#include <rp_buffer.h>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

#include "spdlog/spdlog.h"

namespace matched_filter {

RWBuffer::RWBuffer(int rw_count, size_t rw_sample_ct) {
	// Init receive window buffer memory
	data_.resize(rw_count);
	InitializePinnedMemory(rw_sample_ct);
	spdlog::info("Allocated {0} receive windows of {1} samples ({2} bytes total)", rw_count,
				 rw_sample_ct, rw_sample_ct * sizeof(CCFloat));

	// Init available windows
	for (uint32_t i = 0; i < rw_count; i++) {
		available_windows_.push(i);
	}
}

RWBuffer::~RWBuffer() {
	cudaError_t err;
	for (auto& buffer : data_) {
		err = cudaFreeHost(buffer.GetData());
		if (err != cudaSuccess) {
			spdlog::error("Failed to free RPBuffer: CUDA Error at {0} : {1} -> {2}", __FILE__,
						  __LINE__, cudaGetErrorString(err));
		}
	}
	spdlog::info("Destroyed {0} receive windows", data_.size());
}

void RWBuffer::StoreSamples(const IqPacketHeader* header, const uint8_t* iq_bytes) {
	// TODO(as3): Check we aren't hitting buffer limit

	// Find which receive window data is for
	std::optional<size_t> vector_index;
	ReceiveWindow* receive_window;
	if (vector_index = IsCached(header->pulse_index); vector_index.has_value()) {
		receive_window = GetReceiveWindow(vector_index.value());
		// TODO(as3): Handle optional edge cases
	} else {
		receive_window = GetNextAvailable(header->pulse_index);
		if (receive_window == nullptr) {
			return;
		}
	}

	// Copy buffered IQ into final pinned location
	size_t byte_ct = header->num_samples * sizeof(CCFloat);
	size_t dest_byte_offset = header->sample_offset * sizeof(CCFloat);
	auto* dest = reinterpret_cast<uint8_t*>(receive_window->GetData()) + dest_byte_offset;
	std::memcpy(dest, iq_bytes, byte_ct);

	// Log statistics and flag complete receive windows
	receive_window->AddFilledBytes(byte_ct);
	spdlog::info("Stored some IQ bytes");
}

/** Private */

void RWBuffer::InitializePinnedMemory(size_t rw_sample_ct) {
	cudaError_t err;
	CCFloat* data_ptr;
	for (auto& buffer : data_) {
		size_t byte_ct = rw_sample_ct * sizeof(CCFloat);
		err = cudaHostAlloc((void**)&data_ptr, byte_ct, cudaHostAllocDefault);
		if (err != cudaSuccess) {
			spdlog::error("Failed to allocate RPBuffer: CUDA Error at {0} : {1} -> {2}", __FILE__,
						  __LINE__, cudaGetErrorString(err));
			throw std::runtime_error("Error in RPBuffer::InitializePinnedMemory()");
		}
		buffer.SetData(data_ptr);
		buffer.SetSize(byte_ct);
	}
}

ReceiveWindow* RWBuffer::GetNextAvailable(const uint64_t& pulse_index) {
	// Get next available window
	if (available_windows_.empty()) {
		spdlog::critical("No more available receive windows");
		return nullptr;
		// TODO(as3): Verify this is handled by caller
	}

	// Update local tracking
	auto vector_index = available_windows_.front();
	available_windows_.pop();
	pulse_id_to_index_[pulse_index] = vector_index;
	return &data_.at(vector_index);
}

}  // namespace matched_filter