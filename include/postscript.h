/*
 *  postscript.h
 *  Backend for generarting postscript output
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#define LOGO2PS_FACTOR  1.0     /* conversion factor between LOGO and postscript */
#define PS_MOVETO_X     200     /* postscript moveto in x coord */
#define PS_MOVETO_Y     200     /* postscript moveto in y coord */

/* Postscript interpretation */
void ps_header(FILE *in_file, char *in_filename);
void ps_footer(FILE *in_file);
void ps_ipt_fd(Logo input, float op);
void ps_ipt_lt(Logo input, float op);
void ps_ipt_rt(Logo input, float op);
