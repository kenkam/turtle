/*
 *  test_parser.c
 *  Unittests for parser.c
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "minunit.h"

#define TEST_FILE1      "data/testdata1.txt"    /* file used to test scan_file() */
#define TEST_FILE2      "data/testdata2.txt"   
#define TEST_FILE3      "data/testdata3.txt"   
#define TEST_OUT        "testout.ps"            /* out_file used to test the interpreter */
#define TEST_BAD_INST   "data/testb_inst.txt"   /* test file with bad instruction */
#define TEST_BAD_VAR    "data/testb_var.txt"    /* test file with bad instruction */
#define TEST_BAD_VRNM   "data/testb_varnum.txt" /* test file with bad instruction */
#define TEST_BAD_DO     "data/testb_do.txt"     /* test file with bad do */
#define TEST_BAD_SET    "data/testb_set.txt"    /* test file with bad instruction */

/* used by minunit.h */
int tests_run = 0;

/* helper functions */
Logo setup(int count, char * line);
Logo create_logo(int count);
void insert_line(Logo input, char * line);
void tear_down(Logo input);

/********************************************
 Test for parse_main() in parse.c
 ********************************************/

static char * test_parse_main() {
    char **argv;
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
    
    /* this should fail */
    ret = parse_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    
    /* give it all good files */
    strcpy(argv[1], TEST_FILE1);
    ret = parse_main(argc, argv);
    mu_assert("error, ret != 0", ret == EXIT_SUCCESS);
    strcpy(argv[1], TEST_FILE2);
    ret = parse_main(argc, argv);
    mu_assert("error, ret != 0", ret == EXIT_SUCCESS);
    strcpy(argv[1], TEST_FILE3);
    ret = parse_main(argc, argv);
    mu_assert("error, ret != 0", ret == EXIT_SUCCESS);
    
    /* test bad files */
    strcpy(argv[1], TEST_BAD_INST);
    ret = parse_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_VAR);
    ret = parse_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_VRNM);
    ret = parse_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_DO);
    ret = parse_main(argc, argv);
    mu_assert("error, ret != EXIT_FAILURE", ret == EXIT_FAILURE);
    strcpy(argv[1], TEST_BAD_SET);
    ret = parse_main(argc, argv);
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

/**
 *  tests parse()
 */
static char * test_parse() {
    Logo good, bad;
    int ret;
    printf("Testing %s\n", __FUNCTION__);

    /* a good input structure */
    good = setup(3, "{");
    insert_line(good, "FD 30");
    insert_line(good, "}");    
    ret = parse(good);
    mu_assert("error, ret != 0", ret == 0);
    
    /* a bad input structure */
    bad = setup(3, "}");
    insert_line(bad, "FD 30");
    insert_line(bad, "}");
    ret = parse(bad);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
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
    
    /* a bad input structure that fails because of starting bracket */
    bad1 = setup(3, "}");
    insert_line(bad1, "FD 30");
    insert_line(bad1, "}");
    ret = mainlogo(bad1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    /* a bad input structure that fails because of bad <INSTRCTLST> */
    bad2 = setup(3, "{");
    insert_line(bad2, "BAD 30");
    insert_line(bad2, "}");
    ret = mainlogo(bad2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);    
    
    /* stuff after closing bracket */
    bad3 = setup(4, "{");
    insert_line(bad3, "FD 30");
    insert_line(bad3, "}");
    insert_line(bad3, "FD 30");
    ret = mainlogo(bad3);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);    
    
    tear_down(good);
    tear_down(bad1);
    tear_down(bad2);
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
    
    /* a bad input structure that fails because of bad <INSTRUCTION> */
    bad1 = setup(3, "BAD");
    insert_line(bad1, "FDA 123");
    insert_line(bad1, "}");
    ret = instrctlst(bad1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    /* a bad input structure that fails because of missing '}' */
    bad2 = setup(3, "LT 30");
    insert_line(bad2, "FD 30");
    insert_line(bad2, "RT 20");
    ret = instrctlst(bad2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);    
    
    tear_down(good);
    tear_down(bad1);
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
    ret = instruction(rt);
    mu_assert("error, ret != 0", ret == 0);
    ret = instruction(lt);
    mu_assert("error, ret != 0", ret == 0);
    ret = instruction(set);
    mu_assert("error, ret != 0", ret == 0);
    ret = instruction(dologo);
    mu_assert("error, ret != 0", ret == 0);
    
    bad1 = setup(1, "DOESNOTEXIST 123");
    ret = instruction(bad1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    /* test that if any of the <INSTRUCTION> such as fd are malformed, ret == PARSE_ERR
     */
    bad2 = setup(1, "FD abc990");
    ret = instruction(bad2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    /* tests empty string */
    bad3 = setup(1, "");
    ret = instruction(bad3);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(fd);
    tear_down(lt);
    tear_down(rt);
    tear_down(set);
    tear_down(dologo);
    tear_down(bad1);
    tear_down(bad2);
    tear_down(bad3);
    return 0;
}

/**
 *  tests fd() as well as chk_inst()
 */
static char * test_fd() {
    Logo good, b_varnum, b_inst, b_chk_inst;
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
    ret = fd(good);
    mu_assert("error, ret != 0", ret == 0);
    ret = fd(b_varnum);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = fd(b_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = fd(b_chk_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
    tear_down(b_varnum);
    tear_down(b_inst);
    tear_down(b_chk_inst);
    return 0;
}

/**
 *  tests lt()
 */
static char * test_lt() {
    Logo good, b_varnum, b_inst;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(2, "LT 20");
    insert_line(good, "LT B");
    
    /* bad varnum */
    b_varnum = setup(1, "LT AB1");
    /* bad instruction */
    b_inst = setup(1, "FD 20");
    
    ret = lt(good);
    mu_assert("error, ret != 0", ret == 0);
    good->counter = good->counter + 1;
    ret = lt(good);
    mu_assert("error, ret != 0", ret == 0);
    ret = lt(b_varnum);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = lt(b_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
    tear_down(b_varnum);
    tear_down(b_inst);
    return 0;
}

/**
 *  tests rt()
 */
static char * test_rt() {
    Logo good, b_varnum, b_inst;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(2, "RT 20");
    insert_line(good, "RT A");
    
    /* bad varnum */
    b_varnum = setup(1, "RT 2P");
    /* bad instruction */
    b_inst = setup(1, "LT 20");
    
    ret = rt(good);
    mu_assert("error, ret != 0", ret == 0);
    good->counter = good->counter + 1;
    ret = rt(good);
    mu_assert("error, ret != 0", ret == 0);
    ret = rt(b_varnum);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = rt(b_inst);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
    tear_down(b_varnum);
    tear_down(b_inst);
    return 0;
}

/**
 *  tests dologo()
 */
static char * test_dologo() {
    Logo good, nested, b_num_args, b_var, b_syntax1, b_syntax2, b_varnum, b_missing;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(3, "DO A FROM 1 TO 8 {");
    insert_line(good, "FD 20");
    insert_line(good, "}");
    /* test nested input */
    nested = setup(9, "DO A FROM 1 TO 8 {");
    insert_line(nested, "FD 20");
    insert_line(nested, "DO B FROM C TO D {");
    insert_line(nested, "LT 20");
    insert_line(nested, "DO E FROM 10 TO 200 {");
    insert_line(nested, "RT 20");
    insert_line(nested, "}");
    insert_line(nested, "}");
    insert_line(nested, "}");
    
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
    
    /* test them all */
    ret = dologo(good);
    mu_assert("error, ret != 0", ret == 0);
    ret = dologo(nested);
    mu_assert("error, ret != 0", ret == 0);    
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
    
    tear_down(good);
    tear_down(nested);
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
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(4, "20");
    insert_line(good, "H");
    /* decimals and negative numbers */
    insert_line(good, "2.5");
    insert_line(good, "-2.5");
    /* bad input */
    b_var = setup(1, "AB");
    b_number = setup(1, "209x");
    
    /* test them */
    ret = varnum(good->lines[0], good);
    mu_assert("error, ret != 0", ret == 0);
    ret = varnum(good->lines[1], good);
    mu_assert("error, ret != 0", ret == 0);
    ret = varnum(good->lines[2], good);
    mu_assert("error, ret != 0", ret == 0); 
    ret = varnum(good->lines[3], good);
    mu_assert("error, ret != 0", ret == 0);    
    ret = varnum(b_var->lines[0], b_var);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = varnum(b_number->lines[0], b_number);
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
    Logo good, b_num_args, b_var, b_syntax1, b_syntax2, b_polish;
    int ret;
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
    b_polish = setup(1, "SET A := 9 8 & ;");
    
    /* test them all */
    ret = set(good);
    mu_assert("error, ret != 0", ret == 0);    
    ret = set(b_num_args);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_var);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_syntax1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_syntax2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = set(b_polish);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
    tear_down(b_num_args);
    tear_down(b_var);
    tear_down(b_syntax1);
    tear_down(b_syntax2);
    tear_down(b_polish);
    return 0;
}

/**
 *  tests polish()
 */
static char * test_polish() {
    Logo good, b_polish1, b_polish2, b_missing;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    /* good input */
    good = setup(1, "A B 2 3 4 + - 8 * / * ;");
    /* bad input with bad varnum */
    b_polish1 = setup(1, "ABC 23 + ;");
    /* bad input with bad operator */
    b_polish2 = setup(1, "A B C = & ;");
    /* missing semicolon */
    b_missing = setup(1, "A B 3 + -");
    
    /* test them */
    ret = polish(good->lines[0], good);
    mu_assert("error, ret != 0", ret == 0);
    ret = polish(b_polish1->lines[0], b_polish1);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);    
    ret = polish(b_polish2->lines[0], b_polish2);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    ret = polish(b_missing->lines[0], b_missing);
    mu_assert("error, ret != PARSE_ERR", ret == PARSE_ERR);
    
    tear_down(good);
    tear_down(b_polish1);
    tear_down(b_polish2);
    tear_down(b_missing);
    return 0;
}

/**
 *  tests helper functions is_var() and is_op()
 */
static char * test_helpers() {
    char tmp[VAR_LENGTH+1], str[LINE_LENGTH];
    char * trimmed;
    int ret, i;
    printf("Testing %s\n", __FUNCTION__);
    
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
    
    return 0;
}

/**
 *  Tests scan_file()
 */
static char * test_scan_file() {
    Logo input;
    FILE *file;
    int i, count;
    char line[LINE_LENGTH];
    printf("Testing %s\n", __FUNCTION__);
    
    file = fopen(TEST_FILE1, "r");
    mu_assert("error, cannot open test file\n", file != NULL);
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
        mu_assert("error, '\n' found in line\n",
                  strcspn(input->lines[i], "\n") == strlen(input->lines[i]));
    }
    /* there should be no leading whitespace */
    for (i=0; i<input->num_lines; i++) {
        mu_assert("error, leading whitespace found in line\n", 
                  isspace(input->lines[i][0]) == 0);
    }
    /* if strlen of line is 0, make sure it's a null char */
    for (i=0; i<input->num_lines; i++) {
        if (strlen(input->lines[i]) == 0) {
            mu_assert("error: empty line not null char", strcmp(input->lines[i], "\0") == 0);
        }
    }
    tear_down(input);
    return 0;
}

static char * test_get_filename() {
    char **argv;
    char filename[FILENAME_MAX];
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
    
    ret = get_filename(argc, argv, filename);
    mu_assert("error, ret != 0", ret == 0);
    /* filename should have "filename.txt" */
    mu_assert("error, filename != \"filename.txt\"", strcmp(filename, "filename.txt") == 0);
    
    /* what if the num args are not right? */
    argc = 1;
    strcpy(argv[1], " ");
    /* there should be an error */
    ret = get_filename(argc, argv, filename);
    mu_assert("error, ret != ARGS_ERR", ret == ARGS_ERR);

    for (i=0; i<NUM_ARGS; i++) {
        free(argv[i]);
    }
    free(argv);
    
    return 0;
}

static char * all_tests() {
    mu_run_test(test_parse_main);
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
    mu_run_test(test_helpers);
    mu_run_test(test_scan_file);
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
    /* malloc the data structure */
    input = (Logo) malloc (1 * sizeof(struct logo));
    input->lines = (char **) malloc (count * sizeof(char *));
    for (i=0; i<count; i++) {
        input->lines[i] = (char *) malloc (LINE_LENGTH * sizeof(char));
    }
    input->counter = 0;
    input->num_lines = 0;
    return input;
}

/**
 *  Insert a line to the logo handle
 */
void insert_line(Logo input, char * line) {
    strcpy(input->lines[input->num_lines], line);
    input->num_lines = input->num_lines + 1;
}

/*
 *  Frees everything
 */
void tear_down(Logo input) {
    free_logo(input);
}
