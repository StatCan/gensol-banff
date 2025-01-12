/* ------------------------------------------------------------------ */
/*                                                                    */
/* NAME            EI_FindMatchingFields                              */
/*                                                                    */
/* DESCRIPTION     Find the matching fields for each recipient record */
/*                 and identify them in the StatusFlag variable of    */
/*                 the EIT_DATAREC structure variable.                */

/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/*       Include Header Files                                         */
/* ------------------------------------------------------------------ */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Donor.h"
#include "EI_Edits.h"
#include "EI_Message.h"

#include "EIP_Common.h"

#include "Donor.h"
#include "util.h"

#include "LPInterface.h"
#include "MessageBanffAPI.h"

#include "STC_Memory.h"

/*--------------------------------------------------------------------*/
/* Local functions                                                    */
/*--------------------------------------------------------------------*/

static int FindFields (EIT_EDITS *, EIT_DATAREC *, EIT_FIELDNAMES *);
static int GetActiveEdits (EIT_DATAREC *, EIT_EDITS *, EIT_EDITS *);
static int HasOKFields (int, EIT_STATUS_OF_FIELDS *);

/*--------------------------------------------------------------------*/

EIT_MF_RETURNCODE EI_FindMatchingFields (
    EIT_ALL_RECIPIENTS  * AllRecipients,
    EIT_FIELDNAMES      * SystemMatchingFields,
    EIT_EDITS           * Edits)
{
    int hasOKFields;
    int i;
    int rcode;
    EIT_EDITS TempEdits;

    for (i = 0; i < AllRecipients->NumberofRecipients; i++) {

        EI_EditsDuplicate (&TempEdits, Edits);

        /* Identify if Recipient has at least one field OK                */
        /* within the edit group fields.  If there are no OK fields, then */
        /* the function EI_FindMatchFields is not called since there is   */
        /* no chance of finding any valid field that can serve as a       */
        /* matching field.                                                */
        hasOKFields = HasOKFields (TempEdits.NumberofColumns-1,
            AllRecipients->RecipientData[i].StatusFlag);
        if (hasOKFields == 1) {
            rcode = FindFields (&TempEdits, &AllRecipients->RecipientData[i],
                SystemMatchingFields);
            if (rcode == -1) {
                EI_AddMessage ("EI_FindMatchingFields", EIE_ERROR,
                    M30073, /* Error 'Finding matching fields' for 
                               recipient %s. */
                    AllRecipients->Key[i]);
                EI_EditsFree (&TempEdits);
                return EIE_MATCHFIELDS_FAIL;
            }
        }
        EI_EditsFree (&TempEdits);
    }

    return EIE_MATCHFIELDS_SUCCEED;
}


/*--------------------------------------------------------------------*/
/* Find the matching fields for the current recipient.                */
/*--------------------------------------------------------------------*/
static int FindFields (
    EIT_EDITS * Edits,
    EIT_DATAREC * RecipientData,
    EIT_FIELDNAMES * SystemMatchingFields)
{
    int row; /* Edit row counter    */
    int col; /* Edit column counter */
    int i;
    int rc;

    int MaxIterations;
    int MaxTime;
    char ilperrmsg[1024];
    int lpnb = 0; /* Number of the LP problem object*/
    int simplexrc;

    double objval;  /* objective function value */
    double rhsval;  /* Right hand side */
    EIT_EDITS EditsLP;

    /*MaxIterations and MaxTime are actually currently not used by LP software*/
    MaxIterations = 10 * (Edits->NumberofEquations + Edits->NumberofColumns);
    MaxTime = -1; /* Negative maximum time will be ignored */

    /* Because the LP will use only edits not containing the good fields, but containing
        only FTI fields, a copy of Edits is maded for LP. We make a copy because the
        for (col = 0 ...) loop below uses Edits containing FTI and good fields.*/
     
    EI_EditsDuplicate (&EditsLP, Edits);
    GetActiveEdits (RecipientData, Edits, &EditsLP);

    /*  For each inequality, the simplex method           */
    /*  eliminates redundant edits from the set of active */
    /*  edits (i.e. those edits which do not restrict     */
    /*  the feasible region)                              */

    i = 0;
    while (i < Edits->NumberofInequalities) {
        
        /* Creates a problem object */
        lpnb = LPI_CreateProb(ilperrmsg);
        if (lpnb == 0){
            EI_AddMessage("FindFields", EIE_ERROR, ilperrmsg);
            rc = -1;
            goto TERMINATE;
        }

        if (LPI_CallbackSilence(ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage("FindFields", EIE_ERROR, ilperrmsg);
            rc =  -1; /* -1 meaning error*/
            goto TERMINATE;
        }

        /* Populates the problem object */
        if (LPI_MakeMatrix (lpnb, &EditsLP, EditsLP.NumberofEquations, ilperrmsg) !=
                    LPI_SUCCESS) {
            EI_AddMessage ("FindFields", EIE_ERROR, ilperrmsg);
            rc = -1;
            goto TERMINATE;
        }

        /* Take edit as objective function but not the constant */
        /* LPI_SetObjRow(lpnb, i, ilperrmsg); */
        if (LPI_SetObjRow (lpnb, i, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage ("FindFields", EIE_ERROR, ilperrmsg);
            rc = -1;
            goto TERMINATE;
        }

        /* maximize the objective function */
        simplexrc = LPI_MaximizeObj(lpnb, MaxIterations, MaxTime, ilperrmsg);

        if ((simplexrc == LPI_LP_IT_LIM) || (simplexrc == LPI_LP_TIME_LIM) ||
            (simplexrc == LPI_LP_OTHER_ERR)){/* optimization was not completed*/
            EI_AddMessage("FindFields", EIE_ERROR, ilperrmsg);
            rc = -1;
            goto TERMINATE;
        }

        if (simplexrc == LPI_LP_OPT) {
            LPI_GetObjVal(lpnb, &objval, ilperrmsg);
            LPI_GetRhsVal(lpnb, i, &rhsval, ilperrmsg);
        }

        if ((simplexrc == LPI_LP_UNBND) ||
            ((simplexrc == LPI_LP_OPT) && !EIM_DBL_EQ(objval,rhsval))) {

            /* drop inactive edit */
            EI_EditsDropEquation (i, &EditsLP);
            EI_EditsDropEquation (i, Edits);
            /* LPI_DeleteRow(lpnb, i, ilperrmsg); */ 
        }
        else
            i++; /* Edit is not redundant, increment index */

        LPI_DeleteProb (lpnb, ilperrmsg);
        lpnb = 0;
    }

    rc = 0;
    for (col = 0; col < Edits->NumberofColumns-1; col++) {
        for (row = 0; row < Edits->NumberofEquations; row++) {
            if ((Edits->Coefficient[row][col] != 0.0) &&
                (RecipientData->StatusFlag[col] != FIELDFTI)) {

                /* Set status to MATCH FIELD */
                if (RecipientData->StatusFlag[col] == FIELDMFU)
                    RecipientData->StatusFlag[col] = FIELDMFB;
                else {
                    RecipientData->StatusFlag[col] = FIELDMFS;
                    if (SystemMatchingFields->NumberofFields <
                            Edits->NumberofColumns-1) {
                        if (UTIL_Binary_Reverse (RecipientData->FieldName[col],
                                SystemMatchingFields->FieldName,
                                SystemMatchingFields->NumberofFields) == -1) {

                            SystemMatchingFields->FieldName[
                                    SystemMatchingFields->NumberofFields] =
                                STC_StrDup (RecipientData->FieldName[col]);

                            SystemMatchingFields->NumberofFields++;

                            UTIL_SortStrings_D (SystemMatchingFields->FieldName,
                                SystemMatchingFields->NumberofFields);
                        }
                    }
                }
                rc = 1;
                break;
            }
        }
    }

TERMINATE:
    if (lpnb != 0) {
        if (LPI_DeleteProb(lpnb, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage("FindFields", EIE_ERROR, ilperrmsg);
            rc = -1;
        }
    }
    EI_EditsFree (&EditsLP);
    return rc;
}

/*--------------------------------------------------------------------*/
/* Get the set of active edits.                                       */
/*--------------------------------------------------------------------*/
static int GetActiveEdits (
    EIT_DATAREC  *RecipientData,  /* Current data values */
    EIT_EDITS    *Edits,          /* Returns reduced set of active edits */
    EIT_EDITS    *EditsLP)     /* from current set of edits */
{
    double adjust;     /* buffer for constant row in active edit      */
    int    j;          /* edit column counter                         */
    int    row;        /* active edit row counter                     */
    int    col;        /* active edit column counter                  */
    int    constant;   /* index of constant column in edit            */
    int    active;     /* whether edit involves a field to impute     */

    col = 0; /* ensure uninitialized value is not returned */
    constant = Edits->NumberofColumns - 1;  /* Index of the constant column */

    /*** While more edits remain to be processed equalities and     ***/
    /*** inequalities are checked.                                  ***/
    row = 0;
    while (row < Edits->NumberofEquations) {
        /*** Examine each field to determine whether it is a field  ***/
        /*** to impute and has a non-zero coefficient in the        ***/
        /*** current edit (i.e. see if the edit is active).         ***/

        active = 0;
        for (j = 0; j < constant; j++){
            if ((RecipientData->StatusFlag[j] == FIELDFTI) &&
                    (Edits->Coefficient[row][j] != 0.0)) {
                active = 1;            /* edit is active  */
                break;
            }
        }

        if (active == 1) {
            adjust = Edits->Coefficient[row][constant];
            col = 0;
            /*** Reduce the edit by keeping only fields to impute   ***/
            /*** the LHS, and subtracting coefficient * data value  ***/
            /*** of each good field from the constant on the RHS.   ***/

            for (j = 0; j < constant; j++) {
                /* keep FTI fields */
                if (RecipientData->StatusFlag[j] == FIELDFTI) {
                    ++col;
                }
                else {/* factor out good fields */
                    adjust -= (RecipientData->FieldValue[j] *
                        Edits->Coefficient[row][j]);
        
                    EditsLP->Coefficient[row][j] = 0.0;
                    EditsLP->Coefficient[row][constant] = adjust;
                }
            }

            ++row;
        }
        else {
            /* drop inactive edit */
            EI_EditsDropEquation (row, Edits);
            EI_EditsDropEquation (row, EditsLP);
        }
    }
    return col;
}
#ifdef NEVERTOBEDEFINED
moved to FindFields()
/*******************************************/
/* Get names of system matching fields     */
/*******************************************/
static void GetSystemMatchingFields (
    int EditsNumberofFields,
    EIT_FIELDNAMES * SMF, /* SystemMatchingFields */
    EIT_DATAREC * RecipientData)
{
    int i;

    /*if all fields are already matching fields, there is no need to find more*/
    if (SMF->NumberofFields < EditsNumberofFields) {
        for (i = 0; i < EditsNumberofFields; i++) {
            if (RecipientData->StatusFlag[i] == FIELDMFS) {
                if (UTIL_Binary_Reverse (RecipientData->FieldName[i],
                        SMF->FieldName, SMF->NumberofFields) == -1) {

                    SMF->FieldName[SMF->NumberofFields] =
                        STC_StrDup (RecipientData->FieldName[i]);

                    SMF->NumberofFields++;

                    UTIL_SortStrings_D (SMF->FieldName, SMF->NumberofFields);
                }
            }
        }
    }
}
#endif
/*******************************************/
/* Identify if a recipient has fields that */
/* are eligible to become matching fields  */
/*******************************************/
static int HasOKFields (
    int NumberofFields,
    EIT_STATUS_OF_FIELDS * StatusFlag)
{
    int i;

    for (i = 0; i < NumberofFields; i++)
        if (StatusFlag[i] != FIELDFTI)
            return 1;

    return 0;
}
