/* contient toutes les fonctions reliees aux types algorithms */

/*#include <assert.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Algorithm.h"
#include "EI_Common.h"
#include "EI_Message.h"

#include "STC_Memory.h"

#include "util.h"

#include "MessageBanffAPI.h"

#define DIM(a) (sizeof(a)/sizeof(*(a)))

/* predefined formulas */
#define AUXTREND_F   "FIELDID(H)*AUX1/AUX1(H)"
#define AUXTREND2_F  "FIELDID(H)*(AUX1/AUX1(H)+AUX2/AUX2(H))/2"
#define CURAUX_F     "AUX1"
#define CURAUXMEAN_F "AUX1(A)"
#define CURMEAN_F    "FIELDID(A)"
#define CURRATIO_F   "FIELDID(A)*AUX1/AUX1(A)"
#define CURRATIO2_F  "FIELDID(A)*(AUX1/AUX1(A)+AUX2/AUX2(A))/2"
#define CURREG_F     "INTERCEPT,AUX1"
#define CURREG_E2_F  "INTERCEPT,AUX1,AUX1^2"
#define CURREG2_F    "INTERCEPT,AUX1,AUX2"
#define CURREG3_F    "INTERCEPT,AUX1,AUX2,AUX3"
#define CURSUM2_F    "AUX1+AUX2"
#define CURSUM3_F    "AUX1+AUX2+AUX3"
#define CURSUM4_F    "AUX1+AUX2+AUX3+AUX4"
#define DIFTREND_F   "FIELDID(A)*FIELDID(H)/FIELDID(H,A)"
#define HISTREG_F    "INTERCEPT,FIELDID(H)"
#define PREAUX_F     "AUX1(H)"
#define PREAUXMEAN_F "AUX1(H,A)"
#define PREMEAN_F    "FIELDID(H,A)"
#define PREVALUE_F   "FIELDID(H)"

/* predefined algorithms */
static EIT_ALGORITHM mPredefinedAlgorithm[] = {
    {"AUXTREND",   EIE_ALGORITHM_TYPE_EF, "AT",  AUXTREND_F,   AUXTREND_D},
    {"AUXTREND2",  EIE_ALGORITHM_TYPE_EF, "AT2", AUXTREND2_F,  AUXTREND2_D},
    {"CURAUX",     EIE_ALGORITHM_TYPE_EF, "CA",  CURAUX_F,     CURAUX_D},
    {"CURAUXMEAN", EIE_ALGORITHM_TYPE_EF, "CAM", CURAUXMEAN_F, CURAUXMEAN_D},
    {"CURMEAN",    EIE_ALGORITHM_TYPE_EF, "CM",  CURMEAN_F,    CURMEAN_D},
    {"CURRATIO",   EIE_ALGORITHM_TYPE_EF, "CR",  CURRATIO_F,   CURRATIO_D},
    {"CURRATIO2",  EIE_ALGORITHM_TYPE_EF, "CR2", CURRATIO2_F,  CURRATIO2_D},
    {"CURREG",     EIE_ALGORITHM_TYPE_LR, "LR1", CURREG_F,     CURREG_D},
    {"CURREG_E2",  EIE_ALGORITHM_TYPE_LR, "LRE", CURREG_E2_F,  CURREG_E2_D},
    {"CURREG2",    EIE_ALGORITHM_TYPE_LR, "LR2", CURREG2_F,    CURREG2_D},
    {"CURREG3",    EIE_ALGORITHM_TYPE_LR, "LR3", CURREG3_F,    CURREG3_D},
    {"CURSUM2",    EIE_ALGORITHM_TYPE_EF, "SM2", CURSUM2_F,    CURSUM2_D},
    {"CURSUM3",    EIE_ALGORITHM_TYPE_EF, "SM3", CURSUM3_F,    CURSUM3_D},
    {"CURSUM4",    EIE_ALGORITHM_TYPE_EF, "SM4", CURSUM4_F,    CURSUM4_D},
    {"DIFTREND",   EIE_ALGORITHM_TYPE_EF, "DT",  DIFTREND_F,   DIFTREND_D},
    {"HISTREG",    EIE_ALGORITHM_TYPE_LR, "HLR", HISTREG_F,    HISTREG_D},
    {"PREAUX",     EIE_ALGORITHM_TYPE_EF, "PA",  PREAUX_F,     PREAUX_D},
    {"PREAUXMEAN", EIE_ALGORITHM_TYPE_EF, "PAM", PREAUXMEAN_F, PREAUXMEAN_D},
    {"PREMEAN",    EIE_ALGORITHM_TYPE_EF, "PM",  PREMEAN_F,    PREMEAN_D},
    {"PREVALUE",   EIE_ALGORITHM_TYPE_EF, "PV",  PREVALUE_F,   PREVALUE_D}
};

static void FreeAlgorithm (EIT_ALGORITHM *);

/*********************************************************************
Add an item to the algorithm list.
*********************************************************************/
EIT_RETURNCODE EI_AlgorithmAdd (
    EIT_ALGORITHM_LIST * List,
    char * Name,
    EIT_ALGORITHM_TYPE Type,
    char * Status,
    char * Formula,
    char * Description)
{
    int Index;

    Index = EI_AlgorithmFind (List, Name);

    if (Index == EIM_NOTFOUND) {
        /* not found, add it */
        if (List->NumberEntries == List->NumberAllocatedEntries) {
            /* if not enough space, double the size */
            EIT_ALGORITHM * NewAlgorithm;

            NewAlgorithm = STC_ReallocateMemory (
                sizeof *List->Algorithm * List->NumberAllocatedEntries,
                sizeof *List->Algorithm * List->NumberAllocatedEntries * 2,
                List->Algorithm);
            if (NewAlgorithm == NULL) return EIE_FAIL;

            List->Algorithm = NewAlgorithm;
            List->NumberAllocatedEntries *= 2;
        }
        Index = List->NumberEntries++;
    }
    else {
        /* found, free the old algorithm */
        FreeAlgorithm (&List->Algorithm[Index]);
    }

    List->Algorithm[Index].Name = STC_StrDup (Name);
    if (List->Algorithm[Index].Name == NULL) return EIE_FAIL;
    List->Algorithm[Index].Type = Type;
    List->Algorithm[Index].Status = STC_StrDup (Status);
    if (List->Algorithm[Index].Status == NULL) return EIE_FAIL;
    List->Algorithm[Index].Formula = STC_StrDup (Formula);
    if (List->Algorithm[Index].Formula == NULL) return EIE_FAIL;
    List->Algorithm[Index].Description = STC_StrDup (Description);
    if (List->Algorithm[Index].Description == NULL) return EIE_FAIL;

    return EIE_SUCCEED;
}
/*********************************************************************
Add predefined algorithms to the algorithm list.
*********************************************************************/
EIT_RETURNCODE EI_AlgorithmAddPredefined (
    EIT_ALGORITHM_LIST * List)
{
    int i;
    EIT_RETURNCODE rc;

    for (i = 0; i < DIM (mPredefinedAlgorithm); i++) {
        rc = EI_AlgorithmAdd (List,
            mPredefinedAlgorithm[i].Name,
            mPredefinedAlgorithm[i].Type,
            mPredefinedAlgorithm[i].Status,
            mPredefinedAlgorithm[i].Formula,
            mPredefinedAlgorithm[i].Description);
        if (rc == EIE_FAIL) return EIE_FAIL;
    }
    return EIE_SUCCEED;
}
/*********************************************************************
Allocate and initialise algorithm list.
*********************************************************************/
EIT_ALGORITHM_LIST * EI_AlgorithmAllocate (void)
{
    EIT_ALGORITHM_LIST * List;

    List = STC_AllocateMemory (sizeof *List);
    if (List == NULL) return NULL;

    /* # predefined + an arbitrary number */
    List->NumberAllocatedEntries = DIM (mPredefinedAlgorithm) + 10;
    List->NumberEntries = 0;
    List->Algorithm = STC_AllocateMemory (
        sizeof *List->Algorithm * List->NumberAllocatedEntries);
    if (List->Algorithm == NULL) return NULL;

    return List;
}
/*********************************************************************
use LINEAR search to find the index of the ITEM in the list.
returns EIM_NOTFOUND if not found.
*********************************************************************/
int EI_AlgorithmFind (
    EIT_ALGORITHM_LIST * List,
    char * Name)
{
    int i;

    for (i = 0; i < List->NumberEntries; i++) {
        if (UTIL_StrICmp (List->Algorithm[i].Name, Name) == 0)
            return i;
    }

    return EIM_NOTFOUND;
}
/*********************************************************************
Free algorithm list.
*********************************************************************/
void EI_AlgorithmFree (
    EIT_ALGORITHM_LIST * List)
{
    int i;

    for (i = 0; i < List->NumberEntries; i++)
        FreeAlgorithm (&List->Algorithm[i]);
    STC_FreeMemory (List->Algorithm);
    STC_FreeMemory (List);
}
/**********************************************************************
Print algorithm list.
*********************************************************************/
void EI_AlgorithmPrint (
    EIT_ALGORITHM_LIST * List)
{
    int i;

    printf ("NumberEntries=%d NumberAllocatedEntries=%d\n",
        List->NumberEntries, List->NumberAllocatedEntries);

    for (i = 0; i < List->NumberEntries; i++) {
        printf ("Name=%s Formula=%s\n",
            List->Algorithm[i].Name, List->Algorithm[i].Formula);
    }
}

/**********************************************************************
*********************************************************************/
static void FreeAlgorithm (
    EIT_ALGORITHM * Algorithm)
{
    STC_FreeMemory (Algorithm->Name);
    STC_FreeMemory (Algorithm->Status);
    STC_FreeMemory (Algorithm->Formula);
    STC_FreeMemory (Algorithm->Description);
}
