
#include <exception>

#include "app_config.h"
#include "rp_buffer.h"
#include "spdlog/spdlog.h"
#include "udp_socket.h"

using json = nlohmann::json;

int main() {
	try {
		// Read config file
		matched_filter::AppConfig app_config("/home/astehr3/repos/matched_filter/config.json");
		const matched_filter::ConfigFile& config = app_config.GetConfig();

		// Configure shared memory
		size_t resource_period_size = config.sample_rate_hz * config.pri_s;

		matched_filter::RPBuffer rp_buffer(config.rp_buffer_count, resource_period_size);

		// Configure network connection
		matched_filter::UDPClient client(app_config.GetConfig());

	} catch (const std::exception& err) {
		spdlog::error("Caught exception in main: {0} - shutting down", err.what());
		return -1;
	}

	return 0;
}