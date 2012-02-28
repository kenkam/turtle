/*
 *  test_interpreter.c
 *  Unittests for interpreter.c
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for access() */
#include "interpreter.h"
#include "postscript.h"
#include "minunit.h"

/* define test files */
#define TEST_FILE1      "data/testdata1.txt"    /* file used to test scan_file() */
#define TEST_FILE2      "data/testdata2.txt"   
#define TEST_FILE3      "data/testdata3.txt"   
#define TEST_EXPECT1    "data/testdata1.ps"     /* expected file output */
#define TEST_EXPECT2    "data/testdata2.ps"     /* these files are used for regression */
#define TEST_EXPECT3    "data/testdata3.ps"     
#define TEST_OUT        "testout.ps"            /* out_file used to test the interpreter */
#define TEST_BAD_INST   "data/testb_inst.txt"   /* test file with bad instruction */
#define TEST_BAD_VAR    "data/testb_var.txt"    /* test file with bad var */
#define TEST_BAD_VRNM   "data/testb_varnum.txt" /* test file with bad varnum */
#define TEST_BAD_DO     "data/testb_do.txt"     /* test file with bad do */
#define TEST_BAD_SET    "data/testb_set.txt"    /* test file with bad set */
#define TEST_BAD_POL    "data/testb_polish.txt" /* test file with division by zero */
#define STR_LENGTH      5000                    /* used to store expected values to test against TEST_OUT */

/* TODO: main function */

/* used by minunit.h */
int tests_run = 0;

/* helper functions */
Logo setup(int count, char * line);
Logo create_logo(int count);
void insert_line(Logo input, char * line);
char * get_content(char * filename);
void tear_down(Logo input);

/********************************************
 Test for main() in parse.c
 ********************************************/

static char * test_main() {
    char **argv;
    char *buffer, *expect;
    int argc, i, ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* malloc the argv */
    argv = (char **) malloc (NUM_ARGS * sizeof(char *));
    for (i=0; i<NUM_ARGS; i++) {
        /* give it a length of FILENAME_LENGTH, and use calloc for memset */
        argv[i] = (char *) calloc (FILENAME_LENGTH, sizeof(char));
    }
    argc = i;
    /* give it some values */
    strcpy(argv[0], "parse");
    strcpy(argv[1], "nonexistant.txt");
    strcpy(argv[2], TEST_OUT);
    
    /* this should fail */
    ret = interp_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    
    /* test all data files 
       this is also used for regression tests */
    strcpy(argv[1], TEST_FILE1);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != 0", ret == EXIT_SUCCESS);
    buffer = get_content(TEST_OUT);
    expect = get_content(TEST_EXPECT1);
    mu_assert("error, TEST_OUT is not as expected",
              strcmp(buffer, expect) == 0);
    free(buffer);
    free(expect);

    strcpy(argv[1], TEST_FILE2);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != 0", ret == EXIT_SUCCESS);
    buffer = get_content(TEST_OUT);
    expect = get_content(TEST_EXPECT2);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, expect) == 0);
    free(buffer);
    free(expect);
    
    strcpy(argv[1], TEST_FILE3);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != 0", ret == EXIT_SUCCESS);
    buffer = get_content(TEST_OUT);
    expect = get_content(TEST_EXPECT3);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, expect) == 0);
    free(buffer);
    free(expect);    

    /* test bad files */
    strcpy(argv[1], TEST_BAD_INST);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    /* there should no longer be a TEST_OUT file */
    mu_assert("error, TEST_OUT exists", access(TEST_OUT, F_OK) == -1);
    strcpy(argv[1], TEST_BAD_VAR);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_VRNM);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_DO);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_SET);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_POL);
    ret = interp_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    
    for (i=0; i<NUM_ARGS; i++) {
        free(argv[i]);
    }
    free(argv);

    return 0;
}

/********************************************
 Unittests
 ********************************************/

/* TODO: tests for int_fd, int_rt, int_lt */

/**
 *  tests parse()
 */
static char * test_parse() {
    Logo good, bad;
    int ret, i;
    printf("Testing %s\n", __FUNCTION__);
    
    /* a good input structure */
    good = setup(3, "{");
    insert_line(good, "FD 30");
    insert_line(good, "}");    
    /* free good->vars as parse() makes a new one */
    free(good->vars);
    ret = parse(good);
    mu_assert("error, ret != 0", ret == 0);
    
    /* the input->vars should be initialised */
    for (i=0; i<VARARY_SIZE; i++) {
        mu_assert("good->vars[i].used != 0", good->vars[i].used == 0);
        mu_assert("good->vars[i].used != 0", good->vars[i].data == 0);
    }
    tear_down(good);
    
    /* a bad input structure */
    bad = setup(3, "}");
    insert_line(bad, "FD 30");
    insert_line(bad, "}");
    /* free bad->vars as parse() makes a new one */
    free(bad->vars);
    ret = parse(bad);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);    
    tear_down(bad);

    return 0;
}

/**
 *  tests mainlogo()
 */
static char * test_mainlogo() {
    Logo good, bad1, bad2, bad3;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* a good input structure */
    good = setup(3, "{");
    insert_line(good, "FD 30");
    insert_line(good, "}");    
    ret = mainlogo(good);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(good);
    
    /* a bad input structure that fails because of starting bracket */
    bad1 = setup(3, "}");
    insert_line(bad1, "FD 30");
    insert_line(bad1, "}");
    ret = mainlogo(bad1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(bad1);
    
    /* a bad input structure that fails because of bad <INSTRCTLST> */
    bad2 = setup(3, "{");
    insert_line(bad2, "BAD 30");
    insert_line(bad2, "}");
    ret = mainlogo(bad2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(bad2);    
    
    /* stuff after closing bracket */
    bad3 = setup(4, "{");
    insert_line(bad3, "FD 30");
    insert_line(bad3, "}");
    insert_line(bad3, "FD 30");
    ret = mainlogo(bad3);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(bad3);    
    
    return 0;
}

/**
 *  tests instrctlst()
 */
static char * test_instrctlst() {
    Logo good, bad1, bad2;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* a good input structure. no need for opening { as thats dealt with by
     mainlogo()
     */
    good = setup(3, "FD 30");
    insert_line(good, "RT 30");
    insert_line(good, "}");    
    ret = instrctlst(good);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(good);
    
    /* a bad input structure that fails because of bad <INSTRUCTION> */
    bad1 = setup(3, "BAD");
    insert_line(bad1, "FDA 123");
    insert_line(bad1, "}");
    ret = instrctlst(bad1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(bad1);
    
    /* a bad input structure that fails because of missing '}' */
    bad2 = setup(3, "LT 30");
    insert_line(bad2, "FD 30");
    insert_line(bad2, "RT 20");
    ret = instrctlst(bad2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);  
    tear_down(bad2);  
    
    return 0;
}

/**
 *  tests instruction()
 */
static char * test_instruction() {
    Logo fd, lt, rt, set, dologo, bad1, bad2, bad3;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* create Logo handles for all 5 instructions */
    fd = setup(1, "FD 30");
    lt = setup(1, "LT 30");
    rt = setup(1, "RT 30");
    set = setup(1, "SET A := 30 ;");
    dologo = setup(3, "DO A FROM 1 TO 5 {");
    insert_line(dologo, "FD 20");
    insert_line(dologo, "}");
    
    /* test them all */
    ret = instruction(fd);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(fd);
    ret = instruction(lt);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(lt);
    ret = instruction(rt);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(rt);
    ret = instruction(set);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(set);
    ret = instruction(dologo);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(dologo);
    
    bad1 = setup(1, "DOESNOTEXIST 123");
    ret = instruction(bad1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(bad1);
    
    /* test that if any of the <INSTRUCTION> such as fd are malformed, ret == PARSE_ERR
     */
    bad2 = setup(1, "FD abc990");
    ret = instruction(bad2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(bad2);
    
    /* tests empty string */
    bad3 = setup(1, "");
    ret = instruction(bad3);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(bad3);
    
    return 0;
}

/**
 *  tests fd() as well as chk_inst()
 */
static char * test_fd() {
    Logo good, b_varnum, b_inst, b_chk_inst;
    char *buffer;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(2, "FD 20");
    insert_line(good, "FD A");
    
    /* bad varnum */
    b_varnum = setup(1, "FD AB");
    /* bad instruction */
    b_inst = setup(1, "RT 20");
    /* test chk_inst error checking */
    b_chk_inst = setup(1, "FD");
    
    ret = fd(good);
    mu_assert("error, ret != 0", ret == 0);
    good->counter = good->counter + 1;
    /* set the variable */
    set_var("A", good->vars, 30.0);
    ret = fd(good);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(good);
    
    /* expect postscript and test the output file */
    char str[STR_LENGTH] = "";
    char tmp[LINE_LENGTH];
    sprintf(tmp, "%.2f 0 rlineto\n", 20 * LOGO2PS_FACTOR);
    strcat(str, tmp);
    sprintf(tmp, "%.2f 0 rlineto\n", 30 * LOGO2PS_FACTOR);
    strcat(str, tmp);
    buffer = get_content(TEST_OUT);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, str) == 0);
    free(buffer);
    
    ret = fd(b_varnum);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(b_varnum);
    ret = fd(b_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(b_inst);
    ret = fd(b_chk_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(b_chk_inst);
    
    return 0;
}

/**
 *  tests lt()
 */
static char * test_lt() {
    Logo good, b_varnum, b_inst;
    char *buffer;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(2, "LT 20");
    insert_line(good, "LT B");
    
    ret = lt(good);
    mu_assert("error, ret != 0", ret == 0);
    good->counter = good->counter + 1;
    /* set the variable */
    set_var("B", good->vars, 30.0);
    ret = lt(good);
    mu_assert("error, ret != 0", ret == 0); 
    tear_down(good);
    
    /* expect postscript and test the output file */
    char str[STR_LENGTH] = "";
    char tmp[LINE_LENGTH];
    sprintf(tmp, "%.2f rotate\n", 20 * LOGO2PS_FACTOR);
    strcat(str, tmp);
    sprintf(tmp, "%.2f rotate\n", 30 * LOGO2PS_FACTOR);
    strcat(str, tmp);
    buffer = get_content(TEST_OUT);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, str) == 0);
    free(buffer);
    
    /* bad varnum */
    b_varnum = setup(1, "LT AB1");
    ret = lt(b_varnum);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(b_varnum);
    /* bad instruction */
    b_inst = setup(1, "FD 20");
    ret = lt(b_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(b_inst);
    
    return 0;
}

/**
 *  tests rt()
 */
static char * test_rt() {
    Logo good, b_varnum, b_inst;
    char *buffer;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(2, "RT 20");
    insert_line(good, "RT A");
    ret = rt(good);
    mu_assert("error, ret != 0", ret == 0);
    good->counter = good->counter + 1;
    /* set the variable */
    set_var("A", good->vars, 30.0);
    ret = rt(good);
    mu_assert("error, ret != 0", ret == 0);
    tear_down(good);
    
    /* expect postscript and test the output file */
    char str[STR_LENGTH] = "";
    char tmp[LINE_LENGTH];
    sprintf(tmp, "-%.2f rotate\n", 20 * LOGO2PS_FACTOR);
    strcat(str, tmp);
    sprintf(tmp, "-%.2f rotate\n", 30 * LOGO2PS_FACTOR);
    strcat(str, tmp);
    buffer = get_content(TEST_OUT);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, str) == 0);
    free(buffer);
    
    /* bad varnum */
    b_varnum = setup(1, "RT 2P");
    ret = rt(b_varnum);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);    
    tear_down(b_varnum);
    /* bad instruction */
    b_inst = setup(1, "LT 20");
    ret = rt(b_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    tear_down(b_inst);
    
    return 0;
}

/**
 *  tests dologo()
 */
static char * test_dologo() {
    Logo good, nested, reversed, b_num_args, b_var, b_syntax1, b_syntax2, b_varnum, b_missing;
    int i, j, k, ret;
    float a;
    char str[STR_LENGTH] = "";
    char tmp[LINE_LENGTH];
    char *buffer;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(3, "DO A FROM 1 TO 8 {");
    insert_line(good, "FD A");
    insert_line(good, "}");
    ret = dologo(good);
    mu_assert("error, ret != 0", ret == 0);
    /* test that A is now == 8 */
    get_var("A", good->vars, &a);
    mu_assert("error, A != 8", a == 8);
    tear_down(good);
    /* expecting 8 FD A's here, where A = 1..8 */
    for (i=1; i<=8; i++) {
        sprintf(tmp, "%.2f 0 rlineto\n", i * LOGO2PS_FACTOR);
        strcat(str, tmp);
    }
    buffer = get_content(TEST_OUT);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, str) == 0);
    free(buffer);
    
    /* test nested input */
    nested = setup(9, "DO A FROM 1 TO 3 {");
    insert_line(nested, "FD A");
    insert_line(nested, "DO B FROM C TO D {");
    insert_line(nested, "LT B");
    insert_line(nested, "DO E FROM 1 TO 3 {");
    insert_line(nested, "RT E");
    insert_line(nested, "}");
    insert_line(nested, "}");
    insert_line(nested, "}");
    /* test that vars work in do */
    set_var("C", nested->vars, 1);
    set_var("D", nested->vars, 3);
    ret = dologo(nested);
    mu_assert("error, ret != 0", ret == 0);    
    tear_down(nested);
    /* expect some kind of string here, reconstruct using for loops here */
    memset(str, '\0', STR_LENGTH);
    for (i=1; i<=3; i++) {
        sprintf(tmp, "%.2f 0 rlineto\n", i * LOGO2PS_FACTOR);
        strcat(str, tmp);
        for (j=1; j<=3; j++) {
            sprintf(tmp, "%.2f rotate\n", j * LOGO2PS_FACTOR);
            strcat(str, tmp);
            for (k=1; k<=3; k++) {
                sprintf(tmp, "-%.2f rotate\n", k * LOGO2PS_FACTOR);
                strcat(str, tmp);
            }
        }
    }
    buffer = get_content(TEST_OUT);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, str) == 0);
    free(buffer);
    
    /* reversed input */
    reversed = setup(3, "DO A FROM 8 TO 1 {");
    insert_line(reversed, "FD A");
    insert_line(reversed, "}");
    ret = dologo(reversed);
    mu_assert("error, ret != 0", ret == 0);
    /* test that A is now == 1 */
    get_var("A", reversed->vars, &a);
    mu_assert("error, A != 1", a == 1);
    tear_down(reversed);
    /* expecting 8 FD A's here, where A = 8..1 */
    memset(str, '\0', STR_LENGTH);
    for (i=8; i>=1; i--) {
        sprintf(tmp, "%.2f 0 rlineto\n", i * LOGO2PS_FACTOR);
        strcat(str, tmp);
    }
    buffer = get_content(TEST_OUT);
    mu_assert("error, TEST_OUT is not as expected", 
              strcmp(buffer, str) == 0);
    free(buffer);
    
    /* bad number of arguments */
    b_num_args = setup(3, "DO A FROM TO 8 {");
    insert_line(b_num_args, "FD 20");
    insert_line(b_num_args, "}");
    /* bad var input */
    b_var = setup(3, "DO AB FROM 1 TO 8 {");
    insert_line(b_var, "FD 20");
    insert_line(b_var, "}");
    /* bad syntax (incorrect FROM or TO) */
    b_syntax1 = setup(3, "SET A TO 1 FROM 8 {");
    insert_line(b_syntax1, "FD 20");
    insert_line(b_syntax1, "}");
    /* bad syntax (missing opening bracket) */
    b_syntax2 = setup(3, "DO A FROM 1 TO 8");
    insert_line(b_syntax2, "FD 20");
    insert_line(b_syntax2, "}");
    /* bad varnum */    
    b_varnum = setup(3, "DO A FROM PL TO 8 {");
    insert_line(b_varnum, "FD 20");
    insert_line(b_varnum, "}");
    /* missing ending bracket */
    b_missing = setup(3, "DO A FROM 1 TO 8 {");
    insert_line(b_missing, "FD 20");
    insert_line(b_missing, "LT 20");

    /* test the rest of bad syntax */
    ret = dologo(b_num_args);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = dologo(b_var);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = dologo(b_syntax1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = dologo(b_syntax2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = dologo(b_varnum);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = dologo(b_missing);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(b_num_args);
    tear_down(b_var);
    tear_down(b_syntax1);
    tear_down(b_syntax2);
    tear_down(b_varnum);
    tear_down(b_missing);
    return 0;
}

/**
 *  tests varnum()
 */
static char * test_varnum() {
    Logo good, b_var, b_number;
    int ret;
    float op;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(4, "20");
    /* decimals and negative numbers */
    insert_line(good, "2.5");
    insert_line(good, "-2.5");
    insert_line(good, "A");
    /* bad input */
    b_var = setup(3, "AB");
    /* non existant var */
    insert_line(b_var, "C");
    insert_line(b_var, "3-");
    b_number = setup(1, "209x");
    
    /* test them */
    ret = varnum(good->lines[0], good, &op);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, op not set correctly", op == 20);
    ret = varnum(good->lines[1], good, &op);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, op not set correctly", op == 2.5);
    ret = varnum(good->lines[2], good, &op);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, op not set correctly", op == -2.5);
    /* set the var A */
    set_var("A", good->vars, 20);
    ret = varnum(good->lines[3], good, &op);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, op not set correctly", op == 20);
    ret = varnum(b_var->lines[0], b_var, &op);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = varnum(b_var->lines[1], b_var, &op);
    mu_assert("error, ret != VAR_ERR", ret == VAR_ERR);    
    ret = varnum(b_var->lines[2], b_var, &op);
    mu_assert("error, ret != VAR_ERR", ret == PARSE_ERR);   
    ret = varnum(b_number->lines[0], b_number, &op);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
    tear_down(b_var);
    tear_down(b_number);
    return 0;
}

/**
 *  tests set()
 */
static char * test_set() {
    Logo good, b_num_args, b_var, b_syntax1, b_syntax2, b_polish1, b_polish2, b_polish3;
    int ret;
    float a, num;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(1, "SET A := 3 2 9 8 + - * ;");
    /* bad number of args */
    b_num_args = setup(1, "SET A :=");
    /* bad var input */
    b_var = setup(1, "SET AB := 8 ;");
    /* bad syntax (bad equal sign) */
    b_syntax1 = setup(1, "SET A = 8 ;");
    /* bad syntax (bad instruction) */
    b_syntax2 = setup(1, "DO A := 10 ;");
    /* bad polish (will be tested more thoroughly later) */
    b_polish1 = setup(1, "SET A := 9 8 & ;");
    /* unbalanced polish. parser allows this but the interpreter should flag it
       as an error */
    b_polish2 = setup(1, "SET A := 9 8 8 7 + - ;");
    /* a polish only has a ; */
    b_polish3 = setup(1, "SET A := ;");
    
    /* test them all */
    ret = set(good);
    mu_assert("error, ret != 0", ret == 0);    
    /* this should have set A, calculate the above polish expression */
    num = 3 * (2 - (9 + 8));
    /* get the value in A */
    get_var("A", good->vars, &a);
    mu_assert("error, A != polish expression", a == num);
    ret = set(b_num_args);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_var);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_syntax1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_syntax2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_polish1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_polish2);
    mu_assert("error, ret != POL_ERR", ret == POL_ERR);    
    ret = set(b_polish3);
    mu_assert("error, ret != POL_ERR", ret == POL_ERR);
    
    tear_down(good);
    tear_down(b_num_args);
    tear_down(b_var);
    tear_down(b_syntax1);
    tear_down(b_syntax2);
    tear_down(b_polish1);
    tear_down(b_polish2);
    tear_down(b_polish3);
    return 0;
}

/**
 *  tests polish()
 */
static char * test_polish() {
    Logo good, b_polish1, b_polish2, b_polish3, b_polish4;
    Logo b_polish5, b_polish6, b_zero, b_missing;
    int ret;
    float a, num;
    Stack head = NULL;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(1, "A B 2 3 4 + - 8 * / * ;");
    set_var("A", good->vars, 1);
    set_var("B", good->vars, 3);
    /* bad input with bad varnum */
    b_polish1 = setup(1, "ABC 23 + ;");
    /* bad input with bad operator */
    b_polish2 = setup(1, "1 2 3 = & ;");
    /* unbalanced polish */
    b_polish3 = setup(1, "9 8 8 7 + - ;");
    /* test stack underflow (first operand underflow) */
    b_polish4 = setup(1, "+ 8 ;");    
    /* test stack underflow (second operand underflow) */
    b_polish5 = setup(1, "9 + 8 ;");
    /* test short polish */
    b_polish6 = setup(1, "9 + ;");
    /* test division by zero */
    b_zero = setup(1, "9 0 / ;");    
    /* missing semicolon */
    b_missing = setup(1, "1 2 3 + -");
    
    /* test them */
    ret = polish(good->lines[0], good, &head);
    mu_assert("error, ret != 0", ret == 0);
    pop(&head, &a);
    num = 1 * ((float) 3 / ((2 - (4 + 3)) * 8));
    mu_assert("error, A != polish expression", a == num);
    ret = polish(b_polish1->lines[0], b_polish1, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);    
    ret = polish(b_polish2->lines[0], b_polish2, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = polish(b_polish3->lines[0], b_polish3, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != POL_ERR", ret == POL_ERR);
    ret = polish(b_polish4->lines[0], b_polish4, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != POL_ERR", ret == POL_ERR);       
    ret = polish(b_polish5->lines[0], b_polish5, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != POL_ERR", ret == POL_ERR);   
    ret = polish(b_polish6->lines[0], b_polish6, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != POL_ERR", ret == POL_ERR);
    ret = polish(b_zero->lines[0], b_zero, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != POL_ERR", ret == POL_ERR);    
    ret = polish(b_missing->lines[0], b_missing, &head);
    free_stack(head);
    head = NULL;
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
    tear_down(b_polish1);
    tear_down(b_polish2);
    tear_down(b_polish3);
    tear_down(b_polish4);
    tear_down(b_polish5); 
    tear_down(b_polish6);
    tear_down(b_zero);
    tear_down(b_missing);
    return 0;
}

/**
 *  tests operate()
 */
static char * test_operate() {
    int ret;
    float output;
    printf("Testing %s\n", __FUNCTION__);
    
    /* test + */
    ret = operate('+', 10.0, 20.0, &output);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, output != a + b", output == 30.0);
    /* test - */
    ret = operate('-', 10.0, 20.0, &output);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, output != a + b", output == -10.0);
    /* test * */
    ret = operate('*', 10.0, 20.0, &output);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, output != a + b", output == 200.0);
    /* test / */
    ret = operate('/', 10.0, 20.0, &output);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, output != a + b", output == 0.5);
    /* test divide by 0 error */
    ret = operate('/', 10.0, 0, &output);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    return 0;
}

/**
 *  tests all helper functions
 */
static char * test_helpers() {
    Logo input;
    char tmp[VAR_LENGTH+1], str[LINE_LENGTH];
    char * trimmed;
    int ret, i;
    float output;
    printf("Testing %s\n", __FUNCTION__);
    
    /* test get_var() */
    input = create_logo(0);
    /* this is essentially set_var("A", input->vars, 10); */
    input->vars[0].data = 10;
    input->vars[0].used = 1;
    /* try to get A */
    ret = get_var("A", input->vars, &output);
    mu_assert("error, ret != 0", ret == 0);
    mu_assert("error, A != 10", output == 10);
    /* try to get B which is not initialised */
    ret = get_var("B", input->vars, &output);
    mu_assert("error, ret != VAR_ERR", ret == VAR_ERR);
    
    /* test set_var() */
    set_var("A", input->vars, 20);
    set_var("B", input->vars, 40);
    mu_assert("error, A != 20", input->vars[0].data == 20.0);
    mu_assert("error, B != 40", input->vars[1].data == 40.0);   
    /* setting the var should have flagged this var as used */
    mu_assert("error, B is not initialised", input->vars[0].used == 1);
    
    /* test is_var() from A-Z */
    for (i='A'; i<='Z'; i++) {
        tmp[0] = i;
        tmp[1] = '\0';
        ret = is_var(tmp);
        mu_assert("error, ret != 1", ret == 1);
    }
    /* test bogus inputs */
    /* small letter */
    tmp[0] = 'a';
    ret = is_var(tmp);
    mu_assert("error, ret != 0", ret == 0);
    /* number */
    tmp[0] = '1';
    ret = is_var(tmp);
    mu_assert("error, ret != 0", ret == 0);      
    
    /* test is_op */
    tmp[0] = '+';
    ret = is_op(tmp);
    mu_assert("error, ret != 1", ret == 1);
    tmp[0] = '-';
    ret = is_op(tmp);
    mu_assert("error, ret != 1", ret == 1);
    tmp[0] = '*';
    ret = is_op(tmp);
    mu_assert("error, ret != 1", ret == 1);
    tmp[0] = '/';
    ret = is_op(tmp);
    mu_assert("error, ret != 1", ret == 1);    
    /* test bogus inputs */
    tmp[0] = '=';
    ret = is_op(tmp);
    mu_assert("error, ret != 0", ret == 0);
    tmp[0] = '^';
    ret = is_op(tmp);
    mu_assert("error, ret != 0", ret == 0);
    tmp[0] = '$';
    ret = is_op(tmp);
    mu_assert("error, ret != 0", ret == 0);    
    
    /* test trim_space() */
    strcpy(str, "\t\tFD 30 \t\t   ");
    trimmed = trim_space(str);
    mu_assert("error, trimmed != \"FD 30\"", strsame(trimmed, "FD 30"));
    strcpy(str, "  \t  \tDO A FROM 1 TO 8 { \t\t   ");
    trimmed = trim_space(str);
    mu_assert("error, trimmed != \"DO A FROM 1 TO 8 {\"", strsame(trimmed, "DO A FROM 1 TO 8 {"));
    
    tear_down(input);

    return 0;
}

/**
 *  tests scan_file()
 */
static char * test_scan_file() {
    Logo input;
    FILE *file;
    int i, count;
    char line[LINE_LENGTH];
    printf("Testing %s\n", __FUNCTION__);
    
    file = fopen(TEST_FILE1, "r");
    mu_assert("error, cannot open test file", file != NULL);
    /* scan the file */
    input = scan_file(file);
    /* calculate the number of lines */
    rewind(file);
    count = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        count = count + 1;
    }
    fclose(file);
    
    /* num_lines member should be same as count + 1 since count starts at 0 */
    mu_assert("error, input->num_lines != count + 1", input->num_lines == count + 1);
    /* counter should be 0 */
    mu_assert("error, input->counter != 0", input->counter == 0);
    /* there should be no newlines */
    for (i=0; i<input->num_lines; i++) {
        mu_assert("error, '\n' found in line",
                  strcspn(input->lines[i], "\n") == strlen(input->lines[i]));
    }
    /* there should be no leading whitespace */
    for (i=0; i<input->num_lines; i++) {
        mu_assert("error, leading whitespace found in line", 
                  isspace(input->lines[i][0]) == 0);
    }
    /* if strlen of line is 0, make sure it's a null char */
    for (i=0; i<input->num_lines; i++) {
        if (strlen(input->lines[i]) == 0) {
            mu_assert("error: empty line not null char", strcmp(input->lines[i], "\0") == 0);
        }
    }
    input->vars = NULL;
    free_logo(input);
    return 0;
}

/**
 *  Tests stack functions pop() and push()
 */
static char * test_stack_funcs() {
    Stack head = NULL;
    Stack node;
    int i, ret;
    float output;
    printf("Testing %s\n", __FUNCTION__);
    
    /* try pushing things into the stack */
    push(&head, 10);
    push(&head, 20);
    node = head;
    i = 1;
    while ((node = node->next) != NULL) {
        i = i + 1;
    }
    /* there should be 2 things in the stack now */
    mu_assert("error, i != 2", i == 2);
    /* add a few more */
    push(&head, 30);
    push(&head, 40);
    node = head;
    i = 1;
    while ((node = node->next) != NULL) {
        i = i + 1;
    }
    /* now there's 4 */
    mu_assert("error, i != 4", i == 4);
    
    /* pop 'em */
    pop(&head, &output);
    mu_assert("error, output != 40", output == 40.0);
    pop(&head, &output);
    mu_assert("error, output != 40", output == 30.0);
    pop(&head, &output);
    mu_assert("error, output != 40", output == 20.0);
    pop(&head, &output);
    mu_assert("error, output != 40", output == 10.0);
    
    /* head should be NULL now */
    mu_assert("error, head != NULL", head == NULL);
    
    /* popping empty head will return STACK_ERR */
    ret = pop(&head, &output);
    mu_assert("error, ret != STACK_ERR", ret == STACK_ERR);
    
    return 0;
}

static char * test_get_filename() {
    char **argv;
    char filename[FILENAME_MAX];
    char fileout[FILENAME_MAX];
    int argc, i, ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* malloc the argv */
    argv = (char **) malloc (NUM_ARGS * sizeof(char *));
    for (i=0; i<NUM_ARGS; i++) {
        /* give it a length of 30, and use calloc for memset */
        argv[i] = (char *) calloc (30, sizeof(char));
    }
    argc = i;
    /* give it some values */
    strcpy(argv[0], "parse");
    strcpy(argv[1], "filename.txt");
    strcpy(argv[2], "fileout.txt");
    
    ret = get_filenames(argc, argv, filename, fileout);
    mu_assert("error, ret != 0", ret == 0);
    /* filename should have "filename.txt" */
    mu_assert("error, filename != \"filename.txt\"", strcmp(filename, "filename.txt") == 0);
    mu_assert("error, fileout != \"fileout.txt\"", strcmp(fileout, "fileout.txt") == 0);    
    
    /* what if the num args are not right? */
    argc = 1;
    /* there should be an error */
    ret = get_filenames(argc, argv, filename, fileout);
    mu_assert("error, ret != ARGS_ERR", ret == ARGS_ERR);
    
    /* no output file */
    argc = 2;
    /* there should be an error */
    ret = get_filenames(argc, argv, filename, fileout);
    mu_assert("error, ret != ARGS_ERR", ret == ARGS_ERR);    
   
    for (i=0; i<NUM_ARGS; i++) {
        free(argv[i]);
    }
    free(argv);

    return 0;
}

static char * all_tests() {
    mu_run_test(test_main);
    mu_run_test(test_parse);
    mu_run_test(test_mainlogo);
    mu_run_test(test_instrctlst);
    mu_run_test(test_instruction);
    mu_run_test(test_fd);
    mu_run_test(test_lt);
    mu_run_test(test_rt); 
    mu_run_test(test_dologo);
    mu_run_test(test_varnum);
    mu_run_test(test_set);
    mu_run_test(test_polish);
    mu_run_test(test_operate);
    mu_run_test(test_helpers);
    mu_run_test(test_scan_file);
    mu_run_test(test_stack_funcs);
    mu_run_test(test_get_filename);
    return 0;
}

/**
 *  Boilerplate for minunit.h
 */
int main(int argc, const char * argv[]) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
        return 1;
    }
    else {
        printf("All tests passed\n");
    }
    printf("Tests run: %d\n", tests_run);
    return 0;
}

/********************************************
 Helper Functions
 ********************************************/

/**
 *  Setup the test
 */
Logo setup(int count, char * line) {
    Logo input = create_logo(count);
    insert_line(input, line);
    return input;
}

/**
 *  Creates a empty Logo structure with count empty strings
 */
Logo create_logo(int count) {
    Logo input;
    int i;
    FILE *ofile;
    /* malloc the data structure */
    input = (Logo) malloc (sizeof(struct logo));
    input->lines = (char **) malloc (count * sizeof(char *));
    for (i=0; i<count; i++) {
        input->lines[i] = (char *) malloc (LINE_LENGTH * sizeof(char));
    }
    input->counter = 0;
    input->num_lines = 0;
    /* set the varstack */
    input->vars = (VarStack) malloc (VARARY_SIZE * sizeof(struct _varstack));
    for (i=0; i<VARARY_SIZE; i++) {
        /* set all values to 0 and used to 0 */
        input->vars[i].data = 0;
        input->vars[i].used = 0;
    }
    ofile = fopen(TEST_OUT, "w");
    if (ofile == NULL) {
        fprintf(stderr, "Error: cannot open output file %s for writing\n", TEST_OUT);
        exit(EXIT_FAILURE);
    }
    input->ofile = ofile;
    return input;
}

/**
 *  Insert a line to the logo handle
 */
void insert_line(Logo input, char * line) {
    strcpy(input->lines[input->num_lines], line);
    input->num_lines = input->num_lines + 1;
}

/**
 *  Reads a filename into string and returns it
 */
char * get_content(char * filename) {
    long lSize;
    char * buffer;
    /* open the test output file */
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: cannot open file %s\n", TEST_OUT);
        exit(EXIT_FAILURE);
    }
    /* how long is the file */
    fseek (file, 0, SEEK_END);
    lSize = ftell (file);
    rewind(file);
    /* calloc this string and read file (+1 for null char) */
    buffer = (char *) calloc (lSize + 1, sizeof(char));
    fread(buffer, 1, lSize, file);
    buffer[strlen(buffer)] = '\0';
    fclose(file);
    
    return buffer;
}

/*
 *  Frees everything
 */
void tear_down(Logo input) {
    fclose(input->ofile);
    free_logo(input);
}
