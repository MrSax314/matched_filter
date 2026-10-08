#include "radarsim/protocol/Constants.hpp"

#include <cmath>

namespace radarsim {

double dbToLinear(double db) { return std::pow(10.0, db / 10.0); }

double linearToDb(double linear) { return 10.0 * std::log10(linear); }

} // namespace radarsim
