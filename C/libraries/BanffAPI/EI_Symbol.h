#ifndef EI_SYMBOL_H
#define EI_SYMBOL_H

#include "EI_Common.h"

enum SYMBOL_LOOKUPTYPE {
    EIE_SYMBOL_LOOKUPTYPE_CREATE,
    EIE_SYMBOL_LOOKUPTYPE_FIND
};
typedef enum SYMBOL_LOOKUPTYPE EIT_SYMBOL_LOOKUPTYPE;

struct SYMBOL {
    char * Name;
    EIT_PERIOD Period;
    EIT_AGGREGATE Aggregate;
    double Value;
    struct SYMBOL * Next;
};
typedef struct SYMBOL EIT_SYMBOL;

CLASS_DECLSPEC void EI_SymbolPrint (EIT_SYMBOL *);
CLASS_DECLSPEC EIT_SYMBOL ** EI_SymbolTableAllocate (void);
CLASS_DECLSPEC void EI_SymbolTableFree (EIT_SYMBOL **);
CLASS_DECLSPEC int EI_SymbolTableGetSize (EIT_SYMBOL **);
CLASS_DECLSPEC EIT_SYMBOL * EI_SymbolTableLookup (EIT_SYMBOL **, char *, EIT_PERIOD,
    EIT_AGGREGATE, double, EIT_SYMBOL_LOOKUPTYPE);
CLASS_DECLSPEC void EI_SymbolTablePrint (EIT_SYMBOL **);

#endif
