/*
 *  overrides.h
 *  Overrides system functions for testing
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

void * my_malloc (size_t size);
void malloc_fail(void);
void malloc_ok(void);
void malloc_fail_next(int num);
