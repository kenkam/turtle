/*
 *  intercept.h
 *  For replacing functions within interpreter.c and parser.c
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

/* for testing purposes */
#ifdef INTERCEPT
#define malloc(x) my_malloc(x)
#include "overrides.h" /* contains my_malloc() */
#endif

#ifdef POSTSCRIPT
#define ipt_header(x,y) ps_header(x,y)
#define ipt_footer(x) ps_footer(x)
#define ipt_fd(x,y) ps_ipt_fd(x,y)
#define ipt_lt(x,y) ps_ipt_lt(x,y)
#define ipt_rt(x,y) ps_ipt_rt(x,y)
#include "postscript.h" /* contains ps_foo() */
#endif

/* for extension. overrides interpreting functions in interpreter.c */
#ifdef GUI
#define ipt_header(x,y)    /* gui does not require headers or footers in the output file */
#define ipt_footer(x)      
#define ipt_fd(x,y) gui_ipt_fd(x,y)
#define ipt_lt(x,y) gui_ipt_lt(x,y)
#define ipt_rt(x,y) gui_ipt_rt(x,y)
#include "extension.h" /* contains gui_foo() */
#endif
