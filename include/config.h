#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

namespace matched_filter {

struct AppConfig {
	int port;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AppConfig, port);

}  // namespace matched_filter