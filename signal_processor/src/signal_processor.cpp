#include "signal_processor.h"

#include "spdlog/spdlog.h"

namespace matched_filter {

void SignalProcessor::StartListener() {
	// listener_thread_ = std::thread(&UDPClient::ReadMessages, &connection_, std::ref(rp_buffer_));
	listener_thread_ = std::thread([&] {
		while (running_.load()) {
			connection_.ReadMessage(rp_buffer_);
		}
	});
	spdlog::info("Started lister thread");
}

void SignalProcessor::StopListener() {
	running_.store(false);
	if (listener_thread_.joinable()) {
		listener_thread_.join();
	}
	spdlog::info("Shutdown lister thread");
}

void SignalProcessor::StartProcessing() {
	spdlog::info("Started processing stream");
	return;
}

void SignalProcessor::StopProcessing() {
	spdlog::info("Stopped processing stream");
	return;
}

}  // namespace matched_filter
