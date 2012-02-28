/*
 *  interp.c
 *  A postscript interpreter for the LOGO language
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <ctype.h> /* for isdigit */
#include <stdio.h>
#include <stdlib.h> /* malloc and EXIT_FOO */
#include <string.h> /* strcmp, strcpy, etc */
#include "interpreter.h"

/********************************************
 MAIN
 ********************************************/

int main(int argc, char * argv[]) {
    /* short and sweet */
    return interp_main(argc, argv);
}
