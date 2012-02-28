/*
 *  overrides.c
 *  Overrides system functions for testing
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "overrides.h"

int _fail_malloc = 0;
int _fail_next = 0;

/*
 *  This malloc will fail depending on the extern int _fail_malloc
 */
void * my_malloc (size_t size) {
    if (_fail_malloc == 1) {
        return NULL;
    }
    /* this will fail on _fail_next next calls to malloc. this is useful when you want
       the malloc to fail for a number of times to test all malloc funcs in
       a particular function. (scan_file() for example */
    if (_fail_next-- == 1) {
        _fail_malloc = 1;
    }
    return malloc(size);
}

void malloc_fail(void) {
    _fail_malloc = 1;
}

void malloc_ok(void) {
    _fail_malloc = 0;
}

void malloc_fail_next(int num) {
    _fail_next = num;
}
