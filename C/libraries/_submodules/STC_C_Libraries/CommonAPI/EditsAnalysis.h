/* --------------------------------------------------------------------
This header file contains the local definition to EDIT ANALYSIS
It includes 1) structure definition
            2) function prototypes
---------------------------------------------------------------------*/
#ifndef EDITSANALYSIS_H
#ifndef EDITS_DISABLED
#define EDITSANALYSIS_H

#include "EI_Common.h"
#include "EI_Edits.h"

#include "EqualityMatrix.h"

struct EditsAnalysisStatisticsStruct {
    int NbEqualitiesRead;           /* # of equalities read         */
    int NbInequalitiesRead;         /* # of inequalities read       */

    int NbEqualities;               /* # of equalities              */
    int NbInequalities;             /* # of inequalities            */
    int NbHiddenEqualities;         /* # of hidden equalities       */

    int NbRedundantEqualities;      /* # of redundant equalitites   */
    int NbRedundantInequalities;    /* # of redundant inequalities  */
    int NbRedundantHiddenEqualities;/* # of redundant hidden equal. */
    int NbNoRestriction;            /* # of edits which do not      */
                                    /*   restrict feasible region   */

    int NbVariables;                /* # of variables               */
    int NbUnboundVariables;         /* # of unbounded variables     */
    int NbDeterministicVariables;   /* # of deterministic variables */

    int InconsistencyFlag;          /* flag of inconsistency        */
    int DeterminancyFlag;           /* flag of determinancy         */
};
typedef struct EditsAnalysisStatisticsStruct tEditsAnalysisStatistics;

extern void EditsAnalysisStatisticsInit (tEditsAnalysisStatistics *);
extern void EditsAnalysisStatisticsPrint (tEditsAnalysisStatistics *);
extern void EqualityAnalysis (EIT_EDITS *, tEditsAnalysisStatistics *,
    tEqualityMatrix *, EIT_PRINTF);
extern EIT_RETURNCODE InequalityAnalysis (EIT_EDITS *,
    tEditsAnalysisStatistics *, tEqualityMatrix *, EIT_PRINTF);
#endif
#endif
