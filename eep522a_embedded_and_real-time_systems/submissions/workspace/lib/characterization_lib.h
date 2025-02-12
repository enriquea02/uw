
/*
 * MODULE: 
 *
 * characterization library header
 *
 * DESCRIPTION: 
 * This library contains functions that are useful for Raspberry Pi 4 device characterization.
 * - RAM write speed tests (1KiB, 1MiB, 10MiB, 100MiB for byte, half_word, and word)
 * - Hard Disk Write Speed Tests
 * 
 * ACKNOWLEDGEMENTS:
 * This library borrows from Andrew N. Sloss' prototype.c module developed for EP522A Embedded and Real-Time Systems course
 * 
 */

#ifndef _CHARACTERIZATION_H_
#define _CHARACTERIZATION_H_

/* -------------------------------------------------------------------------- */
/*                                   IMPORTS                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                 Data Types                                 */
/* -------------------------------------------------------------------------- */
typedef uint8_t byte;
typedef uint16_t halfword;
typedef uint32_t word;

/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  CONSTANTS                                 */
/* -------------------------------------------------------------------------- */

/* -------------------- Read / Write Speed Test Constants ------------------- */
const int KiB           = 1024;
const int MiB           = (1024*KiB);
const int TEN_MiB       = (10*MiB);
const int HUNDRED_MiB   = (100*MiB);
const int GiB           = (1024*MiB);

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */
// Function pulls file path variables from UNIX environment variable
int set_base_path(
    const char *path_var_name
);

// Function captures elapsed test time into data log
int data_logger (
    const char *base_log_path,
    const char *logname,
    const char *testname,
    double elapsed_time
);

// Function to perform the copy and measure time for RAM
double ram_write_time (
    void *src, 
    void *dest,
    size_t size,
    size_t element_size
);

// Function to copy a file and measure time for File System
double file_sys_time(
    const char *src_filename, 
    const char *dest_filename, 
    size_t size, 
    size_t element_size
);

// Helper function to create dummy files
int create_dummy_file(
    const char *filename,
    size_t total_size,
    size_t element_size
);

// Function runs all RAM write tests
int ram_write_test (
    char *base_path
);

// Function runs all File System write tests
int file_sys_test (
    char *base_path
);

int main();

#endif