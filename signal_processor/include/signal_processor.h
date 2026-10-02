#pragma once

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
	 * @param rp_count Number of resource periods to allocate
	 * @param rp_size Number of bytes in each resource period
	 * @param port Network port for listening for messages
	 */
	explicit SignalProcessor(int rp_count, size_t rp_size, int port)
		: rp_buffer_(rp_count, rp_size), connection_(port) {};

  private:
	RPBuffer rp_buffer_;
	UDPClient connection_;
};

}  // namespace matched_filter