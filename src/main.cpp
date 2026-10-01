
#include <iostream>

#include "app_config.h"
#include "spdlog/spdlog.h"
#include "udp_socket.h"

using json = nlohmann::json;

int main() {
	matched_filter::AppConfig config("/home/astehr3/repos/matched_filter/config.json");
	matched_filter::UDPClient client(config.GetConfig());

	std::cout << "Hello World!" << std::endl;
	return 0;
}