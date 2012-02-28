===========================
turtle - A Logo Parser in C
===========================

This C project contains a parser, an interpreter, and a GTK+ GUI that would interpret the LOGO language on the fly. 

Compiling
=========

This has been tested on Ubuntu 10.10 with gtk-2.0. Make sure gtk-2.0 is installed correctly, and then simply do

    make extension
    ./extension
  
will fire up the GUI.

Logo Language
=============

The BNF of the language is:

    <MAIN>        ::= "{" <INSTRCTLST>
    <INSTRCTLST>  ::= <INSTRUCTION><INSTRCTLST> | "}"
    <INSTRUCTION> ::= <FD> | <LT> | <RT> | <DO> | <SET>
    <FD>          ::= "FD" <VARNUM>
    <LT>          ::= "LT" <VARNUM>
    <RT>          ::= "RT" <VARNUM>
    <DO>          ::= "DO" <VAR> "FROM" <VARNUM> "TO"
                      <VARNUM> "{" <INSTRCTLST>
    <VAR>         ::= [A-Z]
    <VARNUM>      ::= number | <VAR>
    <SET>         ::= "SET" <VAR> ":=" <POLISH>
    <POLISH>      ::= <OP> <POLISH> | <VARNUM> <POLISH> | ";"
    <OP>          ::= "+" | "-" | "*" | "/"

About
=====

This project was written for an assignment for the University of Bristol. It includes substantial testing using [MinUnit](http://www.jera.com/techinfo/jtns/jtn002.html). There are the [project submission documentation](http://kenkam.github.com/turtle) that discusses the technicalities of the implementation.
