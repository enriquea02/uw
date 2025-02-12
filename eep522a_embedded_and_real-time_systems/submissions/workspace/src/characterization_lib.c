/*
 * MODULE: 
 *
 * characterization library
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

/* -------------------------------------------------------------------------- */
/*                                   IMPORTS                                  */
/* -------------------------------------------------------------------------- */

/* --------------------------- standard libraries --------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h> // For close, read, write
#include <fcntl.h>  // For open

/* ----------------------------- local libraries ---------------------------- */
#include "../lib/characterization_lib.h"

/* -------------------------------------------------------------------------- */
/*                                 Data Types                                 */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */
char base_path[256];
char file_path[256];

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */
// Function pulls file path variables from UNIX environment variable
int set_base_path(
    const char *path_var_name
)
{
    const char *env_path = getenv(path_var_name);

    if (env_path == NULL) {
        fprintf(stderr, "PATH variable '%s' not found.\n", path_var_name);
        return 1; // Indicate an error
    }

    // Construct the variables' path
    int len = snprintf(base_path, sizeof(base_path), "%s", env_path);

    return 0;
}

// Function captures elapsed test time into data log
int data_logger (
    const char *base_log_path,
    const char *logname,
    const char *testname,
    double elapsed_time
)
{
    char *logfile_path = NULL;

    // Verify inputs are valid
    if (!base_log_path || !logname || !testname) {
        fprintf(stderr, "[ERROR] One or more input strings are NULL.\n");
        return 1;
    }

    if (strlen(base_log_path) == 0 || strlen(logname) == 0 || strlen(testname) == 0) {
        fprintf(stderr, "[ERROR] One or more input strings are empty.\n");
        return 1;
    }

    // Allocate memory for the full path (including testname and extension)
    size_t logfile_path_len = strlen(base_log_path) + strlen(logname) + strlen(".log") + 1; // +1 for null terminator
    logfile_path = malloc(logfile_path_len);

    if (logfile_path == NULL) {
        perror("malloc failed");
        return 1;
    }

    int len = snprintf(logfile_path, logfile_path_len, "%s%s.log", base_log_path, logname);

    if (len < 0) {
        perror("snprintf failed");
        free(logfile_path);
        return 1;
    } else if (len >= logfile_path_len) {
        fprintf(stderr, "[ERROR] Log file path too long (truncated).\n");
        free(logfile_path);
        return 1;
    }

    // open logfile and append new information
    FILE *logfile = fopen(logfile_path, "a");

    // verify file opened correctly
    if (logfile == NULL) {
        fprintf(stderr,"[ERROR] Failed to open logfile: %s: %s\n",logfile_path,strerror(errno));
        return 1;
    }

    // print logfile information
    fprintf(logfile, "[INFO] Completed test: %s in %.9f sec\n",testname,elapsed_time);
    fclose(logfile);
    return 0;
}

// Function to perform the copy and measure time for RAM
double ram_write_time (
    void *src, 
    void *dest,
    size_t size,
    size_t element_size
) 
{
    struct timespec start, end;
    double elapsed_time;
    size_t num_elements = size / element_size;

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &start) == -1) {
        perror("clock_gettime failed");
        return -1.0;
    }

    // loop based on the number of elements {KiB, MiB, 10 MiB, 100 MiB}
    for (size_t i = 0; i < num_elements; i++) {
        memcpy(dest, src, element_size);
        dest = (char *)dest + element_size; // Increment by element size
        src = (char *)src + element_size; // Increment src as well
    }

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &end) == -1) {
        perror("clock_gettime failed");
        return -1.0;
    }

    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    return elapsed_time;
}

// Function to copy a file and measure time for File System
double file_sys_time(
    const char *src_filename, 
    const char *dest_filename, 
    size_t size, 
    size_t element_size
) 
{
    struct timespec start, end;
    double elapsed_time;
    size_t num_elements = size / element_size;

    int src_fd, dest_fd;
    void *buffer;

    // Allocate buffer (important to allocate enough for the largest size)
    buffer = malloc(size * element_size);
    if (buffer == NULL) {
        perror("File System Time Creation: Memory allocation failed");
        return -1.0;
    }

    // Verify that source file can be opened. File will be set to read-only
    src_fd = open(src_filename, O_RDONLY);
    if (src_fd == -1) {
        perror("Error opening source file");
        free(buffer);
        return -1.0;
    }

    // Verify that destination file can be opened. Setting write only privilege, file creation privilege, and truncating the file back to zero if it already exists
    dest_fd = open(dest_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Create or truncate dest
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(src_fd);
        free(buffer);
        return -1.0;
    }

    // start clock
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &start) == -1) {
        perror("clock_gettime failed");
        return -1.0;
    }

    // Read and write in chunks
    size_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, size * element_size)) > 0) {
      if (write(dest_fd, buffer, bytes_read) != bytes_read) {
        perror("Write error");
        close(src_fd);
        close(dest_fd);
        free(buffer);
        return -1.0;
      }
    }

    // end clock
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &end) == -1) {
        perror("clock_gettime failed");
        return -1.0;
    }

    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    close(src_fd);
    close(dest_fd);
    free(buffer);
    return elapsed_time;
}

// Helper function to create dummy files
int create_dummy_file(
    const char *filename,
    size_t total_size,
    size_t element_size
)
{
    // Verify that dummy file can be opened. Setting write only privilege, file creation privilege, and truncating the file back to zero if it already exists
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error creating dummy file");
        return 1;
    }

    // Use a smaller buffer size to avoid memory exhaustion
    size_t buffer_size = 32 * MiB; // Adjust as needed, but keep it reasonable
    void *buffer = malloc(buffer_size * element_size);
    if (buffer == NULL) {
        perror("Memory allocation failed for buffer");
        close(fd);
        return 1;
    }

    // Write in chunks
    for (size_t i = 0; i < total_size / (buffer_size * element_size); i++) {
        size_t chunk_size = buffer_size;
        if ((i + 1) * buffer_size * element_size > total_size) {
            chunk_size = (total_size - i * buffer_size * element_size) / element_size;
        }

        // Fill the buffer with some data (important for realistic timings)
        for (size_t j = 0; j < chunk_size; j++) {
            memset(buffer + j * element_size, (i * buffer_size + j) % 256, element_size); // Varying bytes
        }

        ssize_t bytes_written = write(fd, buffer, chunk_size * element_size);
        if (bytes_written == -1) {
            perror("Error writing to dummy file");
            free(buffer);
            close(fd);
            return 1;
        }
        if (bytes_written != chunk_size * element_size) {
          fprintf(stderr, "Write incomplete. Expected %zu, wrote %zd\n", chunk_size * element_size, bytes_written);
          free(buffer);
          close(fd);
          return 1;
        }

    }

    close(fd);
    free(buffer);
    return 0;
}

// Function runs all RAM write tests
int ram_write_test (
    char *base_path
)
{
    byte *src_byte = NULL;
    byte *dest_byte = NULL;
    halfword *src_halfword = NULL;
    halfword *dest_halfword = NULL;
    word *src_word = NULL;
    word *dest_word = NULL;

    size_t mem_size[] = {KiB, MiB, TEN_MiB, HUNDRED_MiB};
    const char *mem_size_str[] = {"1 KiB","1 MiB","10 MiB","100 MiB"};

    /* -------------------------------- Byte Test ------------------------------- */
    // Allocate memory
    src_byte = malloc(HUNDRED_MiB);
    dest_byte = malloc(HUNDRED_MiB);

    if (!src_byte || !dest_byte) 
    {
        perror("Memory byte allocation failed");
        return 1;
    }
    
    // Run RAM write test per memory size test for byte accesses (KiB, MiB, TEN_MiB, HUNDRED_MiB)
    for (int i = 0; i < 4; i++) 
    {
        double byte_write_time = ram_write_time(src_byte,dest_byte,mem_size[i],sizeof(byte));

        // Construct the test name string using snprintf
        char testname[256]; // Adjust size as needed
        snprintf(testname, sizeof(testname), "RAM Byte %s Access Test", mem_size_str[i]);

        data_logger(base_path,"ram_write_test",testname, byte_write_time);
    }

    // Free memory
    free(src_byte); free(dest_byte);

    /* ------------------------------ Halfword Test ----------------------------- */
    // Allocate memory
    src_halfword = malloc(HUNDRED_MiB * sizeof(halfword));
    dest_halfword = malloc(HUNDRED_MiB * sizeof(halfword));

    if (!src_halfword || !dest_halfword) {
        perror("Memory halfword allocation failed");
        return 1;
    }

    // Run RAM write test per memory size test for half-word accesses (KiB, MiB, TEN_MiB, HUNDRED_MiB)
    for (int i = 0; i < 4; i++) 
    {
        double halfword_write_time = ram_write_time(src_halfword,dest_halfword,mem_size[i],sizeof(halfword));

        // Construct the test name string using snprintf
        char testname[256]; // Adjust size as needed
        snprintf(testname, sizeof(testname), "RAM Halfword %s Access Test", mem_size_str[i]);

        data_logger(base_path,"ram_write_test",testname, halfword_write_time);
    }

    // Free memory
    free(src_halfword); free(dest_halfword);

    /* -------------------------------- Word Test ------------------------------- */
    // Allocate memory
    src_word = malloc(HUNDRED_MiB * sizeof(word));
    dest_word = malloc(HUNDRED_MiB * sizeof(word));

    if (!src_word || !dest_word) {
        perror("Memory word allocation failed");
        return 1;
    }

    // Run RAM write test per memory size test for word accesses (KiB, MiB, TEN_MiB, HUNDRED_MiB)
    for (int i = 0; i < 4; i++) 
    {
        double word_write_time = ram_write_time(src_word,dest_word,mem_size[i],sizeof(word));

        // Construct the test name string using snprintf
        char testname[256]; // Adjust size as needed
        snprintf(testname, sizeof(testname), "RAM Word %s Access Test", mem_size_str[i]);

        data_logger(base_path,"ram_write_test",testname, word_write_time);
    }

    // Free memory
    free(src_word); free(dest_word);
}

// Function runs all File System write tests
int file_sys_test (
    char *base_path
)
{
    size_t mem_size[] = {KiB, MiB, TEN_MiB, HUNDRED_MiB, GiB};
    const char *mem_size_str[] = {"1 KiB","1 MiB","10 MiB","100 MiB","1 GiB"};

    if(create_dummy_file("test_byte.dat", TEN_MiB, sizeof(byte)) != 0) return 1;
    if(create_dummy_file("test_halfword.dat", TEN_MiB, sizeof(halfword)) != 0) return 1;
    if(create_dummy_file("test_word.dat", TEN_MiB, sizeof(word)) != 0) return 1;

    /* -------------------------------- Byte Test ------------------------------- */
    // Run File System write test per memory size test for byte accesses (KiB, MiB, TEN_MiB, HUNDRED_MiB, GiB)
    for (int i = 0; i < 3; i++) 
    {
        double byte_write_time = file_sys_time("test_byte.dat", "copy_byte.dat", mem_size[i], sizeof(byte));

        // Construct the test name string using snprintf
        char testname[256]; // Adjust size as needed
        snprintf(testname, sizeof(testname), "File System Byte %s Access Test", mem_size_str[i]);

        data_logger(base_path,"file_sys_write_test",testname, byte_write_time);
    }

    /* ------------------------------ Halfword Test ----------------------------- */
    // Run File System write test per memory size test for half-word accesses (KiB, MiB, TEN_MiB, HUNDRED_MiB, GiB)
    for (int i = 0; i < 3; i++) 
    {
        double halfword_write_time = file_sys_time("test_halfword.dat", "copy_halfword.dat", mem_size[i], sizeof(halfword));

        // Construct the test name string using snprintf
        char testname[256]; // Adjust size as needed
        snprintf(testname, sizeof(testname), "File System Halfword %s Access Test", mem_size_str[i]);

        data_logger(base_path,"file_sys_write_test",testname, halfword_write_time);
    }

    /* -------------------------------- Word Test ------------------------------- */
    // Run File System write test per memory size test for word accesses (KiB, MiB, TEN_MiB, HUNDRED_MiB, GiB)
    for (int i = 0; i < 3; i++) 
    {
        double word_write_time = file_sys_time("test_word.dat", "copy_word.dat", mem_size[i], sizeof(word));

        // Construct the test name string using snprintf
        char testname[256]; // Adjust size as needed
        snprintf(testname, sizeof(testname), "File System Word %s Access Test", mem_size_str[i]);

        data_logger(base_path,"file_sys_write_test",testname, word_write_time);
    }
}

int main()  
{
    // Pull logfile path from OS
    if (set_base_path("LOGFILE_PATH") != 0) // Remember to set LOGFILE_PATH in Raspbian .bash_profile!
    {
        fprintf(stderr, "Failed to set base path.\n");
        return 1; // Exit with error
    }
    snprintf(file_path,sizeof(file_path),"%s",base_path);

    // characterization tests
    ram_write_test(file_path);  // Test 1: RAM write speed tests (1KiB, 1MiB, 10MiB, 100MiB for byte, half_word, and word)
    file_sys_test(file_path);   // Test 2: File system read/write tests (1KiB, 1MiB, 10MiB, 100MiB, 1GiB for byte, half_word, and word)
}