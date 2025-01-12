#ifndef EDITS_DISABLED
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_EditsAddPositivity.h"

#include "STC_Memory.h"

static int FindMaxEditId (EIT_EDITS *);

/*--------------------------------------------------------------------*/
/* Expand the edit structure to hold the positivity edits.            */
/*--------------------------------------------------------------------*/
EIT_RETURNCODE EI_EditsAddPositivity (
    EIT_EDITS * Edits)
{
    int i;
    int j;
    int MaxEditId;
    double ** NewCoefficient;
    int * NewEditId;
    int NewNumberOfEquations;
    int NumberOfVariables;

    NumberOfVariables = Edits->NumberofColumns - 1;
    NewNumberOfEquations = Edits->NumberofEquations + NumberOfVariables;

    /* Allocate space to pointer to edit ids */
    NewEditId = STC_AllocateMemory (NewNumberOfEquations * sizeof *NewEditId);
    if (NewEditId == NULL)
        return EIE_FAIL;

    /* Set the new EditId */
    MaxEditId = FindMaxEditId (Edits);
    for (i = 0; i < NumberOfVariables; i++)
        NewEditId[i] = ++MaxEditId;
    /* Copie the old EditId */
    for (i = 0; i < Edits->NumberofEquations; i++)
        NewEditId[NumberOfVariables+i] = Edits->EditId[i];

    /*
    Add positivity edits to edit structure
    if 3 variables
              |--- constant
              v
     0  0 -1  0
     0 -1  0  0
    -1  0  0  0
    */
    NewCoefficient = STC_AllocateMemory (
        NewNumberOfEquations * sizeof *NewCoefficient);
    if (NewCoefficient == NULL)
        return EIE_FAIL;
    for (i = 0; i < NumberOfVariables; i++) {
        NewCoefficient[i] = STC_AllocateMemory (
            Edits->NumberofColumns * sizeof *NewCoefficient[i]);
        if (NewCoefficient[i] == NULL)
            return EIE_FAIL;
        for (j = 0; j < NumberOfVariables+1; j++)
            NewCoefficient[i][j] = 0.0;
        NewCoefficient[i][NumberOfVariables-i-1] = -1;
    }
    for (i = 0; i < Edits->NumberofEquations; i++)
        NewCoefficient[NumberOfVariables+i] = Edits->Coefficient[i];

    STC_FreeMemory (Edits->EditId);
    Edits->EditId = NewEditId;

    STC_FreeMemory (Edits->Coefficient);
    Edits->Coefficient = NewCoefficient;

    Edits->NumberofEquations += NumberOfVariables;
    Edits->NumberofInequalities += NumberOfVariables;

    return EIE_SUCCEED;
}

/*******************************************************************************
Find EditId with largest value
*******************************************************************************/
static int FindMaxEditId (
    EIT_EDITS * Edits)
{
    int i;
    int MaxEditId;

    MaxEditId = -1;
    for (i = 0 ; i < Edits->NumberofEquations; i++)
        if (Edits->EditId[i] > MaxEditId)
            MaxEditId = Edits->EditId[i];

    return MaxEditId;
}
#endif
