#ifndef EI_STACK_H
#define EI_STACK_H

#include "EI_Common.h"
#include "EI_Symbol.h"

enum EIT_STACKEVALUATE_RETURNCODE {
    EIE_STACKEVALUATE_FAIL,
    EIE_STACKEVALUATE_DIVISIONBYZERO,
    EIE_STACKEVALUATE_SUCCEED
};
typedef enum EIT_STACKEVALUATE_RETURNCODE EIT_STACKEVALUATE_RETURNCODE;

enum EIT_OPERATION {
    EIE_OPERATION_ADD,
    EIE_OPERATION_DIVIDE,
    EIE_OPERATION_MULTIPLY,
    EIE_OPERATION_NEGATE,
    EIE_OPERATION_POWER,
    EIE_OPERATION_PUSH,
    EIE_OPERATION_PUSHSYMBOL,
    EIE_OPERATION_SUBTRACT,
    EIE_OPERATION_INVALID
};
typedef enum EIT_OPERATION EIT_OPERATION;

struct EIT_STACK_ENTRY {
    int Operation;
    char * PlaceHolder;
    EIT_PERIOD Period; /* rene redundant? */
    EIT_AGGREGATE Aggregate; /* rene redundant? */
    double Value; /* rene redundant? */
    EIT_SYMBOL * Symbol;
};
typedef struct EIT_STACK_ENTRY EIT_STACK_ENTRY;

struct EIT_STACK {
    int NumberAllocatedEntries;
    int NumberEntries;
    EIT_STACK_ENTRY * Entry;
};
typedef struct EIT_STACK EIT_STACK;

CLASS_DECLSPEC EIT_STACK * EI_StackAllocate (void);
CLASS_DECLSPEC EIT_BOOLEAN EI_StackAreSymbolsMissing (EIT_STACK *);
CLASS_DECLSPEC void EI_StackEntryPrint (EIT_STACK_ENTRY *);
CLASS_DECLSPEC EIT_STACKEVALUATE_RETURNCODE EI_StackEvaluate (EIT_STACK *, double *);
CLASS_DECLSPEC void EI_StackFree (EIT_STACK *);
CLASS_DECLSPEC void EI_StackPrint (EIT_STACK *);
CLASS_DECLSPEC EIT_RETURNCODE EI_StackPush (EIT_STACK *, EIT_STACK_ENTRY *);
CLASS_DECLSPEC EIT_RETURNCODE EI_StackSetSymbols (EIT_STACK *, char *, EIT_SYMBOL **,
    char *);

#endif
