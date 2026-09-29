#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

namespace matched_filter {

struct Config {
  int port;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, port);

} // namespace matched_filter