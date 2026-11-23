#include "gpustat/report.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>

namespace gpustat {
namespace {

TEST(Aggregates, CalculatesUtilizationMemoryAndThermals) {
    const SystemStats stats{
        .driver_version = "580.178.04",
        .devices = {
            DeviceStats{0U, "GPU A", 80U, 12288U, 6144U, 58, 120.0,
                        {{0U, 40U}, {1U, 50U}}},
            DeviceStats{1U, "GPU B", 20U, 12288U, 3072U, 64, 180.0,
                        {{0U, 60U}}},
            DeviceStats{2U, "GPU C", 50U, 12288U, 4096U, 72, 150.0,
                        {{0U, 70U}, {1U, 80U}}},
        },
    };

    const AggregateStats aggregate = calculate_aggregates(stats);

    EXPECT_DOUBLE_EQ(aggregate.utilization_percent, 50.0);
    EXPECT_EQ(aggregate.memory_total_mib, 36864U);
    EXPECT_EQ(aggregate.memory_used_mib, 13312U);
    EXPECT_NEAR(aggregate.memory_utilization_percent, 36.1111, 0.001);

    ASSERT_TRUE(aggregate.temperature_average_celsius.has_value());
    ASSERT_TRUE(aggregate.temperature_min_celsius.has_value());
    ASSERT_TRUE(aggregate.temperature_max_celsius.has_value());
    EXPECT_NEAR(*aggregate.temperature_average_celsius, 64.6667, 0.001);
    EXPECT_EQ(*aggregate.temperature_min_celsius, 58);
    EXPECT_EQ(*aggregate.temperature_max_celsius, 72);

    ASSERT_TRUE(aggregate.power_total_watts.has_value());
    ASSERT_TRUE(aggregate.power_average_watts.has_value());
    EXPECT_DOUBLE_EQ(*aggregate.power_total_watts, 450.0);
    EXPECT_DOUBLE_EQ(*aggregate.power_average_watts, 150.0);

    ASSERT_TRUE(aggregate.fan_average_percent.has_value());
    ASSERT_TRUE(aggregate.fan_min_percent.has_value());
    ASSERT_TRUE(aggregate.fan_max_percent.has_value());
    // Per-GPU fan means are 45, 60 and 75; GPUs are equally weighted.
    EXPECT_DOUBLE_EQ(*aggregate.fan_average_percent, 60.0);
    EXPECT_EQ(*aggregate.fan_min_percent, 40U);
    EXPECT_EQ(*aggregate.fan_max_percent, 80U);
}

TEST(Aggregates, IgnoresUnavailableOptionalTelemetry) {
    const SystemStats stats{
        .driver_version = "test",
        .devices = {
            DeviceStats{0U, "GPU A", 50U, 1000U, 500U, 60, 100.0,
                        {{0U, 40U}}},
            DeviceStats{1U, "GPU B", 50U, 1000U, 500U, std::nullopt,
                        std::nullopt, {}},
        },
    };

    const AggregateStats aggregate = calculate_aggregates(stats);

    ASSERT_TRUE(aggregate.temperature_average_celsius.has_value());
    EXPECT_DOUBLE_EQ(*aggregate.temperature_average_celsius, 60.0);

    ASSERT_TRUE(aggregate.power_total_watts.has_value());
    EXPECT_DOUBLE_EQ(*aggregate.power_total_watts, 100.0);
    EXPECT_DOUBLE_EQ(*aggregate.power_average_watts, 100.0);

    ASSERT_TRUE(aggregate.fan_average_percent.has_value());
    EXPECT_DOUBLE_EQ(*aggregate.fan_average_percent, 40.0);
}

TEST(Aggregates, HandlesAllOptionalTelemetryUnavailable) {
    const SystemStats stats{
        .driver_version = "test",
        .devices = {
            DeviceStats{0U, "GPU A", 10U, 1000U, 100U,
                        std::nullopt, std::nullopt, {}},
        },
    };

    const AggregateStats aggregate = calculate_aggregates(stats);

    EXPECT_FALSE(aggregate.temperature_average_celsius.has_value());
    EXPECT_FALSE(aggregate.temperature_min_celsius.has_value());
    EXPECT_FALSE(aggregate.temperature_max_celsius.has_value());
    EXPECT_FALSE(aggregate.power_total_watts.has_value());
    EXPECT_FALSE(aggregate.power_average_watts.has_value());
    EXPECT_FALSE(aggregate.fan_average_percent.has_value());
    EXPECT_FALSE(aggregate.fan_min_percent.has_value());
    EXPECT_FALSE(aggregate.fan_max_percent.has_value());
}

TEST(Aggregates, RejectsEmptyDeviceCollection) {
    const SystemStats stats{.driver_version = "test", .devices = {}};
    EXPECT_THROW(
        static_cast<void>(calculate_aggregates(stats)),
        std::invalid_argument);
}

TEST(Report, HandlesNoDetectedDevices) {
    const SystemStats stats{.driver_version = "580.178.04", .devices = {}};
    std::ostringstream output;
    print_report(stats, output);
    EXPECT_NE(output.str().find("No NVIDIA GPUs detected."), std::string::npos);
}

TEST(Report, IncludesTemperaturePowerAndFanData) {
    const SystemStats stats{
        .driver_version = "580.178.04",
        .devices = {
            DeviceStats{0U, "GeForce RTX 3060", 25U, 12288U, 4096U,
                        55, 85.5, {{0U, 35U}, {1U, 40U}}},
        },
    };

    std::ostringstream output;
    print_report(stats, output);
    const std::string report = output.str();

    EXPECT_NE(report.find("580.178.04"), std::string::npos);
    EXPECT_NE(report.find("GeForce RTX 3060"), std::string::npos);
    EXPECT_NE(report.find("Temperature: 55 C"), std::string::npos);
    EXPECT_NE(report.find("Power: 85.5 W"), std::string::npos);
    EXPECT_NE(report.find("Fan 0: 35%"), std::string::npos);
    EXPECT_NE(report.find("Fan 1: 40%"), std::string::npos);
    EXPECT_NE(report.find("Aggregate (1 GPUs)"), std::string::npos);
}

}  // namespace
}  // namespace gpustat
