/*
 *  postscript.c
 *  Backend for generarting postscript output
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <stdio.h>
#include <interpreter.h>
#include "postscript.h"

/********************************************
 Interpreting Functions
 ********************************************/

/**
 *  Postscript headers for output file before interpreting 
 */
void ps_header(FILE *out_file, char *in_filename) {
    /* postscript header */
    fprintf(out_file, "%%!PS-%s\n", in_filename);
    fprintf(out_file, "newpath\n");
    fprintf(out_file, "%d %d moveto\n", PS_MOVETO_X, PS_MOVETO_Y);
}

/**
 *  Postscript footers for output file after interpreting
 */
void ps_footer(FILE *out_file) {
    fprintf(out_file, "stroke\n");
}

/**
 *  Postscript implementation for mapping fd() to rlineto
 */
void ps_ipt_fd(Logo input, float op) {
    fprintf(input->ofile, "%.2f 0 rlineto\n", op * LOGO2PS_FACTOR);
}

/**
 *  Postscript implementation for mapping lt() to rotate
 */
void ps_ipt_lt(Logo input, float op) {
    fprintf(input->ofile, "%.2f rotate\n", op);
}

/**
 *  Postscript implementation for mapping rt() to rotate
 */
void ps_ipt_rt(Logo input, float op) {
    fprintf(input->ofile, "-%.2f rotate\n", op);
}
