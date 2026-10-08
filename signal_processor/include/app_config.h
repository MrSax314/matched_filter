#pragma once

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace matched_filter {

struct ConfigFile {
	std::string ip;
	int port;
	int rw_buffer_count;
	float sample_rate_hz;
	float pri_s;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigFile, ip, port, rw_buffer_count, sample_rate_hz, pri_s);

class AppConfig {
  public:
	AppConfig() = delete;  // force loading of config
	AppConfig(std::string cfg);

	const ConfigFile& GetConfig() { return config_; }

  private:
	ConfigFile config_;
};

}  // namespace matched_filter