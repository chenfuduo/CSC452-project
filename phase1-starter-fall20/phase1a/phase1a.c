#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


#define SIZE (USLOSS_MIN_STACK)


extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    USLOSS_Context  context;
    // you'll need more stuff here
    char *stack;
    int isUsed;
} Context;

static Context   contexts[P1_MAXPROC];

static int currentCid = -1;


/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void)
{
    assert(contexts[currentCid].startFunc != NULL);
    contexts[currentCid].startFunc(contexts[currentCid].startArg);
}

void P1ContextInit(void)
{
    // Check if in kernel mode
    int isKernel = USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE;
    if(!isKernel)
    {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }

    // initialize contexts
    for(int i = 0; i < P1_MAXPROC; i++)
    {
        contexts[i].startFunc = NULL;
        contexts[i].startArg  = NULL;
        contexts[i].isUsed    = 0;
    }
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {

    // Check if in kernel mode
    int isKernel = USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE;
    if(!isKernel)
    {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }

    // Check stack size
    if(stacksize < USLOSS_MIN_STACK)
    {
        return P1_INVALID_STACK;
    }

    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    int result = P1_TOO_MANY_CONTEXTS;
    int i = 0;
    while(i < P1_MAXPROC)
    {
        // Check if free
        if(!contexts[i].isUsed)
        {
            // Allocate the stack and check allocation
            char *newStack = (char *) malloc(stacksize);
            if(newStack == NULL)
            {
                printf("Memory was not allocated successfully.\n");
                exit(1);
            }

            // Set the values of the context
            contexts[i].startFunc = func;
            contexts[i].startArg  = arg;
            contexts[i].stack     = newStack;
            contexts[i].isUsed    = 1;
            *cid = i;

            // initialize the context
            USLOSS_ContextInit(&contexts[i].context, contexts[i].stack, 
                                stacksize, P3_AllocatePageTable(i), launch);
            
            result = P1_SUCCESS;
            break;
        }
        i++;
    }
    return result;
}


int P1ContextSwitch(int cid) {

    // Check if in kernel mode
    int isKernel = USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE;
    if(!isKernel)
    {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }


    // Check cid is valid
    if(cid < 0 || cid > P1_MAXPROC)
    {
        return P1_INVALID_CID;
    }

    // Check context exists
    if(!contexts[cid].isUsed)
    {
        return P1_INVALID_CID;
    }

    // switch to the specified context
    int prevId = currentCid;
    currentCid = cid;

    if(prevId == -1)
    {
        USLOSS_ContextSwitch(NULL, &contexts[cid].context);
    } else {
        USLOSS_ContextSwitch(&contexts[prevId].context, &contexts[cid].context);
    }

    return P1_SUCCESS;
}

int P1ContextFree(int cid) {

    // Check if in kernel mode
    int isKernel = USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE;
    if(!isKernel)
    {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }


    // Check cid is valid
    if(cid < 0 || cid > P1_MAXPROC)
    {
        return P1_INVALID_CID;
    }

    // Check context exists
    if(!contexts[cid].isUsed)
    {
        return P1_INVALID_CID;
    }

    // Check if context is currently running
    if(cid == currentCid)
    {
        return P1_CONTEXT_IN_USE;
    }


    // free the stack and mark the context as unused
    free(contexts[cid].stack);
    contexts[cid].startFunc = NULL;
    contexts[cid].startArg  = NULL;
    contexts[cid].isUsed    = 0;
    P3_FreePageTable(cid);
    return P1_SUCCESS;
}


void 
P1EnableInterrupts(void) 
{
    // Check if in kernel mode
    int isKernel = USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE;
    if(!isKernel)
    {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }


    // set the interrupt bit in the PSR
    int temp = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    assert(temp == USLOSS_DEV_OK);
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{

    // Check if in kernel mode
    int isKernel = USLOSS_PsrGet() & 0x1;
    if(!isKernel)
    {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }


    int enabled = FALSE;
    unsigned int psr = USLOSS_PsrGet();
    // set enabled to TRUE if interrupts are already enabled
    if(psr & USLOSS_PSR_CURRENT_INT)
    {
        enabled = TRUE;
    }
    // clear the interrupt bit in the PSR
    int temp = USLOSS_PsrSet(psr & ~USLOSS_PSR_CURRENT_INT);
    assert(temp == USLOSS_DEV_OK);

    return enabled;
}