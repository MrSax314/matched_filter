#include <app_config.h>

#include <fstream>
#include <iostream>

#include "spdlog/spdlog.h"

namespace matched_filter {

AppConfig::AppConfig(std::string cfg) {
	// Parse config file
	std::ifstream f(cfg);
	using json = nlohmann::json;
	json j = json::parse(f);
	// std::cout << "Loaded config: " << j.dump() << "\n";
	spdlog::info("Loaded config: {}", j.dump());
	config_ = j.get<ConfigFile>();
}

}  // namespace matched_filter