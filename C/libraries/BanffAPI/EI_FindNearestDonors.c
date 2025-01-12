/*----------------------------------------------------------------------*/
/* NAME:        EI_FindNearestDonors                                    */
/*                                                                      */
/* DESCRIPTION: For one recipient with matching fields, search "donors" */
/*              in the KD tree and populate "nearest donors" structure. */
/* Pre-requisite:  Observation processed MUST be a recipient.           */
/*----------------------------------------------------------------------*/

/* For ShowTime():microsec. */
/*#include <sys/time.h>*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Donor.h"
#include "EI_Edits.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "Donor.h"
#include "util.h"

/* ------------------------------------------------------------------ */
/* Preprocessor Symbols and Macros:                                   */
/*--------------------------------------------------------------------*/
#define BRANCH    1
#define NOBRANCH  0
#define OVERLAP   1
#define NOOVERLAP 0

/*--------------------------------------------------------------------*/
/* Local Functions                                                    */
/*--------------------------------------------------------------------*/
static int    bndtest (EIT_TREESEARCH *, int *, double *, double *, double *);
static double calcdist (double *, int *, double *);
static void   initbnds (int, double *, double *);
static int    isbranch (int, int*);
static void   movelists (EIT_TREESEARCH *);
static void   nodeterm (EIT_ALL_DONORS *, int, EIT_KDTREE *, int *, int *, double *, EIT_TREESEARCH *);
static void   print_nearest_donors (EIT_NEARESTDONORS *, EIT_ALL_DONORS *, char *);
static void   print_search_result (EIT_TREESEARCH *, char *);
static void   sortdnor (EIT_TREESEARCH *);
static void   srchdown (EIT_ALL_DONORS *, int, EIT_KDTREE *, double *, double *,
    int *, int *, double *, EIT_TREESEARCH *);

/* For debugging... Give time in microsec. */
/*static void ShowTime (*/
/*    char * Message)*/
/*{*/
/*    static double StaticTime = 0.0;*/
/**/
/*    double NewTime;*/
/*    struct timeval PtrTime;*/
/**/
/*    gettimeofday (&PtrTime, NULL);*/
/*    NewTime = (double) PtrTime.tv_sec + (double)PtrTime.tv_usec/1000000.0;*/
/*    printf("duree(sec)= %f %s\n", NewTime - StaticTime, Message);*/
/*    StaticTime = NewTime;*/
/*}*/

/*****************************************************************/
EIT_FINDNEARESTDONORS_RETURNCODE EI_FindNearestDonors (
    int N,                                /* # of nearest donors to return */
    EIT_DATAREC * RecipientData,          /* One Recipient Data */
    char * RecipientKey,
    int * FTIFields,                      /* List indexes of FTI fields */
    EIT_FIELDNAMES * AllMatchingFields,   /* All matching fields */
    EIT_TRANSFORMED * AllTransformed,     /* Transformed values of all*/
                                          /*  matching fields */
    EIT_ALL_DONORS * AllDonors,           /* Values of All Donors */
    EIT_KDTREE * KDTree,                  /* root node of the donor tree */
    EIT_NEARESTDONORS ** NearestDonors,   /* Populate: Nearest donors */
	int DonorLimit)
{
    double * lbound;
    int * MatchFields;
    double * MatchFieldsValues;
    int rc;
    EIT_TREESEARCH TreeSearch; /* Result of Tree Search */
    double * ubound;

    /* Allocate memory for all variables needed for TreeSearch    */
    /* (except FTIFields: already allocated and identified)       */
    /* For each recipient, call initbnds, srchdown, ...           */
    /* Return nearest donors (extract from TreeSearch structure). */
    rc = DONOR_AllocateTreeSearch (&MatchFields, &MatchFieldsValues,
        &lbound, &ubound, &TreeSearch, AllMatchingFields->NumberofFields,
        AllDonors->NumberOfDonors);
        
    if (rc == DONORE_ALLOCATETREESEARCH_FAIL)
        return EIE_FINDNEARESTDONORS_FAIL;

    DONOR_GetMatchFields (MatchFields, RecipientData, AllTransformed,
        AllMatchingFields);

    DONOR_CopyMatchFields (MatchFieldsValues, AllTransformed,
        RecipientKey, MatchFields);

    /* Initialize the search criteria */
    TreeSearch.needed    = N;
    TreeSearch.maxdist   = -1.0;
    TreeSearch.next_dl   = TreeSearch.d_list;
    TreeSearch.dl_size   = 0;
    TreeSearch.next_ml   = TreeSearch.m_list;
    TreeSearch.ml_size   = 0;
    TreeSearch.next_dist = TreeSearch.distances;

    initbnds (AllDonors->NumberOfFields, lbound, ubound);
    srchdown (AllDonors, DonorLimit, KDTree, lbound, ubound, FTIFields, MatchFields,
        MatchFieldsValues, &TreeSearch);
    sortdnor (&TreeSearch);
    movelists (&TreeSearch);

    if (TreeSearch.dl_size == 0) {
        rc = EIE_FINDNEARESTDONORS_NOTFOUND;
    }
    else {
        *NearestDonors = DONOR_AllocateNearestDonors (TreeSearch.dl_size);
        if (*NearestDonors == NULL) {
            rc = EIE_FINDNEARESTDONORS_FAIL;
        }
        else {
            DONOR_PopulateNearestDonors (TreeSearch.dl_size, TreeSearch.d_list,
                TreeSearch.distances, *NearestDonors);
            rc = EIE_FINDNEARESTDONORS_SUCCEED;
        }
    }

	DONOR_FreeTreeSearch (MatchFields, MatchFieldsValues, lbound, ubound, &TreeSearch);

	return rc;
}

/*--------------------------------------------------------------------*/
/* Perform the Bounds Overlap Ball test for a node.                   */
/* We are trying to prove that all donors in the node must have a     */
/* distance from the recipient which is at least > than the current   */
/* maxdist.                                                           */
/* This is proven if the difference between any recipient match field */
/* and the upper or lower bound (as the case may be) of the           */
/* corresponding donor match fields of the node is > maxdist.         */
/*--------------------------------------------------------------------*/
static int bndtest (
    EIT_TREESEARCH *search,   /* search criteria and results structure    */
    int      *xmch,           /* indexes of recipient match field columns */
    double   *vmch,           /* list of recipient match field values     */
    double   *lbound,         /* lower bounds of match columns for node_x */
    double   *ubound)         /* upper bounds of match columns for node_x */
{
    double diff;

    /*** We don't trim until we have the needed number of donors.
     ******************************************************************/
    if (search->dl_size + search->ml_size < search->needed)
        return OVERLAP;

    for (; *xmch != ENDLIST; xmch++)
    {
        if (vmch[*xmch] <= lbound[*xmch])
        {
            diff = fabs ((double) (lbound[*xmch] - vmch[*xmch]));
            if (diff >= search->maxdist)
                return NOOVERLAP;
        }
        else if (vmch[*xmch] > ubound[*xmch])
        {
            diff = fabs ((double) (vmch[*xmch] - ubound[*xmch]));
            if (diff > search->maxdist)
                return NOOVERLAP;
        }
    }

    return OVERLAP;
}

/*--------------------------------------------------------------------*/
/* Compute the distance between a donor and a recipient record.       */
/* This is the maximum absolute difference between any recipient      */
/* transformed match field and the corresponding donor match field.   */
/*--------------------------------------------------------------------*/
static double calcdist (
    double  *donval,   /* donor transformed match fields, edit fields */
    int     *xmch,     /* list of recipient match field indexes       */
    double  *vmch)     /* list of recipient match field values        */
{
    double  diff;      /* difference between recipient and donor      */
    double  maxdiff;   /* maximum difference                          */
    char    roundstr[10];

    maxdiff = -1.0;
    while (*xmch != ENDLIST)        /* for each recipient match field */
    {
        diff = fabs ((double) (vmch[*xmch] - donval[*xmch]));
        if (diff > maxdiff)
            maxdiff = diff;
        xmch++;
    }

    /* rounding: not to have the same calculated distance
       for a "lot" of donors if keeeping all decimals */
    sprintf (roundstr, "%.7f", maxdiff);
    maxdiff = atof (roundstr);
    return maxdiff;
}

/*--------------------------------------------------------------------*/
/* Initialize the upper and lower bounds of the match columns for the */
/* root node.                                                         */
/*--------------------------------------------------------------------*/
static void initbnds (
    int       mc_count,   /* the number of match columns              */
    double   *lbound,     /* lower bounds of match columns for a node */
    double   *ubound)     /* upper bounds of match columns for a node */
{
    int i;

    for (i = 0; i < mc_count; i++)
    {
        lbound[i] = 0.0;
        ubound[i] = 1.0;
    }
    lbound[i] = ENDLIST;
    ubound[i] = ENDLIST;
}

/*--------------------------------------------------------------------*/
/* Check if the branching field is a matching field.                  */
/*--------------------------------------------------------------------*/
static int isbranch (
    int  brnchfld,  /* index of the branching field                   */
    int *xmch)      /* list of indexes of recipient match fields      */
{
    while (*xmch != ENDLIST)
    {
        if (*xmch++ == brnchfld)
            return BRANCH;
    }
    return NOBRANCH;
}

/****************************************/
/* Move all donors in m_list to d_list. */
/****************************************/
static void movelists (
    EIT_TREESEARCH *search)
{
    int i;

    for (i = 1; i <= search->ml_size; i++)
    {
        *search->next_dl++   = search->next_ml[i];
        *search->next_dist++ = search->maxdist;
    }
    search->dl_size += search->ml_size;
    search->ml_size = 0;
    search->next_ml = search->m_list;
}

/*--------------------------------------------------------------------*/
/* Review each donor in the terminal node (bucket) to see if it       */
/* should be added to the list of nearest donors.                     */
/* A donor is rejected if:                                            */
/*     - one of its fields, corresponding to a recipient FTI field,   */
/*       is an ODE field;                                             */
/*     - its distanct is > maximum distance so far and there are      */
/*       already enough donors in d_list + m_list.                    */
/* Otherwise, it is added to either d_list or m_list, depending on    */
/* its distance.                                                      */
/*--------------------------------------------------------------------*/
static void nodeterm (
    EIT_ALL_DONORS * AllDonors,/* ordered list of all donors, their values */
	int DonorLimit,
    EIT_KDTREE * node_x,       /* current node of the donor tree           */
    int * xfti,                /* indexes of recipient FTI field columns   */
    int * xmch,                /* indexes of recipient match field columns */
    double * vmch,             /* list of recipient match field values     */
    EIT_TREESEARCH *search)    /* search criteria and results structure    */
{
    int     i;
    double  next_dist;  /* distance of nextdonor                      */
    double *next_vals;  /* values array of nextdonor                  */
    int     next_i;     /* index in AllDonors of nextdonor            */

    int NumberAtMaxDist;
    int Overlap;
    int * temp_m_list;

    for (next_i = node_x->start; next_i <= node_x->end; next_i++) {
		if (AllDonors->Gave[next_i] >= DonorLimit) /* I give, I give, and this is the thanks I get! */
			continue; /* reject nextdonor: he gave enough */

        if (DONOR_HasFIELDFTE(AllDonors, next_i, xfti))
            continue; /* reject nextdonor */

        next_vals = AllDonors->Value[next_i];
        next_dist = calcdist (next_vals, xmch, vmch);


        /*** If nextdonor has the maximum distance add it to m_list.
         **************************************************************/
        if (next_dist == search->maxdist)
        {
            *search->next_ml-- = next_i;
            search->ml_size++;
        }


        /*** if nextdonor has > maximum distance but we do not have
         *** enough donors yet:
         *** - move all donors in m_list to d_list; and
         *** - start a new m_list with nextdonor.
         **************************************************************/
        else if (next_dist > search->maxdist)
        {
            if (search->dl_size + search->ml_size < search->needed)
            {
                for (i = 1; i <= search->ml_size; i++)
                {
                    *search->next_dl++   = search->next_ml[i];
                    *search->next_dist++ = search->maxdist;
                }
                search->dl_size   += search->ml_size;
                search->next_ml    = search->m_list;

                *search->next_ml-- = next_i;
                search->ml_size    = 1;
                search->maxdist    = next_dist;
            }
            else
                continue;                         /* reject nextdonor */
        }


        /*** If nextdonor has < maximum distance:
         *** - add it to d_list;
         *** Now, if d_list is full:
         *** - throw away m_list;
         *** - find a new maximum distance in d_list;
         *** - create a new m_list with all donors in d_list that have
         ***   the new maximum distance; and at the same time
         *** - compress d_list because of the removed donors.
         **************************************************************/
        else
        {
            *search->next_dl++   = next_i;
            *search->next_dist++ = next_dist;
            search->dl_size++;

            if (search->dl_size == search->needed)
            {
                /* d_list is full */

                search->maxdist = -1.0;
                for (i = 0; i < search->dl_size; i++)
                    if (search->distances[i] > search->maxdist)
                        search->maxdist = search->distances[i];

                NumberAtMaxDist = 0;
                for (i = 0; i < search->dl_size; i++)
                    if (search->distances[i] == search->maxdist)
                        NumberAtMaxDist++;

                /*
                We must rearrange old d_list into new d_list and new m_list.
                We have to be carefull to not overwrite info if there is an
                overlap. The overlap occurs when more donors are at maxdist
                than there is room to fit them at the end of d_list.
                When the overlap condition occurs, we must allocate (and free)
                temporary space.
                */

                if (NumberAtMaxDist > AllDonors->NumberOfDonors-search->dl_size)
                    Overlap = 1;
                else
                    Overlap = 0;

                if (Overlap) {
                    temp_m_list = STC_AllocateMemory (
                        NumberAtMaxDist * sizeof *temp_m_list);
                    search->next_ml = temp_m_list+NumberAtMaxDist-1;
                }
                else
                    search->next_ml = search->m_list;

                search->ml_size = 0;

                search->next_dl   = search->d_list;
                search->next_dist = search->distances;
                for (i = 0; i < search->dl_size; i++)
                {
                    if (search->distances[i] == search->maxdist)
                    {
                        *search->next_ml-- = search->d_list[i];
                        search->ml_size++;
                    }
                    else
                    {
                        *search->next_dl++   = search->d_list[i];
                        *search->next_dist++ = search->distances[i];
                    }
                }
                search->dl_size -= search->ml_size;

                if (Overlap) {
                    search->next_ml = search->m_list;
                    for (i = 0; i < search->ml_size; i++)
                        *search->next_ml-- = temp_m_list[i];
                    STC_FreeMemory (temp_m_list); /* unusual but necessary! */
                }
            }
        }
    }
}

/*--------------------------------------------------------------------*/
/* Print the "nearest donors" information for one recipient           */
/* (to debug)                                                         */
/*--------------------------------------------------------------------*/
static void print_nearest_donors (
    EIT_NEARESTDONORS * DonorsFound,
    EIT_ALL_DONORS * AllDonors,
    char * RecipientKey)
{
    int i;
    int donor_row;
    
    printf ("\n\n-------- NEAREST DONORS --------\n");
    printf ("Recipient key: %s\n", RecipientKey);
    printf ("Number of nearest donors: %d\n", DonorsFound->NumberofDonors);
    printf ("\n");

    if (DonorsFound->NumberofDonors > 0) {
       printf ("       donor index      distance     donor key\n");
       for (i = 0; i < DonorsFound->NumberofDonors; i++) {
            donor_row = DonorsFound->DonorsIndex[i];
            printf ("%5d:  %-14d  %.7f    %s\n", i, DonorsFound->DonorsIndex[i],
                DonorsFound->Distances[i], AllDonors->Key[donor_row]);
       }
    }

    printf ("---------------------------------------------------\n\n");
}

/*--------------------------------------------------------------------*/
/* Print the search structure.                                        */
/* (to debug)                                                         */
/*--------------------------------------------------------------------*/
static void print_search_result (
    EIT_TREESEARCH *search,
    char *messg)
{
    int i;

    printf ("\n\n-------- SEARCH: %s--------\n", messg);
    printf ("needed:     %d\n", search->needed);
    printf ("maxdist:    %.7f\n", search->maxdist);
    printf ("\n");

    printf ("DL:    size = %d\n", search->dl_size);
    printf ("       donor index    distance\n");
    for (i = 0; i < search->dl_size; i++) {
        /*printf ("%5d:  %-14d  %.7f %.7f\n", i, search->d_list[i],
                search->distances[i] , search->next_dist[i]);*/
        printf ("%5d:  %-14d  %.7f\n", i, search->d_list[i],
                search->distances[i]);
    }

    if (search->ml_size > 0) {
        printf ("\n");
        printf ("ML:    size = %d\n", search->ml_size);
        printf ("       donor index    distance\n");
        for (i = 1; i <= search->ml_size; i++)
            printf ("%5d:  %-14d  %.7f\n",
                              i-1, search->next_ml[i], search->maxdist);
    }
    printf ("---------------------------------------------------\n\n");
}

/*--------------------------------------------------------------------*/
/* Sort the d_list and the corresponding distances list in increasing */
/* order of distances.                                                */
/*--------------------------------------------------------------------*/
static void sortdnor (
    EIT_TREESEARCH *search)
{
    int    i, j, gap, k, tmpindx;
    double tmpdist;

    for (gap = search->dl_size  / 2; gap > 0; gap /= 2)
        for (i = gap; i < search->dl_size; i++)
            for (j = i - gap; j >= 0; j -= gap) {
                k = j + gap;
                if (search->distances[j] <= search->distances[k])
                   break;

                tmpdist = search->distances[j];
                search->distances[j] = search->distances[k];
                search->distances[k] = tmpdist;

                tmpindx = search->d_list[j];
                search->d_list[j] = search->d_list[k];
                search->d_list[k] = tmpindx;
            }
}

/*--------------------------------------------------------------------*/
/* Search down a complete branch (starting at node_x) of the donor    */
/* tree for nearest donors to the current recipient.                  */
/* This is a recursive function which will call itself on the leftson */
/* and on the rightson of node_x, unless one of these sub-branches    */
/* can be trimmed from the search on the basis of the bounds overlap  */
/* test.                                                              */
/* The recursion terminates at each node which is a terminal node.    */
/*--------------------------------------------------------------------*/
static void srchdown (
    EIT_ALL_DONORS *AllDonors,    /* ordered list of all donors, their values */
	int DonorLimit,
    EIT_KDTREE     *node_x,       /* current node of the donor tree           */
    double         *lbound,       /* lower bounds of match columns for node_x */
    double         *ubound,       /* upper bounds of match columns for node_x */
    int            *xfti,         /* indexes of recipient FTI fields columns  */
    int            *xmch,         /* indexes of recipient match field columns */
    double         *vmch,         /* list of recipient match field values     */
    EIT_TREESEARCH *search)       /* search criteria and results structure    */
{
    double    savebound;

    if (node_x->branchid == TERMNODE)
        nodeterm (AllDonors, DonorLimit, node_x, xfti, xmch, vmch, search);

    /*** The bounds overlap test is only performed when the branching
     *** field of node_x is a recipient match field because this is
     *** the only case when the adjustment of bounds for leftson and
     *** rightson have any significance to the distance calculation for
     *** the current recipient.
     *** If the recipient value of the branch field is <= the split
     *** value of the node, it may be possible to trim the rightson
     *** branch; else if recipient value > split value, it may be
     *** possible to trim the leftson branch.
     ******************************************************************/
    else if (isbranch (node_x->branchid, xmch) == BRANCH)
    {
        if (vmch[node_x->branchid] <= node_x->splitval) {
            savebound = ubound[node_x->branchid];
            ubound[node_x->branchid] = node_x->splitval;
            srchdown (AllDonors, DonorLimit, node_x->leftson, lbound, ubound, xfti,
                      xmch, vmch, search);

            ubound[node_x->branchid] = savebound;

            savebound = lbound[node_x->branchid];
            lbound[node_x->branchid] = node_x->splitval;

            if (bndtest (search, xmch, vmch, lbound, ubound) == OVERLAP)
                srchdown (AllDonors, DonorLimit, node_x->rightson, lbound, ubound, xfti,
                          xmch, vmch, search);

            lbound[node_x->branchid] = savebound;
        }
        else {
            savebound = lbound[node_x->branchid];
            lbound[node_x->branchid] = node_x->splitval;
            srchdown (AllDonors, DonorLimit, node_x->rightson, lbound, ubound, xfti,
                      xmch, vmch, search);
            lbound[node_x->branchid] = savebound;

            savebound = ubound[node_x->branchid];
            ubound[node_x->branchid] = node_x->splitval;

            if (bndtest (search, xmch, vmch, lbound, ubound) == OVERLAP)
                srchdown (AllDonors, DonorLimit, node_x->leftson, lbound, ubound, xfti,
                          xmch, vmch, search);

            ubound[node_x->branchid] = savebound;
        }
    }
    else
    {
        savebound = ubound[node_x->branchid];
        ubound[node_x->branchid] = node_x->splitval;
        srchdown (AllDonors, DonorLimit, node_x->leftson, lbound, ubound, xfti, xmch,
                  vmch, search);
        ubound[node_x->branchid] = savebound;

        savebound = lbound[node_x->branchid];
        lbound[node_x->branchid] = node_x->splitval;
        srchdown (AllDonors, DonorLimit, node_x->rightson, lbound, ubound, xfti, xmch,
                  vmch, search);
        lbound[node_x->branchid] = savebound;
    }
}
