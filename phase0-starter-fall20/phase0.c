/********************************************************
 *  Assignment: Phase 0: USLOSS Hello World!
 *      Author: Mutab Alqadda
 *
 *      Course: CSC 452
 *  Instructor: Chris Gniady
 *         TAs: Md Ashiqur Rahman, Chavoosh Ghasemi
 *    Due Date: Sep 9, 2020
 *
 * Description: This is a "Hello World!" program in C
 *              using the USLOSS library.
 ********************************************************/

#include <stdio.h>
#include "usloss.h"


#define SIZE (USLOSS_MIN_STACK)


USLOSS_Context context0;
USLOSS_Context context1;

char stack[2][SIZE];

  // This function serves as the starting address of the
  // Hello context.
void hello(void)
{
    int i;
    for(i = 1; i <= 10; i++)
    {
        USLOSS_Console("%d Hello ", i);
        USLOSS_ContextSwitch(&context0, &context1);
    }

    USLOSS_Halt(0);
}

  // This function serves as the starting address of the
  // World context.
void world(void)
{
    int i;
    for(i = 1; i <= 10; i++)
    {
        USLOSS_Console("World");

          // Print '!' a number of times equal to
          // to the iteration number.
        for(int j = 0; j < i; j++)
        {
            USLOSS_Console("!");
        }
        USLOSS_Console("\n");

        // Switch to the Hello context
        USLOSS_ContextSwitch(&context1, &context0);
    }
}


void
startup(int argc, char **argv)
{
    /*
     * Your code here. If you compile and run this as-is you
     * will get a simulator trap, which is a good opportunity
     * to verify that you get a core file and you can use it
     * with gdb.
     */

    // Initializing the two contexts
    USLOSS_ContextInit(&context0, stack[0], sizeof(stack[0]), NULL, hello);
    USLOSS_ContextInit(&context1, stack[1], sizeof(stack[1]), NULL, world);

    // Switching to the first context
    USLOSS_ContextSwitch(NULL, &context0);
}

// Do not modify anything below this line.

void
finish(int argc, char **argv)
{
    USLOSS_Console("Goodbye.\n");
}

void
test_setup(int argc, char **argv)
{
    // Do nothing.
}

void
test_cleanup(int argc, char **argv)
{
    // Do nothing.
}

