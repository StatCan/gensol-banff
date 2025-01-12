/*--------------------------------------------------------------------*/
/* Preprocessor Include Files:                                        */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_Edits.h"
#include "EI_Statistics.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "MessageBanffAPI.h"


/*--------------------------------------------------------------------*/
/* Preprocessor Symbols and Macros:                                   */
/*--------------------------------------------------------------------*/
#define EPSILON (1.0e-12)


#define PRINT_UNDERLINE(MsgLength) \
Printf ("%.*s", \
MsgLength, \
"----------------------------------------------------------------------------");


/*--------------------------------------------------------------------*/
/* Allocate space for the report tables which are to be printed.      */
/* Note: Table 13 is not allocated since it has only three entries.   */
/*--------------------------------------------------------------------*/
void EI_StatisticsAllocate (
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits)
{
    Statistics->NumberOfEquations = Edits->NumberofEquations;
    Statistics->NumberOfColumns = Edits->NumberofColumns;

    Statistics->Status = STC_AllocateMemory (
        Statistics->NumberOfEquations * sizeof *Statistics->Status);

    /* TABLE11: ALLOCATE SPACE FOR ONE ROW FOR EACH EDIT ID AND *******/
    /*          POSITIVITY EDIT AND 3 COLUMNS.                  *******/
    Statistics->Table11[STATISTICS_PASS] = STC_AllocateMemory (
        Statistics->NumberOfEquations *
            sizeof *Statistics->Table11[STATISTICS_PASS]);
    Statistics->Table11[STATISTICS_MISSING] = STC_AllocateMemory (
        Statistics->NumberOfEquations *
            sizeof *Statistics->Table11[STATISTICS_MISSING]);
    Statistics->Table11[STATISTICS_FAIL] = STC_AllocateMemory (
        Statistics->NumberOfEquations *
            sizeof *Statistics->Table11[STATISTICS_FAIL]);

    /* TABLE12: ONE ROW FOR EACH EDIT ID + 1, AND 3 COLUMNS ***********/
    Statistics->Table12[STATISTICS_PASS] = STC_AllocateMemory (
        (Statistics->NumberOfEquations+1) *
            sizeof *Statistics->Table12[STATISTICS_PASS]);
    Statistics->Table12[STATISTICS_MISSING] = STC_AllocateMemory (
        (Statistics->NumberOfEquations+1) *
            sizeof *Statistics->Table12[STATISTICS_MISSING]);
    Statistics->Table12[STATISTICS_FAIL] = STC_AllocateMemory (
        (Statistics->NumberOfEquations+1) *
            sizeof *Statistics->Table12[STATISTICS_FAIL]);

    /* TABLE13: NOTHING TO DO (not dependant on the size of the edits)*/

    /* TABLE21: ONE ROW FOR EACH VARIABLE AND 4 COLUMNS ***************/
    Statistics->Table21[STATISTICS_PASS] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table21[STATISTICS_PASS]);
    Statistics->Table21[STATISTICS_MISSING] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table21[STATISTICS_MISSING]);
    Statistics->Table21[STATISTICS_FAIL] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table21[STATISTICS_FAIL]);
    Statistics->Table21[STATISTICS_NOTAPPLICABLE] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table21[STATISTICS_NOTAPPLICABLE]);

    /* TABLE5: ONE ROW FOR EACH VARIABLE AND 4 COLUMNS ***************/
    Statistics->Table22[STATISTICS_PASS] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table22[STATISTICS_PASS]);
    Statistics->Table22[STATISTICS_MISSING] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table22[STATISTICS_MISSING]);
    Statistics->Table22[STATISTICS_FAIL] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table22[STATISTICS_FAIL]);
    Statistics->Table22[STATISTICS_NOTAPPLICABLE] = STC_AllocateMemory (
        Statistics->NumberOfColumns *
            sizeof *Statistics->Table22[STATISTICS_NOTAPPLICABLE]);
}

/*--------------------------------------------------------------------*/
/* Apply the edits to a data record.                                  */
/* Set Statistics Status and RecordStatus members accordingly.        */
/* See EI_StatisticsPrintDefinition() for explanations.               */
/*--------------------------------------------------------------------*/
void EI_StatisticsApplyEdits (
    EIT_EDITS * Edits,
    EIT_DATAREC * Data,
    EIT_STATISTICS * Statistics)
{
    register int i, j;

    /* for each edit */
    for (i = 0; i < Edits->NumberofEquations; i++) {
        Statistics->Status[i] = STATISTICS_PASS;

        /* Check missing value first */
        for (j = 0; j < Data->NumberofFields; j++) {
            /* if there is a data value and the      */
            /* coefficient is null, status is MISSING       */
            if ((Data->StatusFlag[j] == FIELDMISS) &&
                    (Edits->Coefficient[i][j] != 0.0)) {
                Statistics->Status[i] = STATISTICS_MISSING;
                break;
            }
        }

        /* If a status of missing has not been assigned. */
        if (Statistics->Status[i] != STATISTICS_MISSING) {

            double Sum; /* scalar product of Edit * Data */

            /* scalar product of data * edit         */
            Sum = 0.0;
            for (j = 0; j < Data->NumberofFields; j++)
                Sum += (Edits->Coefficient[i][j] * Data->FieldValue[j]);

            if (i < Edits->NumberofInequalities) {
                /*
                If this is an inequality edit.
                Constant should be greater or equal to sum of columns.
                */
                if (EIM_DBL_LT (Edits->Coefficient[i][Data->NumberofFields],
                        Sum)) {
                    Statistics->Status[i] = STATISTICS_FAIL;
                }
            }
            else if (i >= Edits->NumberofInequalities) {
                /*
                If this is an equality edit.
                Constant and sum of columns must be equal.
                */
                if (!EIM_DBL_EQ (Edits->Coefficient[i][Data->NumberofFields],
                        Sum)) {
                    Statistics->Status[i] = STATISTICS_FAIL;
                }
            }
        }
    }

    /* Set the record status */
    Statistics->RecordStatus = STATISTICS_PASS;
    for (i = 0; i < Statistics->NumberOfEquations; i++) {
        if (Statistics->Status[i] == STATISTICS_FAIL) {
            Statistics->RecordStatus = STATISTICS_FAIL;
            return;
        }
        if (Statistics->Status[i] == STATISTICS_MISSING)
            Statistics->RecordStatus = STATISTICS_MISSING;
    }
}

/*--------------------------------------------------------------------*/
/* Free space for the report tables which are to be printed.          */
/*--------------------------------------------------------------------*/
void EI_StatisticsFree (
    EIT_STATISTICS * Statistics)
{
    STC_FreeMemory (Statistics->Status);

    STC_FreeMemory (Statistics->Table11[STATISTICS_PASS]);
    STC_FreeMemory (Statistics->Table11[STATISTICS_MISSING]);
    STC_FreeMemory (Statistics->Table11[STATISTICS_FAIL]);

    STC_FreeMemory (Statistics->Table12[STATISTICS_PASS]);
    STC_FreeMemory (Statistics->Table12[STATISTICS_MISSING]);
    STC_FreeMemory (Statistics->Table12[STATISTICS_FAIL]);

    /* TABLE13: NOTHING TO DO */

    STC_FreeMemory (Statistics->Table21[STATISTICS_PASS]);
    STC_FreeMemory (Statistics->Table21[STATISTICS_MISSING]);
    STC_FreeMemory (Statistics->Table21[STATISTICS_FAIL]);
    STC_FreeMemory (Statistics->Table21[STATISTICS_NOTAPPLICABLE]);

    STC_FreeMemory (Statistics->Table22[STATISTICS_PASS]);
    STC_FreeMemory (Statistics->Table22[STATISTICS_MISSING]);
    STC_FreeMemory (Statistics->Table22[STATISTICS_FAIL]);
    STC_FreeMemory (Statistics->Table22[STATISTICS_NOTAPPLICABLE]);
}

/*--------------------------------------------------------------------*/
/* Initialise report counters.                                        */
/*--------------------------------------------------------------------*/
void EI_StatisticsInitialize (
    EIT_STATISTICS * Statistics)
{
    int i;

    for (i = 0; i < Statistics->NumberOfEquations; i++) {
        Statistics->Table11[STATISTICS_PASS][i] = 0;
        Statistics->Table11[STATISTICS_MISSING][i] = 0;
        Statistics->Table11[STATISTICS_FAIL][i] = 0;
    }

    for (i = 0; i< Statistics->NumberOfEquations+1; i++) {
        Statistics->Table12[STATISTICS_PASS][i] = 0;
        Statistics->Table12[STATISTICS_MISSING][i] = 0;
        Statistics->Table12[STATISTICS_FAIL][i] = 0;
    }

    Statistics->Table13[STATISTICS_PASS] = 0;
    Statistics->Table13[STATISTICS_MISSING] = 0;
    Statistics->Table13[STATISTICS_FAIL] = 0;

    for (i = 0; i<Statistics->NumberOfColumns; i++) {
        Statistics->Table21[STATISTICS_PASS][i] = 0;
        Statistics->Table21[STATISTICS_MISSING][i] = 0;
        Statistics->Table21[STATISTICS_FAIL][i] = 0;
        Statistics->Table21[STATISTICS_NOTAPPLICABLE][i] = 0;
    }

    for (i = 0; i < Statistics->NumberOfColumns; i++) {
        Statistics->Table22[STATISTICS_PASS][i] = 0;
        Statistics->Table22[STATISTICS_MISSING][i] = 0;
        Statistics->Table22[STATISTICS_FAIL][i] = 0;
        Statistics->Table22[STATISTICS_NOTAPPLICABLE][i] = 0;
    }
}

/*--------------------------------------------------------------------*/
/* Print the status definitions.                                      */
/*--------------------------------------------------------------------*/
void EI_StatisticsPrintDefinition (
    EIT_PRINTF Printf)
{

    /* Definition (1 ligne <-> M400xx) : 
       For a given record, an edit is
           i) PASS--if all data values associated with non-zero coefficients in                      
                    the edit are non-missing and the edit is satisfied           
          ii) MISS--if one of more data values associated with non-zero
                    coefficients in the edit are missing
         iii) FAIL--if all data values associated with non-zero coefficients in
                    the edit are non-missing and the edit is not satisfied
       A record is of status
           i) PASS--if all edits have status PASS
          ii) MISS--if one of more edits have status MISS, those which do
                    not, have status PASS
         iii) FAIL--if one or more edits have status FAIL
    */
    static char * Definition = {
        M40081 "\n\n"
        M40082 "\n"
        M40083 "\n\n"
        M40084 "\n"
        M40085 "\n\n"
        M40086 "\n"
        M40087 "\n\n\n"
        M40088 "\n\n"
        M40089 "\n\n"
        M40090 "\n"
        M40091 "\n\n"
        M40092 "\n"
    };
    
    Printf ("%s\n", M40080); /* Definition of edit and record statuses */
    PRINT_UNDERLINE(strlen(M40080));    
    Printf ("\n\n");
    Printf ("%s", Definition);
    Printf ("\n");
}

/*--------------------------------------------------------------------*/
/* Print Table 11.                                                    */
/*--------------------------------------------------------------------*/
void EI_StatisticsPrintTable11 (
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits,
    EIT_PRINTF Printf)
{
    register  int i;

    Printf (M40049 "\n"); /* Edit summary statistics */
    PRINT_UNDERLINE(strlen(M40049));
    Printf ("\n\n");

    Printf ("%s\n", M40050);   /* Table 1.1 */
    Printf ("%s\n\n", M40051); /* Counts of records that passed, missed
                                  and failed for each edit. */
    /* Columns name:
       Edit Id.   Records  Records  Records ....................> line 1
                  passed   missed   failed  ....................> line 2
    */
    Printf ("%5s |", " "); PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");

    Printf ("%5s | %-9s | %-12s | %-12s | %-12s |\n", 
        " ", M40052, M40053, M40053, M40053);                /* line 1 */
    Printf ("%5s | %-9s | %-12s | %-12s | %-12s |\n",
        " ", " ", M40054, M40055, M40056);                   /* line 2 */

    Printf ("%5s |", " "); PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");

    /* For each edit id print the counters   */
    for (i = 0; i < Statistics->NumberOfEquations; i++) {
        Printf ("%5s | %9d | %12ld | %12ld | %12ld |\n",
            " ",
            Edits->EditId[i],
            Statistics->Table11[STATISTICS_PASS][i],
            Statistics->Table11[STATISTICS_MISSING][i],
            Statistics->Table11[STATISTICS_FAIL][i]);
        Printf ("%5s |", " "); PRINT_UNDERLINE(11); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        Printf ("\n");
    }

    Printf ("\n\n");
}

/*--------------------------------------------------------------------*/
/* Print Table 12.                                                    */
/*--------------------------------------------------------------------*/
void EI_StatisticsPrintTable12 (
    EIT_STATISTICS * Statistics,
    EIT_PRINTF Printf)
{
    register int  i;

    long Pass;
    long Miss;
    long Fail;

    Printf (M40049 "\n"); /* Edit summary statistics */
    PRINT_UNDERLINE(strlen(M40049));
    Printf ("\n\n");

    Printf ("%s\n", M40057);   /* Table 1.2 */
    Printf ("%s\n\n", M40058); /* Distribution of records that passed, missed 
                                  and failed 'k' edits. */
    /* Columns name:
       Number of edits  Records  Records  Records ................> line 1
             (k)        passed   missed   failed  ................> line 2
    */
    Printf ("%5s |", " "); PRINT_UNDERLINE(18);  Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");

    Printf ("%5s | %-16s | %-12s | %-12s | %-12s |\n", 
         " ", M40059, M40053, M40053, M40053);                /* line 1 */
    Printf ("%5s | %7s%3s%6s | %-12s | %-12s | %-12s |\n",  
         " ", " ", "(k)", " ", M40054, M40055, M40056);       /* line 2 */

    Printf ("%5s |", " "); PRINT_UNDERLINE(18);  Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");
  
    Pass = 0;
    Miss = 0;
    Fail = 0;

    /* For each possible frequency print the counters. */
    for (i = 0; i < Statistics->NumberOfEquations+1; i++) {
        Printf ("%5s | %7s%-3d%6s | %12ld | %12ld | %12ld |\n",
            " ", " ", i, " ",
            Statistics->Table12[STATISTICS_PASS][i],
            Statistics->Table12[STATISTICS_MISSING][i],
            Statistics->Table12[STATISTICS_FAIL][i]);

        Printf ("%5s |", " "); PRINT_UNDERLINE(18);  Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        Printf ("\n");
 
        Pass += Statistics->Table12[STATISTICS_PASS][i];
        Miss += Statistics->Table12[STATISTICS_MISSING][i];
        Fail += Statistics->Table12[STATISTICS_FAIL][i];
    }

    Printf ("%5s | %-16s | %12ld | %12ld | %12ld |\n",
        " ", M40060, Pass, Miss, Fail);

    Printf ("%5s |", " "); PRINT_UNDERLINE(18);  Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");

    Printf("\n\n");
}

/*--------------------------------------------------------------------*/
/* Print Table 13.                                                    */
/*--------------------------------------------------------------------*/
void EI_StatisticsPrintTable13 (
    EIT_STATISTICS * Statistics,
    EIT_PRINTF Printf)
{
    Printf (M40049 "\n"); /* Edit summary statistics */
    PRINT_UNDERLINE(strlen(M40049));
    Printf ("\n\n");

    Printf ("%s\n", M40061);   /* Table 1.3 */
    Printf ("%s\n\n", M40062); /* Overall counts of records that 
                                  passed, missed and failed. */
    /* Columns name:
       Records  Records  Records    Total  ....................> line 1 
       passed   missed   failed            ....................> line 2
    */
    Printf ("%5s |", " ");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");

    Printf ("%5s | %-12s | %-12s | %-12s | %12s |\n", 
        " ", M40053, M40053, M40053, "Total");             /* line 1 */
    Printf ("%5s | %-12s | %-12s | %-12s | %12s |\n",
        " ", M40054, M40055, M40056, " ");                 /* line 2 */

    Printf ("%5s |", " ");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");

    Printf ("%5s | %12ld | %12ld | %12ld | %12ld |\n",
        " ",
        Statistics->Table13[STATISTICS_PASS],
        Statistics->Table13[STATISTICS_MISSING],
        Statistics->Table13[STATISTICS_FAIL],
        Statistics->Table13[STATISTICS_PASS] +
            Statistics->Table13[STATISTICS_MISSING] +
            Statistics->Table13[STATISTICS_FAIL]);

    Printf ("%5s |", " ");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    Printf ("\n");

    Printf("\n\n");
}

/*--------------------------------------------------------------------*/
/* Print Table 21.                                                    */
/*--------------------------------------------------------------------*/
void EI_StatisticsPrintTable21 (
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits,
    EIT_PRINTF Printf)
{

    int i, j;
    int nonzero;           /* Number of edits using variable */
    char name1[17];
    char name2[17];

    Printf (M40049 "\n"); /* Edit summary statistics */
    PRINT_UNDERLINE(strlen(M40049));
    Printf ("\n\n");

    Printf ("%s\n", M40063);   /* Table 2.1 */
    Printf ("%s\n\n", M40064); /* Counts of edit applications of status 
                                  PASS, MISS or FAIL that involve* 
                                  each field. */

    /* Columns name:
           Edit     Edit     Edit     Edit      Number   .........> line 1
    Field  applic.  applic.  applic.  applic.   of edits .........> line 2
           passed   missed   failed   not       involved .........> line 3
                                      involved           .........> line 4  
    */
    Printf ("|"); PRINT_UNDERLINE(18); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");        
    Printf ("\n");

    Printf ("| %-16s | %-9s | %-9s | %-9s | %-9s | %-9s |\n", 
        " ", M40066, M40066, M40066, M40066, M40073);          /* line 1 */
    Printf ("| %-16s | %-9s | %-9s | %-9s | %-9s | %-9s |\n", 
        M40065, M40067, M40067, M40067, M40067, M40074);       /* line 2 */
    Printf ("| %-16s | %-9s | %-10s| %-9s | %-9s | %-10s|\n", 
        " ", M40068, M40069, M40070, M40071, M40072);          /* line 3 */
    Printf ("| %-16s | %-9s | %-9s | %-9s | %-10s| %-9s |\n", 
        " ", " ", " ", " ", M40072, " ");                      /* line 4 */

    Printf ("|"); PRINT_UNDERLINE(18); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");
    PRINT_UNDERLINE(11); Printf ("|");        
    Printf ("\n");
    
    for (i = Statistics->NumberOfColumns - 2; i >= 0; i--) {
        nonzero = 0;
        for (j=0; j < Statistics->NumberOfEquations; j++)
            if (Edits->Coefficient[j][i] != 0.0)
                nonzero++;

        if ( strlen(Edits->FieldName[i]) > 16 ) {
           strcpy(name1, "");
           strcpy(name2, "");
           strncpy(name1, Edits->FieldName[i], 16);
           strcpy(name2, Edits->FieldName[i]+16);
           Printf ("| %-16.16s | %9s | %9s | %9s | %9s | %9s |\n",
               name1, " ", " ", " ", " ", " ");
           Printf ("| %-16.16s | ", 
                name2);
        }
        else
            Printf ("| %-16.16s | ", 
                Edits->FieldName[i]);

        Printf (
            "%9ld | %9ld | %9ld | %9ld |       %3d |\n",
            Statistics->Table21[STATISTICS_PASS][i],
            Statistics->Table21[STATISTICS_MISSING][i],
            Statistics->Table21[STATISTICS_FAIL][i],
            Statistics->Table21[STATISTICS_NOTAPPLICABLE][i],
            nonzero);

        Printf ("|"); PRINT_UNDERLINE(18); Printf ("|");
        PRINT_UNDERLINE(11); Printf ("|");
        PRINT_UNDERLINE(11); Printf ("|");
        PRINT_UNDERLINE(11); Printf ("|");
        PRINT_UNDERLINE(11); Printf ("|");
        PRINT_UNDERLINE(11); Printf ("|");        
        Printf ("\n");
    }
       
    Printf ("\n");    
    Printf ("* %s\n", M40075);
    Printf ("\n\n");
}

/*--------------------------------------------------------------------*/
/* Print Table 22.                                                    */
/*--------------------------------------------------------------------*/
void EI_StatisticsPrintTable22 (
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits,
    EIT_PRINTF Printf)
{
    int i;
    char name1[17];
    char name2[17];

    Printf (M40049 "\n"); /* Edit summary statistics */
    PRINT_UNDERLINE(strlen(M40049));
    Printf ("\n\n");

    Printf ("%s\n", M40076);   /* Table 2.2 */
    Printf ("%s\n\n", M40077); /* Counts of records of status PASS, MISS, or
                                  FAIL for which field j contributed* to the 
                                  overall record status. */

    /* Columns name:
    Field  Records  Records  Records  Records    ...............> line 1
           passed   missed   failed   not appl.  ...............> line 2
    */
    Printf ("|"); PRINT_UNDERLINE(18); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");       
    Printf ("\n");

    Printf ("| %-16s | %-12s | %-12s | %-12s | %-12s |\n", 
        M40065, M40053, M40053, M40053, M40053);            /* line 1 */      
    Printf ("| %-16s | %-12s | %-12s | %-12s | %-12s |\n", 
        " ", M40054, M40055, M40056, M40078);               /* line 2 */

    Printf ("|"); PRINT_UNDERLINE(18); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");
    PRINT_UNDERLINE(14); Printf ("|");  
    Printf ("\n");
    
    for (i = Statistics->NumberOfColumns - 2; i >= 0; i--) {
        if ( strlen(Edits->FieldName[i]) > 16 ) {
           strcpy(name1, "");
           strcpy(name2, "");
           strncpy(name1, Edits->FieldName[i], 16);
           strcpy(name2, Edits->FieldName[i]+16);
           Printf ("| %-16.16s | %12s | %12s | %12s | %12s |\n",
               name1, " ", " ", " ", " ");
           Printf ("| %-16.16s | ", name2);                
        }
        else
            Printf ("| %-16.16s | ", Edits->FieldName[i]);
                
        Printf (
            "%12ld | %12ld | %12ld | %12ld |\n",
            Statistics->Table22[STATISTICS_PASS][i],
            Statistics->Table22[STATISTICS_MISSING][i],
            Statistics->Table22[STATISTICS_FAIL][i],
            Statistics->Table22[STATISTICS_NOTAPPLICABLE][i]);

        Printf ("|"); PRINT_UNDERLINE(18); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");
        PRINT_UNDERLINE(14); Printf ("|");     
        Printf ("\n");
    }
       
    Printf ("\n");    
    Printf ("* %s\n", M40079);
    Printf ("\n\n");

}

/*--------------------------------------------------------------------*/
/* Update Table 11. For each edit, increment the count for the edit   */
/* status type.                                                       */
/*--------------------------------------------------------------------*/
void EI_StatisticsUpdateTable11 (
    EIT_STATISTICS * Statistics)
{
    register int i;

    for (i = 0; i < Statistics->NumberOfEquations; i++) {
        Statistics->Table11[Statistics->Status[i]][i]++;
    }
}

/*--------------------------------------------------------------------*/
/* Update Table 12. Count edit status type frequency.                 */
/*--------------------------------------------------------------------*/
void EI_StatisticsUpdateTable12 (
    EIT_STATISTICS * Statistics)
{
    register int i;
    int Number[3];

    Number[STATISTICS_PASS] = 0;
    Number[STATISTICS_MISSING] = 0;
    Number[STATISTICS_FAIL] = 0;

    for (i = 0; i < Statistics->NumberOfEquations; i++) {
        Number[Statistics->Status[i]]++;
    }

    Statistics->Table12[STATISTICS_PASS][Number[STATISTICS_PASS]]++;
    Statistics->Table12[STATISTICS_MISSING][Number[STATISTICS_MISSING]]++;
    Statistics->Table12[STATISTICS_FAIL][Number[STATISTICS_FAIL]]++;
}

/*--------------------------------------------------------------------*/
/* Update Table 13.                                                   */
/*--------------------------------------------------------------------*/
void EI_StatisticsUpdateTable13 (
    EIT_STATISTICS * Statistics)
{
    /* update Table 13 for the current record */
    Statistics->Table13[Statistics->RecordStatus]++;
}

/*--------------------------------------------------------------------*/
/* Update Table 21.                                                   */
/*--------------------------------------------------------------------*/
void EI_StatisticsUpdateTable21 (
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits)
{
    register int i, j;

    /* For each edit */
    for (i = 0; i < Statistics->NumberOfEquations; i++) {
        /* For each variable in the edit ...     */
        /* Note: 1 is subtracted so that the constant is not processed */
        for (j = 0; j < Statistics->NumberOfColumns - 1; j++) {
            /* If the coefficient is not equal to 0  */
            /* then update the counter based on      */
            /* the editid status type and variable   */
            if (Edits->Coefficient[i][j] != 0.0)
                Statistics->Table21[Statistics->Status[i]][j]++;
            else
                Statistics->Table21[STATISTICS_NOTAPPLICABLE][j]++;
        }
    }
}

/*--------------------------------------------------------------------*/
/* Update Table 22.                                                   */
/*--------------------------------------------------------------------*/
void EI_StatisticsUpdateTable22 (
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits)
{
    register int i, j;
    int StatusIndex;         /* The record status type                */

    switch (Statistics->RecordStatus) {
    case STATISTICS_PASS:
        /*
        If the record status is PASS, update the PASS entry for each variable
        */
        for (i = 0; i < Statistics->NumberOfColumns - 1; i ++)
            Statistics->Table22[STATISTICS_PASS][i]++;
        break;

    case STATISTICS_MISSING:
        /*
        If the record status is MISS, the variable status will be MISS if the
        variable has a non-zero coefficient in any edit with a status of MISS
        */
        for (i = 0; i < Statistics->NumberOfColumns - 1; i++) {
            StatusIndex = STATISTICS_NOTAPPLICABLE;

            for (j = 0; j < Statistics->NumberOfEquations; j++) {
                if ((Statistics->Status[j] == STATISTICS_MISSING) &&
                        (Edits->Coefficient[j][i] != 0.0)) {
                    StatusIndex = STATISTICS_MISSING;
                    break;
                }
            }
            /* Update the proper counter */
            Statistics->Table22[StatusIndex][i]++;
        }
        break;

    case STATISTICS_FAIL:
        /*
        If the record status is FAIL, the variable status will be FAIL if the
        variable has a non-zero coefficient in any edit with a status FAIL
        */
        /* For each variable */
        for (i = 0; i < Statistics->NumberOfColumns - 1; i++) {
            StatusIndex = STATISTICS_NOTAPPLICABLE;
            for (j = 0; j < Statistics->NumberOfEquations; j++) {
                if ((Statistics->Status[j] == STATISTICS_FAIL) &&
                        (Edits->Coefficient[j][i] != 0.0)) {
                    StatusIndex = STATISTICS_FAIL;
                    break;
                }
            }
            /* update variables and status counter */
            Statistics->Table22[StatusIndex][i]++;
        }
        break;

    default:
        break; /* Should never happen */
    }
}

/*--------------------------------------------------------------------*/
/* Initialise report counters.                                        */
/*--------------------------------------------------------------------*/
void EI_StatisticsUpdateTables (
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits)
{
    EI_StatisticsUpdateTable11 (Statistics);
    EI_StatisticsUpdateTable12 (Statistics);
    EI_StatisticsUpdateTable13 (Statistics);
    EI_StatisticsUpdateTable21 (Statistics, Edits);
    EI_StatisticsUpdateTable22 (Statistics, Edits);
}
