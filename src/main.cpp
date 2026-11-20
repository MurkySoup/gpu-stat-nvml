#include "gpustat/nvml_monitor.hpp"
#include "gpustat/report.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        const gpustat::NvmlMonitor monitor;
        const gpustat::SystemStats stats = monitor.collect();
        gpustat::print_report(stats, std::cout);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "gpustat: " << error.what() << '\n';
        return 1;
    }
}
