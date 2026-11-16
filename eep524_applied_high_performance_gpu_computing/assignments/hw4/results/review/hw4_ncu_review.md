**HW4 Nsight Compute capture review**

The three captures are usable for the homework analysis. Their launches, global-load request counts, output stores, shared-memory allocations, and conflict counters match the reviewed implementations. They do not demonstrate that corner turning is faster than the ordinary tiled implementation: it is slower in all three captures. Some cache/DRAM and derived percentage values need the qualifications below.

**Scope and provenance**

Reviewed the original `../hw4_tile8.ncu-repz`, `../hw4_tile16.ncu-repz`, and `../hw4_tile32.ncu-repz` with NVIDIA’s Python Report Interface (`ncu-report` 2026.3.1). Each contains one range, all four expected kernels, and 1,454 metrics per kernel. The recorded GPU is a Tesla T4 with 40 SMs, 32 maximum resident warps per SM, 4 MiB L2, and ECC enabled. Each action used 34 replay passes. The companion JSON includes the exact metric names, units, selected opcode counts, original report hashes, and 132 passing consistency checks. These checks establish agreement with the expected workload; they are not a substitute for GPU correctness verification. Original report files were not changed.

**Recorded kernel durations (microseconds)**

| Multiplication tile | Naive | Tiled | Transpose | Corner turning |
| --- | --- | --- | --- | --- |
| 8 x 8 | 87.488 | 63.072 | 3.552 | 72.032 |
| 16 x 16 | 61.152 | 45.728 | 3.680 | 53.824 |
| 32 x 32 | 52.320 | 48.672 | 3.488 | 58.208 |

These are individual kernel durations reported by Nsight, not full-round-trip application times. Corner-turning duration excludes its separate transpose, host/device copies, allocation, and host-side synchronization overhead. Adding two profiler kernel durations is not a measurement of the host round trip. In these captures 16 x 16 gives the shortest tiled and corner-turning times, while 32 x 32 gives the shortest naive time. Each report contains one captured invocation, so these rankings are observations rather than repeated-run statistics.

**Launches and memory allocations agree with the code**

| Multiplication tile | Block threads | Grid blocks | Static shared bytes/block (tiled and corner) |
| --- | --- | --- | --- |
| 8 x 8 | 64 | 1024 | 512 |
| 16 x 16 | 256 | 256 | 2048 |
| 32 x 32 | 1024 | 64 | 8192 |

Every multiplication launches 65,536 threads. The transpose is fixed at a 32 x 32 block and an 8 x 8 grid in all reports; its padded `float tile[32][33]` uses exactly 4,224 shared bytes per block. This confirms that the transpose did not accidentally inherit the 8 x 8 or 16 x 16 multiplication configuration.

**Global-load traffic follows the expected tiling reduction**

| Tile | Naive warp-level load requests | Tiled requests | Corner requests | Reduction vs naive |
| --- | --- | --- | --- | --- |
| 8 x 8 | 1,048,576 | 131,072 | 131,072 | 8x |
| 16 x 16 | 1,048,576 | 65,536 | 65,536 | 16x |
| 32 x 32 | 1,048,576 | 32,768 | 32,768 | 32x |

For N = 256 and 32-thread warps, the naive kernel makes `2*N^3/32 = 1,048,576` warp-level global-load requests. Shared tiling reduces that count by the tile width. Requests are warp-level instructions, not individual thread loads and not DRAM transactions.

| Tile | Naive load sectors/request | Tiled load sectors/request | Corner load sectors/request |
| --- | --- | --- | --- |
| 8 x 8 | 2.4999 | 3.9906 | 3.9844 |
| 16 x 16 | 2.0000 | 3.9951 | 3.9924 |
| 32 x 32 | 2.5000 | 4.0000 | 4.0000 |

Both shared-memory multiplication variants are close to four 32-byte sectors per warp request, as expected for aligned 32-thread loads of floats. Small departures below four should not be interpreted as better-than-perfect coalescing; different counters can come from different replay passes. The source-correlated ideal-versus-actual sector analysis reports zero excessive global sectors for every kernel.

The naive ratios near 2 or 2.5 do not make naive loading better: its loads include broadcasts and duplicated addresses, and its request count is much larger. In the 16 x 16 mapping a warp spans two output rows; the M load touches two sectors and the N load two sectors. For 8 x 8 the corresponding counts are four plus one; for 32 x 32 they are one plus four. This predicts the aggregate ratios shown above.

**Why corner turning can be slower here**

The ordinary tiled implementation already loads its row-major inputs with efficient access patterns. Repacking M does not reduce the number of global-load requests versus that implementation. Both variants report zero shared-memory load/store bank conflicts and zero source-correlated excessive shared wavefronts. No register-spilling instructions were reported.

For every multiplication tile size, ordinary tiled multiplication executes 655,360 shared-load instructions, including 524,288 `LDS.U` and 131,072 `LDS.U.128` instructions. Corner turning executes 1,048,576 `LDS.U` instructions and no `LDS.U.128`: 60% more shared-load instructions. The measured shared-load wavefront counts are 786,432 versus 1,048,576, a 33.3% increase. Nsight also identifies MIO queue throttling in these kernels. Together these are evidence of additional shared-memory instruction pressure; they support, but do not prove an exclusive cause for, the corner-turning slowdown.

All launches have only 1.6 theoretical waves, and Nsight flags tail effects. The fixed 32 x 32 transpose and the 32 x 32 multiplication grids contain only 64 blocks on a 40-SM GPU. Short workloads and incomplete final waves make resource utilization and timing sensitive to scheduling. Do not explain the 32 x 32 result by assuming lower theoretical occupancy: the profiler’s calculated theoretical occupancy is 100% for these kernels.

**Cache behavior and counters that need care**

| Tile | Naive L1 load hit % | Tiled L1 load hit % | Corner L1 load hit % | Tiled L2 read hit % | Corner L2 read hit % |
| --- | --- | --- | --- | --- | --- |
| 8 x 8 | 83.18 | 39.02 | 36.55 | 99.33 | 99.80 |
| 16 x 16 | 91.32 | 32.33 | 29.13 | 90.66 | 89.36 |
| 32 x 32 | 95.04 | 0.36 | 0.45 | 86.39 | 88.36 |

Naive has more repeated accesses for caches to satisfy. Shared tiling removes many of those accesses, so a lower L1 hit percentage does not imply worse memory use. At 32 x 32 both tiled variants have L1 load hit rates below 0.5%, while their L2 read hit rates remain around 86-88%. Cache hit rates must be interpreted with request counts.

Every kernel reports 8,192 global-store sectors and 8,192 TEX-source L2 write sectors: `8,192*32 = 262,144` bytes, exactly one 256 x 256 float output. Nevertheless `dram__bytes_write.sum` is zero for all actions. This is compatible with the small output remaining dirty in L2 during the kernel’s measurement window. Subsequent host copies can read the cache. Zero DRAM writes is not evidence that output stores are missing. Cache residency is an interpretation consistent with the counts, not direct tracing of later evictions.

DRAM read totals are not equal to source-level requested bytes. They range from about 0.657 to 0.991 MB for multiplication and 0.305 to 0.307 MB for transpose. Source-correlated TEX-to-L2 read misses are 16,384 sectors (512 KiB) for the two multiplication inputs and 8,192 sectors (256 KiB) for transpose, matching the distinct input footprint. Aggregate DRAM totals and some L2 hit/miss totals should not be forced into an exact byte conservation equation across replay passes; system-wide memory activity and profiling precision can contribute. These captures do not isolate the exact origin of every extra DRAM byte.

The 32 x 32 corner kernel reports achieved occupancy 101.5636%. That cannot literally represent more resident warps than the hardware limit. Treat it as an out-of-range derived measurement, not as additional capacity. All captures have 34 replay passes; Nsight documents counter-ratio errors when distributions differ between passes. PM Sampling warnings also state that the interval is too large relative to these short workloads; the transpose lasts only about 3.5 microseconds. Prefer aggregate counters over interpreting sparse sampling timelines.

**Use in the homework**

Use these reports for the memory-access comparison and document the measurement qualifications above. Keep your normal executable round-trip times separate from these per-kernel times. Explain the observed results rather than claiming corner turning must win or that every memory access reaches DRAM. If you want to confirm a specific odd metric, collect a narrowly targeted repeat under the same input and cache policy; there is no evidence here requiring a complete recapture simply to obtain the required comparisons.

**Documentation used to interpret the counters**

- [NVIDIA Python Report Interface](https://docs.nvidia.com/nsight-compute/PythonReportInterface/index.html): reader API.
- [NVIDIA Profiling Guide: Range and Precision](https://docs.nvidia.com/nsight-compute/ProfilingGuide/#range-and-precision): replay and out-of-range counter limitations.
- [NVIDIA Profiling Guide: Cache Control](https://docs.nvidia.com/nsight-compute/ProfilingGuide/#cache-control): default cache flushing between passes.
- [NVIDIA explanation of L2 as a proxy for device memory](https://forums.developer.nvidia.com/t/does-a-kernel-write-back-its-output-data-from-cache-to-global-memory-when-finishing-executing/227542/2): cached stores and later copies.
