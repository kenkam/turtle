/*
 *  test_psr_malloc.c
 *  Tests return value of malloc for parser.c
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "minunit.h"
#include "overrides.h"

#define TEST_FILE1      "data/testdata1.txt"    /* file used to test scan_file() */

/* used by minunit.h */
int tests_run = 0;

static char * test_scan_file() {
    Logo input;
    FILE *file;
    printf("Testing %s\n", __FUNCTION__);
    
    file = fopen(TEST_FILE1, "r");
    mu_assert("error, cannot open test file\n", file != NULL);
    /* turn off malloc */
    malloc_fail();
    input = scan_file(file);
    /* this should be NULL */
    mu_assert("error, input != NULL", input == NULL);
    /* turn it back on to test the 2nd malloc in parser.c */
    malloc_ok();
    malloc_fail_next(1);
    rewind(file);
    input = scan_file(file);
    mu_assert("error, input != NULL", input == NULL);
    /* turn it back on to test the 3rd+ malloc in parser.c */
    malloc_ok();
    malloc_fail_next(2);
    rewind(file);
    input = scan_file(file);
    mu_assert("error, input != NULL", input == NULL);
    fclose(file);
    free(input);
    return 0;
}

static char * all_tests() {
    mu_run_test(test_scan_file);
    return 0;
}

/**
 *  Boilerplate for minunit.h
 */
int main(int argc, const char * argv[]) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
        return 1;
    }
    else {
        printf("All tests passed\n");
    }
    printf("Tests run: %d\n", tests_run);
    return 0;
}
