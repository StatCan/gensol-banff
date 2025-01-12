/* contient toutes les fonctions reliees au type EIT_DATATABLE */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_DataRec.h"
#include "EI_DataTable.h"
#include "EI_Message.h"

#include "STC_Memory.h"

#include "util.h"

static int IsPrime (int);
static int MakePrime (int);
static EIT_RETURNCODE Rehash (EIT_DATATABLE *);

/* double the number of slots in data table when reallocation is needed */
#define INCREASE_MULTIPLIER 2

/* reallocation is needed when there is an average of 5 nodes per slot
in the data table */
#define MAX_DATA_PER_NODE   5

/*
The size of the data table will always be a prime number to get a better
distribution.
*/

/*********************************************************************
Allocates a data table.
the size is only a suggestion, see comment.
*********************************************************************/
EIT_DATATABLE * EI_DataTableAllocate (
    int Size)
{
    EIT_DATATABLE * DataTable;
    int i;

    DataTable = STC_AllocateMemory (sizeof *DataTable);
    if (DataTable == NULL) return NULL;

    /* we get a better distribution when the table size is prime */
    DataTable->Size = MakePrime (Size);
    DataTable->NumberOfData = 0;

    DataTable->Data = STC_AllocateMemory (
        sizeof *DataTable->Data * DataTable->Size);
    if (DataTable->Data == NULL) return NULL;

    for (i = 0; i < DataTable->Size; i++)
        DataTable->Data[i] = NULL;

    return DataTable;
}
/*********************************************************************
Empty a data table.
*********************************************************************/
void EI_DataTableEmpty (
    EIT_DATATABLE * DataTable)
{
    EIT_DATA * Data;
    EIT_DATA * DataNext;
    int i;

    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = DataNext) {
            DataNext = Data->Next;
            EI_DataTableFreeItem (Data);
        }
        DataTable->Data[i] = NULL;
    }
    DataTable->NumberOfData = 0;
}
/*********************************************************************
Frees a data table.
*********************************************************************/
void EI_DataTableFree (
    EIT_DATATABLE * DataTable)
{
    if (DataTable == NULL)
        return;

    EI_DataTableEmpty (DataTable);
    if (DataTable->Data != NULL)
        STC_FreeMemory (DataTable->Data);
    STC_FreeMemory (DataTable);
}
/*********************************************************************
Frees a data item.
*********************************************************************/
void EI_DataTableFreeItem (
    EIT_DATA * Data)
{
    STC_FreeMemory (Data->Key);
    EI_DataRecFree (Data->DataRec[EIE_PERIOD_CURRENT]);
    STC_FreeMemory (Data->DataRec[EIE_PERIOD_CURRENT]);
    EI_DataRecFree (Data->DataRec[EIE_PERIOD_HISTORICAL]);
    STC_FreeMemory (Data->DataRec[EIE_PERIOD_HISTORICAL]);
    STC_FreeMemory (Data);
}
/*********************************************************************
Find name in data table.
with optional create.
*********************************************************************/
EIT_DATATABLELOOKUP_RETURNCODE EI_DataTableLookup (
    EIT_DATATABLE * DataTable,
    char * Key,
    EIT_DATATABLELOOKUPTYPE LookupType,
    EIT_DATA ** Data)
{
    int h;
    EIT_DATA * pData;

    if (LookupType == EIE_DATATABLELOOKUPTYPE_CREATE &&
            DataTable->NumberOfData > DataTable->Size * MAX_DATA_PER_NODE)
        if (Rehash (DataTable) == EIE_FAIL)
            return EIE_DATATABLELOOKUP_FAIL;

    h = UTIL_Hash (Key, DataTable->Size);

    for (pData = DataTable->Data[h]; pData != NULL; pData = pData->Next) {
        if (strcmp (pData->Key, Key) == 0) {
            *Data = pData;
            return EIE_DATATABLELOOKUP_FOUND;
        }
    }

    *Data = NULL;
    if (LookupType == EIE_DATATABLELOOKUPTYPE_CREATE) {
        pData = STC_AllocateMemory (sizeof *pData);
        if (pData == NULL) return EIE_DATATABLELOOKUP_FAIL;

        pData->Key = STC_StrDup (Key);
        if (pData->Key == NULL) return EIE_DATATABLELOOKUP_FAIL;

        pData->ExcludedObs[EIE_PERIOD_CURRENT] = EIE_INCLUDED;
        pData->ExcludedObs[EIE_PERIOD_HISTORICAL] = EIE_INCLUDED;
        pData->DataRec[EIE_PERIOD_CURRENT] = NULL;
        pData->DataRec[EIE_PERIOD_HISTORICAL] = NULL;
        pData->Next = DataTable->Data[h];
        DataTable->Data[h] = pData;
        DataTable->NumberOfData++;

        *Data = pData;
    }

    return EIE_DATATABLELOOKUP_NOTFOUND;
}
/*********************************************************************
Prints data table.
*********************************************************************/
void EI_DataTablePrint (
    EIT_DATATABLE * DataTable)
{
    EIT_DATA * Data;
    int i;

    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
            EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "k=%s er=(%d %d) (hash=%d)\n",
                Data->Key,
                Data->ExcludedObs[EIE_PERIOD_CURRENT],
                Data->ExcludedObs[EIE_PERIOD_HISTORICAL],
                i);
            EI_DataRecPrint (Data->DataRec[EIE_PERIOD_CURRENT]);
            if (Data->DataRec[EIE_PERIOD_HISTORICAL] != NULL)
                EI_DataRecPrint (Data->DataRec[EIE_PERIOD_HISTORICAL]);
            else
                EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "no historical data");
        }
    }
}
/*********************************************************************
Prints data table statistics.
*********************************************************************/
void EI_DataTablePrintStatistics (
    EIT_DATATABLE * DataTable)
{
    EIT_DATA * Data;
    int i;
    int LinkLength;
    int LongestLinkLength;
    int NumberOfEmptySlots;
    int NumberOfFilledSlots;
    int NumberOfCurrentItems;
    int NumberOfHistoricalItems;

    NumberOfEmptySlots = 0;
    NumberOfCurrentItems = 0;
    NumberOfHistoricalItems = 0;
    LongestLinkLength = 0;

    for (i = 0; i < DataTable->Size; i++) {
        if (DataTable->Data[i] == NULL)
            NumberOfEmptySlots++;

        LinkLength = 0;
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
            LinkLength++;
            NumberOfCurrentItems++;
            if (Data->DataRec[EIE_PERIOD_HISTORICAL] != NULL)
                NumberOfHistoricalItems++;
        }
        if (LinkLength > LongestLinkLength)
            LongestLinkLength = LinkLength;
    }

    NumberOfFilledSlots = DataTable->Size - NumberOfEmptySlots;

    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberOfData            = %5d\n", DataTable->NumberOfData);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberOfSlots           = %5d\n", DataTable->Size);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberOfEmptySlots      = %5d    %.2f%% \n",
        NumberOfEmptySlots, 100*(double)NumberOfEmptySlots/DataTable->Size);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberOfFilledSlots     = %5d    %.2f%% \n",
        NumberOfFilledSlots, 100*(double)NumberOfFilledSlots/DataTable->Size);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberOfCurrentItems    = %5d\n", NumberOfCurrentItems);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberOfHistoricalItems = %5d\n", NumberOfHistoricalItems);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "LongestLinkLength       = %5d\n", LongestLinkLength);
    if (NumberOfFilledSlots != 0)
        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "AverageLinkLength       = %8.2f\n",
            (double) NumberOfCurrentItems/NumberOfFilledSlots);
    else
        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "AverageLinkLength       = %8.2f\n", 0.0);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "");
}


/*********************************************************************
returns 0 if n is non-prime, 1 if n is prime
*********************************************************************/
static int IsPrime (int n)
{
    int i;
    int prime;
    int root;

    switch (n) {
    case 0: case 1: case 4: case 6: case 8: case 9: case 10:
        prime = 0;
        break;
    case 2: case 3: case 5: case 7:
        prime = 1;
        break;
    default:
        if (n % 2 == 0 || n % 3 == 0 || n % 5 == 0) {
            prime = 0;
        }
        else {
            prime = 1;
            root = (int) sqrt (n);

            for (i = 7; i <= root; i += 6) {
                if (n % i == 0 || n % (i+4) == 0) {
                    return 0;
                }
            }
        }
        break;
    }

    return prime;
}
/*********************************************************************
returns the next prime number equal or greater than n
*********************************************************************/
static int MakePrime (
    int n)
{
    while (!IsPrime (n))
        n++;
    return n;
}
/*********************************************************************
data table is too small, increase its size.
*********************************************************************/
static EIT_RETURNCODE Rehash (
    EIT_DATATABLE * DataTable)
{
    EIT_DATA * Data;
    EIT_DATA * DataNext;
    unsigned int h;
    int i;
    EIT_DATA ** OldDataArray;
    int OldSize;

/*    EI_DataTablePrintStatistics (DataTable);*/

    OldSize = DataTable->Size;
    OldDataArray = DataTable->Data;

    /* we get a better distribution when the table size is prime */
    DataTable->Size = MakePrime (DataTable->NumberOfData*INCREASE_MULTIPLIER+1);
    DataTable->Data = STC_AllocateMemory (
        sizeof *DataTable->Data * DataTable->Size);
    if (DataTable->Data == NULL) return EIE_FAIL;

    for (i = 0; i < DataTable->Size; i++)
        DataTable->Data[i] = NULL;

    for (i = 0; i < OldSize; i++) {
        for (Data = OldDataArray[i]; Data != NULL; Data = DataNext){

             h = UTIL_Hash (Data->Key, DataTable->Size);

             DataNext = Data->Next;
             Data->Next = DataTable->Data[h];
             DataTable->Data[h] = Data;
        }
    }

    STC_FreeMemory (OldDataArray);

/*    EI_DataTablePrintStatistics (DataTable);*/
/*    EI_DataTablePrint(DataTable);*/

    return EIE_SUCCEED;
}
