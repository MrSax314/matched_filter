#pragma once

namespace radarsim {

inline constexpr double kPi           = 3.14159265358979323846;
inline constexpr double kTwoPi        = 2.0 * kPi;
inline constexpr double kSpeedOfLight = 299'792'458.0; // m/s

/// Converts a power ratio in dB to linear scale.
double dbToLinear(double db);

/// Converts a linear power ratio to dB.
double linearToDb(double linear);

} // namespace radarsim
