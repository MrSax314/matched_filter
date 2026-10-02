#pragma once

#include <sys/socket.h>

#include "rp_buffer.h"

namespace matched_filter {

class UDPClient {
	// TODO(as3): Create 'client' base class to support different network protocols
  public:
	UDPClient() = delete;
	UDPClient(const UDPClient& client) = delete;
	UDPClient(UDPClient&& client) = delete;

	explicit UDPClient(const int& port, const size_t max_message_size);
	~UDPClient();

	void ReadMessage(RPBuffer& buffer);

  private:
	uint8_t* pinned_buffer_;
	size_t max_message_size_;
	int sockfd_ = -1;
	int port_ = 0;
};

}  // namespace matched_filter