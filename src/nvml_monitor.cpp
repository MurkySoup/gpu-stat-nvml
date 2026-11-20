#include "gpustat/nvml_monitor.hpp"

#include <nvml.h>

#include <limits>
#include <stdexcept>
#include <string>

namespace gpustat {
namespace {

void check_nvml(const nvmlReturn_t result, const char* operation) {
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(
            std::string(operation) + ": " + nvmlErrorString(result));
    }
}

[[nodiscard]] std::uint64_t bytes_to_mib(const unsigned long long bytes) {
    constexpr std::uint64_t bytes_per_mib = 1024ULL * 1024ULL;
    return static_cast<std::uint64_t>(bytes) / bytes_per_mib;
}

}  // namespace

NvmlMonitor::NvmlMonitor() {
    check_nvml(nvmlInit_v2(), "NVML initialization");
    initialized_ = true;
}

NvmlMonitor::~NvmlMonitor() noexcept {
    if (initialized_) {
        (void)nvmlShutdown();
    }
}

SystemStats NvmlMonitor::collect() const {
    SystemStats result;

    char driver_version[NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE]{};
    check_nvml(
        nvmlSystemGetDriverVersion(driver_version, sizeof(driver_version)),
        "Reading NVIDIA driver version");
    result.driver_version = driver_version;

    unsigned int device_count = 0;
    check_nvml(nvmlDeviceGetCount_v2(&device_count), "Enumerating NVIDIA GPUs");
    result.devices.reserve(device_count);

    for (unsigned int index = 0; index < device_count; ++index) {
        nvmlDevice_t device{};
        check_nvml(
            nvmlDeviceGetHandleByIndex_v2(index, &device),
            "Opening NVIDIA GPU");

        char name[NVML_DEVICE_NAME_BUFFER_SIZE]{};
        check_nvml(
            nvmlDeviceGetName(device, name, sizeof(name)),
            "Reading GPU name");

        nvmlUtilization_t utilization{};
        check_nvml(
            nvmlDeviceGetUtilizationRates(device, &utilization),
            "Reading GPU utilization");

        nvmlMemory_t memory{};
        check_nvml(
            nvmlDeviceGetMemoryInfo(device, &memory),
            "Reading GPU memory");

        result.devices.push_back(DeviceStats{
            .index = index,
            .name = name,
            .utilization_percent = utilization.gpu,
            .memory_total_mib = bytes_to_mib(memory.total),
            .memory_used_mib = bytes_to_mib(memory.used),
        });
    }

    return result;
}

}  // namespace gpustat
