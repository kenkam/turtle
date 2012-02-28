/*
 *  parser.h
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include "errors.h"   /* error codes */

#define FILENAME_LENGTH 128     /* input filename length */
#define LINE_LENGTH     128     /* maximum input line length */
#define INSTRUCT_LENGTH 3       /* max string length of instruction */
#define OPERAND_LENGTH  10      /* max string length for operands */
#define VAR_LENGTH      1       /* length of <VAR> */
#define FD              "FD"    /* string of <FD> instruction */
#define LT              "LT"    /* string of <LT> instruction */
#define RT              "RT"    /* string of <RT> instruction */
#define SET             "SET"   /* string of <SET> instruction */
#define DO              "DO"    /* string of <DO> instruction */
#define NUM_ARGS        2       /* expecting 2 arguments */ 

#define strsame(A,B) (strcmp(A, B)==0)

struct logo {
    char **lines;
    int num_lines;
    int counter;
};
typedef struct logo * Logo;

/* main function of parse */
int parse_main(int argc, char * argv[]);

/* Parser Functions */
int parse(Logo input);
int mainlogo(Logo input);
int instrctlst(Logo input);
int instruction(Logo input);
int fd(Logo input);
int lt(Logo input);
int rt(Logo input);
int dologo(Logo input);
int varnum(char * operand, Logo input);
int set(Logo input);
int polish(char * po, Logo input);

/* Parser Helper Functions */
int is_var(char * var);
int is_op(char * op);
char * trim_space(char *str);

/* Input File Handling */
Logo scan_file(FILE * in_file);
void free_logo(Logo input);

/* Command line functions */
int get_filename(int argc, char * argv[], char *filename);
