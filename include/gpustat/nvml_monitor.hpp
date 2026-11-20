#pragma once

#include "gpustat/types.hpp"

namespace gpustat {

/**
 * Collects a single, sequential NVML telemetry snapshot.
 *
 * NVML is initialized for the lifetime of the monitor. This class is
 * intentionally non-copyable and non-movable to keep NVML ownership simple.
 */
class NvmlMonitor final {
public:
    NvmlMonitor();
    ~NvmlMonitor() noexcept;

    NvmlMonitor(const NvmlMonitor&) = delete;
    NvmlMonitor& operator=(const NvmlMonitor&) = delete;
    NvmlMonitor(NvmlMonitor&&) = delete;
    NvmlMonitor& operator=(NvmlMonitor&&) = delete;

    /** Query driver version and telemetry for every detected NVIDIA GPU. */
    [[nodiscard]] SystemStats collect() const;

private:
    bool initialized_{false};
};

}  // namespace gpustat
