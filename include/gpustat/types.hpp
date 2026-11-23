#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace gpustat {

/** One physical fan's reported operating speed. */
struct FanStats {
    unsigned int index{};
    unsigned int speed_percent{};
};

/** Telemetry snapshot for one NVIDIA GPU. Memory quantities are MiB. */
struct DeviceStats {
    unsigned int index{};
    std::string name;
    unsigned int utilization_percent{};
    std::uint64_t memory_total_mib{};
    std::uint64_t memory_used_mib{};
    std::optional<int> temperature_celsius;
    std::optional<double> power_watts;
    std::vector<FanStats> fans;
};

/** Telemetry snapshot for all GPUs visible to NVML. */
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

    std::optional<double> temperature_average_celsius;
    std::optional<int> temperature_min_celsius;
    std::optional<int> temperature_max_celsius;

    // Sum across GPUs with a supported power reading.
    std::optional<double> power_total_watts;
    // Arithmetic mean across GPUs with a supported power reading.
    std::optional<double> power_average_watts;

    // Each GPU contributes its own mean fan percentage equally.
    std::optional<double> fan_average_percent;
    std::optional<unsigned int> fan_min_percent;
    std::optional<unsigned int> fan_max_percent;
};

}  // namespace gpustat
