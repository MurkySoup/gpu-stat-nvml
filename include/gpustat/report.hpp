#pragma once

#include "gpustat/types.hpp"

#include <iosfwd>

namespace gpustat {

/** Calculate aggregate statistics; throws if no GPUs are present. */
[[nodiscard]] AggregateStats calculate_aggregates(const SystemStats& stats);

/** Print a human-readable report. */
void print_report(const SystemStats& stats, std::ostream& output);

}  // namespace gpustat
