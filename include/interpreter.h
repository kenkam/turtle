/*
 *  interpreter.h
 *  Essentially the same as parser.h
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include "errors.h"   /* error codes */

/* interpreter related */
#define VARARY_SIZE     26      /* array size for holding variables A-Z */
/* parsing related */
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
#define NUM_ARGS        3       /* num args argc should have */

#define strsame(A,B) (strcmp(A, B)==0)

/* data structure to store variables */
struct _varstack {
    float data;
    int used;
};
typedef struct _varstack * VarStack;

/* internal data structure for passing input file data, output file handler etc */
struct logo {
    char **lines;
    int num_lines;
    int counter;
    FILE *ofile;    /* for fprintf */
    VarStack vars;  /* for SET and VAR */
};
typedef struct logo * Logo;

/* stack data structure for polish */
struct _stack {
    float data;
    struct _stack * next;
};
typedef struct _stack * Stack;

/* main function of interp */
int interp_main(int argc, char * argv[]);

/* Parser Functions */
int parse(Logo input);
int mainlogo(Logo input);
int instrctlst(Logo input);
int instruction(Logo input);
int fd(Logo input);
int lt(Logo input);
int rt(Logo input);
int dologo(Logo input);
int varnum(char * operand, Logo input, float * op);
int set(Logo input);
int polish(char * po, Logo input, Stack * head);
int operate(int oper, float a, float b, float * output);

/* Parser Helper Functions */
int get_var(char * var, VarStack vars, float * output);
int set_var(char * var, VarStack vars, float num);
int is_var(char * var);
int is_op(char * op);
char * trim_space(char *str);

/* Input File Handling */
Logo scan_file(FILE * in_file);
void free_logo(Logo input);
void free_stack(Stack stack);

/* stack functions */
int push(Stack *head, float value);
int pop(Stack *head, float * value);

/* Command line functions */
int get_filenames(int argc, char * argv[], char *input, char *output);
