# gpu-stat-nvml

A small Linux command-line NVIDIA GPU telemetry utility using NVML

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

Example output:

```text
NVIDIA Driver: 580.178.04
GPU 0: GeForce RTX 3060
  Utilization: 25%
  Memory: 4096 / 12288 MiB (33.3%)
Aggregate (1 GPUs):
  Utilization: 25.0%
  Memory: 4096 / 12288 MiB (33.3%)
```

## Semantics

- Memory is reported in MiB (1 MiB = 1,048,576 bytes), not decimal MB.
- Per-GPU utilization is NVML's `utilization.gpu` sample.
- Aggregate GPU utilization is the unweighted arithmetic mean of per-GPU utilization percentages. Each GPU contributes equally.
- Aggregate memory utilization is sum(memory used) / sum(memory total).
- NVML utilization is a sampled driver metric, not a process-level attribution. A single snapshot is not a guarantee of instantaneous utilization.
- The utility reports NVML errors rather than silently substituting zero. Unsupported telemetry or driver communication failures therefore produce a non-zero exit status and a diagnostic on stderr.

## Tests

Install GoogleTest, then configure and build with `-DGPUSTAT_BUILD_TESTS=ON`. CTest runs the aggregation and formatting tests. The tests do not require an NVIDIA GPU because they exercise pure reporting logic.

## Operational considerations

This implementation performs one sequential enumeration and a small fixed set of NVML queries per device. It does not poll continuously. Repeated invocation frequency is controlled by the caller. Avoid tight polling loops; for a basic status command, one snapshot per invocation is sufficient.

NVML initialization and shutdown are paired using object lifetime. Device handles are borrowed NVML handles and are not independently freed. No raw device allocations, streams, events, or CUDA contexts are created.

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

