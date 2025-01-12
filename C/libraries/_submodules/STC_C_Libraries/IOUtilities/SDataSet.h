#ifndef SDATASET_H
#define SDATASET_H

#include "IOUtil.h"
#define S_NOT_SET               -1

#define SVARIABLE_NOT_MANDATORY  0
#define SVARIABLE_MANDATORY      1

#define SVARIABLE_NOT_PRESENT    0
#define SVARIABLE_PRESENT        1

#define SDATASET_TYPE_INPUT      1
#define SDATASET_TYPE_OUTPUT     2

union uValue {
    double d;
    char* s;
};
typedef union uValue uValue;

struct tSVariable {
    char Name[IO_COL_NAME_LEN + 1];
    int Type;
    int Mandatory;
    int Present;
    int Size;
    int Position;
    uValue Value;
};
typedef struct tSVariable tSVariable;

struct tSDataSet {
    char Name[IO_COL_NAME_LEN + 1];
    int Type;
    int NumberOfVariables;
    tSVariable* Variable;
};
typedef struct tSDataSet tSDataSet;

extern IO_RETURN_CODE SDataSetDefineForOutput(tSDataSet*, DSW_generic* dsw);
extern void SDataSetFree(tSDataSet*);
extern void SVariableCopy(tSVariable* Variable, char*);
IO_RETURN_CODE SVariableDefineForInput(
    tSDataSet* DataSet,
    tSVariable* Variable,
    DSR_generic* dsr,
    void** vl_ptr
);
extern IO_RETURN_CODE SVariableDefineForOutput(tSDataSet*, tSVariable*, DSW_generic* dsw, int index_to_add);
extern void SVariableNullTerminate(tSVariable*);

#endif
