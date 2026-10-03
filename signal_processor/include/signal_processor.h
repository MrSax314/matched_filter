#pragma once

#include <cuda_runtime.h>

#include <atomic>
#include <condition_variable>
#include <thread>

#include "rp_buffer.h"
#include "udp_socket.h"

namespace matched_filter {

class SignalProcessor {
  public:
	SignalProcessor() = delete;
	~SignalProcessor() = default;

	/**
	 * @brief Construct a new Signal Processor object
	 *
	 * @param rw_count Number of range windows to allocate
	 * @param rw_sample_ct Number of samples in each range window
	 * @param port Network port for listening for messages
	 */
	explicit SignalProcessor(int rw_count, size_t rw_sample_ct, int port,
							 const size_t max_message_size)
		: rp_buffer_(rw_count, rw_sample_ct), connection_(port, max_message_size) {};

	void StartListener();
	void StopListener();

	void StartProcessing();
	void StopProcessing();

  private:
	RPBuffer rp_buffer_;
	UDPClient connection_;

	// Threads & Streams
	std::thread listener_thread_;
	cudaStream_t processing_stream_;

	// Signal variables
	std::atomic<bool> running_;
	std::condition_variable processing_ready_;
};

}  // namespace matched_filter