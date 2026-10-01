#pragma once

#include <sys/socket.h>

#include "app_config.h"

namespace matched_filter {

class UDPClient {
  public:
	explicit UDPClient(const ConfigFile& cfg);

  private:
	int sockfd = -1;
	int port = 0;
};

}  // namespace matched_filter