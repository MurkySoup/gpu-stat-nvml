#pragma once

#include "gpustat/types.hpp"

#include <iosfwd>

namespace gpustat {

/** Calculate aggregate utilization and memory statistics. */
[[nodiscard]] AggregateStats calculate_aggregates(const SystemStats& stats);

/** Print a concise human-readable report. */
void print_report(const SystemStats& stats, std::ostream& output);

}  // namespace gpustat
