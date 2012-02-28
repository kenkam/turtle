.. 
    /*
     *  README
     *
     *  This file is written in reStructuredText format for Sphinx (http://sphinx.pocoo.org).
     *
     *  Author: Kenneth Kam <kk4053@bris.ac.uk>
     *  Username: kk4053
     *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
     */
     
==========================
Turtle Graphics Assignment
==========================

A prettier version of this document is in ``docs/index.html``. This assignment has 4 parts to the submission:

* parser
* interpreter
* testing strategy
* extension

The parser and interpreter can be made by using ``make parse`` and ``make interp``. The parser only checks for correct grammar and pays no attention to potential errors such as undefined variables or division by zero errors in the polish expression. The interpreter catches these errors and will report them.

When compiling the interpreter, a flag is passed to set a condition for the preprocessor to instruct it to use the interpreting functions provided in ``postscript.h``. There are 5 functions to 'hook' onto:

* ``void ipt_header(FILE *out_file, char *in_filename)``
* ``void ipt_footer(FILE *out_file)``
* ``void ipt_fd(Logo input, float op)``
* ``void ipt_lt(Logo input, float op)``
* ``void ipt_rt(Logo input, float op)``

The header and footer functions allow different backends to write header and footer content to the output file. For example, postscript requires a few instructions, such as ``newpath`` and a ``moveto`` command before the drawing of lines.

This setup is simple and allows for easy additions of new interpreting code. For example, to implement a ppm backend, one would write the interpreting functions to create a valid ppm file, and then hook them on in ``intercept.h``, and then adding an entry in the makefile to include this new backend, using a new preprocessor conditional flag.

It is this design that allows for easy integration of the interpreter with the extension. ``extension.c`` implements the last 3 functions listed above which interacts with a global variable that holds information on the drawing surface of the GUI program.

The testing strategy is outlined in ``test.txt``. 

The extension is a GUI interpreter for the LOGO language. More details in ``extend.txt``.
