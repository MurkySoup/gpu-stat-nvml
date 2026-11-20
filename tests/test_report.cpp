#include "gpustat/report.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>

namespace gpustat {
namespace {

TEST(Aggregates, CalculatesMeanUtilizationAndWeightedMemory) {
    const SystemStats stats{
        .driver_version = "580.178.04",
        .devices = {
            DeviceStats{0U, "GPU A", 80U, 12288U, 6144U},
            DeviceStats{1U, "GPU B", 20U, 12288U, 3072U},
        },
    };

    const AggregateStats aggregate = calculate_aggregates(stats);
    EXPECT_DOUBLE_EQ(aggregate.utilization_percent, 50.0);
    EXPECT_EQ(aggregate.memory_total_mib, 24576U);
    EXPECT_EQ(aggregate.memory_used_mib, 9216U);
    EXPECT_DOUBLE_EQ(aggregate.memory_utilization_percent, 37.5);
}

//TEST(Aggregates, RejectsEmptyDeviceCollection) {
//    const SystemStats stats{.driver_version = "test", .devices = {}};
//    EXPECT_THROW(calculate_aggregates(stats), std::invalid_argument);
//}

TEST(Aggregates, RejectsEmptyDeviceCollection) {
    const SystemStats stats{
        .driver_version = "test",
        .devices = {}
    };

    EXPECT_THROW(
        static_cast<void>(calculate_aggregates(stats)),
        std::invalid_argument
    );
}

TEST(Report, HandlesNoDetectedDevices) {
    const SystemStats stats{.driver_version = "580.178.04", .devices = {}};
    std::ostringstream output;
    print_report(stats, output);
    EXPECT_NE(output.str().find("No NVIDIA GPUs detected."), std::string::npos);
}

TEST(Report, IncludesDeviceAndAggregateInformation) {
    const SystemStats stats{
        .driver_version = "580.178.04",
        .devices = {DeviceStats{0U, "GeForce RTX 3060", 25U, 12288U, 4096U}},
    };
    std::ostringstream output;
    print_report(stats, output);
    EXPECT_NE(output.str().find("580.178.04"), std::string::npos);
    EXPECT_NE(output.str().find("GeForce RTX 3060"), std::string::npos);
    EXPECT_NE(output.str().find("Aggregate (1 GPUs)"), std::string::npos);
}

}  // namespace
}  // namespace gpustat
