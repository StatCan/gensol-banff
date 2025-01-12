#ifndef EI_RESIDUAL_H
#define EI_RESIDUAL_H

#include "EI_Common.h"
#include "EI_DataTable.h"

#define EIM_RESIDUAL_NOTGOOD -0xabd31

struct EIT_RESIDUAL {
    EIT_BOOLEAN IsWeighted;
    int NumberEntries;
    int NumberGoodEntries;
    int NumberAllocatedEntries;
    double SumOfWeights;
    double * CumulativeWeight;
    EIT_DATA ** Data;
    double * Residual; /* a spot to save a residual for later use. */
};
typedef struct EIT_RESIDUAL EIT_RESIDUAL;

CLASS_DECLSPEC EIT_RESIDUAL * EI_ResidualAdd (EIT_RESIDUAL *, EIT_DATA *, double);
CLASS_DECLSPEC EIT_RESIDUAL * EI_ResidualAllocate (void);
CLASS_DECLSPEC void EI_ResidualEmpty (EIT_RESIDUAL *);
CLASS_DECLSPEC void EI_ResidualFree (EIT_RESIDUAL *);
CLASS_DECLSPEC void EI_ResidualPrint (EIT_RESIDUAL *);
CLASS_DECLSPEC void EI_ResidualPrintStatistics (EIT_RESIDUAL *);
CLASS_DECLSPEC int EI_ResidualSelection (EIT_RESIDUAL *);
CLASS_DECLSPEC void EI_ResidualSetIsWeighted (EIT_RESIDUAL *, EIT_BOOLEAN);
#ifdef NEVERTOBEDEFINED
CLASS_DECLSPEC void EI_ResidualRemove (EIT_RESIDUAL *, int);
#endif

#endif
