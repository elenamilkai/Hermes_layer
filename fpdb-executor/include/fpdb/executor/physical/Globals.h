//
// Created by Yifei Yang on 11/17/21.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_GLOBALS_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_GLOBALS_H

namespace fpdb::executor::physical {

/**
 * Default number of tuples that operators should buffer before sending to consumers.
 * Default number of bytes for S3 conversion.
 * Default number of bytes when doing a S3 range scan.
 */
inline constexpr int DefaultBufferSize = 10000;
inline constexpr int DefaultS3ConversionBufferSize = 128 * 1024;
// FIXME: temporary fix of "parseChunkSize < payload size" issue on Airmettle Select
inline constexpr int DefaultS3ConversionBufferSizeAirmettleSelect = 16 * 1024 * 1024;
inline constexpr uint64_t DefaultS3RangeSize = 15 * 1024 * 1024; // 15MB/s This value was tuned on c5n.9xlarge and
// may need to be retuned for different instances with many more cores

/**
 * Parameters used in WLFU, with csv_150MB/ and 200 parallel reqs
 */
// c5a.8x
inline constexpr double vNetwork = 1.16320;     // unit: GB/s
inline constexpr double vS3Scan = 18.00891;     // unit: GB/s
inline constexpr double vS3Filter = 0.32719;    // unit: GPred/s

/**
 * These parameters are for running GET in parallel as a detached operation.
 * We only want to convert ~max cores results at a time since otherwise we get very bad cache thrashing
 * that degrades system performance. Additionally setting a variable sleep retry interval appears to make parallel GET
 * requests perform much faster than using a fixed interval.
 */
inline constexpr int maxConcurrentArrowConversions = 36; // Set to ~#cores
inline constexpr int minimumSleepRetryTimeMS = 5;
inline constexpr int variableSleepRetryTimeMS = 15;

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_GLOBALS_H
