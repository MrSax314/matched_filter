#include <app_config.h>

#include <fstream>
#include <iostream>

namespace matched_filter {

AppConfig::AppConfig(std::string cfg) {
	// Parse config file
	std::ifstream f(cfg);
	using json = nlohmann::json;
	json j = json::parse(f);
	std::cout << "Loaded config: " << j.dump() << "\n";
	config_ = j.get<ConfigFile>();
}

}  // namespace matched_filter