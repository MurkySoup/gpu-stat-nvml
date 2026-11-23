#pragma once

#include "gpustat/types.hpp"

namespace gpustat {

/**
 * Collects one sequential NVML telemetry snapshot.
 *
 * NVML is initialized for the monitor's lifetime. NVML device handles are
 * borrowed handles and are not independently released.
 */
class NvmlMonitor final {
public:
    NvmlMonitor();
    ~NvmlMonitor() noexcept;

    NvmlMonitor(const NvmlMonitor&) = delete;
    NvmlMonitor& operator=(const NvmlMonitor&) = delete;
    NvmlMonitor(NvmlMonitor&&) = delete;
    NvmlMonitor& operator=(NvmlMonitor&&) = delete;

    /** Query driver version and telemetry for every enumerated NVIDIA GPU. */
    [[nodiscard]] SystemStats collect() const;

private:
    bool initialized_{false};
};

}  // namespace gpustat
