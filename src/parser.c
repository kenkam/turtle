/*
 *  parser.c
 *  Functions for parsing the LOGO language
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <ctype.h> /* for isdigit */
#include <stdio.h>
#include <stdlib.h> /* malloc and EXIT_FOO */
#include <string.h> /* strcmp, strcpy, etc */
#include "parser.h"
#include "intercept.h" /* for intercepting malloc and testing */

#define DEBUG_DATA  input->lines[input->counter], input->counter+1

/********************************************
 Parser Main Function
 ********************************************/

int parse_main(int argc, char * argv[]) {
    char filename[FILENAME_LENGTH];
    FILE *in_file; /* input file handle */
    Logo input;    /* data structure to store input lines */ 
    
    /* get and open the file */
    if (get_filename(argc, argv, filename) < 0) {
        return EXIT_FAILURE;
    }
    
    /* open input file */
    in_file = fopen(filename, "r");
    if (in_file == NULL) {
        fprintf(stderr, "Error: failed to open %s\n", filename);
        perror("fopen");
        return EXIT_FAILURE;
    }
    
    /* populate input */
    input = scan_file(in_file);
    fclose(in_file);
    if (input == NULL) {
        /* out of memory */
        fprintf(stderr, "Error: cannot allocate memory for reading input file\n");
        return EXIT_FAILURE;
    }
    
    if (parse(input) < 0) {
        fprintf(stderr, "Error: failed to parse %s\n", filename);
        /* free the input structure */
        free_logo(input);
        return EXIT_FAILURE;
    }
    printf("Successfully parsed %s\n", filename);

    /* free the input structure */
    free_logo(input);
    return EXIT_SUCCESS;
}

/********************************************
 Static Functions
 ********************************************/

/**
 *  Checks line for <INSTRUCTION> <VARNUM> for <FT>, <LT> and <RT>
 */
static int chk_inst(Logo input) {
    char inst[INSTRUCT_LENGTH+1]; 
    char operand[OPERAND_LENGTH+1];
    
    if (sscanf(input->lines[input->counter], "%s %s", inst, operand) != 2) {
        fprintf(stderr, "Error: expected <INSTRUCTION> <VARNUM> but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    /* check the given varnum is either a number or a var */
    if (varnum(operand, input) < 0) {
        return PARSE_ERR;
    }
    return 0;
}

/********************************************
 Parser Functions
 ********************************************/

/**
 *  Parses the input and returns 0 on success, PARSE_ERR on error
 */
int parse(Logo input) {
    if (mainlogo(input) < 0) {
        /* something went wrong */
        return PARSE_ERR;
    }
    return 0;
}

/**
 *  Parses <MAIN> and returns 0 on success, PARSE_ERR on error
 *  <MAIN>      ::= "{" <INSTRCTLST>
 */
int mainlogo(Logo input) {
    int i;
    /* starts with a curly bracket */
    if (strsame(input->lines[input->counter], "{") != 1) {
        /* does not start with a curly bracket */
        fprintf(stderr, "Error: expected '{' but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    /* pass it on to instrctlst now */
    input->counter = input->counter + 1;
    if (instrctlst(input) < 0) {
        /* something went wrong */
        return PARSE_ERR;
    }
    input->counter = input->counter + 1;
    /* check that everything that follows is empty */
    for (i=input->counter; i<input->num_lines; i++) {
        /* these lines must now be empty */
        if (strlen(input->lines[i]) > 0) {
            fprintf(stderr, "Error: %s found after closing bracket on line %d\n", DEBUG_DATA);
            return PARSE_ERR;
        }
    }
    return 0;
}

/**
 *  Parses <INSTRCTLST> and returns 0 on success, PARSE_ERR on error
 *  <INSTRCTLST>      ::= <INSTRUCTION><INSTRCTLST> | "}"
 */
int instrctlst(Logo input) {
    /* is it the curly bracket? */
    if (strsame(input->lines[input->counter], "}")) {
        return 0;
    }
    if (instruction(input) < 0) {
        return PARSE_ERR;
    }
    input->counter = input->counter + 1;
    if (input->counter > input->num_lines-1) {
        fprintf(stderr, "Error: expected '}' on line %d\n", input->counter+1);
        return PARSE_ERR;
    }
    return instrctlst(input);
}

/**
 *  Parses <INSTRUCTION> and returns 0 on success, PARSE_ERR on error
 *  <INSTRUCTION>      ::= <FD> |
 *                         <LT> |
 *                         <RT> |
 *                         <DO> |
 *                         <SET>
 */
int instruction(Logo input) {
    char inst[INSTRUCT_LENGTH+1]; /* +1 for null character */
    int ret; /* return value */
    /* the string can be FD, LT, or RT, SET, or DO */
    if (sscanf(input->lines[input->counter], "%s", inst) != 1) {
        /* empty string, bail */
        fprintf(stderr, "Error: expected <INSTRUCTION> but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    /* what is the instruction? */
    if (strsame(inst, FD)) {
        ret = fd(input);
    } else if (strsame(inst, LT)) {
        ret = lt(input);
    } else if (strsame(inst, RT)) {
        ret = rt(input);
    } else if (strsame(inst, SET)) {
        ret = set(input);
    } else if (strsame(inst, DO)) {
        ret = dologo(input);
    } else {
        /* no valid instruction, give error and bail */
        fprintf(stderr, "Error: expected <INSTRUCTION> but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    return ret;
}

/**
 *  Parses <FD> and returns 0 on success, PARSE_ERR on error
 *  <FD>        ::= FD <VARNUM>
 */
int fd(Logo input) {
    char inst[INSTRUCT_LENGTH+1];
    if (chk_inst(input) < 0) {
        return PARSE_ERR;
    }
    /* check the instruction again for completeness sake, even though 
       instruction() has checked for it once
     */
    sscanf(input->lines[input->counter], "%s", inst);
    if (strsame(inst, FD) != 1) {
        fprintf(stderr, "Error: expected '%s' but got '%s' on line %d\n", FD, DEBUG_DATA);
        return PARSE_ERR;
    }
    return 0;
}

/**
 *  Parses <LT> and returns 0 on success, PARSE_ERR on error
 *  <LT>        ::= LT <VARNUM>
 */
int lt(Logo input) {
    char inst[INSTRUCT_LENGTH+1];
    if (chk_inst(input) < 0) {
        return PARSE_ERR;
    }
    sscanf(input->lines[input->counter], "%s", inst);
    if (strsame(inst, LT) != 1) {
        fprintf(stderr, "Error: expected '%s' but got '%s' on line %d\n", LT, DEBUG_DATA);
        return PARSE_ERR;
    }
    return 0;
}

/**
 *  Parses <RT> and returns 0 on success, PARSE_ERR on error
 *  <RT>        ::= RT <VARNUM>
 */
int rt(Logo input) {
    char inst[INSTRUCT_LENGTH+1];
    if (chk_inst(input) < 0) {
        return PARSE_ERR;
    }
    sscanf(input->lines[input->counter], "%s", inst);
    if (strsame(inst, RT) != 1) {
        fprintf(stderr, "Error: expected '%s' but got '%s' on line %d\n", RT, DEBUG_DATA);
        return PARSE_ERR;
    }
    return 0;
}

/**
 *  Parses <DO> and returns 0 on success, PARSE_ERR on error
 *  <DO>        ::= <VAR> "FROM" <VARNUM> "TO" <VARNUM> { <INSTRCTLST>
 */
int dologo(Logo input) {
    char inst[INSTRUCT_LENGTH+1], fromchar[OPERAND_LENGTH], tochar[OPERAND_LENGTH], curly[VAR_LENGTH+1];
    char var[VAR_LENGTH+1], from[LINE_LENGTH], to[LINE_LENGTH];
    /* no need to check DO as it has already been checked in instruciton */
    if (sscanf(input->lines[input->counter], "%s %s %s %s %s %s %[{]", inst, var, fromchar, from, tochar, to, curly) < 7) {
        fprintf(stderr, "Error: expected DO <VAR> FROM <VARNUM> TO <VARNUM> { but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    /* check the <VAR> token */
    if (is_var(var) != 1) {
        fprintf(stderr, "Error: incorrect <VAR> '%s' on line %d\n", var, input->counter+1);
        return PARSE_ERR;
    }
    /* check syntax for DO, FROM and TO */
    if (strsame(inst, DO) != 1 ||
        strsame(fromchar, "FROM") != 1 ||
        strsame(tochar, "TO") != 1) {
        fprintf(stderr, "Error: expected DO <VAR> FROM <VARNUM> TO <VARNUM> { but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    /* check the 2 varnums */
    if (varnum(from, input)) {
        return PARSE_ERR;
    }
    if (varnum(to, input)) {
        return PARSE_ERR;
    }
    /* next line, and hand over to instrctlst */
    input->counter = input->counter + 1;
    return instrctlst(input);
}

/**
 *  Parses <VARNUM> and returns 0 on success, PARSE_ERR on error
 *  <VARNUM>    ::= [0-9]+ | <VAR>
 */
int varnum(char * operand, Logo input) {
    int i;
    
    /* it could be a <VAR> */
    if (is_var(operand)) {
        return 0;
    } else {
        /* check operand is [-0-9.] */
        for (i=0; i<strlen(operand); i++) {
            if (isdigit(operand[i]) < 1 &&
                operand[i] != '.' &&
                operand[i] != '-') {
                fprintf(stderr, "Error: expected number or a <VAR> but got '%s' on line %d\n", operand, input->counter+1);
                return PARSE_ERR;
            }
        }
    }
    return 0;
}

/**
 *  Parses <SET> and returns 0 on success, PARSE_ERR on error
 *  <SET>       ::= SET <VAR> ":=" <POLISH>
 */
int set(Logo input) {
    char inst[INSTRUCT_LENGTH+1], var[VAR_LENGTH+1], sink[LINE_LENGTH], equal[OPERAND_LENGTH];
    char *po;
    if (sscanf(input->lines[input->counter], "%s %s %s %s", inst, var, equal, sink) < 4) {
        fprintf(stderr, "Error: expected SET <VAR> := <POLISH> but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    /* check that var is correct */
    if (is_var(var) != 1) {
        fprintf(stderr, "Error: incorrect <VAR> '%s' on line %d\n", var, input->counter+1);
        return PARSE_ERR;
    }
    /* check the syntax */
    if (strsame(inst, SET) != 1 ||
        strsame(equal, ":=") != 1) {
        fprintf(stderr, "Error: expected SET <VAR> := <POLISH> but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    /* capture the entire polish expression as po now only contains the first
       bit of the polish because %s will stop on whitespace. Plus 2 because
       the polish expression starts 2 char after a equal sign
    */
    po = input->lines[input->counter];
    /* move the po pointer to 2 chars after '=', ie. the start of the polish */
    po = po + strcspn(input->lines[input->counter], "=") + 2;
    /* check the polish expression */
    return polish(po, input);
}

/**
 *  Parses <POLISH> and returns 0 on success, PARSE_ERR on error
 *  <POLISH>    ::= <OP> <POLISH> | <VARNUM> <POLISH> | ;
 */
int polish(char * po, Logo input) {
    char tok[OPERAND_LENGTH];
    if (strsame(po, ";")) {
        /* base case, bye bye polish */
        return 0;
    }
    /* copy the token up to a whitespace and give it a null char */
    strncpy(tok, po, strcspn(po, " "));
    tok[strcspn(po, " ")] = '\0';
    if (is_op(tok) != 1) {
        if (varnum(tok, input) != 0) {
            return PARSE_ERR;
        }
    }
    /* fast forward the pointer to the next bit */
    /* is there a space left? */
    if (strcspn(po, " ") == strlen(po)) {
        /* end of polish expression but no ';' */
        fprintf(stderr, "Error: expected ; to end <POLISH> but got '%s' on line %d\n", DEBUG_DATA);
        return PARSE_ERR;
    }
    po = po + strcspn(po, " ") + 1;
    return polish(po, input);
}

/********************************************
 Parser Helper Functions
 ********************************************/

/**
 *  Returns 1 if the string var is a correct <VAR> ([A-Z]), else 0
 */
int is_var(char * var) {
    if (strlen(var) == 1 &&
        isupper(var[0])) { /* match [A-Z] */
        return 1;
    } 
    return 0;
}

/**
 *  Returns 1 if the string op == +, -, *, or /, else 0
 */
int is_op(char * op) {
    if (strsame(op, "+")) {
        return 1;
    } else if (strsame(op, "-")) {
        return 1;
    } else if (strsame(op, "*")) {
        return 1;
    } else if (strsame(op, "/")) {
        return 1;
    }
    return 0;
}

/**
 *  Removes leading and trailing whitespace from a string
 */
char * trim_space(char *str) {
    char *end;
    /* skip leading whitespace */
    while (isspace(*str)) {
        str = str + 1;
    }
    /* remove trailing whitespace */
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end = end - 1;
    }
    /* write null character */
    *(end+1) = '\0';
    return str;
}

/********************************************
 Input File Handling
 ********************************************/

/**
 *  Scans the input file and creates a data structure to store its content
 */
Logo scan_file(FILE * in_file) {
    int i, count;
    char line[LINE_LENGTH];
    char *trimmed;
    Logo input;
    
    /* count the number of lines */
    count = 0;
    while (fgets(line, sizeof(line), in_file) != NULL) {
        count = count + 1;
    }
    count = count + 1; /* count is offset by 1 */
    
    /* malloc the data structure */
    input = (Logo) malloc (sizeof(*input));
    if (input == NULL) {
        return NULL;
    }
    input->lines = (char **) malloc (count * sizeof(char *));
    if (input->lines == NULL) {
        /* free input */
        free(input);
        return NULL;
    }
    for (i=0; i<count; i++) {
        /* could call calloc here but more test code will be needed */
        input->lines[i] = (char *) malloc (LINE_LENGTH * sizeof(char));
        if (input->lines[i] == NULL) {
            /* free input and lines */
            free(input->lines);
            free(input);
            return NULL;
        }
        /* input made properly, memset it */
        memset(input->lines[i], '\0', LINE_LENGTH);
    }
    
    /* populate input */
    rewind(in_file);
    count = 0;
    while (fgets(line, sizeof(line), in_file) != NULL) {
        /* trim leading and trailing whitespace */
        trimmed = trim_space(line);
        /* remove trailing newline */
        if (trimmed[strlen(trimmed) - 1] == '\n') {
            trimmed[strlen(trimmed) - 1] = '\0';
        }
        strcpy(input->lines[count], trimmed);
        count = count + 1;
    }
    /* tell the struct how big the 2D char array is */
    input->num_lines = count + 1;
    /* and set the counter to 0 */
    input->counter = 0;
    return input;
}

/**
 *  Frees the input
 */
void free_logo(Logo input) {
    int i;
    for (i=0; i<input->num_lines; i++) {
        free(input->lines[i]);
    }
    free(input->lines);
    free(input);
}

/********************************************
 Command Line Functions
 ********************************************/

/**
 *  Verifies command line arguments and returns the filename as a string
 */
int get_filename(int argc, char * argv[], char *filename) {
    /* check for input */
    if (argc != NUM_ARGS) {
        fprintf(stderr, "Error: Requires one argument only.\n");
        fprintf(stderr, "Usage: parse <filename>\n");
        return ARGS_ERR;
    }
    
    if (sscanf(argv[1], "%s", filename) != 1) {
        fprintf(stderr, "Error: filename is in an incorrect format.\n");
        return ARGS_ERR;
    }
    return 0;
}
