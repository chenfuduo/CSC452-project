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
    char stackk[SIZE];
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
    // initialize contexts

    // TODO - Check if in kernel mode
    /*
    if(USLOSS_PsrGet() != 1)
    {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }*/

    int i;
    for(i = 0; i < P1_MAXPROC; i++)
    {
        contexts[i].startFunc = NULL;
        contexts[i].startArg  = NULL;
        contexts[i].isUsed    = 0;
    }
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {

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
            contexts[i].startFunc = func;
            contexts[i].startArg  = arg;
            contexts[i].isUsed    = 1;
            contexts[i].stack     = (char *) malloc(stacksize);
            *cid = i;

            USLOSS_ContextInit(&contexts[i].context, contexts[i].stackk, 
                                sizeof(contexts[i].stackk), P3_AllocatePageTable(i), launch);
            /*
            if(arg == NULL)
            {
                USLOSS_ContextInit(&contexts[i].context, contexts[i].stackk, 
                                sizeof(contexts[i].stackk), P3_AllocatePageTable(i), (void *) func);
            } else 
            {
                USLOSS_ContextInit(&contexts[i].context, contexts[i].stackk, 
                                sizeof(contexts[i].stackk), P3_AllocatePageTable(i), launch);
            }*/
            //USLOSS_ContextInit(&contexts[i].context, contexts[i].stack, 
            //                    sizeof(contexts[i].stack), P3_AllocatePageTable(i), (void *) func);
            result = P1_SUCCESS;
            break;
        }
        i++;
    }
    return result;
}

int P1ContextSwitch(int cid) {
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
    contexts[cid].startFunc = NULL;
    contexts[cid].startArg  = NULL;
    contexts[cid].isUsed    = 0;
    free(contexts[cid].stack);
    P3_FreePageTable(cid);
    return P1_SUCCESS;
}


void 
P1EnableInterrupts(void) 
{
    // set the interrupt bit in the PSR
    int temp = USLOSS_PsrSet(USLOSS_PsrGet() & 0xd);
    assert(temp == USLOSS_DEV_OK);
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
    int enabled = FALSE;
    unsigned int psr = USLOSS_PsrGet();
    // set enabled to TRUE if interrupts are already enabled
    if(psr & 0x2)
    {
        enabled = TRUE;
    }
    // clear the interrupt bit in the PSR
    int temp = USLOSS_PsrSet(psr & 0xd);
    assert(temp == USLOSS_DEV_OK);

    return enabled;
}
