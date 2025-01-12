#ifndef EI_RANDOMERRORRESULT_H
#define EI_RANDOMERRORRESULT_H

#include "EI_Common.h"

struct RANDOMERRORRESULT {
    char * Recipient;
    char * Donor;
    char * FieldId;
    double Residual;
    double RandomError;
    double OriginalValue;
    double ImputedValue;
};
typedef struct RANDOMERRORRESULT EIT_RANDOMERRORRESULT;

struct RANDOMERRORRESULT_LIST {
    int NumberAllocatedEntries;
    int NumberEntries;
    EIT_RANDOMERRORRESULT * RandomErrorResult;
};
typedef struct RANDOMERRORRESULT_LIST EIT_RANDOMERRORRESULT_LIST;

CLASS_DECLSPEC EIT_RANDOMERRORRESULT_LIST * EI_RandomErrorResultAdd (
    EIT_RANDOMERRORRESULT_LIST *, char *, char *, char *, double, double,
    double, double);
CLASS_DECLSPEC EIT_RANDOMERRORRESULT_LIST * EI_RandomErrorResultAllocate (void);
CLASS_DECLSPEC void EI_RandomErrorResultEmpty (EIT_RANDOMERRORRESULT_LIST *);
CLASS_DECLSPEC void EI_RandomErrorResultFree (EIT_RANDOMERRORRESULT_LIST *);
CLASS_DECLSPEC void EI_RandomErrorResultPrint (EIT_RANDOMERRORRESULT_LIST *);

#endif
