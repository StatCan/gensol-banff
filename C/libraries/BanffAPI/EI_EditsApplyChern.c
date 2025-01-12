#include <stdlib.h>

#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_EditsApplyChern.h"
#include "EI_EditsConsistency.h"

/*
This function tries to answer the question: must we use the chernikova
algorithm to find variables in error. Since chernikova algorithm is very costly
we want to minimize its use.
*/
EIT_EDITSAPPLYCHERN_RETURNCODE EI_EditsApplyChern (
    int MissingFlag, /* Indicates whether there is invalid data in the record*/
    EIT_EDITS * Edits, /* edits of an edit group */
    EIT_DATAREC * Data)/* respondent edit field values */
{
    int i;
    int j;
    int rc;
    EIT_EDITS TempEdits;

    rc = EI_EditsApplyOnValues (Edits, Data->FieldValue);

    if (rc == EIE_EDITSAPPLY_SUCCEED)
        if (!MissingFlag)
            return EIE_EDITSAPPLYCHERN_SUCCEED;
        else /* MissingFlag */
            return EIE_EDITSAPPLYCHERN_FAILNOCHERN;

    if (rc == EIE_EDITSAPPLY_FAIL)
        if (!MissingFlag)
            return EIE_EDITSAPPLYCHERN_FAILDOCHERN;

    /* rc == EIE_EDITSAPPLY_FAIL && MissingFlag */
    EI_EditsDuplicate (&TempEdits, Edits);
    for (j = 0; j < TempEdits.NumberofEquations; j++) {
        for (i = 0; i < TempEdits.NumberofColumns-1; i++) {
            if (Data->StatusFlag[i] == FIELDOK) {
                TempEdits.Coefficient[j][TempEdits.NumberofColumns-1] -=
                    TempEdits.Coefficient[j][i] * Data->FieldValue[i];
                TempEdits.Coefficient[j][i] = 0.0;
            }
        }
    }
    rc = EI_EditsConsistency (&TempEdits);
    EI_EditsFree (&TempEdits);
    if (rc == EIE_EDITSCONSISTENCY_CONSISTENT)
        return EIE_EDITSAPPLYCHERN_FAILNOCHERN;
    else if (rc == EIE_EDITSCONSISTENCY_INCONSISTENT)
        return EIE_EDITSAPPLYCHERN_FAILDOCHERN;

    return EIE_EDITSAPPLYCHERN_FAIL;
}
