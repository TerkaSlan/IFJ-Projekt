![logo](http://i.imgur.com/mrVumMH.png)

  Repository for our project to the IFJ course at FIT, BUT : Interpreter for simplified version of Java SE 8

  **Team:**	[Michal Charvát](https://github.com/CharvN), [Terka Slanináková](https://github.com/TerkaSlan), [Katarína Grešová](https://github.com/kati-ka), [Marek Schauer](https://github.com/marekschauer), [Jakub Handzuš](https://github.com/JakubHandzus)

  **Project Specification (cz):** https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2016.pdf

  How to run: Download/clone this repository, run "make", provide a source file or use one from /tests directory (".code" extension). Supports only x64 platform.

 **Stage 1: Basic data structures, supporting modules, simple lexical analysis**

 **Stage 2: Symbol table, grammar, basic scanner implementation, elaborate lexical analysis**

 **Stage 3: Parser, interpret, expressions and instruction modules implementation.**

 **Stage 4: Testing, debugging, fixing, optimalizations, low-priority bonus extensions**

_Week 1 [4.10. - 11.10]_

   #0 Initial Setup - **Terka** :white_check_mark:

   #1 The string data type implementation + test file - **Jakub + Marek** :white_check_mark:

   #2 [Depends on #1] The token data type implementation + test file - **Katka** :white_check_mark:

   #3 Simple error handling module + test file - **Michal** :white_check_mark:

   #4 [Depends on #1 and #2] Scanner implementation + test file - **Terka** :white_check_mark:

_Week 2 [12.10 - 18.10]_

  #5 Symbol table DS specification - **Michal** :white_check_mark:

  #6 Symbol table operations (hashtable) - **Michal** :white_check_mark:

  #7 Finish error + Result codes - **Michal** :white_check_mark:

  #10 Instruction set - **Michal** :white_check_mark:

  #11 Grammar, basic parser implementation - **Katka** :white_check_mark:

  #12 Builtins - **Jakub + Marek** :white_check_mark:

  #13 Conversions - **Terka** :white_check_mark:

_Week 3 [19.10 - 25.10]_

  Finishing half-done tasks from previous weeks

_Week 4 [26.10 - 1.11]_

  #14 Extend conversions to deal with octal numbers in string escape sequences - **Terka** :white_check_mark:

  #15 Basic interpreter implementation - **Michal** :white_check_mark:

_Week 5 [2.11 - 8.11]_

  #16 Elaborate conversions needed in later phases of interpretation - **Terka** :white_check_mark:

  #17 [LOW] Add support for doubles represented in other than decimal bases - **Terka** :white_check_mark:

_Week 6 [9.11 - 15.11]_

  #18 [HIGH] Expressions module implementation - **Katka** :white_check_mark:

_Week 7 [16.11 - 22.11]_

  #19 [HIGH] Deal with the newly-found need for 2 runs of parser - **Terka** :white_check_mark:

  #20 Internal documentation on how to work and communicate with symbol table - **Michal** :white_check_mark:

  #21 [LOW] Mechanism to print out the instruction in our internal code for debugging purposes - **Jakub, Marek** :white_check_mark:

  #22 [HIGH] Main.c file - **Terka** :white_check_mark:

  #23 [HIGH] New Makefile for 1. submission - **Michal** :white_check_mark:

  #24 [HIGH] Implement recursive freeing of globalSymbolTable in Main.c for 1. submission - **Michal** :white_check_mark:

  #25 [LOW] Solve leaks originating at interpret module for 2. submission - **Michal** :white_check_mark:

  #26 [LOW] Add additinal tests to cover all instructions for 2. submission - **Michal** :white_check_mark:

  #27 [LOW] Create as much general tests as possible for 2. submission - **ALL** :white_check_mark:

  #28 [HIGH] Adjust Parser and Interpret modules to deal with the need of initialization of static variables before runtime - **Terka, Michal** :white_check_mark:

  #29 [LOW] Make all print functions within the Builtin module behave according to their java counterparts - **Jakub, Marek** :white_check_mark:

_Week 8 [23.11 - 29.11]_

  Focus all efforts to make program functional for 1st submission

_Week 9 [30.11 - 6.12]_

  Finish parser implementation - **Terka**, **Michal**, **Katka** :white_check_mark:

  Fix general bugs - **ALL** :white_medium_square:

  Adjust Makefile for 2nd submission (/tests directory) - **Michal** :white_check_mark:

  Start to work on documentation - **Marek, Jakub** :white_check_mark:

  Focus all efforts to make program functional for 2nd submission

_Week 10 [7.12 - 14.12]_

  #30 FEATURE ADDITION: Move the addition of local variabled to symbol table to second run to overcome specific inapproriate overshadowing issues - **Terka** :white_check_mark:

  #31 FEATURE ADDITION: Add support for bases covered by BASE extension into readData() function - **Terka** :white_check_mark:
