#include "gpustat/nvml_monitor.hpp"

#include <nvml.h>

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

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


[[nodiscard]] std::optional<int> read_temperature(
    const nvmlDevice_t device) {

    unsigned int temperature_celsius = 0U;

    const nvmlReturn_t status = nvmlDeviceGetTemperature(
        device,
        NVML_TEMPERATURE_GPU,
        &temperature_celsius
    );

    if (status == NVML_SUCCESS) {
        return static_cast<int>(temperature_celsius);
    }

    if (status == NVML_ERROR_NOT_SUPPORTED) {
        return std::nullopt;
    }

    check_nvml(status, "Reading GPU temperature");
    return std::nullopt;
}

[[nodiscard]] std::optional<double> read_power_watts(
    const nvmlDevice_t device) {
    unsigned int power_milliwatts = 0U;
    const nvmlReturn_t status =
        nvmlDeviceGetPowerUsage(device, &power_milliwatts);

    if (status == NVML_SUCCESS) {
        return static_cast<double>(power_milliwatts) / 1000.0;
    }
    if (status == NVML_ERROR_NOT_SUPPORTED) {
        return std::nullopt;
    }
    check_nvml(status, "Reading GPU power usage");
    return std::nullopt;
}

[[nodiscard]] std::vector<FanStats> read_fans(const nvmlDevice_t device) {
    unsigned int fan_count = 0U;
    const nvmlReturn_t count_status =
        nvmlDeviceGetNumFans(device, &fan_count);

    if (count_status == NVML_ERROR_NOT_SUPPORTED) {
        return {};
    }
    check_nvml(count_status, "Reading GPU fan count");

    std::vector<FanStats> fans;
    fans.reserve(fan_count);

    for (unsigned int fan_index = 0U; fan_index < fan_count; ++fan_index) {
        unsigned int speed_percent = 0U;
        const nvmlReturn_t status =
            nvmlDeviceGetFanSpeed_v2(device, fan_index, &speed_percent);

        if (status == NVML_SUCCESS) {
            fans.push_back(FanStats{
                .index = fan_index,
                .speed_percent = speed_percent,
            });
        } else if (status != NVML_ERROR_NOT_SUPPORTED) {
            check_nvml(status, "Reading GPU fan speed");
        }
    }

    return fans;
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

    unsigned int device_count = 0U;
    check_nvml(nvmlDeviceGetCount_v2(&device_count), "Enumerating NVIDIA GPUs");
    result.devices.reserve(device_count);

    for (unsigned int index = 0U; index < device_count; ++index) {
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
            .temperature_celsius = read_temperature(device),
            .power_watts = read_power_watts(device),
            .fans = read_fans(device),
        });
    }

    return result;
}

}  // namespace gpustat
