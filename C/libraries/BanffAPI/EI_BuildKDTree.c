/* ------------------------------------------------------------------ */
/*                                                                    */
/* NAME            EI_BuildKDTree                                     */
/*                                                                    */
/* DESCRIPTION     This function builds the k-d tree used in the      */
/*                 search of a closest donor.                         */
/*                                                                    */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/*       Include Header Files                                         */
/* ------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "EI_Common.h"
#include "EI_Donor.h"
#include "EI_Message.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "Donor.h"

#define   BUCKETSIZE 16

/* return codes for FindBranchField */
typedef enum {
    CREATETREE_FAIL    = EIE_FAIL,
    CREATETREE_SUCCEED = EIE_SUCCEED
}CREATETREE_RETURNCODE;

/* ------------------------------------------------------------------ */
/*       Local functions                                              */
/* ------------------------------------------------------------------ */

static EIT_KDTREE * AllocateNode (void);
static CREATETREE_RETURNCODE CreateTree (EIT_ALL_DONORS *, EIT_KDTREE *);
static int FindBranchField (EIT_ALL_DONORS *, int, int);
static double GetSplitValue (EIT_ALL_DONORS *, int, int, int);
static void SortTransValues (int, int, int, EIT_ALL_DONORS *);

static void UTIL_PrintTime (void);

/* ------------------------------------------------------------------ */

EIT_KDTREE_RETURNCODE EI_BuildKDTree (
    /* Pointer to structure with transformed values of all matching fields*/
    /* for donors only*/
    /* Pointeur à la structure contenant les valeurs transformées de*/
    /* tous les champs d'appariement pour les donneurs seulement*/
    EIT_ALL_DONORS *AllDonors,
    /* K-D Tree Structure/Structure de l'arbre K-D*/
    EIT_KDTREE **KDTree,
    /* Transformed values of all fields/Valeurs transformées de tous*/
    /* les champs.*/
    EIT_TRANSFORMED * AllTransformed,
    /* All matching fields structure */
    /**/
    /* Structure de tous les champs d'appariement */
    EIT_FIELDNAMES * MatchingFields,
    /* All Respondents Structure / Structure de tous les répondants */
    EIT_ALL_RESPONDENTS * AllRespondents)
{
    DONORT_CREATEALLDONORS_RETURNCODE rcode;
    EIT_KDTREE * RootNode;
    CREATETREE_RETURNCODE CreateTree_rcode;

    rcode = DONOR_CreateAllDonors (AllRespondents, AllDonors, MatchingFields,
                AllTransformed);

    if (rcode == DONORE_CREATEALLDONORS_FAIL)
        return EIE_KDTREE_FAIL;

    RootNode = AllocateNode ();
    if (RootNode == NULL)
        return EIE_KDTREE_FAIL;

    *KDTree = RootNode;

    if (AllDonors->NumberOfFields > 0)
    {
                             /* This node is associated with all      */
                             /* donor records                         */
        RootNode->start = 0;
        RootNode->end   = AllDonors->NumberOfDonors - 1;

                             /* Create the Donor Search Tree          */
        CreateTree_rcode = CreateTree (AllDonors, RootNode);
        if (CreateTree_rcode == CREATETREE_FAIL)
            return EIE_KDTREE_FAIL;
    }

    return EIE_KDTREE_SUCCEED;
}
/*******************************************
Print the k-d tree
For debugging
*******************************************/
void EI_PrintKDTree (
    EIT_KDTREE * KDTree,
    int Depth)
{
    int i;
	char s[10001];
	char t[10001];

	s[0] = '\0';
	if (Depth == 0) {
		sprintf (t, "ROOT:     ");
		strcat (s, t);
	}
    else {
		for (i = 0; i < Depth; i++) {
			sprintf (t, "  ");
			strcat (s, t);
		}
		if (KDTree->branchid == TERMNODE) {
			sprintf (t, "TERMINAL: ");
			strcat (s, t);
		}
		else {
			sprintf (t, "Node:     ");
			strcat (s, t);
		}
    }
	for (i = Depth; i < 11; i++) {
		sprintf (t, ". ");
		strcat (s, t);
	}
    if (KDTree->branchid == TERMNODE) {
		sprintf (t, "Records[%5d - %5d]", KDTree->start, KDTree->end);
		strcat (s, t);
		EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, s);
    }
    else {
        sprintf (t, "Records[%5d - %5d]  SC=%-2d  SV=%.7f",
			KDTree->start, KDTree->end, KDTree->branchid, KDTree->splitval);
		strcat (s, t);
		EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, s);
        Depth++;
        EI_PrintKDTree (KDTree->leftson, Depth);
        EI_PrintKDTree (KDTree->rightson, Depth);
    }
}
/* ------------------------------------------------------------------ */
/*                                                                    */
/*       Allocate a Donor Search Tree node                            */
/*                                                                    */
/* ------------------------------------------------------------------ */
static EIT_KDTREE * AllocateNode (void)
{
    EIT_KDTREE *CurrentNode;     /* Pointer to a Donor Search Tree Node   */

                             /* Allocate a donor search tree node     */
    CurrentNode = STC_AllocateMemory (sizeof *CurrentNode);
    if (CurrentNode == NULL)
        return NULL;

    CurrentNode->start = TERMNODE;
    CurrentNode->end   = TERMNODE;

    CurrentNode->leftson  = (EIT_KDTREE *) NULL;
    CurrentNode->rightson = (EIT_KDTREE *) NULL;

    CurrentNode->branchid = TERMNODE;
    CurrentNode->splitval = TERMNODE;

    return (CurrentNode);
}

/* ------------------------------------------------------------------ */
/*                                                                    */
/*       Create the current node                                      */
/*                                                                    */
/* ------------------------------------------------------------------ */
static CREATETREE_RETURNCODE CreateTree (
    EIT_ALL_DONORS *AllDonors,  /* Pointer to the transformed values     */
    EIT_KDTREE *CurrentNode)    /* Pointer to the current node           */
{
  register int i;
  int rcode;
                             /* number of donor records <= bucket size*/
                             /* Note: # of donor equal                */
                             /*           'end - start + 1'           */
                             /*                                       */
                             /* 'end - start + 1' <= bucket is like   */
                             /* 'end - start' < bucket size           */
  if ((CurrentNode->end - CurrentNode->start) < BUCKETSIZE)
     return TERMNODE;
                             /* Find the branching field. If none is  */
                             /* found then this is a terminal node.   */
  CurrentNode->branchid =
             FindBranchField (AllDonors, CurrentNode->start, CurrentNode->end);

  if (CurrentNode->branchid == TERMNODE)
     return CREATETREE_SUCCEED;
                              /* This node is not a terminal node     */

                              /* Sort the transformed values          */
                              /* associated with the current node and */
                              /* branching field                      */
  SortTransValues (CurrentNode->start, CurrentNode->end,
                  CurrentNode->branchid, AllDonors);

                             /* calculate a value to split the set of */
                             /* transformed values associated with    */
                             /* this node and the branching field     */
                             /* into two groups.                      */
  CurrentNode->splitval = GetSplitValue (AllDonors, CurrentNode->start,
                                CurrentNode->end, CurrentNode->branchid);

                             /* Allocate the left child node          */
  CurrentNode->leftson = AllocateNode ();
  if (CurrentNode->leftson == NULL)
    return CREATETREE_FAIL;

                             /* Allocate the right child node         */
  CurrentNode->rightson = AllocateNode ();
  if (CurrentNode->rightson == NULL)
    return CREATETREE_FAIL;

                             /* Determine which record ids belong to  */
                             /* each node. If the transformed value of*/
                             /* donor is <= split value then it is    */
                             /* assigned to the left child node.      */
                             /* Otherwise, to the right child.        */
  CurrentNode->leftson->start = CurrentNode->start;
  CurrentNode->rightson->end  = CurrentNode->end;

  for (i = CurrentNode->start; i <= CurrentNode->end; i++)
     if (AllDonors->Value[i][CurrentNode->branchid] > CurrentNode->splitval)
     {
        CurrentNode->leftson->end    = i - 1;
        CurrentNode->rightson->start = i;
        break;
     }
                             /* create the left child node            */
  rcode = CreateTree (AllDonors, CurrentNode->leftson);
  if (rcode == CREATETREE_FAIL)
    return CREATETREE_FAIL;

                             /* create the right child node           */
  rcode = CreateTree (AllDonors, CurrentNode->rightson);
  if (rcode == CREATETREE_FAIL)
    return CREATETREE_FAIL;

  return CREATETREE_SUCCEED;
}

/* ------------------------------------------------------------------ */
/*                                                                    */
/*  Find the branching field by choosing the match field with the     */
/*  largest range between its maximum and minimum transformed values. */
/*  If two or more match fields have the same range then the first is */
/*  arbitrarily the branching field. If the maximum range is 0, then  */
/*  all donors have the same values within each match field, and      */
/*  therefore no branching field exists.                              */
/*                                                                    */
/* ------------------------------------------------------------------ */
static int FindBranchField (
    EIT_ALL_DONORS *AllDonors,/* Pointer to transformed values table   */
    int start,                /* The range of transformed values used  */
    int end)                  /* in finding the branching field        */
{
  double max_min;            /* The maximun minus minimun is assigned */
                             /* to a temporary variable to avoid round*/
                             /* off error (max - min will be double)  */

  double minval;             /* The minimum transformed value         */
  double maxval;             /* The maximum transformed value         */
  double range;              /* The largest difference between minval */
                             /* and maxval                            */
  int branchid;              /* Match field with the largest range    */
  int col;                   /* Index counter                         */
  int row;                   /* Index counter                         */

  range = DBL_EPSILON;
                             /* If no range is found then an invalid  */
                             /* branching field will be returned      */
  branchid = TERMNODE;

                             /* For each match field                  */
  for (col = 0; col < AllDonors->NumberOfFields; col ++)
  {
                             /* Find the min and max transformed value*/
                             /* for the current match field           */
     minval = AllDonors->Value[start][col];
     maxval = minval;
     for (row = start + 1; row <= end; row++)
     {
        if (AllDonors->Value[row][col] < minval)
           minval = AllDonors->Value[row][col];
        else if (AllDonors->Value[row][col] > maxval)
           maxval = AllDonors->Value[row][col];
     }
                             /* If the difference between max and min */
                             /* values of this match field is >       */
                             /* the difference between any previous   */
                             /* one, then this match field becomes    */
                             /* the new branching field.              */
     max_min =  maxval - minval;
/*
printf ("min ==> %.16f max ==> %.16f col ==> %d\n", minval, maxval, col);
printf ("max_min ==> %.16f range ==> %.16f\n", max_min, range);
*/
     if (max_min > range)
     {
        range = max_min + EIM_DBL_EPSILON;
        branchid = col;
     }
  }
  return (branchid);
}


/* ------------------------------------------------------------------ */
/*       Find the split value                                         */
/* ------------------------------------------------------------------ */
static double GetSplitValue (
    EIT_ALL_DONORS *AllDonors,/* donor records                         */
    int start,                /* Set of donor records on which to      */
    int end,                  /* calculate the split value             */
    int branch)               /* The field on which to find split value*/
{
   int   numelem;            /* Number of elements involved           */
   int   midpos;             /* Middle position in the sorted array   */
   int   midstart;           /* Middle value starting and ending index*/
   int   midend;             /*                                       */
   double midval;            /* The middle value                      */
   double splitval;          /* The infamous split value              */
   int   i;                  /* Index counter                         */

                             /* The number of transformed values      */
                             /* associated with this node             */
   numelem = end - start + 1;
                             /* Determine the position of the middle  */
                             /* transformed value in the sorted set   */
                             /* NB: only the integer portion is saved */
                             /* NB: -1 because C arrays begin at 0    */
   midpos =  start + ((numelem + 1) / 2) - 1;
   midval =  AllDonors->Value[midpos][branch];

                             /* Determine the position of the next    */
                             /* smallest value in the ordered set     */

   midstart = start - 1;
   for (i = midpos -1; i >= start; i--)
    {
      if (AllDonors->Value[i][branch] <  midval)
      {
         midstart = i;
         break;
      }
    }
                             /* Determine the position of the next    */
                             /* largest value in the ordered set      */

   midend = end + 1;        /* An invalid array entry number         */
   for (i = midpos + 1; i <= end; i++)
    {
      if (AllDonors->Value[i][branch] > midval)
      {
         midend = i;
         break;
      }
    }
                             /* If number of elements before midvalue */
                             /*    is < than the number of elements   */
                             /*    after the midvalue                 */
                             /* then                                  */
                             /*     the splitvalue is the average of  */
                             /*     the next value greater than the   */
                             /*     midvalue and the midvalue itself  */
                             /* Otherwise                             */
                             /*     splitvalue is the average of the  */
                             /*     first value below the midvalue and*/
                             /*     the midvalue itself.              */
   if ((midstart - start) <  (end - midend))
      splitval = ((midval + AllDonors->Value[midend][branch]) / 2);
   else
      splitval = ((midval + AllDonors->Value[midstart][branch]) / 2);

   return (splitval);
}

/* ------------------------------------------------------------------ */
/* Sort in ascending order the values of the branching field          */
/* ------------------------------------------------------------------ */
static void SortTransValues (
    int start,                 /* lowest  index in subcolumn to be sorted     */
    int end,                   /* highest index in subcolumn to be sorted     */
    int sortid,                /* index of the column to be sorted            */
    EIT_ALL_DONORS * AllDonors)/* matrix of donor match field and edit fields */
{
    int     gap;
    int     i;
    int     j;
    int     lowpos;
    int     hipos;
    int     numelem;     /* number of elements in subcolumn           */
    double *ptrdouble;   /* used while swapping positions of 2 donors */
    char   *ptrchar;
    EIT_DONOR_STATUSFLAG flag;

    numelem = end - start + 1;

    for (gap = numelem / 2; gap > 0; gap /= 2)
    {
        for (i = gap; i < numelem; i++)
        {
            for (j = i - gap; j >= 0; j -= gap)
            {
                lowpos = j + start;
                hipos  = lowpos + gap;
                if (AllDonors->Value[lowpos][sortid] <=
                        AllDonors->Value[hipos][sortid])
                    break;

                ptrdouble = AllDonors->Value[lowpos];
                AllDonors->Value[lowpos] = AllDonors->Value[hipos];
                AllDonors->Value[hipos]  = ptrdouble;

                /* swap keys, make them follow data */
                ptrchar = AllDonors->Key[lowpos];
                AllDonors->Key[lowpos] = AllDonors->Key[hipos];
                AllDonors->Key[hipos]  = ptrchar;

                /* swap flags, make them follow data */
                flag = AllDonors->DonorStatusFlag[lowpos];
                AllDonors->DonorStatusFlag[lowpos] =
                    AllDonors->DonorStatusFlag[hipos];
                AllDonors->DonorStatusFlag[hipos]  = flag;
            }
        }
    }
}

static void UTIL_PrintTime (void) {
    time_t t;
    t = time (NULL);
    printf ("%s", ctime (&t));
}
