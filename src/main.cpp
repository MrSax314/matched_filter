
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
		const size_t max_message_size_b = 1500;	 // bytes
		// TODO(as3): Move to config file

		// Receive window is from start of transmit to start of following transmit,
		// so the number of samples is simply the sampling rate * pulse rate interval
		size_t receive_window_sample_ct = config.sample_rate_hz * config.pri_s;	 //
		matched_filter::SignalProcessor signal_processor(
		  config.rw_buffer_count, receive_window_sample_ct, config.port, max_message_size_b);

		// Start threads and streams
		signal_processor.StartListener();
		signal_processor.StartProcessing();

		sleep(3);

		// Kill threads and streams
		signal_processor.StopListener();
		signal_processor.StopProcessing();

	} catch (const std::exception& err) {
		spdlog::error("Caught exception in main: {0} - shutting down", err.what());
		return -1;
	}

	return 0;
}