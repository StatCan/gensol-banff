/* contient toutes les fonctions reliees au stack */

/*
rene
Wish list:
cree EI_StackSimplify () qui simplifie le stack c est a dire
  calculer les valeurs constante une fois seulement 1+2->3
  enlever + 0
  enlever * 1
  enlever ^ 1
  et plus
*/

/* rene */
/*#include <errno.h>*/
#include <signal.h>


#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Message.h"
#include "EI_Stack.h"
#include "EI_Symbol.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "util.h"
#include "MessageBanffAPI.h"

static EIT_STACKEVALUATE_RETURNCODE Evaluate (EIT_STACK *, double[], double *);
static char * OperationTranslator (EIT_OPERATION);

/* debug */
enum {DEBUG = 0};

/*********************************************************************
Allocate and Initialize New Stack
*********************************************************************/
EIT_STACK * EI_StackAllocate (void) {
    EIT_STACK * Stack;

    Stack = STC_AllocateMemory (sizeof *Stack);
    if (Stack == NULL) return NULL;

    Stack->NumberAllocatedEntries = 1;
    Stack->NumberEntries = 0;
    Stack->Entry = STC_AllocateMemory (sizeof *Stack->Entry);
    if (Stack->Entry == NULL) return NULL;

    return Stack;
}
/*********************************************************************
check if all aux variable of formula was replace by a real variable
*********************************************************************/
EIT_BOOLEAN EI_StackAreSymbolsMissing (
    EIT_STACK * Stack)
{
    int i;

    for (i = 0; i < Stack->NumberEntries; i++)
        if (Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL &&
               Stack->Entry[i].Symbol == NULL)
            return EIE_TRUE;

    return EIE_FALSE;
}
/*********************************************************************
Print stack entry (debug function).
*********************************************************************/
void EI_StackEntryPrint (
    EIT_STACK_ENTRY * Entry)
{
    printf ("Operation=%s ",
        OperationTranslator (Entry->Operation));
    if (Entry->Operation == EIE_OPERATION_PUSH)
        printf ("Value=%g\n", Entry->Value);
    else if (Entry->Operation == EIE_OPERATION_PUSHSYMBOL) {
        printf ("PlaceHolder=%s ", Entry->PlaceHolder);
        printf ("Period=%d ", Entry->Period);
        printf ("Aggregate=%d\n", Entry->Aggregate);
        if (Entry->Symbol != NULL) {
            printf (" ");
            EI_SymbolPrint (Entry->Symbol);
        }
    }
    else
        printf ("\n");
}

/* rene: trap pow() errors */
/*void allo (int sig) {*/
/*    printf ("\nentre dans allo\nsig=%d\n", sig); fflush (NULL);*/
/*    signal (sig, &allo);*/
/*    printf ("sort de allo\n"); fflush (NULL);*/
/*    errno=ERANGE;*/
/*}*/
/*********************************************************************
Evaluate Estimator Function Stack
returns the stack value
wrapper to trap floating point exception error
*********************************************************************/
EIT_STACKEVALUATE_RETURNCODE EI_StackEvaluate (
    EIT_STACK * Stack,
    double * Result)
{
/*    void (*AFunction)(int);*/
    double * Expr;
    EIT_STACKEVALUATE_RETURNCODE rc;

    Expr = STC_AllocateMemory (Stack->NumberEntries * sizeof *Expr);
    if (Expr == NULL) return EIE_STACKEVALUATE_FAIL;

/* rene: signal handling not done in Evaluate */
/*    AFunction = signal (SIGFPE, &allo);*/
    rc = Evaluate (Stack, Expr, Result);
/*    signal (SIGFPE, AFunction);*/

    STC_FreeMemory (Expr);

    return rc;
}
/*********************************************************************
Free Stack
*********************************************************************/
void EI_StackFree (
    EIT_STACK * Stack)
{
    int i;

    if (Stack == NULL) return;

    for (i = 0; i < Stack->NumberEntries; i++)
        if (Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL)
            STC_FreeMemory (Stack->Entry[i].PlaceHolder);
    STC_FreeMemory (Stack->Entry);
    STC_FreeMemory (Stack);
}
/*********************************************************************
Print stack (debug function)
*********************************************************************/
void EI_StackPrint (
    EIT_STACK * Stack)
{
    int i;

    printf ("NumberEntries=%d NumberAllocatedEntries=%d\n",
        Stack->NumberEntries, Stack->NumberAllocatedEntries);

    for (i = 0; i < Stack->NumberEntries; i++)
        EI_StackEntryPrint (&Stack->Entry[i]);
}
/*********************************************************************
Push an item to stack.
*********************************************************************/
EIT_RETURNCODE EI_StackPush (
    EIT_STACK * Stack,
    EIT_STACK_ENTRY * Entry)
{
    assert (0 <= Entry->Operation && Entry->Operation < EIE_OPERATION_INVALID);

    if (Stack->NumberEntries == Stack->NumberAllocatedEntries) {
        /* if not enough space, double the stack size */
        EIT_STACK_ENTRY * NewEntry;

        NewEntry = STC_ReallocateMemory (
            sizeof *NewEntry * Stack->NumberAllocatedEntries,
            sizeof *NewEntry * Stack->NumberAllocatedEntries * 2,
            Stack->Entry);
        if (NewEntry == NULL) return EIE_FAIL;

        Stack->Entry = NewEntry;
        Stack->NumberAllocatedEntries *= 2;
    }

    Stack->Entry[Stack->NumberEntries] = *Entry;

    Stack->NumberEntries++;

    return EIE_SUCCEED;
}
/*********************************************************************
Set the Symbol in Stack that is identified by PlaceHolder
with the Symbol in SymbolTable that is identified by Name.
Must find all occurences of PlaceHolder.
returns -1 if no match are found.
*********************************************************************/
EIT_RETURNCODE EI_StackSetSymbols (
    EIT_STACK * Stack,
    char * PlaceHolder,
    EIT_SYMBOL ** SymbolTable,
    char * Name)
{
    int i;
    EIT_RETURNCODE rc;

    rc = EIE_FAIL;
    for (i = 0; i < Stack->NumberEntries; i++) {
        if (Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL) {
            if (strcmp (Stack->Entry[i].PlaceHolder, PlaceHolder) == 0) {
                rc = EIE_SUCCEED;
                Stack->Entry[i].Symbol = EI_SymbolTableLookup (SymbolTable,
                    Name, Stack->Entry[i].Period, Stack->Entry[i].Aggregate,
                    EIM_MISSING_VALUE, EIE_SYMBOL_LOOKUPTYPE_CREATE);
            }
        }
    }

    return rc;
}

/*********************************************************************
Evaluate Estimator Function Stack
returns the stack value
returns HUGE_VAL when something goes terribly wrong!
plus a message is added to the message list.
*********************************************************************/
static EIT_STACKEVALUATE_RETURNCODE Evaluate (
    EIT_STACK * Stack,
    double Expr[],
    double * Value)
{
    int i;
    double Left;
    double Right;
    int StackIndex;

    *Value = DBL_MAX;

    if (DEBUG) EI_StackPrint (Stack);
    for (i=0, StackIndex=0; StackIndex < Stack->NumberEntries; StackIndex++) {
        switch (Stack->Entry[StackIndex].Operation) {
        case EIE_OPERATION_ADD:
            Right = Expr[--i];
            Left = Expr[--i];
            if (DEBUG) printf ("%g + %g\n", Left, Right);
            Expr[i++] = Left + Right;
            break;

        case EIE_OPERATION_DIVIDE:
            Right = Expr[--i];
            Left = Expr[--i];
            if (EIM_DBL_EQ (Right, 0.0) == 1)
                return EIE_STACKEVALUATE_DIVISIONBYZERO;
            if (DEBUG) printf ("%g / %g\n", Left, Right);
            Expr[i++] = Left / Right;
            break;

        case EIE_OPERATION_MULTIPLY:
            Right = Expr[--i];
            Left = Expr[--i];
            if (DEBUG) printf ("%g * %g\n", Left, Right);
            Expr[i++] = Left * Right;
            break;

        case EIE_OPERATION_NEGATE:
            if (DEBUG) printf ("Negating %g\n", Expr[i-1]);
            Expr[i-1] *= -1.0;
            break;

        case EIE_OPERATION_POWER:
            Right = Expr[--i];
            Left = Expr[--i];
            if (DEBUG) printf ("%g ^ %g\n", Left, Right);
/*printf ("\navant pow()\n"); fflush (NULL);*/
/*            errno = 0;*/
            Expr[i++] = pow (Left, Right);
/*printf ("\napres pow()\n"); fflush (NULL);*/
/*            if (errno != 0) {*/
/*                printf ("\nerrno != 0 %d (%s)\n", errno, strerror(errno));*/
/*                EI_AddMessage ("Evaluation", EIE_ERROR, strerror(errno));*/
/*                return EIE_STACKEVALUATE_FAIL;*/
/*            }*/
/*            else {*/
/*                printf ("\nerrno == 0\n", errno);*/
/*            }*/
            break;

        case EIE_OPERATION_PUSH:
            if (DEBUG) printf ("constant %g\n", Stack->Entry[StackIndex].Value);
            Expr[i++] = Stack->Entry[StackIndex].Value;
            break;

        case EIE_OPERATION_PUSHSYMBOL:
            if (Stack->Entry[StackIndex].Symbol == NULL) {
                EI_AddMessage ("EI_StackEvaluate()", EIE_ERROR,
                    M30104 "\n"); /* A variable has no value. */
                return EIE_STACKEVALUATE_FAIL;
            }
            if (DEBUG) EI_SymbolPrint (Stack->Entry[StackIndex].Symbol);
            Expr[i++] = Stack->Entry[StackIndex].Symbol->Value;
            break;

        case EIE_OPERATION_SUBTRACT:
            Right = Expr[--i];
            Left = Expr[--i];
            if (DEBUG) printf ("%g - %g\n", Left, Right);
            Expr[i++] = Left - Right;
            break;

        default:
            /* will never happen: was trapped earlier */
            puts ("EI_StackEvaluate (): Bad operation!");
            EI_AddMessage ("EI_StackEvaluate()", EIE_ERROR,
                M30082 "\n"); /* Illegal operation. */
            return EIE_STACKEVALUATE_FAIL;
        }

        /* when the Number is close to zero set it to zero */
        if (EIM_DBL_EQ (Expr[i-1], 0.0)) Expr[i-1] = 0.0;

        if (DEBUG) printf ("Expr[%d]\t=%g\n", i-1, Expr[i-1]);
    }

    *Value = Expr[0];

    return EIE_STACKEVALUATE_SUCCEED;
}
/***********************************************************************
returns a description for an operator.
***********************************************************************/
static char * OperationTranslator (
    EIT_OPERATION Operation)
{
    assert (0 <= Operation && Operation < EIE_OPERATION_INVALID);

#define CASE(X) case(X):return(#X);

    switch (Operation) {
    CASE (EIE_OPERATION_ADD)
    CASE (EIE_OPERATION_DIVIDE)
    CASE (EIE_OPERATION_MULTIPLY)
    CASE (EIE_OPERATION_NEGATE)
    CASE (EIE_OPERATION_POWER)
    CASE (EIE_OPERATION_PUSH)
    CASE (EIE_OPERATION_PUSHSYMBOL)
    CASE (EIE_OPERATION_SUBTRACT)
    default:
        puts ("OperationTranslator (): Unknown Operation"); fflush (NULL);
        exit (-1);
    }
    return "Will never be here!";
}
