#ifndef EI_STATISTICS_H
#define EI_STATISTICS_H

#include "EI_Common.h"
#include "EI_Edits.h"

/* TABLE Indexes */
#define STATISTICS_PASS           0
#define STATISTICS_MISSING        1
#define STATISTICS_FAIL           2
#define STATISTICS_NOTAPPLICABLE  3

/*** TABLE DEFINITIONS: COLUMN 0 = PASS ***************************/
/***                    COLUMN 1 = MISS ***************************/
/***                    COLUMN 2 = FAIL ***************************/
/***                    COLUMN 3 = NOTAPPLICABLE   ****************/
typedef struct {
    int NumberOfEquations;
    int NumberOfColumns;
    int * Status;
    int RecordStatus;
    long * Table11[3]; /* 3 states */
    long * Table12[3]; /* 3 states */
    long   Table13[3]; /* 3 states */
    long * Table21[4]; /* 3 states + Not applicable */
    long * Table22[4]; /* 3 states + Not applicable */
} EIT_STATISTICS;

CLASS_DECLSPEC void EI_StatisticsAllocate (EIT_STATISTICS *, EIT_EDITS *);
CLASS_DECLSPEC void EI_StatisticsApplyEdits (EIT_EDITS *, EIT_DATAREC *,
    EIT_STATISTICS *);
CLASS_DECLSPEC void EI_StatisticsFree (EIT_STATISTICS *);
CLASS_DECLSPEC void EI_StatisticsInitialize (EIT_STATISTICS *);
CLASS_DECLSPEC void EI_StatisticsPrintDefinition (EIT_PRINTF);
CLASS_DECLSPEC void EI_StatisticsPrintTable11 (EIT_STATISTICS *, EIT_EDITS *,
    EIT_PRINTF);
CLASS_DECLSPEC void EI_StatisticsPrintTable12 (EIT_STATISTICS *, EIT_PRINTF);
CLASS_DECLSPEC void EI_StatisticsPrintTable13 (EIT_STATISTICS *, EIT_PRINTF);
CLASS_DECLSPEC void EI_StatisticsPrintTable21 (EIT_STATISTICS *, EIT_EDITS *,
    EIT_PRINTF);
CLASS_DECLSPEC void EI_StatisticsPrintTable22 (EIT_STATISTICS *, EIT_EDITS *,
    EIT_PRINTF);
CLASS_DECLSPEC void EI_StatisticsUpdateTables (EIT_STATISTICS *, EIT_EDITS *);
CLASS_DECLSPEC void EI_StatisticsUpdateTable11 (EIT_STATISTICS *);
CLASS_DECLSPEC void EI_StatisticsUpdateTable12 (EIT_STATISTICS *);
CLASS_DECLSPEC void EI_StatisticsUpdateTable13 (EIT_STATISTICS *);
CLASS_DECLSPEC void EI_StatisticsUpdateTable21 (EIT_STATISTICS *, EIT_EDITS *);
CLASS_DECLSPEC void EI_StatisticsUpdateTable22 (EIT_STATISTICS *, EIT_EDITS *);

#endif
