#include <asm-generic/errno.h>
#include <cuda_runtime.h>
#include <rp_buffer.h>

#include <cstdlib>

#include "spdlog/spdlog.h"

namespace matched_filter {

RPBuffer::RPBuffer(int rp_count, size_t rp_size) {
	// Size number of resource periods
	data_.resize(rp_count);
	InitializePinnedMemory(rp_size);
	spdlog::info("Allocated {0} resource periods", rp_count);
}

RPBuffer::~RPBuffer() {
	cudaError_t err;
	for (auto& buffer : data_) {
		err = cudaFreeHost((void**)buffer.GetData());
		if (err != cudaSuccess) {
			spdlog::error("Failed to free RPBuffer: CUDA Error at {0} : {1} -> {2}", __FILE__,
						  __LINE__, cudaGetErrorString(err));
		}
	}
	spdlog::info("Destroyed {0} resource periods", data_.size());
}

ResourcePeriod RPBuffer::GetNextAvailable(const int& buffer_size) { return data_.at(next_idx_); }

/** Private */

void RPBuffer::InitializePinnedMemory(size_t rp_size) {
	cudaError_t err;
	cuComplex* data_ptr;
	for (auto& buffer : data_) {
		data_ptr = buffer.GetData();
		err = cudaHostAlloc((void**)&data_ptr, rp_size, cudaHostAllocDefault);
		if (err != cudaSuccess) {
			spdlog::error("Failed to allocate RPBuffer: CUDA Error at {0} : {1} -> {2}", __FILE__,
						  __LINE__, cudaGetErrorString(err));
		}
	}
}

}  // namespace matched_filter