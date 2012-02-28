/*
 *  test_int_malloc.c
 *  Tests return value of malloc for interpreter.c
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "postscript.h"
#include "minunit.h"
#include "overrides.h"

#define TEST_FILE1      "data/testdata1.txt"    /* file used to test scan_file() */
#define TEST_OUT        "testout.ps"            /* out_file used to test the interpreter */

/* used by minunit.h */
int tests_run = 0;

/* helper functions */
Logo setup(int count, char * line);
Logo create_logo(int count);
void insert_line(Logo input, char * line);
void tear_down(Logo input);

/**
 *  Tests 3 malloc() in scan_file()
 */
static char * test_scan_file() {
    Logo input;
    FILE *file;
    printf("Testing %s\n", __FUNCTION__);
    
    file = fopen(TEST_FILE1, "r");
    mu_assert("error, cannot open test file\n", file != NULL);
    /* turn off malloc */
    malloc_fail();
    input = scan_file(file);
    /* this should be NULL */
    mu_assert("error, input != NULL", input == NULL);
    /* turn it back on to test the 2nd malloc in parser.c */
    malloc_ok();
    malloc_fail_next(1);
    rewind(file);
    input = scan_file(file);
    mu_assert("error, input != NULL", input == NULL);
    /* turn it back on to test the 3rd+ malloc in parser.c */
    malloc_ok();
    malloc_fail_next(2);
    rewind(file);
    input = scan_file(file);
    mu_assert("error, input != NULL", input == NULL);
    
    fclose(file);
    malloc_ok();    
    return 0;
}

/**
 *  Tests a malloc() in parse()
 */
static char * test_parse() {
    Logo input;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    
    input = setup(1, "{");
    malloc_fail();
    /* parse() sets a new ->var */
    free(input->vars);
    ret = parse(input);
    mu_assert("error, ret != MEM_ERR", MEM_ERR);
    
    tear_down(input);
    malloc_ok();
    return 0;
}

/**
 *  Test polish() handles things gracefully if the stack doesn't work
 */
static char * test_polish() {
    Logo input;
    Stack head = NULL;
    Stack node, tmp;
    int ret;
    printf("Testing %s\n", __FUNCTION__);
    input = setup(1, "20 30 + ;");
    malloc_fail_next(2); /* fail push on operated number */
    ret = polish(input->lines[0], input, &head);
    mu_assert("error, ret != STACK_ERR", ret == STACK_ERR);
    /* reset and make it fail on a number instead */
    ret = polish(input->lines[0], input, &head);
    mu_assert("error, ret != STACK_ERR", ret == STACK_ERR);
    /* free the stack */
    node = head;
    while (node) {
        tmp = node->next;
        free(node);
        node = tmp;
    }
    
    tear_down(input);
    malloc_ok();
    return 0;
}

/**
 *  Test push() returns STACK_ERR on malloc error
 */
static char * test_push() {
    Stack head = NULL;
    int ret;
    printf("Testing %s\n", __FUNCTION__);

    /* pushing things fail on malloc error */
    malloc_fail();
    ret = push(&head, 10);
    mu_assert("error, ret != STACK_ERR", ret == STACK_ERR);
    
    return 0;
}

static char * all_tests() {
    mu_run_test(test_scan_file);
    mu_run_test(test_parse);
    mu_run_test(test_polish);
    mu_run_test(test_push);
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
    input = (Logo) calloc (1, sizeof(struct logo));
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
 *  Frees everything
 */
void tear_down(Logo input) {
    fclose(input->ofile);
    free_logo(input);
}
