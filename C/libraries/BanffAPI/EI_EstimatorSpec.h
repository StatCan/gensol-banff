#ifndef EI_ESTIMATORSPEC_H
#define EI_ESTIMATORSPEC_H

#include "EI_AcceptableKeysResult.h"
#include "EI_Algorithm.h"
#include "EI_Average.h"
#include "EI_Common.h"
#include "EI_RandomErrorResult.h"
#include "EI_Regression.h"
#include "EI_Residual.h"
#include "EI_Stack.h"

#define EIM_ESTIM_PERIOD_CURRENT_LETTER       'C'
#define EIM_ESTIM_PERIOD_HISTORICAL_LETTER    'H'
#define EIM_ESTIM_PERIOD_CURRENT_STRING       "C"
#define EIM_ESTIM_PERIOD_HISTORICAL_STRING    "H"
#define EIM_ESTIM_PERIOD_NOT_SPECIFIED_STRING ""

#define EIM_ESTIM_COUNTCRITERIA_NOT_SPECIFIED   (-1)
#define EIM_ESTIM_PERCENTCRITERIA_NOT_SPECIFIED (-1.0)

struct EIT_LATERALVARIABLE {
    int NumberOfVariables;
    int * Index;
    int * Period;
};
typedef struct EIT_LATERALVARIABLE EIT_LATERALVARIABLE;

struct EIT_ESTIMATORSPEC {
    char * AlgorithmName;
    char * FieldId;
    char * AuxVariables;
    char * WeightVariable;
    char * VarianceVariable;
    double VarianceExponent;
    char * VariancePeriod; /* use EIM_ESTIM_PERIOD_CURRENT_STRING or */
                           /* EIM_ESTIM_PERIOD_HISTORICAL_STRING     */
    char * ExcludeImputed;
    char * ExcludeOutliers;
    char * RandomError;
    int CountCriteria;
    double PercentCriteria;

    EIT_BOOLEAN Active;

    int FieldIdIndex; /* Index of FieldId in Data DataRec */

    EIT_LATERALVARIABLE LateralVariable;

    EIT_ALGORITHM * Algorithm; /* points to an item in the EIT_ALGORITHM_LIST */
                               /* never free this pointer */

    EIT_AVERAGE * Average; /* link list of averages */

    EIT_REGRESSION * Regression;

    EIT_STACK * Stack;

    /* empty Residual, RandomErrorResult and 
       AcceptableKeysResult in EI_Reactivate() */
    EIT_RESIDUAL * Residual;
    EIT_RANDOMERRORRESULT_LIST * RandomErrorResult;
    EIT_ACCEPTABLEKEYSRESULT_LIST * AcceptableKeysResult;

    /* initialize counters in EI_Reactivate() */
    int FTI;
                            /* # successfull imputations    */
    int Imputed;
                            /* # failed imputations due to  */
    int DivisionByZero;     /* Division by Zero error       */
    int NegativeImputation; /* Negative imputed values      */
};
typedef struct EIT_ESTIMATORSPEC EIT_ESTIMATORSPEC;

struct EIT_ESTIMATORSPEC_LIST {
    int NumberAllocatedEntries;
    int NumberEntries;
    int ExclVarSpecified[2]; /* 2 periods */
    EIT_ESTIMATORSPEC * Estimator; /* array of estimators */
};
typedef struct EIT_ESTIMATORSPEC_LIST EIT_ESTIMATORSPEC_LIST;

CLASS_DECLSPEC EIT_RETURNCODE EI_EstimatorAdd (EIT_ESTIMATORSPEC_LIST *, char *, char *,
    char *, char *, char *, double, char *, char *, char *, char *, int,
    double);
CLASS_DECLSPEC EIT_ESTIMATORSPEC_LIST * EI_EstimatorAllocate (void);
CLASS_DECLSPEC void EI_EstimatorFree (EIT_ESTIMATORSPEC_LIST *);
CLASS_DECLSPEC void EI_EstimatorListPrint (EIT_ESTIMATORSPEC_LIST *);
CLASS_DECLSPEC void EI_EstimatorSpecPrint (EIT_ESTIMATORSPEC *);

#endif
