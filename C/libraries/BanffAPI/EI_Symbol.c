/* contient toutes les fonctions reliees au table de symboles */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Symbol.h"

#include "STC_Memory.h"

#include "util.h"

enum {
    SYMBOL_TABLE_SIZE = 53 /* a prime number */
};

/*********************************************************************
Prints symbol table.
*********************************************************************/
void EI_SymbolPrint (
    EIT_SYMBOL * Symbol)
{
    printf ("Variable=%-5s Period=%d Aggregate=%d Value=%f\n",
        Symbol->Name, Symbol->Period, Symbol->Aggregate, Symbol->Value);
}
/*********************************************************************
Allocates a symbol table.
*********************************************************************/
EIT_SYMBOL ** EI_SymbolTableAllocate (void)
{
    int i;
    EIT_SYMBOL ** SymbolTable;

    SymbolTable = STC_AllocateMemory (sizeof *SymbolTable * SYMBOL_TABLE_SIZE);
    for (i = 0; i < SYMBOL_TABLE_SIZE; i++)
        SymbolTable[i] = NULL;

    return SymbolTable;
}
/*********************************************************************
Frees a symbol table.
*********************************************************************/
void EI_SymbolTableFree (
    EIT_SYMBOL ** SymbolTable)
{
    int i;
    EIT_SYMBOL * Symbol;
    EIT_SYMBOL * SymbolNext;

    for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        for (Symbol = SymbolTable[i]; Symbol != NULL; ) {
            SymbolNext = Symbol->Next;
            STC_FreeMemory (Symbol->Name);
            STC_FreeMemory (Symbol);
            Symbol = SymbolNext;
        }
    }
    STC_FreeMemory (SymbolTable);
}
/*********************************************************************
Return the size of the symbol table.
*********************************************************************/
int EI_SymbolTableGetSize (
    EIT_SYMBOL ** SymbolTable) /* not used at this point */
{
    return SYMBOL_TABLE_SIZE;
}
/*********************************************************************
Find name in symbol table.
with optional create.
*********************************************************************/
EIT_SYMBOL * EI_SymbolTableLookup (
    EIT_SYMBOL ** SymbolTable,
    char * Name,
    EIT_PERIOD Period,
    EIT_AGGREGATE Aggregate,
    double Value,
    EIT_SYMBOL_LOOKUPTYPE LookupType)
{
    int h;
    EIT_SYMBOL * Symbol;

    h = UTIL_Hash (Name, SYMBOL_TABLE_SIZE);

    for (Symbol = SymbolTable[h]; Symbol != NULL; Symbol = Symbol->Next)
        if (strcmp (Symbol->Name, Name) == 0 &&
                Symbol->Period == Period &&
                Symbol->Aggregate == Aggregate)
            return Symbol;

    Symbol = NULL;
    if (LookupType == EIE_SYMBOL_LOOKUPTYPE_CREATE) {
        Symbol = STC_AllocateMemory (sizeof *Symbol);
        Symbol->Name      = STC_StrDup (Name);
        Symbol->Period    = Period;
        Symbol->Aggregate = Aggregate;
        Symbol->Value     = Value;
        Symbol->Next      = SymbolTable[h];
        SymbolTable[h]    = Symbol;
    }

    return Symbol;
}
/*********************************************************************
Prints symbol table.
*********************************************************************/
void EI_SymbolTablePrint (
    EIT_SYMBOL ** SymbolTable)
{
    int i;
    EIT_SYMBOL * Symbol;

    for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        for (Symbol = SymbolTable[i]; Symbol != NULL; Symbol = Symbol->Next) {
            printf ("hash=%d ", i);
            EI_SymbolPrint (Symbol);
        }
    }
}
