#pragma once

#include <sys/socket.h>

#include "app_config.h"

namespace matched_filter {

class UDPClient {
  public:
	UDPClient() = delete;
	UDPClient(const UDPClient& client) = delete;
	UDPClient(UDPClient&& client) = delete;

	explicit UDPClient(const ConfigFile& cfg);
	~UDPClient();

	void ReadMessage();

  private:
	int sockfd_ = -1;
	int port_ = 0;
};

}  // namespace matched_filter