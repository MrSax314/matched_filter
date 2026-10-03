#include <asm-generic/errno.h>
#include <cuda_runtime.h>
#include <rp_buffer.h>

#include <cstdlib>
#include <stdexcept>

#include "spdlog/spdlog.h"

namespace matched_filter {

RWBuffer::RWBuffer(int rw_count, size_t rw_sample_ct) {
	// Init receive window buffer memory
	data_.resize(rw_count);
	InitializePinnedMemory(rw_sample_ct * sizeof(cuComplex));
	spdlog::info("Allocated {0} receive windows of {1} samples ({2} bytes total)", rw_count,
				 rw_sample_ct, rw_sample_ct * sizeof(cuComplex));
}

RWBuffer::~RWBuffer() {
	cudaError_t err;
	for (auto& buffer : data_) {
		err = cudaFreeHost((void**)buffer.GetData());
		if (err != cudaSuccess) {
			spdlog::error("Failed to free RPBuffer: CUDA Error at {0} : {1} -> {2}", __FILE__,
						  __LINE__, cudaGetErrorString(err));
		}
	}
	spdlog::info("Destroyed {0} receive windows", data_.size());
}

ReceiveWindow RWBuffer::GetNextAvailable(const int& buffer_size) { return data_.at(next_idx_); }

/** Private */

void RWBuffer::InitializePinnedMemory(size_t rw_sample_ct) {
	cudaError_t err;
	cuComplex* data_ptr;
	for (auto& buffer : data_) {
		data_ptr = buffer.GetData();
		err =
		  cudaHostAlloc((void**)&data_ptr, rw_sample_ct * sizeof(cuComplex), cudaHostAllocDefault);
		if (err != cudaSuccess) {
			spdlog::error("Failed to allocate RPBuffer: CUDA Error at {0} : {1} -> {2}", __FILE__,
						  __LINE__, cudaGetErrorString(err));
			throw std::runtime_error("Error in RPBuffer::InitializePinnedMemory()");
		}
	}
}

}  // namespace matched_filter