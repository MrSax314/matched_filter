
#include <unistd.h>

#include <exception>

#include "app_config.h"
#include "signal_processor.h"
#include "spdlog/spdlog.h"

using json = nlohmann::json;

int main() {
	try {
		// Read config file
		matched_filter::AppConfig app_config("/home/astehr3/repos/matched_filter/config.json");
		const matched_filter::ConfigFile& config = app_config.GetConfig();

		// Configure signal processor
		const size_t max_message_size = 1500;  // TODO(as3): Move to config file
		size_t resource_period_size = config.sample_rate_hz * config.pri_s;
		matched_filter::SignalProcessor signal_processor(
		  config.rp_buffer_count, resource_period_size, config.port, max_message_size);

		// Start threads and streams
		signal_processor.StartListener();
		signal_processor.StartProcessing();

		sleep(3);

		// Kill threads and streams
		signal_processor.StopProcessing();
		signal_processor.StopListener();

	} catch (const std::exception& err) {
		spdlog::error("Caught exception in main: {0} - shutting down", err.what());
		return -1;
	}

	return 0;
}