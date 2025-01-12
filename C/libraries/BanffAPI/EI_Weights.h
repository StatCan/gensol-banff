#ifndef EI_WEIGHTS_H
#define EI_WEIGHTS_H

#include "EI_Common.h"
#include "EI_Edits.h"

struct WeightsStructure {
    char   **FieldName;
    double  *Value;
    int      NumberOfFields;
};
typedef struct WeightsStructure EIT_WEIGHTS;

CLASS_DECLSPEC int EI_WeightsAllocate (EIT_EDITS *, EIT_WEIGHTS *);

CLASS_DECLSPEC void EI_WeightsFree (EIT_WEIGHTS *);

CLASS_DECLSPEC int EI_WeightsParse (char *, EIT_WEIGHTS *);

CLASS_DECLSPEC void EI_WeightsPrint (EIT_WEIGHTS *);

#endif
