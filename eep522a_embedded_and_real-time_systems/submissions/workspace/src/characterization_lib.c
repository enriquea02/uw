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
    const char *logfile_path,
    const char *testname,
    time_t elapsed_time
)
{
    // open logfile and append new information
    FILE *logfile = fopen(logfile_path, "a");

    // verify file opened correctly
    if (logfile == NULL) {
        fprintf(stderr,"[ERROR] Failed to open logfile: %s: %s\n",logfile_path,strerror(errno));
        return 1;
    }

    // print logfile information
    fprintf(logfile, "[INFO] Completed test: %s in %ld sec",testname,elapsed_time);
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
    clock_t start_time, end_time;
    double elapsed_cpu_time;

    start_time = clock();
    memcpy(dest, src, size * element_size); 
    end_time = clock();

    elapsed_cpu_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    return elapsed_cpu_time;
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

        data_logger(base_path,"ram_write_test.log", byte_write_time);
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

        data_logger(base_path,"ram_write_test.log", halfword_write_time);
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

        data_logger(base_path,"ram_write_test.log", word_write_time);
    }

    // Free memory
    free(src_word); free(dest_word);
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
    ram_write_test(file_path); // Test 1: RAM write speed tests (1KiB, 1MiB, 10MiB, 100MiB for byte, half_word, and word)
}