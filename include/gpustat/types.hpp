#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gpustat {

/** Telemetry snapshot for one NVIDIA GPU. Memory quantities are MiB. */
struct DeviceStats {
    unsigned int index{};
    std::string name;
    unsigned int utilization_percent{};
    std::uint64_t memory_total_mib{};
    std::uint64_t memory_used_mib{};
};

/** Aggregate telemetry for all enumerated NVIDIA GPUs. */
struct SystemStats {
    std::string driver_version;
    std::vector<DeviceStats> devices;
};

/** Aggregate values derived from a non-empty device collection. */
struct AggregateStats {
    double utilization_percent{};
    std::uint64_t memory_total_mib{};
    std::uint64_t memory_used_mib{};
    double memory_utilization_percent{};
};

}  // namespace gpustat
