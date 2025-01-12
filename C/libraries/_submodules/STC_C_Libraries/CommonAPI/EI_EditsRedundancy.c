#ifndef EDITS_DISABLED
#include <stdlib.h>

#include "EI_Edits.h"
#include "EI_EditsRedundancy.h"
#include "EI_Message.h"

#include "EditsAnalysis.h"
#include "EqualityMatrix.h"
#include "util.h"

#include "MessageCommonAPI.h"

/*Return all non-redundant edits.
*/
EIT_EDITSREDUNDANCY_RETURNCODE EI_EditsRedundancy (EIT_EDITS *Edits)
{
    tEditsAnalysisStatistics EditsAnalysisStatistics;
    tEqualityMatrix EqualityMatrix;

    /*  int MaximumNumberOfIterations;*/
    /* This is currently not used.
         We could also use a time constraint.
         We would then return an error upon violation.
      */
    /*  MaximumNumberOfIterations = 10 * (Edits->NumberofEquations +*/
    /*                    Edits->NumberofColumns);*/

    /* Initialize statistics */
    EditsAnalysisStatisticsInit (&EditsAnalysisStatistics);

    EditsAnalysisStatistics.NbEqualitiesRead =
        Edits->NumberofEquations - Edits->NumberofInequalities;
    EditsAnalysisStatistics.NbInequalitiesRead = Edits->NumberofInequalities;
    EditsAnalysisStatistics.NbVariables = Edits->NumberofColumns - 1;


    /*
      First make the Equality Analysis
      */
    if (EqualityMatrixAllocate(Edits->NumberofColumns-1, &EqualityMatrix) == -1){
        return EIE_EDITSREDUNDANCY_FAIL;
    }
    EqualityAnalysis (Edits, &EditsAnalysisStatistics, &EqualityMatrix,
        NULL); /*the NULL silences printing*/


    if (EditsAnalysisStatistics.DeterminancyFlag != 0) {
        /* All inequalities are redundant */
        while (Edits->NumberofInequalities > 0) {
            EditsAnalysisStatistics.NbRedundantInequalities++;
            /* Edits.NumberofInequalities decremented */
            EI_EditsDropEquation (0, Edits);
        }
    }
    if (EditsAnalysisStatistics.InconsistencyFlag != 0) {
        /* Some inconsistency has been met    */
        while (Edits->NumberofInequalities > 0) {
            /* Edits.NumberofInequalities decremented */
            EI_EditsDropEquation (0, Edits);
        }
        EqualityMatrixFree (&EqualityMatrix);
        return EIE_EDITSREDUNDANCY_INCONSISTENT;
    }
    else {
        /* Perform the Inequality Analysis */

        /*Silence printing with NULL parameter:*/
            if (InequalityAnalysis(Edits, &EditsAnalysisStatistics,
                                   &EqualityMatrix, NULL) != EIE_SUCCEED) {
                EI_AddMessage("EI_EditsRedundancy", EIE_ERROR,
                    M30010, "InequalityAnalysis()"); /* %s failed. */
                return EIE_EDITSREDUNDANCY_FAIL;
            }


        if (EditsAnalysisStatistics.InconsistencyFlag != 0) {
            /* Some inconsistency has been met */
            EqualityMatrixFree (&EqualityMatrix);
            return EIE_EDITSREDUNDANCY_INCONSISTENT;
        }
    }
    EqualityMatrixFree (&EqualityMatrix);
    return EIE_EDITSREDUNDANCY_SUCCESS;
}
#endif
