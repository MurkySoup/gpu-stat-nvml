# gpustat

# gpu-stat-nvml

A lightweight Linux NVIDIA GPU telemetry CLI using the NVIDIA Management Library (NVML). This implementation is C++20 and does not require NVCC or the CUDA Runtime.

---

## Features

- NVIDIA driver version.
- Enumerates visible NVIDIA GPUs and reports device names.
- Per-GPU GPU utilization and framebuffer memory use.
- Per-GPU GPU die temperature.
- Per-GPU board power draw.
- Per-physical-fan speed percentage.
- Aggregate utilization and memory utilization.
- Aggregate temperature average, minimum, and maximum.
- Aggregate board power total and average per reporting GPU.
- Aggregate fan percentage average, minimum, and maximum.

---

## Requirements

- Linux
- NVIDIA driver with NVML support (`libnvidia-ml.so`)
- NVML development header (`nvml.h`), commonly supplied by the NVIDIA driver
  development package or CUDA toolkit
- CMake 3.24+
- C++20 compiler (GCC 11+ or a suitable Clang release)
- Optional: GoogleTest for unit tests

CUDA Runtime and NVCC are not required. NVML queries the driver and does not launch kernels, create CUDA contexts, allocate device memory, or copy data between host and GPU.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
sudo cmake --install build
```

If NVML is installed in a non-standard location, set
`-DNVML_INCLUDE_DIR=/path/to/include` and
`-DNVML_LIBRARY=/path/to/libnvidia-ml.so`.

Run:

```bash
gpustat
```

Example output

```text
NVIDIA Driver: 580.178.04
GPU 0: GeForce RTX 3060
  Utilization: 25%
  Memory: 4096 / 12288 MiB (33.3%)
  Temperature: 55 C
  Power: 85.5 W
  Fan 0: 35%
  Fan 1: 40%
GPU 1: GeForce RTX 3060
  Utilization: 75%
  Memory: 8192 / 12288 MiB (66.7%)
  Temperature: 64 C
  Power: 112.0 W
  Fan 0: 55%
Aggregate (2 GPUs):
  Utilization: 50.0%
  Memory: 12288 / 24576 MiB (50.0%)
  Temperature: 59.5 C average (55 C min, 64 C max)
  Power: 197.5 W total (98.8 W average/GPU)
  Fan speed: 46.3% average/GPU (35% min, 55% max)
```

## Metric semantics and caveats

- Memory is reported in MiB (1 MiB = 1,048,576 bytes).
- GPU utilization is NVML's sampled `utilization.gpu` percentage. Aggregate utilization is the unweighted arithmetic mean across enumerated GPUs.
- Memory aggregate is sum(used) / sum(total).
- Temperature is the GPU die temperature in whole degrees Celsius, using `nvmlDeviceGetTemperatureV`.
- Aggregate temperature is the arithmetic mean of available GPU readings; minimum and maximum are over available GPU readings.
- Power is returned by `nvmlDeviceGetPowerUsage` in milliwatts and displayed in watts. It represents GPU board power and associated circuitry, not whole-system wall power. On Ampere (except GA100) and newer GPUs, NVML documents this value as averaged over a one-second interval; behavior varies by architecture.
- Aggregate power is the sum of available GPU readings; the average is the arithmetic mean across GPUs with a power reading.
- Fan speed is reported per physical fan using `nvmlDeviceGetFanSpeed_v2`. NVML describes this as intended operating speed, expressed as a percentage of the product's maximum noise-tolerance fan speed; it can exceed 100%. It is not an RPM measurement and may not reveal a mechanically stalled fan.
- Per-GPU fan data lists every fan that successfully reports. Aggregate fan average first averages fans within each GPU, then averages those GPU means equally. Min/max are over all individual fan readings. This prevents GPUs with more fans from receiving more weight in the aggregate mean.
- Unsupported optional telemetry is shown as `N/A` and excluded from aggregation. Unexpected NVML errors fail the snapshot with a diagnostic instead of silently substituting zero.
- Values are a sequential snapshot, not an atomic synchronized measurement. NVML queries occur one after another; workload and power can change between readings.

## Tests

Install GoogleTest, then configure and build with `-DGPUSTAT_BUILD_TESTS=ON`. CTest runs the aggregation and formatting tests. The tests do not require an NVIDIA GPU because they exercise pure reporting logic.

## Operational design

- No CUDA context, kernels, streams, device allocations, or device-host copies.
- No background thread or polling loop.
- One sequential set of NVML queries per device per invocation.
- RAII-style NVML initialization/shutdown ownership.
- Device handles are borrowed NVML handles; no per-handle deallocation.
- The program only reads telemetry. It never changes fan policy, clocks, power limits, persistence mode, or other device configuration.

## Suggested validation

```bash
ctest --test-dir build --output-on-failure
clang-tidy -p build src/*.cpp
clang-format -i include/gpustat/*.hpp src/*.cpp tests/*.cpp
```

On a GPU-equipped host, compare results with `nvidia-smi` and exercise:
- no NVIDIA driver / NVML unavailable;
- one GPU and multiple GPUs;
- GPU with no workload and GPU under load;
- memory use changes while querying;
- permissions and container/device visibility;
- driver reset or GPU removal during collection.

For this telemetry-only application, Nsight Systems/Compute and Compute Sanitizer are not normally applicable: there are no kernels or device memory operations to profile or sanitize. Use them only if future revisions add CUDA work. ASan/UBSan can be enabled with compiler flags; TSan is useful only if concurrent collection is introduced. Valgrind Memcheck can check host-side resource handling.

---

# License

This tool is released under the Apache 2.0 license. See the LICENSE file in this repo for details.

# Built With

* [C++20](https://en.wikipedia.org/wiki/C%2B%2B)

## Author

**Rick Pelletier**

