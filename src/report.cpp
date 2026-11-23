#include "gpustat/report.hpp"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <stdexcept>

namespace gpustat {

AggregateStats calculate_aggregates(const SystemStats& stats) {
    if (stats.devices.empty()) {
        throw std::invalid_argument("Cannot aggregate an empty GPU collection");
    }

    AggregateStats aggregate{};
    double utilization_sum = 0.0;
    double temperature_sum = 0.0;
    double power_sum = 0.0;
    double fan_gpu_average_sum = 0.0;
    std::size_t temperature_count = 0U;
    std::size_t power_count = 0U;
    std::size_t fan_gpu_count = 0U;

    for (const DeviceStats& device : stats.devices) {
        utilization_sum += static_cast<double>(device.utilization_percent);
        aggregate.memory_total_mib += device.memory_total_mib;
        aggregate.memory_used_mib += device.memory_used_mib;

        if (device.temperature_celsius.has_value()) {
            const int temperature = *device.temperature_celsius;
            temperature_sum += static_cast<double>(temperature);
            ++temperature_count;

            aggregate.temperature_min_celsius =
                aggregate.temperature_min_celsius.has_value()
                    ? std::min(*aggregate.temperature_min_celsius, temperature)
                    : temperature;
            aggregate.temperature_max_celsius =
                aggregate.temperature_max_celsius.has_value()
                    ? std::max(*aggregate.temperature_max_celsius, temperature)
                    : temperature;
        }

        if (device.power_watts.has_value()) {
            power_sum += *device.power_watts;
            ++power_count;
        }

        if (!device.fans.empty()) {
            double device_fan_sum = 0.0;
            for (const FanStats& fan : device.fans) {
                device_fan_sum += static_cast<double>(fan.speed_percent);

                aggregate.fan_min_percent =
                    aggregate.fan_min_percent.has_value()
                        ? std::min(*aggregate.fan_min_percent,
                                   fan.speed_percent)
                        : fan.speed_percent;
                aggregate.fan_max_percent =
                    aggregate.fan_max_percent.has_value()
                        ? std::max(*aggregate.fan_max_percent,
                                   fan.speed_percent)
                        : fan.speed_percent;
            }

            fan_gpu_average_sum +=
                device_fan_sum / static_cast<double>(device.fans.size());
            ++fan_gpu_count;
        }
    }

    aggregate.utilization_percent =
        utilization_sum / static_cast<double>(stats.devices.size());

    if (aggregate.memory_total_mib > 0U) {
        aggregate.memory_utilization_percent =
            100.0 * static_cast<double>(aggregate.memory_used_mib) /
            static_cast<double>(aggregate.memory_total_mib);
    }

    if (temperature_count > 0U) {
        aggregate.temperature_average_celsius =
            temperature_sum / static_cast<double>(temperature_count);
    }

    if (power_count > 0U) {
        aggregate.power_total_watts = power_sum;
        aggregate.power_average_watts =
            power_sum / static_cast<double>(power_count);
    }

    if (fan_gpu_count > 0U) {
        aggregate.fan_average_percent =
            fan_gpu_average_sum / static_cast<double>(fan_gpu_count);
    }

    return aggregate;
}

void print_report(const SystemStats& stats, std::ostream& output) {
    output << "NVIDIA Driver: " << stats.driver_version << '\n';

    if (stats.devices.empty()) {
        output << "No NVIDIA GPUs detected.\n";
        return;
    }

    output << std::fixed << std::setprecision(1);

    for (const DeviceStats& device : stats.devices) {
        const double memory_percent = device.memory_total_mib == 0U
            ? 0.0
            : 100.0 * static_cast<double>(device.memory_used_mib) /
                  static_cast<double>(device.memory_total_mib);

        output << "GPU " << device.index << ": " << device.name << '\n'
               << "  Utilization: " << device.utilization_percent << "%\n"
               << "  Memory: " << device.memory_used_mib << " / "
               << device.memory_total_mib << " MiB ("
               << memory_percent << "%)\n";

        output << "  Temperature: ";
        if (device.temperature_celsius.has_value()) {
            output << *device.temperature_celsius << " C\n";
        } else {
            output << "N/A\n";
        }

        output << "  Power: ";
        if (device.power_watts.has_value()) {
            output << *device.power_watts << " W\n";
        } else {
            output << "N/A\n";
        }

        if (device.fans.empty()) {
            output << "  Fan speed: N/A\n";
        } else {
            for (const FanStats& fan : device.fans) {
                output << "  Fan " << fan.index << ": "
                       << fan.speed_percent << "%\n";
            }
        }
    }

    const AggregateStats aggregate = calculate_aggregates(stats);
    output << "Aggregate (" << stats.devices.size() << " GPUs):\n"
           << "  Utilization: " << aggregate.utilization_percent << "%\n"
           << "  Memory: " << aggregate.memory_used_mib << " / "
           << aggregate.memory_total_mib << " MiB ("
           << aggregate.memory_utilization_percent << "%)\n";

    output << "  Temperature: ";
    if (aggregate.temperature_average_celsius.has_value()) {
        output << *aggregate.temperature_average_celsius
               << " C average ("
               << *aggregate.temperature_min_celsius << " C min, "
               << *aggregate.temperature_max_celsius << " C max)\n";
    } else {
        output << "N/A\n";
    }

    output << "  Power: ";
    if (aggregate.power_total_watts.has_value()) {
        output << *aggregate.power_total_watts << " W total ("
               << *aggregate.power_average_watts << " W average/GPU)\n";
    } else {
        output << "N/A\n";
    }

    output << "  Fan speed: ";
    if (aggregate.fan_average_percent.has_value()) {
        output << *aggregate.fan_average_percent
               << "% average/GPU ("
               << *aggregate.fan_min_percent << "% min, "
               << *aggregate.fan_max_percent << "% max)\n";
    } else {
        output << "N/A\n";
    }
}

}  // namespace gpustat
