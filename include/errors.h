/*
 *  errors.h
 *  Error codes
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#define PARSE_ERR -1    /* parsing errors */
#define MEM_ERR   -2    /* out ot memory errors */
#define VAR_ERR   -3    /* unknown variable (for interpreter.c) */
#define STACK_ERR -4    /* illegal operations with stack (for interpreter.c) */
#define ARGS_ERR  -5    /* error on arguments provided for executable */
#define POL_ERR   -6    /* error on the polish expression (for interpreter.c) */
