/*
 * MODULE: 
 *
 * characterization library
 *
 * DESCRIPTION: 
 * This library contains functions that are useful for Raspberry Pi 4 device characterization.
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

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */
// Function pulls file path variables from UNIX environment variable
int set_base_path(
    const char *path_var_name
)
{
    const char *base_path = getenv(path_var_name);
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
        fprintf(stderr,"[ERROR] Failed to open logfile: %s: %s\n",logfile,strerror(errno));
        return 1;
    }

    // print logfile information
    fprintf(logfile, "[INFO] Completed test: ","%s","in %ld sec",testname,elapsed_time);
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

// Function summarizes all RAM write tests
void ram_write_test ()
{
    byte *src_byte, *dest_byte;
    halfword *src_halfword, *dest_halfword;
    word *src_word, *dest_word;

    size_t mem_size[] = {KiB, MiB, TEN_MiB, HUNDRED_MiB}
    const char *size_names[] = 
    test_byte

    // Byte Test
    src_byte = malloc(HUNDRED_MiB); // Allocate memory
    dest_byte = malloc(HUNDRED_MiB);

    if (!src_byte || !dest_byte) {
        perror("Memory byte allocation failed");
        return 1;
    }
    
    for(int i; i < 4; i++) {
        double byte_write_time = ram_write_time(src_byte,dest_byte,mem_size[i],sizeof(byte));
    }


    free(src_byte); free(dest_byte);    // Free memory

    // Halfword Test
    src_halfword = malloc(HUNDRED_MiB * sizeof(halfword)); // Allocate memory
    dest_halfword = malloc(HUNDRED_MiB * sizeof(halfword));

    if (!src_halfword || !dest_halfword) {
        perror("Memory halfword allocation failed");
        return 1;
    }

    free(src_halfword); free(dest_halfword);    // Free memory

    // Word Test
    src_word = malloc(HUNDRED_MiB * sizeof(word));  // Allocate memory
    dest_word = malloc(HUNDRED_MiB * sizeof(word));

    if (!src_word || !dest_word) {
        perror("Memory word allocation failed");
        return 1;
    }

    free(src_word); free(dest_word);    // Free memory
}

int main()  
{
    set_base_path("LOGFILE_PATH");  // Set LOGFILE_PATH in Raspbian .bash_profile

    // Test 1: RAM write (1KiB, 1MiB, 10MiB, 100MiB for byte, half_word, and word)
    start_time = 
    data_logger(base_path,"one_kib_ram_test",elapsed_time)
    snprintf(base_path,sizeof(file_path),"Data Log File Location: ","%s",file_path);
}