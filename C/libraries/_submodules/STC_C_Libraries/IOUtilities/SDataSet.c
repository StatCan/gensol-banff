#ifdef _DEBUG
#include <assert.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>


#include "IO_Messages.h"

#include "SDataSet.h"
#include "SUtil.h"

#include "STC_Memory.h"

static void VariableFree(tSVariable*);

/*********************************************************************
Define data set for output
*********************************************************************/
IO_RETURN_CODE SDataSetDefineForOutput(
    tSDataSet* OutDataSet,
    DSW_generic* dsw)
{
    if (IORC_SUCCESS != DSW_allocate(dsw, OutDataSet->NumberOfVariables)) {
        return IORC_FAIL_INIT_OUT_DATASET;
    }

    for (int i = 0; i < OutDataSet->NumberOfVariables; i++){
        if (IORC_SUCCESS != SVariableDefineForOutput(OutDataSet, &OutDataSet->Variable[i], dsw, i)) {
            return IORC_FAIL_INIT_OUT_DATASET;
        }
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return IORC_FAIL_INIT_OUT_DATASET;
    }

    return IORC_SUCCESS;
}
/*********************************************************************
Free data set space and stuff
*********************************************************************/
void SDataSetFree(
    tSDataSet* DataSet)
{
    int i;

    //if (DataSet->File == NULL) return;

    for (i = 0; i < DataSet->NumberOfVariables; i++) {
        VariableFree(&DataSet->Variable[i]);
    }
    STC_FreeMemory(DataSet->Variable);
    //DataSet->Variable = NULL;

    //DataSet->Name[0] = '\0';
    //DataSet->Type = S_NOT_SET;
    //DataSet->NumberOfVariables = 0;
}
/*********************************************************************
copy a string to the SAS variable and pad with blanks
*********************************************************************/
void SVariableCopy(
    tSVariable* Variable,
    char* s)
{
    size_t Length;

    IOUtil_strcpy(Variable->Value.s, s);
    //Length = strlen(s);
    //memset(Variable->Value.s + Length, ' ', Variable->Size - Length);
}
/*********************************************************************
Checks that the variable VariableName exist in File and
is a character variable.
Allocates the space needed.
*********************************************************************/
IO_RETURN_CODE SVariableDefineForInput(
    tSDataSet* DataSet,
    tSVariable* Variable,
    DSR_generic* dsr,
    void** vl_ptr)
{
    int VariableType;

    Variable->Present = SVARIABLE_NOT_PRESENT;

    Variable->Position = SUtil_GetVariablePosition(dsr, Variable->Name);
    if (Variable->Position == S_NOT_SET) {
        if (Variable->Mandatory) {
            if (Variable->Type == SUTIL_VARIABLE_TYPE_NUMERIC)
                IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgNumericVarNotInDataSet SAS_NEWLINE,
                    Variable->Name, DataSet->Name);
            else
                IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgCharacterVarNotInDataSet SAS_NEWLINE,
                    Variable->Name, DataSet->Name);
            return IORC_ERROR;
        }
        return IORC_SUCCESS;
    }

    IO_VAR_TYPE vt = DSR_get_col_type(dsr, Variable->Position);
    if (vt == IOVT_NUM) {
        VariableType = SUTIL_VARIABLE_TYPE_NUMERIC;
    }
    else if (vt == IOVT_CHAR) {
        VariableType = SUTIL_VARIABLE_TYPE_CHARACTER;
    }
    else {
        return IORC_ERROR;
    }

    if (VariableType != Variable->Type) {
        if (Variable->Type == SUTIL_VARIABLE_TYPE_NUMERIC)
            IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgVarNotNumericInDataSet SAS_NEWLINE,
                Variable->Name, DataSet->Name);
        else
            IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgVarNotCharacterInDataSet SAS_NEWLINE,
                Variable->Name, DataSet->Name);

        return IORC_ERROR;
    }

    Variable->Present = SVARIABLE_PRESENT;
    Variable->Size = DSR_get_column_length(dsr, Variable->Position);
    if (Variable->Type == SUTIL_VARIABLE_TYPE_CHARACTER)
        Variable->Value.s = STC_AllocateMemory(Variable->Size + 1);

    if (Variable->Type == SUTIL_VARIABLE_TYPE_NUMERIC)
        Variable->Size = sizeof(double);

    *vl_ptr = Variable->Type == SUTIL_VARIABLE_TYPE_NUMERIC ?
        (char*)&Variable->Value.d : Variable->Value.s;

    return IORC_SUCCESS;
}
/*********************************************************************
Define numeric variable for output
*********************************************************************/
IO_RETURN_CODE SVariableDefineForOutput(
    tSDataSet* DataSet,
    tSVariable* Variable,
    DSW_generic* dsw,
    int index_to_add)
{
    if (Variable->Type == SUTIL_VARIABLE_TYPE_CHARACTER) {
        Variable->Value.s = STC_AllocateMemory(Variable->Size + 1);
    }

    void* ptr = (void*) (Variable->Type == SUTIL_VARIABLE_TYPE_NUMERIC ?
        (void*)&Variable->Value.d : Variable->Value.s);
    IO_VAR_TYPE var_type = Variable->Type == SUTIL_VARIABLE_TYPE_NUMERIC ?
        IOVT_NUM : IOVT_CHAR;
    return DSW_define_column(dsw, index_to_add, Variable->Name, ptr, var_type);
}
/*********************************************************************
Put a Null character at the end of a string.
Needed because SAS_XVGET () does not null terminate the strings.
*********************************************************************/
void SVariableNullTerminate(
    tSVariable* v)
{
    SUtil_NullTerminate(v->Value.s, v->Size);
}


/*********************************************************************
Frees tSVariable.
*********************************************************************/
static void VariableFree(
    tSVariable* Variable)
{
    if (Variable->Type == SUTIL_VARIABLE_TYPE_CHARACTER) {
        STC_FreeMemory(Variable->Value.s);
    }
    //Variable->Name[0] = '\0';
    //Variable->Type = S_NOT_SET;
    //Variable->Mandatory = S_NOT_SET;
    //Variable->Present = S_NOT_SET;
    //Variable->Size = S_NOT_SET;
    //Variable->Position = S_NOT_SET;
}
