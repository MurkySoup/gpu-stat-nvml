#include "gpustat/report.hpp"

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

    for (const DeviceStats& device : stats.devices) {
        utilization_sum += static_cast<double>(device.utilization_percent);
        aggregate.memory_total_mib += device.memory_total_mib;
        aggregate.memory_used_mib += device.memory_used_mib;
    }

    aggregate.utilization_percent =
        utilization_sum / static_cast<double>(stats.devices.size());

    if (aggregate.memory_total_mib > 0U) {
        aggregate.memory_utilization_percent =
            100.0 * static_cast<double>(aggregate.memory_used_mib) /
            static_cast<double>(aggregate.memory_total_mib);
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
    }

    const AggregateStats aggregate = calculate_aggregates(stats);
    output << "Aggregate (" << stats.devices.size() << " GPUs):\n"
           << "  Utilization: " << aggregate.utilization_percent << "%\n"
           << "  Memory: " << aggregate.memory_used_mib << " / "
           << aggregate.memory_total_mib << " MiB ("
           << aggregate.memory_utilization_percent << "%)\n";
}

}  // namespace gpustat
