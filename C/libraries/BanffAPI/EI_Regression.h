#ifndef EI_REGRESSION_H
#define EI_REGRESSION_H

#include "EI_Common.h"
#include "EI_DataTable.h"

struct EIT_AUX {
    char * VariableName;
    char * BetaName;
    int AuxIndex;
    EIT_PERIOD Period;
    double ExponentValue;
    struct EIT_AUX * Next;
};
typedef struct EIT_AUX EIT_AUX;

struct EIT_REGRESSION {
    int FieldIdIndex;
    int WeightIndex; /*Period is always current.*/
    int VarianceIndex;
    EIT_PERIOD VariancePeriod;
    double VarianceExponent;
    EIT_BOOLEAN Intercept;
    int NumberOfAux;
    EIT_AUX * Aux;  /*size: NumberOfAux */
    int Count;      /*# of acceptable records.*/
    double * Betas; /*size: NumberOfAux + Intercept */
};
typedef struct EIT_REGRESSION EIT_REGRESSION;
 
CLASS_DECLSPEC void EI_RegressionPrint (EIT_REGRESSION *);
CLASS_DECLSPEC void EI_PrintAux (EIT_REGRESSION *);
CLASS_DECLSPEC void EI_PrintBetas (EIT_REGRESSION *);
CLASS_DECLSPEC EIT_REGRESSION * EI_RegressionAllocate (void);
CLASS_DECLSPEC void EI_RegressionFree (EIT_REGRESSION *);

#endif
